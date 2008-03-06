/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.c

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
#include <sysmenu.h>
#include <firm/format/from_firm.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
#define CARD_BANNER_INDEX			( LAUNCHER_TITLE_LIST_NUM - 1 )

#define SYSTEM_APP_KEY_OFFSET		1 // �t�@�[�����瑗���Ă���SYSTEM�A�v���L�[�̂��߂̃I�t�Z�b�g
#define LAUNCHER_KEY_OFFSET			0 // �t�@�[�����瑗���Ă���Launcher�L�[�̂��߂̃I�t�Z�b�g
#define ROM_HEADER_HASH_OFFSET		(0x0) // ��������ROM�w�b�_�n�b�V�������o�����߂̃I�t�Z�b�g

#define SIGN_HEAP_ADDR	0x023c0000	// �����v�Z�̂��߂̃q�[�v�̈�J�n�A�h���X
#define SIGN_HEAP_SIZE	0x1000		// �����v�Z�̂��߂̃q�[�v�T�C�Y
#define ARM9_ENCRYPT_DEF_SIZE	0x800	// ARM9FLX�̐擪�Í��������̃T�C�Y

#define	DIGEST_HASH_BLOCK_SIZE_SHA1					(512/8)
#define TWL_ROM_HEADER_HASH_CALC_DATA_LEN	0xe00 // ROM�w�b�_�̃n�b�V���v�Z���镔���̒���TWL��
#define NTR_ROM_HEADER_HASH_CALC_DATA_LEN	0x160 // ROM�w�b�_�̃n�b�V���v�Z���镔���̒���DS��

#define AUTH_KEY_BUFFER_LEN 128
#define MB_AUTH_SIGN_SIZE				(128)	/* digital sign size */

typedef	struct	MbAuthCode
{
	char		magic_code[2];			// �}�W�b�N�i���o�[
	u16		version;			// �o�[�W����
	u8		sign[MB_AUTH_SIGN_SIZE];	// ����
	u32		serial_number;			// �V���A���ԍ�
} MbAuthCode;	// 16byte

// extern data-----------------------------------------------------------------
extern const u8 g_devPubKey[ 3 ][ 0x80 ];

// function's prototype-------------------------------------------------------
static s32  ReadFile( FSFile* pf, void* buffer, s32 size );
static void SYSMi_EnableHotSW( BOOL enable );
static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle );
static void SYSMi_Relocate( void );
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static OSThread			s_thread;
static OSThread			s_auth_thread;
static TWLBannerFile	s_bannerBuf[ LAUNCHER_TITLE_LIST_NUM ] ATTRIBUTE_ALIGN(32);
static AuthResult		s_authResult = AUTH_RESULT_PROCESSING;	// ROM���،���

static MbAuthCode s_authcode;

// const data------------------------------------------------------------------
static const OSBootType s_launcherToOSBootType[ LAUNCHER_BOOTTYPE_MAX ] = {
    OS_BOOTTYPE_ILLEGAL,	// ILLEGAL
    OS_BOOTTYPE_ROM,		// ROM
    OS_BOOTTYPE_NAND,		// TEMP
    OS_BOOTTYPE_NAND,		// NAND
    OS_BOOTTYPE_MEMORY,		// MEMORY
};

// HMAC_SHA1�p��
static const u8 s_digestDefaultKey[ DIGEST_HASH_BLOCK_SIZE_SHA1 ] = 
{
    0x21, 0x06, 0xc0, 0xde,
    0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d,
    0x46, 0xf2, 0xed, 0x01,

    0x76, 0xe3, 0xcc, 0x08,
    0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf,
    0x9a, 0x62, 0x78, 0x34,

    0x8f, 0x6d, 0x63, 0x3c,
    0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23,
    0xd2, 0xcf, 0xae, 0xc2,

    0x32, 0x67, 0x8d, 0xfe,
    0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37,
    0x87, 0x46, 0x58, 0x24,
};

// �_�E�����[�h�A�v�������p���J��
static const u8 nitro_dl_sign_key[AUTH_KEY_BUFFER_LEN] = {
  0x9E,0xC1,0xCC,0xC0,0x4A,0x6B,0xD0,0xA0,0x6D,0x62,0xED,0x5F,0x15,0x67,0x87,0x12,
  0xE6,0xF4,0x77,0x1F,0xD8,0x5C,0x81,0xCE,0x0C,0xD0,0x22,0x31,0xF5,0x89,0x08,0xF5,
  0xBE,0x04,0xCB,0xC1,0x4F,0x63,0xD9,0x5A,0x98,0xFF,0xEB,0x36,0x0F,0x9C,0x5D,0xAD,
  0x15,0xB9,0x99,0xFB,0xC6,0x86,0x2C,0x0A,0x0C,0xFC,0xE6,0x86,0x03,0x60,0xD4,0x87,
  0x28,0xD5,0x66,0x42,0x9C,0xF7,0x04,0x14,0x4E,0x6F,0x73,0x20,0xC3,0x3E,0x3F,0xF5,
  0x82,0x2E,0x78,0x18,0xD6,0xCD,0xD5,0xC2,0xDC,0xAA,0x1D,0x34,0x91,0xEC,0x99,0xC9,
  0xF7,0xBF,0xBF,0xA0,0x0E,0x1E,0xF0,0x25,0xF8,0x66,0x17,0x54,0x34,0x28,0x2D,0x28,
  0xA3,0xAE,0xF0,0xA9,0xFA,0x3A,0x70,0x56,0xD2,0x34,0xA9,0xC5,0x9E,0x5D,0xF5,0xE1
};

//================================================================================
// for register SCFG_OP
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         SCFG_GetBondingOption

  Description:  Get bonding option data

  Arguments:    None

  Returns:      option data
 *---------------------------------------------------------------------------*/
