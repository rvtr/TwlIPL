/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     nitroSettingsEx.c

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
#include <sysmenu/machineSettings/common/nitroSettings.h>
#include "spi.h"

// define data----------------------------------------------------------

#define NCD_EX_FORCE_ENABLE							// ���̃X�C�b�`���`����ƁASYSM�o�[�W�����Ɋւ�炸�����I��NitroConfigDataEx���L���ɂȂ�B
													// �����@��ł́A���̃t���O��ON,OFF�Ɋ֌W�Ȃ����퓮�삷�邪�A�A�v���r���h���f�o�b�K�⑼���@�œ��삳����ۂɁA������ON�łȂ��ƃ_���B
													//   �ǂݍ��܂�Ă��Ȃ�NCDEX�����[�h���Ď��S���Ă��܂��̂Œ��ӁB

#define SAVE_COUNT_MAX					0x0080		// NitroConfigData.saveCount�̍ő�l
#define SAVE_COUNT_MASK					0x007f		// NitroConfigData.saveCount�̒l�͈̔͂��}�X�N����B(0x00-0x7f�j
#define NCD_NOT_CORRECT					0x00ff		// NITRO�ݒ�f�[�^���ǂݏo����Ă��Ȃ� or �L���Ȃ��̂��Ȃ����Ƃ������B
#define NVRAM_RETRY_NUM					8			// NVRAM���g���C��

// NVRAM�X�e�[�^�X���W�X�^�l
#define SR_WIP							0x01		// 0:READY       1:���C�g�A�C���[�X��
#define SR_WEN							0x02		// 0:���C�g�֎~  1:���C�g����
#define SR_EER							0x20		// 1:�C���[�X�G���[�����iSANYO��FLASH�̂݁j

#define READ_IPL2_HEADER_ADDR			0x18		// IPL2�w�b�_�̂����A�ǂݍ��݂��K�v�ȕ����̐擪�A�h���X
#define READ_IPL2_HEADER_SIZE			0x0a		// IPL2�w�b�_�̂����A�ǂݍ��݂��K�v�ȃT�C�Y
#define NCD_ROM_ADDR_SHIFT				3

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
		u8	timestamp[5];				// IPL2�^�C���X�^���v [0]:��,[1]:��,[2]:��,[3]:��,[4]:�N
		u8	ipl2_type;					// IPL2�^�C�v(nitroConfigData.h�Œ�`��IPL2_TYPE...�j
		u8	rsv[2];
	} version;
	
	u16		ncd_rom_addr;
	
	u8		pad[ 0x16 ];					// ���L���b�V�����C���ɍ��킹�邽�߂̃p�f�B���O�B�{���͕K�v�Ȃ��B
} IPL2HeaderPart;	// 0x20bytes


// function's prototype-------------------------------------------------
static void NCD_ReadIPL2Header( void );
static int  NVRAMm_checkCorrectNCD(NCDStoreEx *ncdsp);
static BOOL NCD_CheckDataValue( NCDStoreEx *ncdsp );
static BOOL NVRAMm_ExecuteCommand( int nv_state, u32 addr, u16 size, u8 *srcp );
static void Callback_NVRAM(PXIFifoTag tag, u32 data, BOOL err);

// const data-----------------------------------------------------------

// global variables-----------------------------------------------------
NitroConfigDataEx ncdEx;

// static variables-----------------------------------------------------
static volatile BOOL	nv_cb_occurred;
static volatile u16		nv_result;
static u16 				ena_ncd_num = NCD_NOT_CORRECT;
static u16 				next_saveCount;
static NCDStoreEx		ncds[2] ATTRIBUTE_ALIGN(32);

static IPL2HeaderPart	ipl2Header ATTRIBUTE_ALIGN(32);
static BOOL				read_ipl2h = FALSE;

// function's description-----------------------------------------------

//----------------------------------------------------------------------
// IPL2�w�b�_���̓ǂݏo��
//----------------------------------------------------------------------
// �R���|�[�l���g��ł̎g�p��

// IPL2�w�b�_�̓ǂݏo��
static void NCD_ReadIPL2Header( void )
{
	if( !read_ipl2h ) {
		OS_TPrintf( "IPL2Header:%x\n",   sizeof(IPL2HeaderPart) );
		DC_InvalidateRange( &ipl2Header, sizeof(IPL2HeaderPart) );
		while( !NVRAMm_ExecuteCommand( COMM_RD, READ_IPL2_HEADER_ADDR, READ_IPL2_HEADER_SIZE, (u8 *)&ipl2Header ) ) {}
		read_ipl2h = TRUE;
	}
}

