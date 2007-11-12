/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     NTRSettings.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu/settings/common/NTRSettings.h>
#include "spi.h"

// define data----------------------------------------------------------
#if 0
#define DEBUG_Printf					OS_TPrintf
#else
#define DEBUG_Printf(...)				((void)0)
#endif

#define SAVE_COUNT_MAX					0x0080		// NTRSettingsData.saveCount�̍ő�l
#define SAVE_COUNT_MASK					0x007f		// NTRSettingsData.saveCount�̒l�͈̔͂��}�X�N����B(0x00-0x7f�j
#define NSD_NOT_CORRECT					0x00ff		// NTR�ݒ�f�[�^���ǂݏo����Ă��Ȃ� or �L���Ȃ��̂��Ȃ����Ƃ������B
#define NVRAM_RETRY_NUM					8			// NVRAM���g���C��

// NVRAM�X�e�[�^�X���W�X�^�l
#define SR_WIP							0x01		// 0:READY       1:���C�g�A�C���[�X��
#define SR_WEN							0x02		// 0:���C�g�֎~  1:���C�g����
#define SR_EER							0x20		// 1:�C���[�X�G���[�����iSANYO��FLASH�̂݁j

#define READ_IPL2_HEADER_ADDR			0x18		// IPL2�w�b�_�̂����A�ǂݍ��݂��K�v�ȕ����̐擪�A�h���X
#define READ_IPL2_HEADER_SIZE			0x0a		// IPL2�w�b�_�̂����A�ǂݍ��݂��K�v�ȃT�C�Y
#define NSD_ROM_ADDR_SHIFT				3

// NVRAM�֘A���M�R�}���h�X�e�[�g
static enum NvramCommState{
	COMM_READY = 0,
	COMM_RD,
	COMM_WE,
	COMM_WR,
	COMM_RDSR_WE,
	COMM_RDSR_WR,
	COMM_SRST
}NvramCommState;


// IPL2�w�b�_�̈ꕔ�i0x18����̃f�[�^)
typedef struct IPL2HeaderPart {
	struct {
		u8	timestamp[5];				// NTR-IPL2�^�C���X�^���v [0]:��,[1]:��,[2]:��,[3]:��,[4]:�N
		u8	ipl2_type;					// NTR-IPL2�^�C�v
		u8	rsv[2];
	} version;
	
	u16		nsd_rom_addr;
	
	u8		pad[ 0x16 ];				// ���L���b�V�����C���ɍ��킹�邽�߂̃p�f�B���O�B�{���͕K�v�Ȃ��B
} IPL2HeaderPart;	// 0x20bytes


// function's prototype-------------------------------------------------
u32  NSD_GetNSDRomAddr( void );			// NTRSetting�f�[�^��NVRAM�i�[�A�h���X���擾
u8   NSD_GetIPL2Type( void );			// NTR-IPL2�^�C�v���擾
const u8 *NSD_GetIPL2Timestamp( void );	// NTR-IPL2�̃^�C���X�^���v���擾

static void NSDi_ReadIPL2Header( void );
static BOOL NSDi_CheckCorrectNSD( NSDStoreEx (*pNSDStoreExArray)[2], u8 region );
static BOOL NSDi_CheckDataValue( NSDStoreEx *pNSDStore, u8 region );
static BOOL NVRAMm_ExecuteCommand( int nvState, u32 addr, u16 size, u8 *pSrc );
static void Callback_NVRAM( PXIFifoTag tag, u32 data, BOOL err );

// static variables-----------------------------------------------------
static NSDStoreEx		s_NSDStoreEx ATTRIBUTE_ALIGN(32);
static IPL2HeaderPart	s_IPL2Header ATTRIBUTE_ALIGN(32);
static BOOL				s_isReadIPL2H = FALSE;
static volatile BOOL	s_nvCbOccurred;
static volatile u16		s_nvResult;
static int 				s_indexNSD = NSD_NOT_CORRECT;

#ifndef SDK_FINALROM
static NSDStoreEx (*s_pNSDStoreExArray)[2];
#endif
// global variables-----------------------------------------------------
NTRSettingsData			*g_pNSD   = &s_NSDStoreEx.nsd;
NTRSettingsDataEx		*g_pNSDEx = &s_NSDStoreEx.nsd_ex;

