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
#include "fs_wram.h"

// define data-----------------------------------------------------------------
#define CARD_BANNER_INDEX			( LAUNCHER_TITLE_LIST_NUM - 1 )

#define LAUNCHER_KEY_INDEX			0 // �t�@�[�����瑗���Ă��錮�̂���Launcher�L�[�̃C���f�b�N�X
#define SYSTEM_APP_KEY_INDEX		1 // �t�@�[�����瑗���Ă���SYSTEM�A�v���L�[�̃C���f�b�N�X
#define SECURE_APP_KEY_INDEX		2 // �t�@�[�����瑗���Ă���SECURE�A�v���L�[�̃C���f�b�N�X
#define USER_APP_KEY_INDEX			3 // �t�@�[�����瑗���Ă���USER�A�v���L�[�̃C���f�b�N�X

#define ROM_HEADER_HASH_OFFSET		(0x0) // ��������ROM�w�b�_�n�b�V�������o�����߂̃I�t�Z�b�g

#define SIGN_HEAP_ADDR	0x023c0000	// �����v�Z�̂��߂̃q�[�v�̈�J�n�A�h���X
#define SIGN_HEAP_SIZE	0x1000		// �����v�Z�̂��߂̃q�[�v�T�C�Y
#define ARM9_ENCRYPT_DEF_SIZE	0x800	// ARM9FLX�̐擪�Í��������̃T�C�Y

#define	DIGEST_HASH_BLOCK_SIZE_SHA1					(512/8)
#define TWL_ROM_HEADER_HASH_CALC_DATA_LEN	0xe00 // ROM�w�b�_�̃n�b�V���v�Z���镔���̒���TWL��
#define NTR_ROM_HEADER_HASH_CALC_DATA_LEN	0x160 // ROM�w�b�_�̃n�b�V���v�Z���镔���̒���DS��

#define AUTH_KEY_BUFFER_LEN 128
#define MB_AUTH_SIGN_SIZE				(128)	/* digital sign size */

#define WRAM_SLOT_FOR_FS	5
#define WRAM_SIZE_FOR_FS	MI_WRAM_SIZE_96KB

typedef	struct	MbAuthCode
{
	char		magic_code[2];			// �}�W�b�N�i���o�[
	u16		version;			// �o�[�W����
	u8		sign[MB_AUTH_SIGN_SIZE];	// ����
	u32		serial_number;			// �V���A���ԍ�
} MbAuthCode;	// 16byte

typedef struct CalcHMACSHA1CallbackArg
{
	SVCHMACSHA1Context	ctx;
	u32					hash_length;
} CalcHMACSHA1CallbackArg;

typedef struct CalcSHA1CallbackArg
{
	SVCSHA1Context	ctx;
	u32					hash_length;
} CalcSHA1CallbackArg;

// extern data-----------------------------------------------------------------
extern const u8 g_devPubKey[ 4 ][ 0x80 ];

// function's prototype-------------------------------------------------------
static s32  ReadFile( FSFile* pf, void* buffer, s32 size );

static void SYSMi_LoadTitleThreadFunc( TitleProperty *pBootTitle );
static BOOL SYSMi_CheckTitlePointer( TitleProperty *pBootTitle );
static void SYSMi_makeTitleIdList( void );
static AuthResult SYSMi_AuthenticateHeader( TitleProperty *pBootTitle, ROM_Header *head );

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
static OSThread			s_thread;
static OSThread			s_auth_thread;
static TWLBannerFile	s_bannerBuf[ LAUNCHER_TITLE_LIST_NUM ] ATTRIBUTE_ALIGN(32);
static AuthResult		s_authResult = AUTH_RESULT_PROCESSING;	// ROM���،���

static MbAuthCode s_authcode;

static BOOL				s_loadstart = FALSE;

static NAMTitleId *s_pTitleIDList = NULL;
static int s_listLength = 0;

static u8 *s_calc_hash = NULL;

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
//
// ���擾
//
//
// ============================================================================

// �J�[�h�^�C�g���̎擾
BOOL SYSM_GetCardTitleList( TitleProperty *pTitleList_Card )
{
	BOOL retval = FALSE;
	
	if(s_loadstart)
	{
		// ���[�h�J�n���Ă�����A�����w�b�_��^�C�g�����͕ύX���Ȃ�
		return retval;
	}
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
			pTitleList_Card->flags.isAppLoadCompleted = FALSE;
			pTitleList_Card->flags.isAppRelocate = TRUE;
		}
		
		// �^�C�g�����t���O�̃Z�b�g
		pTitleList_Card->flags.bootType = LAUNCHER_BOOTTYPE_ROM;
		pTitleList_Card->titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
		retval = TRUE;
	}
	
	return retval;
}