// SharedArea Access ver.
static inline u16 SCFG_GetBondingOption(void)
{
	return (u16)(*(u8*)(HW_SYS_CONF_BUF+HWi_WSYS08_OFFSET) & HWi_WSYS08_OP_OPT_MASK);
}

// ============================================================================
//
// ���擾
//
// ============================================================================

// �J�[�h�^�C�g���̎擾
BOOL SYSM_GetCardTitleList( TitleProperty *pTitleList_Card )
{
	BOOL retval = FALSE;
	
	// [TODO:] ROM�w�b�_�� platform_code ��NTR,TWL-HYB,TWL-LTD�ȊO�̂���
	//                     region_code���{�̏��ƈႤ����
	//         �̏ꍇ�́A����ɔF���ł��Ȃ��^�C�g���ł��邱�Ƃ������B
	
	if( SYSMi_GetWork()->flags.hotsw.isCardStateChanged ) {
		
		MI_CpuClear32( pTitleList_Card, sizeof(TitleProperty) );
		
		// ROM�w�b�_�o�b�t�@�̃R�s�[
		if( SYSM_IsExistCard() ) {
			u16 id = (u16)OS_GetLockID();
			(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );						// ARM7�Ɣr�����䂷��
			
			// ROM�w�b�_�̃��[�h
			(void)SYSMi_CopyCardRomHeader();
			
			// �o�i�[�f�[�^�̃��[�h
			(void)SYSMi_CopyCardBanner();
			
			SYSMi_GetWork()->flags.hotsw.isCardStateChanged = FALSE;							// �J�[�h���X�V�t���O�𗎂Ƃ�
			(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );					// ARM7�Ɣr�����䂷��
			OS_ReleaseLockID( id );

			pTitleList_Card->pBanner = &s_bannerBuf[ CARD_BANNER_INDEX ];
			pTitleList_Card->flags.isValid = TRUE;
			pTitleList_Card->flags.isAppLoadCompleted = TRUE;
			pTitleList_Card->flags.isAppRelocate = TRUE;
		}
		
		retval = TRUE;
	}
	
	// �^�C�g�����t���O�̃Z�b�g
	pTitleList_Card->flags.bootType = LAUNCHER_BOOTTYPE_ROM;
	pTitleList_Card->titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
	
	return retval;
}

// �J�[�hROM�w�b�_��ARM7�o�b�t�@����ARM9�o�b�t�@�ւ̃R�s�[
BOOL SYSMi_CopyCardRomHeader( void )
{
	BOOL retval = FALSE;

	if( SYSM_IsExistCard() ) {
		// ROM�w�b�_�̃��[�h
		DC_InvalidateRange( (void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );	// �L���b�V���P�A
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BAK, (void *)SYSM_CARD_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );	// ROM�w�b�_�R�s�[
		SYSMi_GetWork()->cardHeaderCrc16 = SYSMi_GetWork()->cardHeaderCrc16_bak;			// ROM�w�b�_CRC�R�s�[

		retval = TRUE;
	}

	return retval;
}

// �J�[�h�o�i�[��ARM7�o�b�t�@����ARM9�o�b�t�@�ւ̃R�s�[
BOOL SYSMi_CopyCardBanner( void )
{
	BOOL retval = FALSE;

	if( SYSM_IsExistCard() ) {
		// �o�i�[�f�[�^�̃��[�h
		SYSMi_ReadCardBannerFile( SYSM_GetCardRomHeader()->banner_offset, &s_bannerBuf[ CARD_BANNER_INDEX ] );

		retval = TRUE;
	}

	return retval;
}