// const data-----------------------------------------------------------
static const u16 s_validLangBitmapList[] = {				// ��TWL�ɍ��킹�������ǂ������B
	NTR_LANG_BITMAP_WW,				// TWL_REGION_JAPAN
	NTR_LANG_BITMAP_WW,				// TWL_REGION_AMERICA
	NTR_LANG_BITMAP_WW,				// TWL_REGION_EUROPE
	NTR_LANG_BITMAP_WW,				// TWL_REGION_AUSTRALIA
	NTR_LANG_BITMAP_CHINA,			// TWL_REGION_CHINA
	NTR_LANG_BITMAP_KOREA,			// TWL_REGION_KOREA
};


// function's description-----------------------------------------------

//----------------------------------------------------------------------
// IPL2�w�b�_���̓ǂݏo��
//----------------------------------------------------------------------
// �R���|�[�l���g��ł̎g�p��

// IPL2�w�b�_�̓ǂݏo��
static void NSDi_ReadIPL2Header( void )
{
	if( !s_isReadIPL2H ) {
		OS_TPrintf( "IPL2Header:%x\n",   sizeof(IPL2HeaderPart) );
		DC_InvalidateRange( &s_IPL2Header, sizeof(IPL2HeaderPart) );
		while( !NVRAMm_ExecuteCommand( COMM_RD, READ_IPL2_HEADER_ADDR, READ_IPL2_HEADER_SIZE, (u8 *)&s_IPL2Header ) ) {}
		s_isReadIPL2H = TRUE;
	}
}

// IPL2�^�C�v�̎擾
u8 NSD_GetIPL2Type( void )
{
	NSDi_ReadIPL2Header();
	return s_IPL2Header.version.ipl2_type;
}

// IPL2�^�C���X�^���v�̎擾
const u8 *NSD_GetIPL2Timestamp( void )
{
	NSDi_ReadIPL2Header();
	return s_IPL2Header.version.timestamp;
}

// NSD�i�[ROM�A�h���X�̎擾
u32 NSD_GetNSDRomAddr( void )
{
	NSDi_ReadIPL2Header();
	return (u32)( s_IPL2Header.nsd_rom_addr << NSD_ROM_ADDR_SHIFT );
}

//----------------------------------------------------------------------
// NTR�ݒ�f�[�^�̃��[�h
//----------------------------------------------------------------------

// NTR�ݒ�f�[�^���[�h�ς�
BOOL NSD_IsReadSettings( void )
{
	return ( s_indexNSD != NSD_NOT_CORRECT );
}


BOOL NSD_ReadSettings( u8 region, NSDStoreEx (*pTempBuffer)[2] )
{
	NSDStoreEx *pNSDStoreEx = (NSDStoreEx *)pTempBuffer;
#ifndef SDK_FINALROM
	s_pNSDStoreExArray = pTempBuffer;
	OS_TPrintf( "NSDStoreBuff : %08x %08x\n", &(*s_pNSDStoreExArray)[ 0 ], &(*s_pNSDStoreExArray)[ 1 ] );
#endif
	
	DC_InvalidateRange( pNSDStoreEx, sizeof(NSDStoreEx) * 2 );
	
	// �t���b�V������j�d������Ă���NTR�ݒ�f�[�^��ǂݏo���B
	while( !NVRAMm_ExecuteCommand( COMM_RD, NSD_GetNSDRomAddr(),                       sizeof(NSDStoreEx), (u8 *)&pNSDStoreEx[ 0 ] ) ) {}
	while( !NVRAMm_ExecuteCommand( COMM_RD, NSD_GetNSDRomAddr() + SPI_NVRAM_PAGE_SIZE, sizeof(NSDStoreEx), (u8 *)&pNSDStoreEx[ 1 ] ) ) {}
	OS_TPrintf("NSD read addr=%08x\n", NSD_GetNSDRomAddr() );
	
	// �ǂݏo�����f�[�^�̂ǂ��炪�L�����𔻒肷��B
	if( NSDi_CheckCorrectNSD( pTempBuffer, region ) ) {
		// �L����NTR�ݒ�f�[�^��ÓI�o�b�t�@�ɓ]��
		MI_CpuCopyFast( (void *)&pNSDStoreEx[ s_indexNSD ], (void *)&s_NSDStoreEx, sizeof(NSDStoreEx) );
	}else {
		// �L���ȃf�[�^���Ȃ��Ȃ�A�o�b�t�@���N���A����
		OS_TPrintf( "NSD clear.\n" );
		NSD_ClearSettings();
		return FALSE;
	}
	
	OS_TPrintf("Use NSD[%d]   : saveCount = %d\n", s_indexNSD, s_NSDStoreEx.saveCount);
	
	return TRUE;
}