// IPL2�^�C�v�̎擾
u8 NCD_GetIPL2Type( void )
{
	NCD_ReadIPL2Header();
	return ipl2Header.version.ipl2_type;
}

// IPL2�o�[�W�����̎擾
u8 *NCD_GetIPL2Version( void )
{
	NCD_ReadIPL2Header();
	return ipl2Header.version.timestamp;
}

// NCD�i�[ROM�A�h���X�̎擾
u32 NCD_GetNCDRomAddr( void )
{
	NCD_ReadIPL2Header();
	return (u32)( ipl2Header.ncd_rom_addr << NCD_ROM_ADDR_SHIFT );
}

//----------------------------------------------------------------------
// NITRO�ݒ�f�[�^�̃��[�h
//----------------------------------------------------------------------
int NVRAMm_ReadNitroConfigData(NitroConfigData *dstp)
{
	int			result = 0;
	NCDStoreEx	*ncdsp = &ncds[ 0 ];
	
	DC_InvalidateRange( ncdsp, sizeof(NCDStoreEx) * 2 );
	
	// �t���b�V������j�d������Ă���NITRO�ݒ�f�[�^��ǂݏo���B
	while( !NVRAMm_ExecuteCommand( COMM_RD, NCD_GetNCDRomAddr(),                       sizeof(NCDStoreEx), (u8 *)&ncdsp[0]) ) {}
	while( !NVRAMm_ExecuteCommand( COMM_RD, NCD_GetNCDRomAddr() + SPI_NVRAM_PAGE_SIZE, sizeof(NCDStoreEx), (u8 *)&ncdsp[1]) ) {}
	OS_TPrintf("NCD read addr=%08x\n", NCD_GetNCDRomAddr() );
	
	// �ǂݏo�����f�[�^�̂ǂ��炪�L�����𔻒肷��B
	if(NVRAMm_checkCorrectNCD(ncdsp)) {
		next_saveCount	= 1;
		result			= 1;
		goto END;													// �L���ȃf�[�^���Ȃ���΃G���[�I���B
	}
	next_saveCount = (u8)((ncdsp[ena_ncd_num].saveCount + 1) & SAVE_COUNT_MASK);
	
	// �L����NITRO�ݒ�f�[�^���o�b�t�@�ɓ]��
	if( dstp != NULL ) {
		SVC_CpuCopy( (void *)&ncdsp[ ena_ncd_num ].ncd,    (void *)dstp, sizeof(NitroConfigData), 16);
		SVC_CpuCopy( (void *)&ncdsp[ ena_ncd_num ].ncd_ex, (void *)&ncdEx, sizeof(NitroConfigDataEx), 16);
	}
	
END:
	return result;
}


//----------------------------------------------------------------------
// NITRO�ݒ�f�[�^�̃��C�g
//----------------------------------------------------------------------
void NVRAMm_WriteNitroConfigData( NitroConfigData *srcp )
{
	NCDStoreEx *ncdsp = &ncds[ 0 ];
	u16			size  = sizeof(NCDStore);
	u32			flash_addr;
	int			retry;
	
	// �܂�NITRO�ݒ�f�[�^�����[�h����Ă��Ȃ���΁A���[�h���s���ĕK�v�ȏ����擾����B
	if( ena_ncd_num == NCD_NOT_CORRECT ) {
		if( NVRAMm_ReadNitroConfigData( NULL ) ) {
			ena_ncd_num = 0;										// �L���ȃf�[�^���Ȃ����"0"���Ƀ��C�g����B
		}
	}
	
	// NCD   ��CRC�A�Z�[�u�J�E���g�l�A���C�g�A�h���X�̎Z�o�B
	ncdsp->ncd				= *srcp;								// *GetNCDWork();�@�ł��ꏏ���B
	ncdsp->ncd.version		= NITRO_CONFIG_DATA_VERSION;			// �o�[�W���������݂̂��̂ɐݒ�B
	ncdsp->crc16	 		= SVC_GetCRC16( 0xffff, (const void *)&ncdsp->ncd,    sizeof(NitroConfigData) );
	ncdsp->saveCount 		= next_saveCount;
	next_saveCount	 		= (u8)( ( next_saveCount + 1 ) & SAVE_COUNT_MASK );
	
	// NCD_EX��CRC�Z�o�B
#ifndef NCD_EX_FORCE_ENABLE
	if( ( NCD_GetIPL2Type() != IPL2_TYPE_NTR_WW ) && ( NCD_GetIPL2Type() & IPL2_TYPE_NCD_EX_FLAG ) )
#endif
	{
		ncdsp->ncd_ex			= *GetNCDExWork();
		ncdsp->ncd_ex.version	= NITRO_CONFIG_DATA_EX_VERSION;		// �o�[�W���������݂̂��̂ɐݒ�B
		ncdsp->ncd_ex.valid_language_bitmap = VALID_LANG_BITMAP;
		ncdsp->crc16_ex		 	= SVC_GetCRC16( 0xffff, (const void *)&ncdsp->ncd_ex, sizeof(NitroConfigDataEx) );
		size					= sizeof(NCDStoreEx);				// ���������݃T�C�Y��NCDStoreEx�Ɋg���B
	}
	
	// NITRO�ݒ�f�[�^�̃��C�g
	DC_FlushRange(ncdsp, sizeof(NCDStoreEx));
	retry = NVRAM_RETRY_NUM;
	while( retry-- ) {
		ena_ncd_num	   ^= 0x01;										// ���g���C�̓x�ɏ������݃A�h���X��؂�ւ���B
		flash_addr		= NCD_GetNCDRomAddr() + ena_ncd_num * SPI_NVRAM_PAGE_SIZE;
		OS_TPrintf("NCD write addr=%08x\n", flash_addr );
		
		if( NVRAMm_ExecuteCommand( COMM_WE, flash_addr, size, (u8 *)ncdsp) ) {
			OS_TPrintf("NVRAM Write succeeded.\n");
			break;
		}
		SVC_WaitByLoop( 0x4000 );
		OS_TPrintf("NVRAM Write retry = %d.\n", NVRAM_RETRY_NUM - retry );
	}
}