// NAND�^�C�g�����X�g�̎擾
int SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int listNum )
{
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, ���̏������w�肵�ă^�C�g�����X�g���擾����B
	// �Ƃ肠����ALL
	OSTick start;
	int l;
	int getNum;
	int validNum = 0;
	NAMTitleId titleIDArray[ LAUNCHER_TITLE_LIST_NUM ];
	NAMTitleId *pTitleIDList = NULL;
	
	
	if( listNum > LAUNCHER_TITLE_LIST_NUM ) {
		OS_TPrintf( "Warning: TitleList_Nand num over LAUNCHER_TITLE_LIST_NUM(%d)\n", LAUNCHER_TITLE_LIST_NUM );
	}
	
	// �C���X�g�[������Ă���^�C�g���̎擾
	start = OS_GetTick();
	getNum = NAM_GetNumTitles();
	OS_TPrintf( "NAM_GetNumTitles : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	pTitleIDList = SYSM_Alloc( sizeof(NAMTitleId) * getNum );
	if( pTitleIDList == NULL ) {
		OS_TPrintf( "%s: alloc error.\n", __FUNCTION__ );
		return 0;
	}
	start = OS_GetTick();
	(void)NAM_GetTitleList( pTitleIDList, (u32)getNum );
	OS_TPrintf( "NAM_GetTitleList : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	
	// �擾�����^�C�g�������[���`�Ώۂ��ǂ������`�F�b�N
	for( l = 0; l < getNum; l++ ) {
		// "Not Launch"�łȂ��@���@"Data Only"�łȂ��@�Ȃ�L���ȃ^�C�g���Ƃ��ă��X�g�ɒǉ�
		if( ( pTitleIDList[ l ] & ( TITLE_ID_NOT_LAUNCH_FLAG_MASK | TITLE_ID_DATA_ONLY_FLAG_MASK ) ) == 0 ) {
			titleIDArray[ validNum ] = pTitleIDList[ l ];
			SYSMi_ReadBanner_NAND( pTitleIDList[ l ], &s_bannerBuf[ validNum ] );
			validNum++;
		}
	}
	SYSM_Free( pTitleIDList );
	
	// �O�̂��ߎc��̈��0�N���A
	for( l = validNum; l < LAUNCHER_TITLE_LIST_NUM; l++ ) {
		titleIDArray[ l ] = 0;
	}
	
	// �ŏI���X�g�ɑ΂��āA�J�[�h�A�v���������������������N���A
	MI_CpuClearFast( &pTitleList_Nand[ 1 ], sizeof(TitleProperty) * ( listNum - 1 ) );
	
	listNum = ( validNum < listNum ) ? validNum : listNum;
	
	for(l=0;l<listNum;l++)
	{
		pTitleList_Nand[l+1].titleID = titleIDArray[l];
		pTitleList_Nand[l+1].pBanner = &s_bannerBuf[l];
		if( titleIDArray[l] ) {
			pTitleList_Nand[l+1].flags.isValid = TRUE;
			pTitleList_Nand[l+1].flags.bootType = LAUNCHER_BOOTTYPE_NAND;
		}
	}
	// return : *TitleProperty Array
	return listNum;
}

// �w��t�@�C�����[�h
static s32 ReadFile(FSFile* pf, void* buffer, s32 size)
{
    u8* p = (u8*)buffer;
    s32 remain = size;

    while( remain > 0 )
    {
        const s32 len = MATH_IMin(1024, remain);
        const s32 readLen = FS_ReadFile(pf, p, len);

        if( readLen < 0 )
        {
            return readLen;
        }
        if( readLen != len )
        {
            return size - remain + readLen;
        }

        remain -= readLen;
        p      += readLen;
    }

    return size;
}


// ============================================================================
//
// �A�v���N��
//
// ============================================================================

static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle )
{
	enum
	{
	    region_header = 0,
	    region_arm9_ntr,
	    region_arm7_ntr,
	    region_arm9_twl,
	    region_arm7_twl,
	    region_max
	};
	// [TODO:]DS�_�E�����[�h�v���C�����pictochat����NTR�g��NAND�A�v���̎��́AROM�w�b�_��ޔ�����
	// ���A�ǂ���NTR-ROM�w�b�_���̍Ĕz�u�́Areboot���C�u�����ōs���B�炵���B
	// �Ƃ肠��������̂܂܂ɂ��Ă����A�����Ȃ���΍l����B
	
	// ���[�h
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    BOOL isTwlApp = TRUE;
	
	switch( pBootTitle->flags.bootType )
	{
	case LAUNCHER_BOOTTYPE_NAND:
		// NAND
    	NAM_GetTitleBootContentPathFast(path, pBootTitle->titleID);
		break;
	case LAUNCHER_BOOTTYPE_ROM:
		// TODO:CARD���ǂ̏ꍇ�̏���
		break;
	case LAUNCHER_BOOTTYPE_TEMP:
		// tmp�t�H���_
		STD_TSNPrintf( path, 256, OS_TMP_APP_PATH, pBootTitle->titleID );
		break;
	default:
		// unknown
		return;
	}

	FS_InitFile( file );
    bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);

    if( ! bSuccess )
    {
OS_TPrintf("RebootSystem failed: cant open file\n");
        return;
    }

    {
        int     i;
        u32     source[region_max];
        u32     length[region_max];
        u32     destaddr[region_max];
        static u8   header[HW_TWL_ROM_HEADER_BUF_SIZE] ATTRIBUTE_ALIGN(32);
        s32 readLen;
        ROM_Header *head = (ROM_Header *)header;

        // �܂�ROM�w�b�_��ǂݍ���
        // (�{���Ȃ炱����SRL�̐���������)
        bSuccess = FS_SeekFile(file, 0x00000000, FS_SEEK_SET);

        if( ! bSuccess )
        {
OS_TPrintf("RebootSystem failed: cant seek file(0)\n");
            FS_CloseFile(file);
            return;
        }

        readLen = FS_ReadFile(file, header, (s32)sizeof(header));

        if( readLen != (s32)sizeof(header) )
        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", header, 0, sizeof(header), readLen);
            FS_CloseFile(file);
            return;
        }

        if( head->s.nintendo_logo_crc16 != 0xCF56 )
        {
int i, j;
for( i = 0; i < 0x20; ++i )
{
for( j = 0; j < 0x10; ++j )
{
OS_TPrintf("%02X ", header[i * 0x10 + j]);
}
OS_TPrintf("\n");
}
OS_TPrintf("RebootSystem failed: logo CRC error\n");
            FS_CloseFile(file);
            return;
        }
        
        if( !(head->s.platform_code & PLATFORM_CODE_FLAG_TWL) )
        {
			//NTR��pROM or NTR TWL������Ή��̃A�v��
			isTwlApp = FALSE;
			if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP)
			{
				// NTR-DL�A�v���̏ꍇ��DL�A�v�������f�[�^���擾���Ă���
				u32 valid_size = ( head->s.rom_valid_size ? head->s.rom_valid_size : 0x01000000 );

		        bSuccess = FS_SeekFile(file, (s32)valid_size, FS_SEEK_SET);
		        if( ! bSuccess )
		        {
OS_TPrintf("RebootSystem failed: cant seek file(0)\n");
		            FS_CloseFile(file);
		            return;
		        }
		        readLen = FS_ReadFile(file, &s_authcode, (s32)sizeof(s_authcode));
		        if( readLen != (s32)sizeof(s_authcode) )
		        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", &s_authcode, 0, sizeof(s_authcode), readLen);
		            FS_CloseFile(file);
		            return;
		        }
			}
		}
		
        // �e�̈��ǂݍ���
        source  [region_header  ] = 0x00000000;
        length  [region_header  ] = HW_TWL_ROM_HEADER_BUF_SIZE;
        destaddr[region_header  ] = HW_TWL_ROM_HEADER_BUF;
		
        source  [region_arm9_ntr] = head->s.main_rom_offset;
        length  [region_arm9_ntr] = head->s.main_size;
        destaddr[region_arm9_ntr] = (u32)head->s.main_ram_address;
		
        source  [region_arm7_ntr] = head->s.sub_rom_offset;
        length  [region_arm7_ntr] = head->s.sub_size;
        destaddr[region_arm7_ntr] = (u32)head->s.sub_ram_address;
		
		if( isTwlApp )
		{
	        source  [region_arm9_twl] = head->s.main_ltd_rom_offset;
	        length  [region_arm9_twl] = head->s.main_ltd_size;
	        destaddr[region_arm9_twl] = (u32)head->s.main_ltd_ram_address;
			
	        source  [region_arm7_twl] = head->s.sub_ltd_rom_offset;
	        length  [region_arm7_twl] = head->s.sub_ltd_size;
	        destaddr[region_arm7_twl] = (u32)head->s.sub_ltd_ram_address;
        }
        
        // �̈�ǂݍ��ݐ�̃`�F�b�N�y�эĔz�u���f�[�^�̍쐬
        // �Q�[���J�[�h�̍Ĕz�u��񂪏������܂�Ă���̂ŁAnand�A�v�����[�h�O�Ɉ�U�N���A
        MI_CpuClearFast(SYSMi_GetWork()->romRelocateInfo, sizeof(Relocate_Info) * RELOCATE_INFO_NUM);
		for( i=0; i<RELOCATE_INFO_NUM; i++ )
		{
			if ( !isTwlApp && i >= ARM9_LTD_STATIC ) continue;// nitro�ł͓ǂݍ��܂Ȃ��̈�
			if ( !SYSM_CheckLoadRegionAndSetRelocateInfo( (RomSegmentName)i, &(destaddr[i+region_arm9_ntr]), length[i+region_arm9_ntr],
				 &(SYSMi_GetWork()->romRelocateInfo[i]), isTwlApp ) )
			{
	OS_TPrintf("RebootSystem failed: ROM Load Region error\n");
	            FS_CloseFile(file);
				return;
			}
		}

        for (i = region_header; i < region_max; ++i)
        {
            u32 len = length[i];
            
            if ( !isTwlApp && i >= region_arm9_twl ) continue;// nitro�ł͓ǂݍ��܂Ȃ��̈�

            bSuccess = FS_SeekFile(file, (s32)source[i], FS_SEEK_SET);

            if( ! bSuccess )
            {
OS_TPrintf("RebootSystem failed: cant seek file(%d)\n", source[i]);
                FS_CloseFile(file);
                return;
            }

            readLen = FS_ReadFile(file, (void *)destaddr[i], (s32)len);

            if( readLen != (s32)len )
            {
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", source[i], len);
                FS_CloseFile(file);
                return;
            }
        }

        (void)FS_CloseFile(file);

    }

	// ROM�w�b�_�o�b�t�@���R�s�[
	MI_CpuCopy32( (void *)HW_TWL_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	
	SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
}