//----------------------------------------------------------------------
// NTR�ݒ�f�[�^�̃��C�g
//----------------------------------------------------------------------
BOOL NSD_WriteSettings( u8 region )
{
	int retry;
	u32 nvramAddr;
	
	// �܂�NTR�ݒ�f�[�^�����[�h����Ă��Ȃ���΁A���[�h���s���ĕK�v�ȏ����擾����B
	if( !NSD_IsReadSettings() ) {
		OS_TPrintf( "ERROR: Need call NSD_ReadSetting.\n" );
		return FALSE;
	}
	
	// NSD   ��CRC�A�Z�[�u�J�E���g�l�A���C�g�A�h���X�̎Z�o�B
	s_NSDStoreEx.nsd.version    = NTR_SETTINGS_DATA_VERSION;	// �o�[�W���������݂̂��̂ɐݒ�B
	s_NSDStoreEx.crc16          = SVC_GetCRC16( 0xffff, (const void *)&s_NSDStoreEx.nsd, sizeof(NTRSettingsData) );
	s_NSDStoreEx.saveCount      = (u8)( ( s_NSDStoreEx.saveCount + 1 ) & SAVE_COUNT_MASK );
	
	// NSD_EX��CRC�Z�o�B
	s_NSDStoreEx.nsd_ex.version = NTR_SETTINGS_DATA_EX_VERSION;	// �o�[�W���������݂̂��̂ɐݒ�B
	s_NSDStoreEx.nsd_ex.valid_language_bitmap = s_validLangBitmapList[ region ];
	s_NSDStoreEx.crc16_ex       = SVC_GetCRC16( 0xffff, (const void *)&s_NSDStoreEx.nsd_ex, sizeof(NTRSettingsDataEx) );
	
	// NTR�ݒ�f�[�^�̃��C�g
	DC_FlushRange( &s_NSDStoreEx, sizeof(NSDStoreEx) );
	retry = NVRAM_RETRY_NUM;
	while( retry-- ) {
		s_indexNSD ^= 0x01;									// ���g���C�̓x�ɏ������݃A�h���X��؂�ւ���B
		nvramAddr = NSD_GetNSDRomAddr() + s_indexNSD * SPI_NVRAM_PAGE_SIZE;
		OS_TPrintf("NSD write addr=%08x\n", nvramAddr );
		
		if( NVRAMm_ExecuteCommand( COMM_WE, nvramAddr, sizeof(NSDStoreEx), (u8 *)&s_NSDStoreEx ) ) {
			OS_TPrintf("NVRAM Write succeeded.\n");
			break;
		}
		SVC_WaitByLoop( 0x4000 );
		OS_TPrintf("NVRAM Write retry = %d.\n", NVRAM_RETRY_NUM - retry );
	}
	return TRUE;
}


//----------------------------------------------------------------------
// �~���[�����O����Ă���NTR�ݒ�f�[�^�̂ǂ��炪�L�����𔻒�
//----------------------------------------------------------------------