//----------------------------------------------------------------------
// �~���[�����O����Ă���NITRO�ݒ�f�[�^�̂ǂ��炪�L�����𔻒�
//----------------------------------------------------------------------

static int NVRAMm_checkCorrectNCD(NCDStoreEx *ncdsp)
{
	u16 i;
	u16 ncd_valid = 0;
	
	// �e�~���[�f�[�^��CRC & saveCount�������`�F�b�N
	for(i = 0; i < 2; i++) {
		u16  crc;
		BOOL invalid = FALSE;
		
		crc = SVC_GetCRC16( 0xffff, (const void *)&ncdsp[i].ncd, sizeof(NitroConfigData) );
		
		if(    ( ncdsp[ i ].crc16          != crc )				// CRC���������AsaveCount�l��0x80�ȉ��ŁA���o�[�W��������v����f�[�^�𐳓��Ɣ��f�B
			|| ( ncdsp[ i ].ncd.version    != NITRO_CONFIG_DATA_VERSION )
			|| ( ncdsp[ i ].saveCount      >= SAVE_COUNT_MAX ) ) {
			OS_TPrintf("NCD   crc error.\n");
			invalid = TRUE;
		}
		
		// NCDEx���L����IPL2Type�Ȃ�΁ANCDEx��CRC�`�F�b�N���s���B
#ifndef NCD_EX_FORCE_ENABLE
		if( ( NCD_GetIPL2Type() != IPL2_TYPE_NTR_WW ) && ( NCD_GetIPL2Type() & IPL2_TYPE_NCD_EX_FLAG ) )
#endif
		{
			crc = SVC_GetCRC16( 0xffff, (const void *)&ncdsp[i].ncd_ex, sizeof(NitroConfigDataEx) );
			
			if(   ( ncdsp[ i ].crc16_ex       != crc )
			   || ( ncdsp[ i ].ncd_ex.version != NITRO_CONFIG_DATA_EX_VERSION ) ) {
				OS_TPrintf("NCDEx crc error.\n");
				invalid = TRUE;
			}
		}
		// NCD, NCDEx��CRC���������Ȃ�A�f�[�^�̒��g���`�F�b�N�B
		if( !invalid ) {
			if( NCD_CheckDataValue( &ncdsp[ i ] ) ) {
				ncd_valid  |= 0x01 << i;						// �f�[�^�����������l�łȂ������`�F�b�N�B
				ena_ncd_num = i;								// �L����NCD�̃C���f�b�N�X��؂�ւ���B
			}else {
				invalid = TRUE;
			}
		}
		
		if( ncd_valid & ( 0x01 << i ) ) {
			OS_TPrintf("NCD mirror%d is valid.:saveCount = %d\n", i, ncdsp[i].saveCount);
		}else {
			OS_TPrintf("NCD mirror%d is invalid.\n", i);
		}
	}
	
	
	if( ncd_valid == 0 ) {
		return 1;
	}else if( ncd_valid == 0x03 ) {									
	// �~���[�����O���ꂽNCD�������Ƃ��ɐ����ȏꍇ�A�Z�[�u�J�E���g�l���傫������L���Ƃ���B
		u16 saveCount = (u8)( ( ncdsp[ 0 ].saveCount + 1 ) & SAVE_COUNT_MASK );
		if( saveCount != ncdsp[ 1 ].saveCount ) {
			ena_ncd_num = 0;
		}
	}
	
	OS_TPrintf("use NCD mirror%d.:saveCount = %d\n", ena_ncd_num, ncdsp[ena_ncd_num].saveCount);
	
	return 0;
}