// �w��^�C�g����ʃX���b�h�Ń��[�h�J�n����
void SYSM_StartLoadTitle( TitleProperty *pBootTitle )
{
#define THREAD_PRIO 17
#define STACK_SIZE 0xc00
	static u64 stack[ STACK_SIZE / sizeof(u64) ];
	
	SYSMi_EnableHotSW( FALSE );
	
	// �ꉞ�A�A�v�����[�h�J�n�O�Ɍ��،��ʂ�PROCESSING�ɃZ�b�g
	s_authResult = AUTH_RESULT_PROCESSING;
	// �A�v�������[�h��ԂȂ�A���[�h�J�n
	if( !pBootTitle->flags.isAppLoadCompleted ) {
		SYSMi_GetWork()->flags.common.isLoadSucceeded = FALSE;
		OS_InitThread();
		OS_CreateThread( &s_thread, (void (*)(void *))SYSMi_LoadTitleThreadFunc, (void*)pBootTitle, stack+STACK_SIZE/sizeof(u64), STACK_SIZE,THREAD_PRIO );
		OS_WakeupThreadDirect( &s_thread );
	}else if( pBootTitle->flags.isAppRelocate ) {
	// �A�v�����[�h�ς݂ŁA�Ĕz�u�v������Ȃ�A�Ĕz�u�i�J�[�h�̂ݑΉ��j
		SYSMi_Relocate();
		SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
	}else
	{
		// �A�v�����[�h�ς݂ŁA�Ĕz�u�v���Ȃ�
		SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
	}
	
	if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM ) {
		SYSMi_GetWork()->flags.common.isCardBoot = TRUE;
	}else if(pBootTitle->flags.isAppLoadCompleted)
	{
		// �J�[�h�u�[�g�łȂ��A���[�h�ς݂̏ꍇ�͍��̂Ƃ��뉽�����Ȃ�
	}
}