static BOOL NSDi_CheckCorrectNSD( NSDStoreEx (*pNSDStoreExArray)[2], u8 region )
{
	NSDStoreEx *pNSDStoreEx = (NSDStoreEx *)pNSDStoreExArray;
	u16 i;
	u16 nsd_valid = 0;
	
	// �e�~���[�f�[�^��CRC & saveCount�������`�F�b�N
	for( i = 0; i < 2; i++ ) {
		u16  crc;
		BOOL isInvalid = FALSE;
		
		// NSD ��CRC�`�F�b�N���s���B
		crc = SVC_GetCRC16( 0xffff, (const void *)&pNSDStoreEx[i].nsd, sizeof(NTRSettingsData) );
		
		if(    ( pNSDStoreEx[ i ].crc16       != crc )				// CRC���������AsaveCount�l��0x80�ȉ��ŁA���o�[�W��������v����f�[�^�𐳓��Ɣ��f�B
			|| ( pNSDStoreEx[ i ].nsd.version != NTR_SETTINGS_DATA_VERSION )
			|| ( pNSDStoreEx[ i ].saveCount   >= SAVE_COUNT_MAX ) ) {
			OS_TPrintf("NSD   crc error.\n");
			isInvalid = TRUE;
		}
		
		// NSDEx ��CRC�`�F�b�N���s���B
		crc = SVC_GetCRC16( 0xffff, (const void *)&pNSDStoreEx[i].nsd_ex, sizeof(NTRSettingsDataEx) );
		
		if(   ( pNSDStoreEx[ i ].crc16_ex       != crc )
		   || ( pNSDStoreEx[ i ].nsd_ex.version != NTR_SETTINGS_DATA_EX_VERSION ) ) {
			OS_TPrintf("NSDEx crc error.\n");
			isInvalid = TRUE;
		}
		
		// NSD, NSDEx��CRC���������Ȃ�A�f�[�^�̒��g���`�F�b�N�B
		if( !isInvalid ) {
			if( NSDi_CheckDataValue( &pNSDStoreEx[ i ], region ) ) {	// �f�[�^�����������l�łȂ������`�F�b�N�B
				nsd_valid  |= 0x01 << i;								// "�L��"�t���O���Z�b�g
				s_indexNSD = i;										// NCD�̃C���f�b�N�X���؂�ւ��B
			}else {
				isInvalid = TRUE;
			}
		}
		
		if( nsd_valid & ( 0x01 << i ) ) {
			OS_TPrintf("NSD[%d] valid : saveCount = %d\n", i, pNSDStoreEx[i].saveCount);
		}else {
			OS_TPrintf("NSD[%d] invalid.\n", i);
		}
	}
	
	
	if( nsd_valid == 0 ) {
		s_indexNSD = 1;			// �ŏ���Write����"0"�ɂȂ�悤��"1"�ɂ��Ă���
		return FALSE;
	}else if( nsd_valid == 0x03 ) {									
		// �~���[�����O���ꂽNSD�������Ƃ��ɐ����ȏꍇ�A�Z�[�u�J�E���g�l���傫������L���Ƃ���B
		u16 saveCount = (u8)( ( pNSDStoreEx[ 0 ].saveCount + 1 ) & SAVE_COUNT_MASK );
		s_indexNSD = ( saveCount == pNSDStoreEx[ 1 ].saveCount ) ? (u16)1 : (u16)0;
	}
	return TRUE;
}


// NTR�ݒ�f�[�^�̒l���������l���`�F�b�N�B	// FALSE:�������Ȃ��BTRUE�F�������B
static BOOL NSDi_CheckDataValue( NSDStoreEx *pNSDStoreEx, u8 region )
{
	NTRSettingsData   *pNSD   = &pNSDStoreEx->nsd;
	NTRSettingsDataEx *pNSDEx = &pNSDStoreEx->nsd_ex;
	u16 validLangBitmap = s_validLangBitmapList[ region ];
	
	//pNSD->option;
	// NSD��language�`�F�b�N�i NSD����language�́A���E�p�E�ƁE���E�ɁE���̂U����̂����́A�Ή�����݂̂̒l�ƂȂ�B�j
	if( ~( NTR_LANG_BITMAP_WW & validLangBitmap ) & ( 0x0001 << pNSD->option.language ) ) {
		OS_TPrintf("NSD: invalid language        : org:%02d ex:%02d bitmap:%04x\n",
				   pNSD->option.language, pNSDEx->language, pNSDEx->valid_language_bitmap );
		return FALSE;
	}
	
	// NSDEx��language�`�F�b�N�i������ɂ́A���E�؂�����j
	if( ( ~validLangBitmap & ( 0x0001 << pNSDEx->language ) ) ||
		( pNSDEx->valid_language_bitmap != validLangBitmap ) ) {
		OS_TPrintf("NSDEx: invalid language    : org:%02d ex:%02d bitmap:%04x\n",
				   pNSD->option.language, pNSDEx->language, pNSDEx->valid_language_bitmap );
		return FALSE;
	}
	
	//pNSD->owner;
	// favoriteColor��4bit�Ȃ̂Ŕ͈͊O�͂Ȃ��B
	
	// birthday
	if( pNSD->option.isSetBirthday ) {
		if( ( pNSD->owner.birthday.month > 12 ) || ( pNSD->owner.birthday.day > 31 ) ) {
			OS_TPrintf("NSD: invalid birthday        : %02d/%02d\n", pNSD->owner.birthday.month, pNSD->owner.birthday.day );
			return FALSE;
		}
	}
	
	// nickname
	if( pNSD->option.isSetNickname ) {
		if( pNSD->owner.nickname.length > NTR_NICKNAME_LENGTH ) {
			OS_TPrintf("NSD: invalid nickname length : %02d\n", pNSD->owner.nickname.length );
			return FALSE;
		}
	}
	
	// comment
	if( pNSD->owner.comment.length  > NTR_COMMENT_LENGTH ) {
		OS_TPrintf("NSD: invalid comment  length     : %02d\n", pNSD->owner.comment.length );
		return FALSE;
	}
	
	//pNSD->alarm;
	if( ( pNSD->alarm.hour > 23 ) || ( pNSD->alarm.minute > 59 ) ) {
		OS_TPrintf("NSD: invalid alarm time          : %02d:%02d\n", pNSD->alarm.hour, pNSD->alarm.minute );
		return FALSE;
	}
	
	//pNSD->tp;
	// TP�L�����u���[�V�����l�́ATP_CalcCalibrateParam�Œl�̃`�F�b�N�����Ă���̂ŁA�`�F�b�N���Ȃ��B
	
//	OS_TPrintf( "NSD: correct data.\n" );
	return TRUE;
}