// NITRO�ݒ�f�[�^�̒l���������l���`�F�b�N�B	// FALSE:�������Ȃ��BTRUE�F�������B
static BOOL NCD_CheckDataValue( NCDStoreEx *ncdsp )
{
	NitroConfigData   *ncdp   = &ncdsp->ncd;
	NitroConfigDataEx *ncdexp = &ncdsp->ncd_ex;
	
	//ncdp->option;
	// NCD��language�`�F�b�N
	if( ~( LANG_BITMAP_WW & VALID_LANG_BITMAP ) & ( 0x0001 << ncdp->option.language ) ) {
		OS_TPrintf("NCD: invalid language        : org:%02d ex:%02d bitmap:%04x\n",
				   ncdp->option.language, ncdexp->language, ncdexp->valid_language_bitmap );
		return FALSE;
	}
	// NCDEx��language�`�F�b�N�iNCDEx���L���Ȃ̂́A���L��IPL2�^�C�v�̂��́j
#ifndef NCD_EX_FORCE_ENABLE
	if( ( NCD_GetIPL2Type() != IPL2_TYPE_NTR_WW ) && ( NCD_GetIPL2Type() & IPL2_TYPE_NCD_EX_FLAG ) )
#endif
	{
		if(   ( ~VALID_LANG_BITMAP & ( 0x0001 << ncdexp->language ) )
		   || ( ncdexp->valid_language_bitmap != VALID_LANG_BITMAP  ) ) {
			
			OS_TPrintf("NCDEx: invalid language    : org:%02d ex:%02d bitmap:%04x\n",
					   ncdp->option.language, ncdexp->language, ncdexp->valid_language_bitmap );
			return FALSE;
		}
	}
	
	//ncdp->owner;
	// favoriteColor��4bit�Ȃ̂Ŕ͈͊O�͂Ȃ��B
	// birthday
	if( ncdp->option.input_birthday ) {
		if( ( ncdp->owner.birthday.month > 12 ) || ( ncdp->owner.birthday.day > 31 ) ) {
			OS_TPrintf("NCD: invalid birthday        : %02d/%02d\n", ncdp->owner.birthday.month, ncdp->owner.birthday.day );
			return FALSE;
		}
	}
	
	// nickname
	if( ncdp->option.input_nickname ) {
		if( ncdp->owner.nickname.length > NCD_NICKNAME_LENGTH ) {
			OS_TPrintf("NCD: invalid nickname length : %02d\n", ncdp->owner.nickname.length );
			return FALSE;
		}
	}
	
	// comment
	if( ncdp->owner.comment.length  > NCD_COMMENT_LENGTH ) {
		OS_TPrintf("NCD: invalid comment  length     : %02d\n", ncdp->owner.comment.length );
		return FALSE;
	}
	
	//ncdp->alarm;
	if( ( ncdp->alarm.hour > 23 ) || ( ncdp->alarm.minute > 59 ) ) {
		OS_TPrintf("NCD: invalid alarm time          : %02d:%02d\n", ncdp->alarm.hour, ncdp->alarm.minute );
		return FALSE;
	}
	
	//ncdp->tp;
	// TP�L�����u���[�V�����l�́ATP_CalcCalibrateParam�Œl�̃`�F�b�N�����Ă���̂ŁA�`�F�b�N���Ȃ��B
	
	OS_TPrintf( "NCD: correct data.\n" );
	return TRUE;
}