// �J�[�h�A�v���P�[�V�����̍Ĕz�u
static void SYSMi_Relocate( void )
{
	u32 size;
	u32 *dest = SYSM_GetCardRomHeader()->main_ram_address;
	// NTR�Z�L���A�̈�̍Ĕz�u
	DC_InvalidateRange( (void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );	// �L���b�V���P�A
	size = ( SYSM_GetCardRomHeader()->main_size < SECURE_AREA_SIZE ) ?
			 SYSM_GetCardRomHeader()->main_size : SECURE_AREA_SIZE;
	// rom�̍Ĕz�u�����Q�Ƃ��āA�Z�L���A�̈�̍Ĕz�u���ύX����K�v�����������ׂ�
	if( SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC].src != NULL )
	{
		dest = (u32 *)SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC].src;
	}
	MI_CpuCopyFast( (void *)SYSM_CARD_NTR_SECURE_BUF, dest, size );
	
	if( SYSM_GetCardRomHeader()->platform_code & PLATFORM_CODE_FLAG_TWL ) {
		// TWL���[�h
		// TWL�Z�L���A�̈�̍Ĕz�u
		dest = SYSM_GetCardRomHeader()->main_ltd_ram_address;
		DC_InvalidateRange( (void *)SYSM_CARD_TWL_SECURE_BUF, SECURE_AREA_SIZE );	// �L���b�V���P�A
		size = ( SYSM_GetCardRomHeader()->main_ltd_size < SECURE_AREA_SIZE ) ?
				 SYSM_GetCardRomHeader()->main_ltd_size : SECURE_AREA_SIZE;
		// rom�̍Ĕz�u�����Q�Ƃ��āA�Z�L���A�̈�̍Ĕz�u���ύX����K�v�����������ׂ�
		if( SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC].src != NULL )
		{
			dest = (u32 *)SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC].src;
		}
		MI_CpuCopyFast( (void *)SYSM_CARD_TWL_SECURE_BUF, dest, size );
		// TWL-ROM�w�b�_���̍Ĕz�u
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	}else {
		// NTR���[�h
		// TWL-ROM�w�b�_���̍Ĕz�u
		//   �����`���[��ROM�w�b�_���c���Ă����R�s�[�̈���N���A
		MI_CpuClearFast( (void *)HW_TWL_ROM_HEADER_BUF, SYSM_CARD_ROM_HEADER_SIZE );
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_TWL_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BUF, (void *)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
	}
}

// �A�v�����[�h�ς݁H
BOOL SYSM_IsLoadTitleFinished( void )
{
	if( SYSMi_GetWork()->flags.common.isLoadSucceeded ) {
		return TRUE;
	}
	return OS_IsThreadTerminated( &s_thread );
}