// NTR�ݒ�f�[�^�̃N���A
void NSD_ClearSettings( void )
{
	NSDStoreEx *pNSDStoreEx = &s_NSDStoreEx;
	
	s_indexNSD = 1;							// ���C�g�O�ɔ��]�����̂ŁA"0"�����I�������悤��"1"�ɂ��Ă���
	
	MI_CpuClear16( pNSDStoreEx, sizeof(NSDStoreEx) );
	// �����l��0�ȊO�̂���
	pNSDStoreEx->nsd.version    = NTR_SETTINGS_DATA_VERSION;
	pNSDStoreEx->nsd_ex.version = NTR_SETTINGS_DATA_EX_VERSION;
	pNSDStoreEx->nsd.owner.birthday.month = 1;
	pNSDStoreEx->nsd.owner.birthday.day   = 1;
	OS_TPrintf( "NSDStoreEx cleared.\n" );
}


// NTR�ݒ�f�[�^�̃j�b�N�l�[���E�F�E�a�����̏������B
void NSD_ClearOwnerInfo( void )
{
	MI_CpuClear16( &GetNSD()->owner, sizeof(NTROwnerInfo) );
	GetNSD()->owner.birthday.month	= 1;
	GetNSD()->owner.birthday.day	= 1;
	GetNSD()->option.isSetBirthday	= 0;
	GetNSD()->option.isSetUserColor	= 0;
	GetNSD()->option.isSetNickname	= 0;
}