// �J�[�hROM�w�b�_��ARM7�o�b�t�@����ARM9�o�b�t�@�ւ̃R�s�[
BOOL SYSMi_CopyCardRomHeader( void )
{
	BOOL retval = FALSE;

	if( SYSM_IsExistCard() ) {
		// ROM�w�b�_�̃��[�h
		DC_InvalidateRange( (void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_APP_ROM_HEADER_SIZE );	// �L���b�V���P�A
		MI_CpuCopyFast( (void *)SYSM_CARD_ROM_HEADER_BAK, (void *)SYSM_APP_ROM_HEADER_BUF, SYSM_APP_ROM_HEADER_SIZE );	// ROM�w�b�_�R�s�[
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

// �C���|�[�g����Ă��邷�ׂĂ�NAND�A�v����񋓂������X�g�̏���
// SYSM_GetNandTitleList�����SYSM_TryToBootTitle�O�ɌĂԕK�v����
BOOL SYSM_InitNandTitleList( void )
{
	OSTick start;

	if( s_pTitleIDList != NULL ) return TRUE;

	// �C���|�[�g����Ă���^�C�g���̎擾
	start = OS_GetTick();
	s_listLength = NAM_GetNumTitles();
	OS_TPrintf( "NAM_GetNumTitles : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	s_pTitleIDList = SYSM_Alloc( sizeof(NAMTitleId) * s_listLength );
	if( s_pTitleIDList == NULL ) {
		OS_TPrintf( "%s: alloc error.\n", __FUNCTION__ );
		return FALSE;
	}
	start = OS_GetTick();
	(void)NAM_GetTitleList( s_pTitleIDList, (u32)s_listLength );
	OS_TPrintf( "NAM_GetTitleList : %dus\n", OS_TicksToMicroSeconds( OS_GetTick() - start ) );
	
	return TRUE;
}

// NAND�A�v�����X�g�̉��
void SYSM_FreeNandTitleList( void )
{
	if(s_pTitleIDList != NULL)
	{
		SYSM_Free( s_pTitleIDList );
		s_pTitleIDList = NULL;
	}
}

// ���[���`�ΏۂƂȂ�NAND�^�C�g�����X�g�̎擾
// listNum�ɂ́ApTitleList_Nand�̒�����^����
// ������ő�̃^�C�g�����́A(LAUNCHER_TITLE_LIST_NUM - 1)�ɐ��������i�����`���[���\���ł���ő吔����J�[�h�Ԃ�����������j
// return:�擾����NAND�^�C�g���̐�
int SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int listNum )
{
															// filter_flag : ALL, ALL_APP, SYS_APP, USER_APP, Data only, ���̏������w�肵�ă^�C�g�����X�g���擾����B
	// �Ƃ肠����ALL
	int l;
	int validNum = 0;
	NAMTitleId titleIDArray[ LAUNCHER_TITLE_LIST_NUM - 1 ];// ���[���`�\�ȃ^�C�g�����X�g�̈ꎞ�u����
	
	if( s_pTitleIDList == NULL ) return -1;
	
	// �擾�����^�C�g�������[���`�Ώۂ��ǂ������`�F�b�N
	for( l = 0; l < s_listLength; l++ ) {
		// "Not Launch"�łȂ��@���@"Data Only"�łȂ��@�Ȃ�L���ȃ^�C�g���Ƃ��ă��X�g�ɒǉ�
		if( ( s_pTitleIDList[ l ] & ( TITLE_ID_NOT_LAUNCH_FLAG_MASK | TITLE_ID_DATA_ONLY_FLAG_MASK ) ) == 0 ) {
			titleIDArray[ validNum ] = s_pTitleIDList[ l ];
			SYSMi_ReadBanner_NAND( s_pTitleIDList[ l ], &s_bannerBuf[ validNum ] );
			validNum++;
			if( !( validNum < LAUNCHER_TITLE_LIST_NUM - 1 ) )// �ő�(LAUNCHER_TITLE_LIST_NUM - 1)�܂�
			{
				break;
			}
		}
	}
	
	// �O�̂��ߎc��̈��0�N���A
	for( l = validNum; l < LAUNCHER_TITLE_LIST_NUM - 1; l++ ) {
		titleIDArray[ l ] = 0;
	}
	
	// �ŏI���X�g�ɑ΂��āA�J�[�h�A�v���������������������N���A
	MI_CpuClearFast( &pTitleList_Nand[ 1 ], sizeof(TitleProperty) * ( listNum - 1 ) );
	
	listNum--; // �J�[�h�̂Ԃ�����Ă���
	
	// �����ɗ^����ꂽ���X�g�̒���-1 �ƁA���[���`�\�^�C�g�����X�g�̒����̔�r
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
//
// �A�v�����[�h
//
//
// ============================================================================

static void SYSMi_CalcHMACSHA1Callback(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg)
{
	CalcHMACSHA1CallbackArg *cba = (CalcHMACSHA1CallbackArg *)arg;
	u32 calc_len = ( cba->hash_length < len ? cba->hash_length : len );
	OSIntrMode enabled = OS_DisableInterrupts();// WRAM�؂�ւ��r���Ŋ��荞�ݔ������ʃX���b�h��WRAM�؂�ւ������S�̉\��������̂Ŏb��Ή�
	MI_SwitchWramSlot( wram, slot, MI_WRAM_SIZE_32KB, MI_WRAM_ARM9, MI_WRAM_ARM7 );// Wram��7�ɃX�C�b�`
	SYSM_StartDecryptAESRegion_W( addr, orig_addr, len ); // AES�̈�f�N���v�g
	MI_SwitchWramSlot( wram, slot, MI_WRAM_SIZE_32KB, MI_WRAM_ARM7, MI_WRAM_ARM9 );// Wram��7�ɃX�C�b�`���Ă��܂��Ă���̂Ŗ߂�
	OS_RestoreInterrupts(enabled);// ���荞�݋���
	if( calc_len == 0 ) return;
	cba->hash_length -= calc_len;
	SVC_HMACSHA1Update( &cba->ctx, addr, calc_len );
}

static void SYSMi_CalcSHA1Callback(const void* addr, const void* orig_addr, u32 len, MIWramPos wram, s32 slot, void* arg)
{
	CalcSHA1CallbackArg *cba = (CalcSHA1CallbackArg *)arg;
	u32 calc_len = ( cba->hash_length < len ? cba->hash_length : len );
	OSIntrMode enabled = OS_DisableInterrupts();// WRAM�؂�ւ��r���Ŋ��荞�ݔ������ʃX���b�h��WRAM�؂�ւ������S�̉\��������̂Ŏb��Ή�
	MI_SwitchWramSlot( wram, slot, MI_WRAM_SIZE_32KB, MI_WRAM_ARM9, MI_WRAM_ARM7 );// Wram��7�ɃX�C�b�`
	SYSM_StartDecryptAESRegion_W( addr, orig_addr, len ); // AES�̈�f�N���v�g
	MI_SwitchWramSlot( wram, slot, MI_WRAM_SIZE_32KB, MI_WRAM_ARM7, MI_WRAM_ARM9 );// Wram��7�ɃX�C�b�`���Ă��܂��Ă���̂Ŗ߂�
	OS_RestoreInterrupts(enabled);// ���荞�݋���
	if( calc_len == 0 ) return;
	cba->hash_length -= calc_len;
	SVC_SHA1Update( &cba->ctx, addr, calc_len );
}

static void SYSMi_FinalizeHotSWAsync( TitleProperty *pBootTitle, ROM_Header *head )
{
	HotSwApliType hotsw_type;

	DC_StoreRange( head, sizeof(ROM_Header) );

	switch( pBootTitle->flags.bootType )
	{
		case LAUNCHER_BOOTTYPE_NAND:
		case LAUNCHER_BOOTTYPE_TEMP:
			if ( head->s.platform_code & PLATFORM_CODE_FLAG_TWL )
			{
				hotsw_type = HOTSW_APLITYPE_TWL_NAND;
			}
			else
			{
				hotsw_type = HOTSW_APLITYPE_NTR_NAND;
			}
			break;
		case LAUNCHER_BOOTTYPE_ROM:
		default:
			hotsw_type = HOTSW_APLITYPE_CARD;
			break;
	}

	HOTSW_FinalizeHotSWAsync( hotsw_type );
}

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
	// ���ANTR-ROM�w�b�_�͋������p�b�`�ƃf�o�b�K�p�b�`�𓖂Ă�K�v�����邽�߁A�Ĕz�u��reboot���C�u�����ōs���B
	
	// ���[�h
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    BOOL isTwlApp = TRUE;
    BOOL isCardApp = FALSE;
	
	switch( pBootTitle->flags.bootType )
	{
	case LAUNCHER_BOOTTYPE_NAND:
		// NAND
    	NAM_GetTitleBootContentPathFast(path, pBootTitle->titleID);
		break;
	case LAUNCHER_BOOTTYPE_ROM:
		// CARD
		isCardApp = TRUE;
		break;
	case LAUNCHER_BOOTTYPE_TEMP:
		// tmp�t�H���_
		STD_TSNPrintf( path, 256, OS_TMP_APP_PATH, pBootTitle->titleID );
		break;
	default:
		// unknown
		return;
	}

	if(!isCardApp)
	{
		FS_InitFile( file );
	    bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);
    }else
    {
		bSuccess = TRUE;
	}

    if( ! bSuccess )
    {
OS_TPrintf("RebootSystem failed: cant open file\n");
		goto ERROR;
    }

    {
        int     i;
        u32     source[region_max];
        u32     length[region_max];
        u32     destaddr[region_max];
        static u8   header[HW_TWL_ROM_HEADER_BUF_SIZE] ATTRIBUTE_ALIGN(32);
        s32 readLen;
        ROM_Header *head = (ROM_Header *)header;

		// WRAM���pRead�֐��̏����AWRAMC�̃X���b�g�̂������3����������Ă���
		FS_InitWramTransfer(3);
		MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
		MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
		MI_FreeWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
		MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM7 );
		MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_ARM9 );
		MI_CancelWramSlot_C( WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, MI_WRAM_DSP );
		
		// �n�b�V���i�[�p�o�b�t�@�i�q�[�v�������Ă��邯�ǕύX���邩���j
		s_calc_hash = SYSM_Alloc( region_max * SVC_SHA1_DIGEST_SIZE );

        // �܂�ROM�w�b�_��ǂݍ���
        if(!isCardApp)
        {
        	bSuccess = FS_SeekFile(file, 0x00000000, FS_SEEK_SET);
		}else
		{
			bSuccess = TRUE;
		}

        if( ! bSuccess )
        {
OS_TPrintf("RebootSystem failed: cant seek file(0)\n");
			goto ERROR;
        }

		//�w�b�_�ǂݍ��݂Ɠ����Ɋe��n�b�V���v�Z
		{
            BOOL result;
            u32 len = MATH_ROUNDUP( (s32)sizeof(header), SYSM_ALIGNMENT_LOAD_MODULE );
			CalcSHA1CallbackArg arg;
            SVC_SHA1Init( &arg.ctx );
            arg.hash_length = (u32)( isTwlApp ? TWL_ROM_HEADER_HASH_CALC_DATA_LEN : NTR_ROM_HEADER_HASH_CALC_DATA_LEN );
            if(!isCardApp)
	        {
	            result = FS_ReadFileViaWram(file, (void *)header, (s32)len, MI_WRAM_C,
	            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcSHA1Callback, &arg );
	        }else
	        {
				result = HOTSW_ReadCardViaWram((void*) 0, (void*)header, (s32)len, MI_WRAM_C,
	            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcSHA1Callback, &arg );
			}
            SVC_SHA1GetHash( &arg.ctx, &s_calc_hash[0] );
			if ( !result )
			{
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", 0, len);
				goto ERROR;
			}
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
			goto ERROR;
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
					goto ERROR;
		        }
		        readLen = FS_ReadFile(file, &s_authcode, (s32)sizeof(s_authcode));
		        if( readLen != (s32)sizeof(s_authcode) )
		        {
OS_TPrintf("RebootSystem failed: cant read file(%p, %d, %d, %d)\n", &s_authcode, 0, sizeof(s_authcode), readLen);
					goto ERROR;
		        }
			}
		}
		
		//[TODO:]���̎��_�Ńw�b�_�̐���������
		// ��ROM�w�b�_�F��
		if( AUTH_RESULT_SUCCEEDED != SYSMi_AuthenticateHeader( pBootTitle, head ) )
		{
			goto ERROR;
		}
		
		// �������̌��؂��ꂽ�w�b�_���A�{���̃w�b�_�o�b�t�@�փR�s�[
		MI_CpuCopy8( head, (void*)SYSM_APP_ROM_HEADER_BUF, HW_TWL_ROM_HEADER_BUF_SIZE );
		
		// �w�b�_�ǂݍ��݊�������̏���
		// �w�b�_�ǂݍ��݊����t���O�𗧂Ă�
		SYSMi_GetWork()->flags.common.isHeaderLoadCompleted = TRUE;
		// HOTSW�I�������L����
		SYSMi_FinalizeHotSWAsync( pBootTitle, (void*)SYSM_APP_ROM_HEADER_BUF );
		
        // �e�̈��ǂݍ���
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
				goto ERROR;
			}
		}
		
		// AES�������i�w�b�_�ƍĔz�u��񂪂�����Ă���j
		(void)SYSM_InitDecryptAESRegion_W( (ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF );

        for (i = region_arm9_ntr; i < region_max; ++i)
        {
			BOOL result;
			
            u32 len = MATH_ROUNDUP( length[i], SYSM_ALIGNMENT_LOAD_MODULE );// AES�����DMA�]���T�C�Y�̎d�l�ŁA���[�h�T�C�Y��32�o�C�g�A���C�����g�ɕ␳
            
            if ( !isTwlApp && i >= region_arm9_twl ) continue;// nitro�ł͓ǂݍ��܂Ȃ��̈�
	        if(!isCardApp)
	        {
	            bSuccess = FS_SeekFile(file, (s32)source[i], FS_SEEK_SET);
            }else
            {
				bSuccess = TRUE;
			}

            if( ! bSuccess )
            {
OS_TPrintf("RebootSystem failed: cant seek file(%d)\n", source[i]);
				goto ERROR;
            }

OS_TPrintf("RebootSystem : Load VIA WRAM %d.\n", i);
            // �����Ń��[�h�����Ɠ����Ƀn�b�V���v�Z��AES����������Ă��܂�
            // �ʃX���b�h�œ���WRAM�g�����Ƃ���Ƒ����R�P��̂Œ���
            
            // �R�[���o�b�N�֐��ɗ^�������������������Read
            if( !isTwlApp && pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP )
            {
				// NTR�_�E�����[�h�A�v���̃��W���[��
				CalcSHA1CallbackArg arg;
	            SVC_SHA1Init( &arg.ctx );
	            arg.hash_length = (u32)length[i];
	            if(!isCardApp)
		        {
		            result = FS_ReadFileViaWram(file, (void *)destaddr[i], (s32)len, MI_WRAM_C,
		            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcSHA1Callback, &arg );
		        }else
		        {
					result = HOTSW_ReadCardViaWram((void *)source[i], (void *)destaddr[i], (s32)len, MI_WRAM_C,
		            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcSHA1Callback, &arg );
				}
	            SVC_SHA1GetHash( &arg.ctx, &s_calc_hash[i * SVC_SHA1_DIGEST_SIZE] );
			}else
			{
				//[TODO:]TWL�A�v����NTRNAND�A�v���͂��̂܂܂ŗǂ����ANTR�J�[�h�A�v����DHT�̂��ߍX�ɏ����𕪂���K�v����
				//       �w�b�_�����p�Ƀw�b�_��SHA1�iHMAC�łȂ��j���Ƃ�i����͏�ł���Ă���j
				//       DHT�`�F�b�Nphase1�p�̃n�b�V�����v�Z�iDHT_CheckHashPhase1Update �֐��j���A���ʂ܂ŏo���Ă���
				//       DHT�`�F�b�N�p�Ƀw�b�_��HMACSHA1���v�Z����K�v�����邪�ʓ|�Ȃ̂Ńw�b�_�ǂݍ��݌��
				//       DHT_CheckHashPhase1Init ���ĂׂΗǂ��C������i�ŏ��P��32KB�������������j
				// ����ȊO
				CalcHMACSHA1CallbackArg arg;
	            SVC_HMACSHA1Init( &arg.ctx, (void *)s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1 );
	            arg.hash_length = length[i];
	            if(!isCardApp)
		        {
		            result = FS_ReadFileViaWram(file, (void *)destaddr[i], (s32)len, MI_WRAM_C,
		            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcHMACSHA1Callback, &arg );
		        }else
		        {
					result = HOTSW_ReadCardViaWram((void *)source[i], (void *)destaddr[i], (s32)len, MI_WRAM_C,
		            							WRAM_SLOT_FOR_FS, WRAM_SIZE_FOR_FS, SYSMi_CalcHMACSHA1Callback, &arg );
		            //speed test code
		            /*
		            HOTSW_ReadCardData((void *)source[i], (void *)destaddr[i], (u32)len);
		            SVC_HMACSHA1Update( &arg.ctx, (void *)destaddr[i], length[i] );
		            */
				}
	            SVC_HMACSHA1GetHash( &arg.ctx, &s_calc_hash[i * SVC_SHA1_DIGEST_SIZE] );
			}
			if ( !result )
			{
OS_TPrintf("RebootSystem failed: cant read file(%d, %d)\n", source[i], len);
				goto ERROR;
			}
        }

		if(!isCardApp)
		{
	        (void)FS_CloseFile(file);
        }

    }
	SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
	return;
	