// TWL�A�v�������NTR�g��NAND�A�v�����ʂ̃w�b�_�F�؏���
static AuthResult SYSMi_AuthenticateTWLHeader( TitleProperty *pBootTitle )
{
	ROM_Header *head;
	OSTick start,prev;
	start = OS_GetTick();
	
	head = ( ROM_Header *)HW_TWL_ROM_HEADER_BUF;
	
	// NAND�A�v���̏ꍇ�ANAM_CheckTitleLaunchRights()���Ă�Ń`�F�b�N
	if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_NAND )
	{
		if( NAM_OK != NAM_CheckTitleLaunchRights( pBootTitle->titleID ))
		{
			OS_TPrintf("Authenticate failed: NAM_CheckTitleLaunchRights failed.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf("Authenticate : NAM_CheckTitleLaunchRights succeed. %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
		}
	}

	// ��������
    {
		const u8 *key;
		u32 hi;
		u8 keynum;
		SignatureData sigbuf;
		u8 calculated_hash[SVC_SHA1_DIGEST_SIZE];
		SVCSignHeapContext con;
		int l;
		u32 *module_addr[RELOCATE_INFO_NUM];
		u32 module_size[RELOCATE_INFO_NUM];
		u8 *hash_addr[RELOCATE_INFO_NUM];
		int module_num;
		BOOL b_dev = FALSE;
		
		// pBootTitle->titleID��ROM�w�b�_��titleID�̈�v�m�F������B
		if( pBootTitle->titleID != head->s.titleID )
		{
			//TWL�Ή�ROM�ŁA�w�b�_��titleID���N���w�肳�ꂽID�ƈႤ
			OS_TPrintf( "Authenticate failed: header TitleID error\n" );
			OS_TPrintf( "Authenticate failed: selectedTitleID=%.16llx\n", pBootTitle->titleID );
			OS_TPrintf( "Authenticate failed: headerTitleID=%.16llx\n", head->s.titleID );
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf( "Authenticate : header TitleID check succeed.\n" );
		}
		
		prev = OS_GetTick();
		hi = head->s.titleID_Hi;
		keynum = (u8)( (hi & TITLE_ID_SECURE_FLAG_MASK) ? 1 : ( (hi & TITLE_ID_HI_APP_TYPE_MASK) ? 0 : 2 ) ); // keynum = 0:SystemApp 1:SecureApp 2:UserApp
		// �A�v����ʂƃ{���f�B���O�I�v�V�����ɂ���Ďg�����𕪂���
// #define LNC_PDTKEY_DBG
#ifdef LNC_PDTKEY_DBG
		{
			// ���i�Ō��f�o�O�p�R�[�h
			u8 *gamecode = (u8 *)&(pBootTitle->titleID);
			// ���i�Ō��擾
			key = ((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->rsa_pubkey[SYSTEM_APP_KEY_OFFSET + keynum];
			b_dev = TRUE; // �J���ł̃X���[�t���O
			// ���i�ł�Launcher�͐�p�̌����g���B�J���ł͍��̂Ƃ���SystemApp�̌��ő�p
			if( gamecode[3] == 'L' && gamecode[2] == 'N' && gamecode[1] == 'C' )
			{
				key = ((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->rsa_pubkey[LAUNCHER_KEY_OFFSET];
			}
		}
#else
	    if( SCFG_GetBondingOption() == 0 ) {
			u8 *gamecode = (u8 *)&(pBootTitle->titleID);
			// ���i�Ō��擾
			key = ((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->rsa_pubkey[SYSTEM_APP_KEY_OFFSET + keynum];
			// ���i�ł�Launcher�͐�p�̌����g���B�J���ł͍��̂Ƃ���SystemApp�̌��ő�p
			if( gamecode[3] == 'L' && gamecode[2] == 'N' && gamecode[1] == 'C' )
			{
				key = ((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->rsa_pubkey[LAUNCHER_KEY_OFFSET];
			}
	    }else {
			// �J����
			key = g_devPubKey[keynum];
			b_dev = TRUE;
	    }
#endif
	    // ���������ŕ���
	    MI_CpuClear8( &sigbuf, sizeof(sigbuf) );
	    SVC_InitSignHeap( &con, (void *)SIGN_HEAP_ADDR, SIGN_HEAP_SIZE );// �q�[�v�̏�����
	    if( !SVC_DecryptSign( &con, sigbuf.digest, head->signature, key ))
	    {
			OS_TPrintf("Authenticate failed: Sign decryption failed.\n");
			if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
		// �w�b�_�̃n�b�V��(SHA1)�v�Z
		SVC_CalcSHA1( calculated_hash, (const void*)head, TWL_ROM_HEADER_HASH_CALC_DATA_LEN );
	    // �����̃n�b�V���l�ƃw�b�_�̃n�b�V���l���r
	    if(!SVC_CompareSHA1(sigbuf.digest, (const void *)calculated_hash))
	    {
			OS_TPrintf("Authenticate failed: Sign check failed.\n");
			if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf("Authenticate : Sign check succeed. %dms.\n", OS_TicksToMilliSeconds(OS_GetTick() - prev));
		}
	    
		// ���ꂼ��ARM9,7��FLX�����LTD�ɂ��ăn�b�V�����v�Z���ăw�b�_�Ɋi�[����Ă���n�b�V���Ɣ�r
		module_addr[ARM9_STATIC] = head->s.main_ram_address;
		module_addr[ARM7_STATIC] = head->s.sub_ram_address;
		module_size[ARM9_STATIC] = head->s.main_size;
		module_size[ARM7_STATIC] = head->s.sub_size;
		hash_addr[ARM9_STATIC] = &(head->s.main_static_digest[0]);
		hash_addr[ARM7_STATIC] = &(head->s.sub_static_digest[0]);
		module_num = 2;
		
		// NITRO�A�v���̊g���ł͎g��Ȃ��̈�
		if( head->s.platform_code != 0 )
		{
			module_addr[ARM9_LTD_STATIC] = head->s.main_ltd_ram_address;
			module_addr[ARM7_LTD_STATIC] = head->s.sub_ltd_ram_address;
			module_size[ARM9_LTD_STATIC] = head->s.main_ltd_size;
			module_size[ARM7_LTD_STATIC] = head->s.sub_ltd_size;
			hash_addr[ARM9_LTD_STATIC] = &(head->s.main_ltd_static_digest[0]);
			hash_addr[ARM7_LTD_STATIC] = &(head->s.sub_ltd_static_digest[0]);
			module_num = RELOCATE_INFO_NUM;
		}
		
		for( l=0; l<module_num ; l++ )
		{
			static const char *str[4]={"ARM9_STATIC","ARM7_STATIC","ARM9_LTD_STATIC","ARM7_LTD_STATIC"};
			prev = OS_GetTick();
			// �ꎞ�I�Ɋi�[�ʒu�����炵�Ă���ꍇ�́A�Ĕz�u��񂩂烂�W���[���i�[�A�h���X���擾
			if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL )
			{
				module_addr[l] = (u32 *)SYSMi_GetWork()->romRelocateInfo[l].src;
			}
			// �n�b�V���v�Z
			if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM && l == 0)
			{
				// �J�[�h�̏ꍇ��ARM9_STATIC�n�b�V���`�F�b�N
				// �J�[�h�ǂݍ��ݎ��Awork2�ɈÍ����I�u�W�F�N�g�����̃n�b�V���v�Z�ς݂̃R���e�L�X�g���ۑ������̂�
				// �����p����ARM9_STATIC�c��̕������v�Z
				SVCHMACSHA1Context ctx;
				u16 id;
                
				SVC_HMACSHA1Init( &ctx, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1 );
                
				// ARM7�Ƃ�hmac_sha1_context�̔r������J�n
                id = (u16)OS_GetLockID();
				(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
                
				SYSMi_GetWork2()->hmac_sha1_context.sha1_ctx.sha_block = ctx.sha1_ctx.sha_block;// ���̊֐��|�C���^����ARM7��ARM9�ŕς��Ȃ��ƃ_��
				ctx = SYSMi_GetWork2()->hmac_sha1_context;										// SYSMi_GetWork2�͔�L���b�V���Ȃ̂ŃX�^�b�N�iDTCM�܂��̓L���b�V���̈�j�փR�s�[

                // ARM7�Ƃ�hmac_sha1_context�̔r������I��
				(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
				OS_ReleaseLockID( id );
                
				SVC_HMACSHA1Update( &ctx,
									(const void*)((u32)module_addr[l] + ARM9_ENCRYPT_DEF_SIZE),
									(module_size[l] - ARM9_ENCRYPT_DEF_SIZE) );
				SVC_HMACSHA1GetHash( &ctx, calculated_hash );
			}else
			{
				SVC_CalcHMACSHA1( calculated_hash, (const void*)module_addr[l], module_size[l],
								 (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1 );
			}
			// ��r
		    if(!SVC_CompareSHA1((const void *)hash_addr[l], (const void *)calculated_hash))
		    {
				OS_TPrintf("Authenticate failed: %s module hash check failed.\n", str[l]);
				if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
			}else
			{
				OS_TPrintf("Authenticate : %s module hash check succeed. %dms.\n", str[l], OS_TicksToMilliSeconds(OS_GetTick() - prev));
			}
		}
	}
	OS_TPrintf("Authenticate : total %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
	return AUTH_RESULT_SUCCEEDED;
}

// NTR�œ���NAND�A�v���ipictochat���j�̃w�b�_�F�؏���
static AuthResult SYSMi_AuthenticateNTRNandAppHeader( TitleProperty *pBootTitle)
{
	return SYSMi_AuthenticateTWLHeader( pBootTitle );
}

// NTR�Ń_�E�����[�h�A�v���iTMP�A�v���j�̃w�b�_�F�؏���
static AuthResult SYSMi_AuthenticateNTRDownloadAppHeader( TitleProperty *pBootTitle)
{

	ROM_Header *head;
	OSTick start;
	start = OS_GetTick();
	
	head = ( ROM_Header *)HW_TWL_ROM_HEADER_BUF;

	// ��������
    {
		u8 buf[0x80];
		SVCSignHeapContext con;
		u8 calculated_hash[SVC_SHA1_DIGEST_SIZE * 3 + sizeof(u32)];
		u8 final_hash[SVC_SHA1_DIGEST_SIZE];
		int l;
		u32 *module_addr[RELOCATE_INFO_NUM];
		u32 module_size[RELOCATE_INFO_NUM];
		u8 *hash_addr[RELOCATE_INFO_NUM];
		int module_num;

		// [TODO:]pBootTitle->titleID�ƁA����ɂ�������NTR�w�b�_�̂Ȃ�炩�̃f�[�^�Ƃ̈�v�m�F������B

		// NTR�_�E�����[�h�A�v�������̃}�W�b�N�R�[�h�`�F�b�N
		if( s_authcode.magic_code[0] != 'a' || s_authcode.magic_code[1] != 'c' ) {
			OS_TPrintf("Authenticate failed: Invalid AuthCode.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}

		// NTR�_�E�����[�h�A�v�������iDER�t�H�[�}�b�g�j�̌v�Z�A�n�b�V���̎擾�B
	    MI_CpuClear8( buf, 0x80 );
	    SVC_InitSignHeap( &con, (void *)SIGN_HEAP_ADDR, SIGN_HEAP_SIZE );// �q�[�v�̏�����
	    if( !SVC_DecryptSignDER( &con, buf, s_authcode.sign, nitro_dl_sign_key ))
	    {
			OS_TPrintf("Authenticate failed: Sign decryption failed.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
		
		// ���ꂼ��ARM9,7��FLX�ɂ��ăn�b�V�����v�Z���āA�����3����ׂ����̂ɑ΂��Ă܂��n�b�V�����Ƃ�
		module_addr[ARM9_STATIC] = head->s.main_ram_address;
		module_addr[ARM7_STATIC] = head->s.sub_ram_address;
		module_size[ARM9_STATIC] = head->s.main_size;
		module_size[ARM7_STATIC] = head->s.sub_size;
		hash_addr[ARM9_STATIC] = &(head->s.main_static_digest[0]);
		hash_addr[ARM7_STATIC] = &(head->s.sub_static_digest[0]);
		module_num = 2;
		
		// �w�b�_
		SVC_CalcSHA1( calculated_hash, (const void*)head, NTR_ROM_HEADER_HASH_CALC_DATA_LEN );
		// ���W���[��
		for( l=0; l<module_num ; l++ )
		{
			// �ꎞ�I�Ɋi�[�ʒu�����炵�Ă���ꍇ�́A�Ĕz�u��񂩂烂�W���[���i�[�A�h���X���擾
			if( SYSMi_GetWork()->romRelocateInfo[l].src != NULL )
			{
				module_addr[l] = (u32 *)SYSMi_GetWork()->romRelocateInfo[l].src;
			}
			// �n�b�V���v�Z
			{
				SVC_CalcSHA1( &calculated_hash[SVC_SHA1_DIGEST_SIZE * (l+1)], (const void*)module_addr[l], module_size[l]);
			}
		}
		// �V���A���i���o�[�t��
		*(u32 *)(&(calculated_hash[SVC_SHA1_DIGEST_SIZE * 3])) = s_authcode.serial_number;
		// �ŏI�n�b�V���v�Z
		SVC_CalcSHA1( final_hash, calculated_hash, SVC_SHA1_DIGEST_SIZE * 3 + sizeof(u32));
		
		// �v�Z�����ŏI�n�b�V���ƁA�������瓾���n�b�V���Ƃ��r
	    if(!SVC_CompareSHA1((const void *)buf, (const void *)final_hash))
	    {
			OS_TPrintf("Authenticate failed: hash check failed.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf("Authenticate : hash check succeed.\n");
		}
	}
	OS_TPrintf("Authenticate : total %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
	
	return AUTH_RESULT_SUCCEEDED;
}

// �w�b�_�F��
static AuthResult SYSMi_AuthenticateHeader( TitleProperty *pBootTitle)
{
	ROM_Header_Short *hs = ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
	// [TODO:]�F�،��ʂ͂ǂ������[�N�ɕۑ����Ă���
	if( hs->platform_code & PLATFORM_CODE_FLAG_TWL )
	{
		// TWL�A�v��
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate :TWL_NAND start.\n" );
				return SYSMi_AuthenticateTWLHeader( pBootTitle );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate :TWL_ROM start.\n" );
				return SYSMi_AuthenticateTWLHeader( pBootTitle );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate :TWL_TEMP start.\n" );
				if (!hs->permit_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					return AUTH_RESULT_AUTHENTICATE_FAILED;
				}
				return SYSMi_AuthenticateTWLHeader( pBootTitle );
			default:
				return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
	}
	else
	{
		if( hs->platform_code & PLATFORM_CODE_FLAG_NOT_NTR )
		{
			// TWL�ł�NTR�ł��Ȃ��s���ȃA�v��
			OS_TPrintf( "Authenticate :NOT NTR NOT TWL.\n" );
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
		// NTR�A�v��
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate :NTR_NAND start.\n" );
				return SYSMi_AuthenticateNTRNandAppHeader( pBootTitle );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate :NTR_TEMP start.\n" );
				if (!hs->permit_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					return AUTH_RESULT_AUTHENTICATE_FAILED;
				}
				return SYSMi_AuthenticateNTRDownloadAppHeader( pBootTitle );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate :NTR_ROM start.\n" );
				// NTR�J�[�h�͍��̂Ƃ���F�ؗ\�薳��
				return AUTH_RESULT_SUCCEEDED;
			default:
				return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
	}
}

// �F�؏����̃X���b�h
static void SYSMi_AuthenticateTitleThreadFunc( TitleProperty *pBootTitle )
{
	// ���[�h��
	if( !SYSM_IsLoadTitleFinished() ) {
		s_authResult = AUTH_RESULT_PROCESSING;
		return;
	}
	// ���[�h�����H
	if( SYSMi_GetWork()->flags.common.isLoadSucceeded == FALSE )
	{
		s_authResult = AUTH_RESULT_TITLE_LOAD_FAILED;
		return;
	}
	// �p�����[�^�`�F�b�N
	if( !SYSMi_CheckTitlePointer( pBootTitle ) ) {
		s_authResult = AUTH_RESULT_TITLE_POINTER_ERROR;
		return;
	}
#if 0
	// �G���g���A�h���X�̐��������`�F�b�N
	if( !SYSMi_CheckEntryAddress() ) {
		s_authResult = AUTH_RESULT_ENTRY_ADDRESS_ERROR;
		return;
	}
#endif
	
	// BOOTTYPE_MEMORY��NTR���[�h��FS����Ńu�[�g����ƁA��NitroSDK�Ńr���h���ꂽ�A�v���̏ꍇ�A
	// ROM�A�[�J�C�u�ɃJ�[�h�����蓖�Ă��āAFS�Ŋ֌W�Ȃ��J�[�h�ɃA�N�Z�X�ɂ����Ă��܂��̂ŁA�����h�~����B
	if( ( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_MEMORY ) &&
		( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->platform_code ) == 0 ) &&
		( ( (( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF)->fat_size ) > 0 )
		) {
		s_authResult = AUTH_RESULT_TITLE_BOOTTYPE_ERROR;
		return;
	}
	
	// ��ROM�w�b�_�F��
	s_authResult = SYSMi_AuthenticateHeader( pBootTitle );
}


// ���[�h�ς݂̎w��^�C�g����ʃX���b�h�Ō��؊J�n����
void SYSM_StartAuthenticateTitle( TitleProperty *pBootTitle )
{
	static u64 stack[ STACK_SIZE / sizeof(u64) ];
	s_authResult = AUTH_RESULT_PROCESSING;
	OS_InitThread();
	OS_CreateThread( &s_auth_thread, (void (*)(void *))SYSMi_AuthenticateTitleThreadFunc, (void*)pBootTitle, stack+STACK_SIZE/sizeof(u64), STACK_SIZE,THREAD_PRIO );
	OS_WakeupThreadDirect( &s_auth_thread );
}

// ���؍ς݁H
BOOL SYSM_IsAuthenticateTitleFinished( void )
{
	if(s_authResult == AUTH_RESULT_SUCCEEDED)
	{
		return TRUE;
	}
	return OS_IsThreadTerminated( &s_auth_thread );
}

// ���[�h�ς݂̎w��^�C�g���̔F�؂ƃu�[�g���s��
AuthResult SYSM_TryToBootTitle( TitleProperty *pBootTitle )
{
	if(s_authResult != AUTH_RESULT_SUCCEEDED)
	{
		return s_authResult;
	}
	
	// �}�E���g���̓o�^
	SYSMi_SetBootAppMountInfo( pBootTitle );
	
	// HW_WM_BOOT_BUF�ւ̃u�[�g���Z�b�g
	( (OSBootInfo *)OS_GetBootInfo() )->boot_type = s_launcherToOSBootType[ pBootTitle->flags.bootType ];
	
	BOOT_Ready();	// never return.
	
	return AUTH_RESULT_SUCCEEDED;
}


#if 0
// �w��^�C�g���̔F�؁����[�h�@���P�t���[������I����B
// ����������Ǝg��Ȃ�����
void SYSM_LoadAndAuthenticateTitleThread( TitleProperty *pBootTitle )
{
	SYSMi_LoadTitleThreadFunc( pBootTitle );
	OS_JoinThread(&s_thread);
	
	// �F��
	return SYSM_AuthenticateTitle( pBootTitle );
}
#endif


// �w��^�C�g�����u�[�g�\�ȃ|�C���^���`�F�b�N
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle )
{
#pragma unused( pBootTitle )
	
	return TRUE;
}

#if 0
void CheckDigest( void )
{
	int i;
	for( i = 0; i < 4; i++ ) {
		if( SYSMi_GetWork()->reloc_info[ i ].src ) {
			
		}else {
		}
	}
}
#endif


// �����}���L���^�������Z�b�g
void SYSMi_EnableHotSW( BOOL enable )
{
	enable = enable ? 1 : 0;
	
	// ���݂̒l�Ɠ����Ȃ牽���������^�[��
	if( SYSMi_GetWork()->flags.hotsw.isEnableHotSW == enable ) {
		return;
	}
	
	{
		HotSwPxiMessage msg;

        msg.msg.value = enable;
        msg.msg.ctrl  = TRUE;

	    while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
    	{
        	// do nothing
    	}
        
	}
	
	// �l���ω�����܂ŃX���[�v���đ҂B
	while( SYSMi_GetWork()->flags.hotsw.isEnableHotSW != enable ) {
		OS_Sleep( 2 );
	}
}