//----------------------------------------------------------------------
// NVRAM�ւ̃A�N�Z�X���[�`���{�� ( nvState <- COMM_RD or COMM_WE )
//----------------------------------------------------------------------
static BOOL NVRAMm_ExecuteCommand( int nvState, u32 addr, u16 size, u8 *pSrc )
{
	static u8 sr_buf[ 32 ] ATTRIBUTE_ALIGN(32);
    OSTick	start;
	BOOL	isSending = FALSE;
	u8		*pSR = (u8 *)sr_buf;
	
	PXI_SetFifoRecvCallback( PXI_FIFO_TAG_NVRAM , Callback_NVRAM );
	
	while( 1 ) {
		//---------------------------------------
		// NVRAM�R�}���h�𔭍s����
		//---------------------------------------
		if( !isSending ) {
			
			s_nvCbOccurred	= FALSE;
			
			switch( nvState ) {
			  case COMM_RD:
				isSending	= SPI_NvramReadDataBytes( addr, size, pSrc );
				break;
				
			  case COMM_WE:
				isSending	= SPI_NvramWriteEnable();
				break;
				
			  case COMM_WR:
				isSending	= SPI_NvramPageWrite( addr, size , pSrc );
				start		= OS_GetTick();
				break;
				
			  case COMM_RDSR_WE:
			  case COMM_RDSR_WR:
				isSending	= SPI_NvramReadStatusRegister( pSR );
				break;
				
			  case COMM_SRST:
				isSending	= SPI_NvramSoftwareReset();
				break;
			}
		//---------------------------------------
		// �R�}���h���s���ʁi�R�[���o�b�N�����j��҂��Č��ʂ���������
		//---------------------------------------
		}else { // isSending == TRUE
			if( s_nvCbOccurred == TRUE ) {							// �R�[���o�b�N������҂B
				
				isSending = FALSE;
				
				if( s_nvResult == SPI_PXI_RESULT_SUCCESS ) {
					switch( nvState ) {
					  case COMM_RD:
						return TRUE;
						
					  case COMM_WE:
						nvState = COMM_RDSR_WE;
						break;
						
					  case COMM_WR:
						nvState = COMM_RDSR_WR;
						break;
						
					  case COMM_RDSR_WE:
					  case COMM_RDSR_WR:
						
						DC_InvalidateRange( pSR, 1 );
						
						if( nvState == COMM_RDSR_WE ) {				// ���C�g�C�l�[�u���m�F�X�e�[�g�Ȃ�
							if( ( *pSR & SR_WEN ) ) {
								nvState = COMM_WR;
							}else {
								OS_TPrintf("NVRAM ERR: Write Enable Invalid.\n");
								return FALSE;
							}
						}else {
							if( ( *pSR & SR_WIP ) == 0 ) {		// ���C�g�^�C���[�X�I��
								return TRUE;
							}else {
								if(	  ( *pSR & SR_EER )			// SR_EER�������Ă�����G���[
								   || ( OS_TicksToMilliSeconds( OS_GetTick() - start ) > 4000 ) ) {
																		// �R�}���h���s����4�b�o�߂�����G���[�i���ی��j
									DEBUG_Printf( "NVRAM SR : %02x\n", *pSR );
									nvState = COMM_SRST;
								}else {
									SVC_WaitByLoop( 0x4000 );
								}
							}
						}
						break;
						
					  case COMM_SRST:
						OS_TPrintf("NVRAM ERR: PageErase Timeout and SoftReset.\n");
						return FALSE;
					}
				}else {  // s_nvResult != SPI_PXI_RESULT_SUCCESS
					OS_TPrintf("NVRAM ERR: NVRAM PXI command failed.\n");
					return FALSE;
				}
			}
		}
	}
}


//----------------------------------------------------------------------
// �R�[���o�b�N
//----------------------------------------------------------------------
static void Callback_NVRAM( PXIFifoTag tag, u32 data, BOOL err )
{
	#pragma unused(tag)
	
	u16 command		= (u16)( ( ( data & SPI_PXI_DATA_MASK ) & 0x7f00 ) >> 8 );
	
	s_nvResult		= (u16)( data & 0x00ff );
	s_nvCbOccurred	= TRUE;											// �R�[���o�b�N�����t���OTRUE
	
	if( err ) {
		OS_TPrintf("NVRAM-ARM9: Received PXI data is error.\n");
		s_nvResult = 0x00ff;
	}
	
	switch(command){												// �R�}���h���\��
	  case SPI_PXI_COMMAND_NVRAM_READ:
		DEBUG_Printf("NVRAM-ARM9:ReadDataBytes");
		break;
	  case SPI_PXI_COMMAND_NVRAM_WREN:
		DEBUG_Printf("NVRAM-ARM9:WriteEnable");
		break;
	  case SPI_PXI_COMMAND_NVRAM_PW:
		DEBUG_Printf("NVRAM-ARM9:PageWrite");
		break;
	  case SPI_PXI_COMMAND_NVRAM_RDSR:
		DEBUG_Printf("NVRAM-ARM9:ReadStatusRegister");
		break;
	  case SPI_PXI_COMMAND_NVRAM_WRDI:
		DEBUG_Printf("NVRAM-ARM9:WriteDisable");
		break;
	  case SPI_PXI_COMMAND_NVRAM_PE:
		DEBUG_Printf("NVRAM-ARM9:PageErase");
		break;
	  case SPI_PXI_COMMAND_NVRAM_SR:
		DEBUG_Printf("NVRAM-ARM9:SoftwareReset");
		break;
	  default:
		DEBUG_Printf("NVRAM-ARM9:?????");
		break;
	}
	if( s_nvResult != SPI_PXI_RESULT_SUCCESS ) {
		OS_TPrintf(" Error! ->%x", s_nvResult );
	}
	DEBUG_Printf("\n");
}