//----------------------------------------------------------------------
// NVRAM�ւ̃A�N�Z�X���[�`���{�� ( nv_state <- COMM_RD or COMM_WE )
//----------------------------------------------------------------------
static BOOL NVRAMm_ExecuteCommand( int nv_state, u32 addr, u16 size, u8 *srcp )
{
    OSTick	start;
	BOOL	nv_sending	 = FALSE;
	u8		*nvram_srp	 = (u8 *)&ncds[1];
	
	PXI_SetFifoRecvCallback( PXI_FIFO_TAG_NVRAM , Callback_NVRAM );
	
	while( 1 ) {
		//---------------------------------------
		// NVRAM�R�}���h�𔭍s����
		//---------------------------------------
		if( !nv_sending ) {
			
			nv_cb_occurred	= FALSE;
			
			switch( nv_state ) {
			  case COMM_RD:
				nv_sending	= SPI_NvramReadDataBytes( addr, size, srcp );
				break;
				
			  case COMM_WE:
				nv_sending	= SPI_NvramWriteEnable();
				break;
				
			  case COMM_WR:
				nv_sending	= SPI_NvramPageWrite( addr, size , srcp );
				start		= OS_GetTick();
				break;
				
			  case COMM_RDSR_WE:
			  case COMM_RDSR_WR:
				nv_sending	= SPI_NvramReadStatusRegister( nvram_srp );
				break;
				
			  case COMM_SRST:
				nv_sending	= SPI_NvramSoftwareReset();
				break;
			}
		//---------------------------------------
		// �R�}���h���s���ʁi�R�[���o�b�N�����j��҂��Č��ʂ���������
		//---------------------------------------
		}else { // nv_sending == TRUE
			if( nv_cb_occurred == TRUE ) {							// �R�[���o�b�N������҂B
				
				nv_sending = FALSE;
				
				if( nv_result == SPI_PXI_RESULT_SUCCESS ) {
					switch( nv_state ) {
					  case COMM_RD:
						return TRUE;
						
					  case COMM_WE:
						nv_state = COMM_RDSR_WE;
						break;
						
					  case COMM_WR:
						nv_state = COMM_RDSR_WR;
						break;
						
					  case COMM_RDSR_WE:
					  case COMM_RDSR_WR:
						
						DC_InvalidateRange( nvram_srp, 1 );
						
						if( nv_state == COMM_RDSR_WE ) {				// ���C�g�C�l�[�u���m�F�X�e�[�g�Ȃ�
							if( ( *nvram_srp & SR_WEN ) ) {
								nv_state = COMM_WR;
							}else {
								OS_TPrintf("NVRAM ERR: Write Enable Invalid.\n");
								return FALSE;
							}
						}else {
							if( ( *nvram_srp & SR_WIP ) == 0 ) {		// ���C�g�^�C���[�X�I��
								return TRUE;
							}else {
								if(	  ( *nvram_srp & SR_EER )			// SR_EER�������Ă�����G���[
								   || ( OS_TicksToMilliSeconds( OS_GetTick() - start ) > 4000 ) ) {
																		// �R�}���h���s����4�b�o�߂�����G���[�i���ی��j
									OS_TPrintf( "NVRAM SR : %02x\n", *nvram_srp );
									nv_state = COMM_SRST;
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
				}else {  // nv_result != SPI_PXI_RESULT_SUCCESS
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
	
	nv_result		= (u16)( data & 0x00ff );
	nv_cb_occurred	= TRUE;											// �R�[���o�b�N�����t���OTRUE
	
	if( err ) {
		OS_TPrintf("NVRAM-ARM9: Received PXI data is error.\n");
		nv_result = 0x00ff;
	}
	
	switch(command){												// �R�}���h���\��
	  case SPI_PXI_COMMAND_NVRAM_READ:
		OS_TPrintf("NVRAM-ARM9:ReadDataBytes");
		break;
	  case SPI_PXI_COMMAND_NVRAM_WREN:
		OS_TPrintf("NVRAM-ARM9:WriteEnable");
		break;
	  case SPI_PXI_COMMAND_NVRAM_PW:
		OS_TPrintf("NVRAM-ARM9:PageWrite");
		break;
	  case SPI_PXI_COMMAND_NVRAM_RDSR:
		OS_TPrintf("NVRAM-ARM9:ReadStatusRegister");
		break;
	  case SPI_PXI_COMMAND_NVRAM_WRDI:
		OS_TPrintf("NVRAM-ARM9:WriteDisable");
		break;
	  case SPI_PXI_COMMAND_NVRAM_PE:
		OS_TPrintf("NVRAM-ARM9:PageErase");
		break;
	  case SPI_PXI_COMMAND_NVRAM_SR:
		OS_TPrintf("NVRAM-ARM9:SoftwareReset");
		break;
	  default:
		OS_TPrintf("NVRAM-ARM9:?????");
		break;
	}
	if( nv_result != SPI_PXI_RESULT_SUCCESS ) {
		OS_TPrintf(" Error! ->%x", nv_result );
	}
	OS_TPrintf("\n");
}