ERROR:
	if(!isCardApp)
	{
        (void)FS_CloseFile(file);
    }
}


// �w��^�C�g����ʃX���b�h�Ń��[�h�J�n����
void SYSM_StartLoadTitle( TitleProperty *pBootTitle )
{
#define THREAD_PRIO 17
#define STACK_SIZE 0xc00
	static u64 stack[ STACK_SIZE / sizeof(u64) ];
	
	HOTSW_EnableHotSWAsync( FALSE );
	// �l���ω�����܂ŃX���[�v���đ҂B
	while( HOTSW_isEnableHotSW() != FALSE ) {
		OS_Sleep( 2 );
	}
	
	// DataOnly�ȃA�v���̓��[�h���N�������Ȃ�
	if( pBootTitle->titleID & TITLE_ID_DATA_ONLY_FLAG_MASK )
	{
		OS_TPrintf("SYSM_StartLoadTitle failed: This App has Data_Only flag.\n");
		return;
	}
    
	s_loadstart = TRUE;
	// ���̂���CardRom�w�b�_�o�b�t�@��ROM�w�b�_���㏑���œǂݍ��ނ̂�
	// ���̎��_��HotSW���~�܂��Ă��Ȃ��ƁA����ɃJ�[�h��ROM�w�b�_
	// ���㏑�����Ă��܂��\��������

	// �ꉞ�A�A�v�����[�h�J�n�O�Ɍ��،��ʂ�PROCESSING�ɃZ�b�g
	s_authResult = AUTH_RESULT_PROCESSING;
	// �A�v�������[�h��ԂȂ�A���[�h�J�n
	if( !pBootTitle->flags.isAppLoadCompleted ) {
		SYSMi_GetWork()->flags.common.isLoadFinished  = FALSE;
		
		SYSMi_GetWork()->flags.common.isLoadSucceeded = FALSE;
		OS_InitThread();
		OS_CreateThread( &s_thread, (void (*)(void *))SYSMi_LoadTitleThreadFunc, (void*)pBootTitle, stack+STACK_SIZE/sizeof(u64), STACK_SIZE,THREAD_PRIO );
		OS_WakeupThreadDirect( &s_thread );

	}else {
		// �A�v�����[�h�ς�
		SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
		SYSMi_GetWork()->flags.common.isLoadFinished  = TRUE;
	}
	
	if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM ) {
		SYSMi_GetWork()->flags.common.isCardBoot = TRUE;
	}else if(pBootTitle->flags.isAppLoadCompleted)
	{
		// �J�[�h�u�[�g�łȂ��A���[�h�ς݂̏ꍇ�͍��̂Ƃ��뉽�����Ȃ�
	}
}

// �A�v�����[�h�ς݂��ǂ������`�F�b�N
BOOL SYSM_IsLoadTitleFinished( void )
{
	// ���[�h�ς݂̎��́A���TRUE
	if( !SYSMi_GetWork()->flags.common.isLoadFinished ) {
		/*
		if( SYSMi_GetWork()->flags.common.isCardBoot ) {
			// �J�[�h�u�[�g�̎��́AHOTSW���C�u�����̃��[�h�������`�F�b�N�B
			SYSMi_GetWork()->flags.common.isLoadFinished  = SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted;
			SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
		}else {
		*/
		// NAND�u�[�g�̎��́A���[�h�X���b�h�̊������`�F�b�N�B
		SYSMi_GetWork()->flags.common.isLoadFinished = OS_IsThreadTerminated( &s_thread );
	}
	return SYSMi_GetWork()->flags.common.isLoadFinished ? TRUE : FALSE;
}


// ============================================================================
//
//
// �A�v���F��
//
//
// ============================================================================

// �������A�v���i��DS�_�E�����[�h�A�v�������j���ʂ̃w�b�_�F�؏���
static AuthResult SYSMi_AuthenticateHeaderWithSign( TitleProperty *pBootTitle, ROM_Header *head )
{
	// ��������
	const u8 *key;
	u32 hi;
	u8 keynum;
	SignatureData sigbuf;
	SVCSignHeapContext con;
	BOOL b_dev = FALSE;
	char *gamecode = (char *)&(pBootTitle->titleID);
	OSTick start,prev;
	start = OS_GetTick();
	
	// pBootTitle->titleID��ROM�w�b�_��titleID�̈�v�m�F������B
	// [TODO:]�z���C�g���X�g�}�X�^�����O���ꂽNTR�A�v���ōs��Ȃ��ꍇ��SYSMi_AuthenticateTWLHeader�ֈړ�
	if( pBootTitle->titleID != head->s.titleID )
	{
		//TWL�Ή�ROM�ŁA�w�b�_��titleID���N���w�肳�ꂽID�ƈႤ
		OS_TPrintf( "Authenticate_Header failed: header TitleID error\n" );
		OS_TPrintf( "Authenticate_Header failed: selectedTitleID=%.16llx\n", pBootTitle->titleID );
		OS_TPrintf( "Authenticate_Header failed: headerTitleID=%.16llx\n", head->s.titleID );
		return AUTH_RESULT_AUTHENTICATE_FAILED;
	}else
	{
		OS_TPrintf( "Authenticate_Header : header TitleID check succeed.\n" );
	}
	
	prev = OS_GetTick();
	hi = head->s.titleID_Hi;
	// Launcher�͐�p�̌����g��
	if(  0 == STD_CompareNString( &gamecode[1], "ANH", 3 ) )
	{
		keynum = LAUNCHER_KEY_INDEX;
	}else
	{
		// keynum = 1:SystemApp 2:SecureApp 3:UserApp
		keynum = (u8)( (hi & TITLE_ID_HI_SECURE_FLAG_MASK) ? SECURE_APP_KEY_INDEX
						: ( (hi & TITLE_ID_HI_APP_TYPE_MASK) ? SYSTEM_APP_KEY_INDEX : USER_APP_KEY_INDEX )
					);
	}
	// �A�v����ʂƃ{���f�B���O�I�v�V�����ɂ���Ďg�����𕪂���
// #define LNC_PDTKEY_DBG
#ifdef LNC_PDTKEY_DBG
	{
		// ���F�f�o�O�p�R�[�h�B
		// �J���pTS�{�[�h�ŊJ����ROM����ѐ��i��ROM�̏����`�F�b�N��AES�f�N���v�g���f�o�O���邽�߂̃R�[�h
		if( head->s.developer_encrypt )
		{
			// �J���Ō��擾
			key = g_devPubKey[keynum];
		}else
		{
			// ���i�Ō��擾
			key = ((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->rsa_pubkey[keynum];
		}
		// �f�o�b�K���L����TLF�ǂݍ��݂Ȃ�΁A�n�b�V���`�F�b�N�X���[�t���O�𗧂Ă�
		if(SYSMi_GetWork()->flags.hotsw.isOnDebugger && SYSMi_GetWork()->romEmuInfo.isTlfRom )
		{
			b_dev = TRUE;
		}
	}
#else
    if( SCFG_GetBondingOption() == 0 ) {
		// ���i�Ō��擾
		key = ((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->rsa_pubkey[keynum];
    }else {
		// �J����
		key = g_devPubKey[keynum];
		// �f�o�b�K���L����TLF�ǂݍ��݂Ȃ�΁A�n�b�V���`�F�b�N�X���[�t���O�𗧂Ă�
		if(SYSMi_GetWork()->flags.hotsw.isOnDebugger && SYSMi_GetWork()->romEmuInfo.isTlfRom )
		{
			b_dev = TRUE;
		}
    }
#endif
    // ���������ŕ���
    MI_CpuClear8( &sigbuf, sizeof(sigbuf) );
    SVC_InitSignHeap( &con, (void *)SIGN_HEAP_ADDR, SIGN_HEAP_SIZE );// �q�[�v�̏�����
    if( !SVC_DecryptSign( &con, sigbuf.digest, head->signature, key ))
    {
		OS_TPrintf("Authenticate_Header failed: Sign decryption failed.\n");
		if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
	}
	if(s_calc_hash)
	{
	    // �����̃n�b�V���l�ƃw�b�_�̃n�b�V���l���r
	    if(!SVC_CompareSHA1(sigbuf.digest, (const void *)&s_calc_hash[0]))
	    {
			OS_TPrintf("Authenticate_Header failed: Sign check failed.\n");
			if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf("Authenticate_Header : Sign check succeed. %dms.\n", OS_TicksToMilliSeconds(OS_GetTick() - prev));
		}
	}else
	{
		OS_TPrintf("Authenticate_Header failed: Sign check failed.\n");
		if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
	}
	OS_TPrintf("Authenticate_Header : total %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
	
	return AUTH_RESULT_SUCCEEDED;
}

// TWL�A�v���ANTR�g��NAND�A�v�� ���ʂ̃w�b�_�F�؏���
static AuthResult SYSMi_AuthenticateTWLHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
	if( head->s.enable_signature )
	{
		return SYSMi_AuthenticateHeaderWithSign( pBootTitle, head );
	}else
	{
		// �����L���t���O�������Ă��Ȃ����FAILED
		OS_TPrintf("Authenticate_Header failed: Sign check flag is OFF.\n");
		return AUTH_RESULT_AUTHENTICATE_FAILED;
	}
}

// TWL�A�v���ANTR�g��NAND�A�v�� ���ʂ̔F��
static AuthResult SYSMi_AuthenticateTWLTitle( TitleProperty *pBootTitle )
{
	ROM_Header *head;
	OSTick start,prev;
	start = OS_GetTick();
	
	head = ( ROM_Header *)SYSM_APP_ROM_HEADER_BUF;
	
	// NAND�A�v���̏ꍇ�ANAM_CheckTitleLaunchRights()���Ă�Ń`�F�b�N
	if( pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_NAND )
	{
		s32 result = NAM_CheckTitleLaunchRights( pBootTitle->titleID );
		if( NAM_OK != result)
		{
			OS_TPrintf("Authenticate failed: NAM_CheckTitleLaunchRights failed. %d \n",result);
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}else
		{
			OS_TPrintf("Authenticate : NAM_CheckTitleLaunchRights succeed. %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );
		}
	}

	// �n�b�V����r
    {
		int l;
		u32 *module_addr[RELOCATE_INFO_NUM];
		u32 module_size[RELOCATE_INFO_NUM];
		u8 *hash_addr[RELOCATE_INFO_NUM];
		int module_num;
		BOOL b_dev = FALSE;
		char *gamecode = (char *)&(pBootTitle->titleID);
	    
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
			
			if(s_calc_hash)
			{
				// �A�v�������[�h���鎞�Ɍv�Z�����n�b�V��������
			    if(!SVC_CompareSHA1((const void *)hash_addr[l], (const void *)&s_calc_hash[(l+1) * SVC_SHA1_DIGEST_SIZE]))
			    {
					OS_TPrintf("Authenticate failed: %s module hash check failed.\n", str[l]);
					if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
				}else
				{
					OS_TPrintf("Authenticate : %s module hash check succeed. %dms.\n", str[l], OS_TicksToMilliSeconds(OS_GetTick() - prev));
				}
			}else
			{
				OS_TPrintf("Authenticate failed: %s module hash calc failed.\n", str[l]);
				if(!b_dev) return AUTH_RESULT_AUTHENTICATE_FAILED;
			}
		}
	}
	OS_TPrintf("Authenticate : total %d ms.\n", OS_TicksToMilliSeconds(OS_GetTick() - start) );

	// ���ior�J�����@�ł�NAND�A�v����NAND�A�J�[�h�A�v���̓J�[�h����̂݃u�[�g����
	if ( ! SYSM_IsRunOnDebugger() )
	{
		if ( ( (pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_NAND ||
				pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_TEMP)  && !(head->s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK) ) ||
			   (pBootTitle->flags.bootType == LAUNCHER_BOOTTYPE_ROM    &&  (head->s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK) ) )
		{
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
	}

	return AUTH_RESULT_SUCCEEDED;
}

// NTR�œ���NAND�A�v���ipictochat���j�̃w�b�_�F�؏���
static AuthResult SYSMi_AuthenticateNTRNandAppHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
	return SYSMi_AuthenticateTWLHeader( pBootTitle, head );
}

// NTR�œ���NAND�A�v���ipictochat���j�̔F��
static AuthResult SYSMi_AuthenticateNTRNandTitle( TitleProperty *pBootTitle)
{
	return SYSMi_AuthenticateTWLTitle( pBootTitle );
}

// NTR�Ń_�E�����[�h�A�v���iTMP�A�v���j�̃w�b�_�F�؏���
static AuthResult SYSMi_AuthenticateNTRDownloadAppHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
#pragma unused(pBootTitle, head)
	// [TODO:]������static�ɗ���ł���̂ŁA����ȊO�Ƀw�b�_�F�؏����������
	return AUTH_RESULT_SUCCEEDED;
}

// NTR�Ń_�E�����[�h�A�v���iTMP�A�v���j�̔F��
static AuthResult SYSMi_AuthenticateNTRDownloadTitle( TitleProperty *pBootTitle)
{
#pragma unused(pBootTitle)
	ROM_Header *head;
	OSTick start;
	start = OS_GetTick();
	
	head = ( ROM_Header *)SYSM_APP_ROM_HEADER_BUF;

	// ��������
    {
		u8 buf[0x80];
		SVCSignHeapContext con;
		u8 final_hash[SVC_SHA1_DIGEST_SIZE];

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
		
		// ���ꂼ��header,ARM9FLX,ARM7FLX�ɂ��ăn�b�V�����v�Z���āA�����3����ׂ����̂ɑ΂��Ă܂��n�b�V�����Ƃ�
		if(s_calc_hash)
		{
			// �V���A���i���o�[�t��
			*(u32 *)(&(s_calc_hash[SVC_SHA1_DIGEST_SIZE * 3])) = s_authcode.serial_number;
			// �ŏI�n�b�V���v�Z
			SVC_CalcSHA1( final_hash, s_calc_hash, SVC_SHA1_DIGEST_SIZE * 3 + sizeof(u32));
		}else
		{
			OS_TPrintf("Authenticate failed: hash check failed.\n");
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
		
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

// NTR�ŃJ�[�h�A�v���̃w�b�_�F�؏���
static AuthResult SYSMi_AuthenticateNTRCardAppHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
	AuthResult ret;
	if( head->s.enable_nitro_whitelist_signature )
	{
		// �}�X�^�����O�ς�NTR�J�[�h�A�v���̏����`�F�b�N�i����TWL�A�v���Ɠ����j
		ret = SYSMi_AuthenticateHeaderWithSign( pBootTitle, head );
		if( ret == AUTH_RESULT_SUCCEEDED )
		{
#ifdef DHT_TEST
			// [TODO:]ret��߂܂���OK�Ȃ�hash�l��static�ϐ��ɕۑ����Ă����ADHT��phase1��phase2�Ŏg��
#endif
		}
	}else
	{
		// [TODO:]NTR�J�[�h �z���C�g���X�g����
		// �z���C�g���X�g����
		ret = AUTH_RESULT_SUCCEEDED;
		if( ret == AUTH_RESULT_SUCCEEDED )
		{
#ifdef DHT_TEST
			// [TODO:]ret��߂܂���OK�Ȃ�hash�l��static�ϐ��ɕۑ����Ă����ADHT��phase1��phase2�Ŏg��
#endif
		}
	}
	return ret;
}

// NTR�ŃJ�[�h�A�v���̔F��
static AuthResult SYSMi_AuthenticateNTRCardTitle( TitleProperty *pBootTitle)
{
#pragma unused(pBootTitle)
	// [TODO:]DHT�`�F�b�Nphase2�iphase1��static�̓ǂݍ��ݎ��ɕ��s�����j
#ifdef DHT_TEST
	
#endif
	return AUTH_RESULT_SUCCEEDED;
}

// �w�b�_�F��
static AuthResult SYSMi_AuthenticateHeader( TitleProperty *pBootTitle, ROM_Header *head )
{
	ROM_Header_Short *hs = ( ROM_Header_Short *)head;
	// [TODO:]�F�،��ʂ͂ǂ������[�N�ɕۑ����Ă����H
	if( hs->platform_code & PLATFORM_CODE_FLAG_TWL )
	{
		// TWL�A�v��
		// �F�؏���
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate_Header :TWL_NAND start.\n" );
				return SYSMi_AuthenticateTWLHeader( pBootTitle, head );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate_Header :TWL_ROM start.\n" );
				return SYSMi_AuthenticateTWLHeader( pBootTitle, head );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate_Header :TWL_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					return AUTH_RESULT_AUTHENTICATE_FAILED;
				}
				return SYSMi_AuthenticateTWLHeader( pBootTitle, head );
			default:
				return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
	}
	else
	{
		if( hs->platform_code & PLATFORM_CODE_FLAG_NOT_NTR )
		{
			// TWL�ł�NTR�ł��Ȃ��s���ȃA�v��
			OS_TPrintf( "Authenticate_Header failed :NOT NTR NOT TWL.\n" );
			return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
		// NTR�A�v��
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate_Header :NTR_NAND start.\n" );
				return SYSMi_AuthenticateNTRNandAppHeader( pBootTitle, head );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate_Header :NTR_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate_Header failed : TMP flag error.\n");
					return AUTH_RESULT_AUTHENTICATE_FAILED;
				}
				return SYSMi_AuthenticateNTRDownloadAppHeader( pBootTitle, head );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate_Header :NTR_ROM start.\n" );
				return SYSMi_AuthenticateNTRCardAppHeader( pBootTitle, head );
			default:
				return AUTH_RESULT_AUTHENTICATE_FAILED;
		}
	}
}

// �F��
static AuthResult SYSMi_AuthenticateTitleCore( TitleProperty *pBootTitle)
{
	ROM_Header_Short *hs = ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF;
	// [TODO:]�F�،��ʂ͂ǂ������[�N�ɕۑ����Ă����H
	if( hs->platform_code & PLATFORM_CODE_FLAG_TWL )
	{
		// TWL�A�v��
		// �F�؏���
		switch( pBootTitle->flags.bootType )
		{
			case LAUNCHER_BOOTTYPE_NAND:
				OS_TPrintf( "Authenticate :TWL_NAND start.\n" );
				return SYSMi_AuthenticateTWLTitle( pBootTitle );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate :TWL_ROM start.\n" );
				return SYSMi_AuthenticateTWLTitle( pBootTitle );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate :TWL_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					return AUTH_RESULT_AUTHENTICATE_FAILED;
				}
				return SYSMi_AuthenticateTWLTitle( pBootTitle );
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
				return SYSMi_AuthenticateNTRNandTitle( pBootTitle );
			case LAUNCHER_BOOTTYPE_TEMP:
				OS_TPrintf( "Authenticate :NTR_TEMP start.\n" );
				if (!hs->permit_landing_tmp_jump)
				{
					OS_TPrintf("Authenticate failed: TMP flag error.\n");
					return AUTH_RESULT_AUTHENTICATE_FAILED;
				}
				return SYSMi_AuthenticateNTRDownloadTitle( pBootTitle );
			case LAUNCHER_BOOTTYPE_ROM:
				OS_TPrintf( "Authenticate :NTR_ROM start.\n" );
				return SYSMi_AuthenticateNTRCardTitle( pBootTitle );
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
		( ( (( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->platform_code ) == 0 ) &&
		( ( (( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF)->fat_size ) > 0 )
		) {
		s_authResult = AUTH_RESULT_TITLE_BOOTTYPE_ERROR;
		return;
	}
	
	// �F��
	s_authResult = SYSMi_AuthenticateTitleCore( pBootTitle );
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
	
	if(s_calc_hash)
	{
		// �n�b�V���l�ۑ��̈���
		SYSM_Free( s_calc_hash );
		s_calc_hash = NULL;
	}
	
	// ���i�{�̂̂�TWL�ݒ�f�[�^�Ƀu�[�g����^�C�g����TitleID��platformCode��ۑ��B
    if( !SYSM_IsRunOnDebugger() ) {
		u8 *pBuffer = SYSM_Alloc( LCFG_WRITE_TEMP );
		if( pBuffer != NULL ) {
			LCFG_TSD_SetLastTimeBootSoftTitleID ( pBootTitle->titleID );
			LCFG_TSD_SetLastTimeBootSoftPlatform( (u8)SYSM_GetAppRomHeader()->platform_code );
			(void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
			SYSM_Free( pBuffer );
		}
	}
	
	// �}�E���g���̓o�^
	SYSMi_GetWork2()->bootTitleProperty = *pBootTitle;
	SYSMi_SetBootSRLPathToWork2( pBootTitle );
	
	// HW_WM_BOOT_BUF�ւ̃u�[�g���Z�b�g
	( (OSBootInfo *)OS_GetBootInfo() )->boot_type = s_launcherToOSBootType[ pBootTitle->flags.bootType ];
	
	// �^�C�g��ID���X�g�̍쐬
	SYSMi_makeTitleIdList();
	SYSM_FreeNandTitleList();
	
	BOOT_Ready();	// never return.
	
	return AUTH_RESULT_SUCCEEDED;
}

// �^�C�g��ID���X�g�̍쐬
static void SYSMi_makeTitleIdList( void )
{
	// [TODO:]���݃u�[�g�s�^�C�g���ɂ��Ă������悤�ɂ��Ă��邪
	// ����ŗǂ��̂��H
	// �J�[�h�ȊO�͂����Ƒ����^�C�~���O�ō���̂ł́H->������
	OSTitleIDList *list = ( OSTitleIDList * )HW_OS_TITLE_ID_LIST;
	ROM_Header_Short *hs = ( ROM_Header_Short *)SYSM_APP_ROM_HEADER_BUF;
	int l;
	u8 count = 0;
	
	if( s_pTitleIDList == NULL )
	{
		OS_TPrintf("SYSMi_makeTitleIdList failed: SYSM_InitNandTitleList() is not called.\n");
		return;
	}
	
	// �Ƃ肠�����[���N���A
	MI_CpuClear8( (void *)HW_OS_TITLE_ID_LIST, HW_OS_TITLE_ID_LIST_SIZE );

	// ���ꂩ��N������A�v����TWL�A�v���łȂ�
	if( !hs->platform_code )
	{
		return;
	}

	for(l=-1;l<s_listLength;l++) // -1�̓J�[�h�A�v���̓��ʏ����p
	{
		ROM_Header_Short e_hs;
		ROM_Header_Short *pe_hs;
		int m;
		BOOL same_maker_code = TRUE;
		char path[256];
		FSFile file[1];
		BOOL bSuccess;
		s32 readLen;
		char *gamecode;
		if(l==-1)
		{
			// �J�[�h�A�v��
			if(SYSM_IsExistCard())
			{
				pe_hs = (ROM_Header_Short *)SYSM_CARD_ROM_HEADER_BAK;// BAK�̒l���g��
			}else
			{
				continue;
			}
		}else
		{
			if(s_pTitleIDList[l] == NULL)
			{
				continue;
			}
			// rom�w�b�_�ǂݍ���
			NAM_GetTitleBootContentPathFast(path, s_pTitleIDList[l]);
			FS_InitFile( file );
		    bSuccess = FS_OpenFileEx(file, path, FS_FILEMODE_R);
			if( ! bSuccess )
			{
				OS_TPrintf("SYSMi_makeTitleIdList failed: cant open file(%s)\n",path);
			    FS_CloseFile(file);
			    continue;
			}
			bSuccess = FS_SeekFile(file, 0x00000000, FS_SEEK_SET);
			if( ! bSuccess )
			{
				OS_TPrintf("SYSMi_makeTitleIdList failed: cant seek file(0)\n");
			    FS_CloseFile(file);
			    continue;
			}
			readLen = FS_ReadFile(file, &e_hs, (s32)sizeof(e_hs));
			if( readLen != (s32)sizeof(e_hs) )
			{
			OS_TPrintf("SYSMi_makeTitleIdList failed: cant read file(%p, %d, %d, %d)\n", e_hs, 0, sizeof(e_hs), readLen);
			    FS_CloseFile(file);
			    continue;
			}
			FS_CloseFile(file);
			pe_hs = (ROM_Header_Short *)&e_hs;
		}
		
		for(m=0;m<MAKER_CODE_MAX;m++)
		{
			if(hs->maker_code[m] != pe_hs->maker_code[m])
			{
				same_maker_code = FALSE;
			}
		}
		
		// �����`���[�̓��X�g�ɓ���Ȃ�
		gamecode = (char *)&(pe_hs->titleID);
		if(  0 == STD_CompareNString( &gamecode[1], "ANH", 3 ) )
		{
			continue;
		}
		
		// �Z�L���A�A�v���̏ꍇ���A���[�J�[�R�[�h�������ꍇ��
		if( (hs->titleID & TITLE_ID_SECURE_FLAG_MASK) ||
		    ( same_maker_code ) )
		{
			// �Z�L���A�A�v���̃f�[�^�̓}�E���g�����Ȃ�
			if( !(pe_hs->titleID & TITLE_ID_SECURE_FLAG_MASK) )
			{
				// ���X�g�ɒǉ�
				list->TitleID[count] = pe_hs->titleID;
				// sameMakerFlag��ON
				list->sameMakerFlag[count/8] |= (u8)(0x1 << (count%8));
				// Prv,Pub���ꂼ��Z�[�u�f�[�^�����邩���āA���݂���΃t���OON
				if(pe_hs->public_save_data_size != 0)
				{
					list->publicFlag[count/8] |= (u8)(0x1 << (count%8));
				}
				if(pe_hs->private_save_data_size != 0)
				{
					list->privateFlag[count/8] |= (u8)(0x1 << (count%8));
				}
			}
		}
		
		// �W�����v�\�Ȃ��(�ꉞData Only�t���O�����Ă������A�W�����vAPI�ł����鎖)
		if( pe_hs->permit_landing_normal_jump && !( hs->titleID & TITLE_ID_DATA_ONLY_FLAG_MASK ) )
		{
			// ���X�g�ɒǉ����ăW�����v�\�t���OON
			list->TitleID[count] = pe_hs->titleID;
			list->appJumpFlag[count/8] |= (u8)(0x1 << (count%8));
		}
		
		// �����܂ł̂����ɁAlist->TitleID[count]���ҏW����Ă�����count�C���N�������g
		if( list->TitleID[count] != NULL )
		{
			count++;
		}
	}
	list->num = count;
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