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
#include <sysmenu/mcu.h>
#include <firm/format/from_firm.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
extern void LCFG_VerifyAndRecoveryNTRSettings( void );

// function's prototype-------------------------------------------------------
static void SYSMi_CopyLCFGData( void );
static TitleProperty *SYSMi_CheckDebuggerBannerViewModeBoot( void );
static TitleProperty *SYSMi_CheckShortcutBoot1( void );
static TitleProperty *SYSMi_CheckShortcutBoot2( void );
static void SYSMi_CheckCardCloneBoot( void );
void SYSMi_SendKeysToARM7( void );
static OSTitleId SYSMi_getTitleIdOfMachineSettings( void );

// global variable-------------------------------------------------------------
void *(*SYSMi_Alloc)( u32 size  );
void  (*SYSMi_Free )( void *ptr );

#define SYSM_DEBUG_
#ifdef SYSM_DEBUG_
SYSM_work       *pSysm;                                         // �f�o�b�K�ł�SYSM���[�N�̃E�H�b�`�p
ROM_Header_Short *pRomHeader;
#endif
// static variable-------------------------------------------------------------

static TitleProperty s_bootTitleBuf;

// const data------------------------------------------------------------------

// ============================================================================
//
// ������
//
// ============================================================================

// SystemMenu�̏�����
void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
#ifdef SYSM_DEBUG_
    pSysm = SYSMi_GetWork();
    pRomHeader = (ROM_Header_Short *)0x027fc000;
#endif /* SYSM_DEBUG_ */

    // ARM7�Ŏg�p���镪�̌���n��
    SYSMi_SendKeysToARM7();

    // �����`���[�̃}�E���g���Z�b�g
    //SYSMi_SetLauncherMountInfo();

    // ARM7�R���|�[�l���g�p�v���e�N�V�������j�b�g�̈�ύX
    OS_SetProtectionRegion( 2, SYSM_OWN_ARM7_MMEM_ADDR, 512KB );

    SYSM_SetAllocFunc( pAlloc, pFree );

    //  PXI_SetFifoRecvCallback( SYSMENU_PXI_FIFO_TAG, SYSMi_PXIFifoRecvCallback );

    reg_OS_PAUSE |= REG_OS_PAUSE_CHK_MASK;                          // PAUSE���W�X�^�̃`�F�b�N�t���O�̃Z�b�g
}


// �A���[�i�Đݒ�
void SYSM_SetArena( void )
{
    // ARM9�p�u�[�g�R�[�h�z�u�̂��߁A�A���[�iHi�ʒu��������
    OS_SetMainArenaHi( (void *)SYSM_OWN_ARM9_MMEM_ADDR_END );
}


// �V�X�e�����j���[���C�u�����p�������A���P�[�^�̐ݒ�
void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
    SYSMi_Alloc = pAlloc;
    SYSMi_Free  = pFree;
}


// ������Alloc
void *SYSM_Alloc( u32 size )
{
    void *p = SYSMi_Alloc( size );
    OS_TPrintf( "SYSM_Alloc : 0x%08x  0x%xbytes\n", p, size );
    return p;
}


// ������Free
void SYSM_Free( void *ptr )
{
    OS_TPrintf( "SYSM_Free  : 0x%08x\n", ptr );
    SYSMi_Free( ptr );
}


// ARM7�Ŏg�p���镪�̌���n��
void SYSMi_SendKeysToARM7( void )
{
    MI_SetWramBank(MI_WRAM_ARM9_ALL);
    // DS�݊�Blowfish�e�[�u����ARM7�֓n��
    MI_CpuCopyFast( &((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->ds_blowfish, (void *)HW_WRAM_0, sizeof(BLOWFISH_CTX) );
    DC_FlushRange( (void *)HW_WRAM_0, sizeof(BLOWFISH_CTX) );
    MI_SetWramBank(MI_WRAM_ARM7_ALL);
}


// ============================================================================
//
// ���擾
//
// ============================================================================

// �p�����[�^���[�h
TitleProperty *SYSM_ReadParameters( void )
{
    TitleProperty *pBootTitle = NULL;

    //NAM�̏�����
    NAM_Init( SYSM_Alloc, SYSM_Free );

    //-----------------------------------------------------
    // HW���̃��[�h
    //-----------------------------------------------------
    // �m�[�}����񃊁[�h
    if( !LCFG_ReadHWNormalInfo() ) {
        OS_TPrintf( "HW Normal Info Broken!\n" );
        SYSMi_GetWork()->flags.common.isBrokenHWNormalInfo = TRUE;
        SYSM_SetFatalError( TRUE );
    }
    // �Z�L���A��񃊁[�h
    if( !LCFG_ReadHWSecureInfo() ) {
        OS_TPrintf( "HW Secure Info Broken!\n" );
        SYSMi_GetWork()->flags.common.isBrokenHWSecureInfo = TRUE;
        SYSM_SetFatalError( TRUE );
    }

    //-----------------------------------------------------
    // �{�̐ݒ�f�[�^�̃��[�h�i���K��HWSecureInfor���[�h��Ɏ��s���邱�ƁBLanguageBitmap�𔻒�Ɏg�����߁j
    //-----------------------------------------------------
    {
        u8 *pBuffer = SYSM_Alloc( LCFG_READ_TEMP );
        if( pBuffer ) {
			LCFG_ReadTWLSettings( (u8 (*)[LCFG_READ_TEMP])pBuffer );		   // NAND����TWL�{�̐ݒ�f�[�^�����[�h
            SYSM_Free( pBuffer );
        }else {
	        SYSM_SetFatalError( TRUE );
		}
	    LCFG_VerifyAndRecoveryNTRSettings();  		                          	// NTR�ݒ�f�[�^��ǂݏo���āATWL�ݒ�f�[�^�ƃx���t�@�C���A�K�v�Ȃ烊�J�o��
    }
	
    //-----------------------------------------------------
    // �V�X�e���̈�ɖ{�̐ݒ�Ȃǂ��R�s�[
    //-----------------------------------------------------
	// NTR�J�[�h�A�v��ARM9�R�[�h�̃��[�h�̈�ƃ������������������A�擪0x4000�̓Z�L���A�̈�ŕʃo�b�t�@�Ɋi�[�����̂ŁA
	// �����ł����̃p�����[�^�����[�h���Ă����v�B
	SYSMi_CopyLCFGData();
	
    //-----------------------------------------------------
    // ����ON/OFF�t���O�����ƂɁALED��ݒ肷��B
    //-----------------------------------------------------
	{
		BOOL enable;
		if( LCFG_THW_IsForceDisableWireless() ) {
			enable = FALSE;
		}else {
			enable = LCFG_TSD_IsAvailableWireless();
		}
		SYSMi_SetWirelessLED( enable );
	}
	
    //-----------------------------------------------------
    // �e��f�o�C�X�ݒ�
    //-----------------------------------------------------
    // �o�b�N���C�g�P�x�ݒ�
#ifdef SDK_SUPPORT_PMIC_2
    if ( SYSMi_GetMcuVersion() <= 1 )
    {
        // X2�{�[�h�ȑO�����P�x�ݒ肷��
        SYSM_SetBackLightBrightness( LCFG_TWL_BACKLIGHT_LEVEL_MAX );
    }
#endif // SDK_SUPPORT_PMIC_2
	
    // TP�L�����u���[�V����
	SYSM_CaribrateTP();
    // RTC�␳
    SYSMi_WriteAdjustRTC();
    // RTC�l�̃`�F�b�N
    SYSMi_CheckRTC();

    //-----------------------------------------------------
	// ARM7�̏����҂�
    //-----------------------------------------------------
	
    // ARM7�̃����`���[�p�����[�^�擾����������̂�҂�
    while( !SYSMi_GetWork()->flags.common.isARM9Start ) {
        SVC_WaitByLoop( 0x1000 );
    }
//#ifdef DEBUG_USED_CARD_SLOT_B_
    // ARM7�̃J�[�h�`�F�b�N������҂�
    while( !SYSMi_GetWork()->flags.hotsw.is1stCardChecked ) {
        SVC_WaitByLoop( 0x1000 );
    }
//#endif


	//-----------------------------------------------------
    // �����`���[�p�����[�^�̔���
    //-----------------------------------------------------
	if( SYSM_IsHotStart() ) {
		// �z�b�g�X�^�[�g���́A��{���S�f���X�L�b�v
		SYSM_SetLogoDemoSkip( TRUE );
		
		if( LCFG_TSD_GetLastTimeBootSoftPlatform() == PLATFORM_CODE_NTR ) {
		    // �O��u�[�g��NTR�Ȃ�A�����`���[�p�����[�^����
			SYSMi_GetWork()->flags.common.isValidLauncherParam = 0;
			MI_CpuClear32( &SYSMi_GetWork()->launcherParam, sizeof(LauncherParam) );
		}
		
		if( SYSMi_GetWork()->flags.common.isValidLauncherParam ) {
		    // ���S�f���X�L�b�v�����H
			if( !SYSM_GetLauncherParamBody()->v1.flags.isLogoSkip ) {
	            SYSM_SetLogoDemoSkip( FALSE );
	        }
			
	        // �A�v�����ڋN���̎w�肪�������烍�S�f�����΂��Ďw��A�v���N��
			if( SYSM_GetLauncherParamBody()->v1.bootTitleID ) {
	            s_bootTitleBuf.titleID = SYSM_GetLauncherParamBody()->v1.bootTitleID;
	            s_bootTitleBuf.flags = SYSM_GetLauncherParamBody()->v1.flags;
	            s_bootTitleBuf.pBanner = (TWLBannerFile *)(*(TWLBannerFile **)(SYSM_GetLauncherParamBody()->v1.rsv));
	            pBootTitle = &s_bootTitleBuf;
	        }
		}
	}
	
    // �A�v���W�����v�łȂ��Ƃ��ɂ́A�A�v���ԃp�����^���N���A
    // [TODO:]���炩����NTR�J�[�h�̃Z�L���A�̈��ޔ������ɒ���0x2000000���烍�[�h���Ă���ꍇ���e�͂Ȃ������̂Œ���
    if( !pBootTitle )
    {
    	MI_CpuClearFast((void *)HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_SIZE);
	}

    //-----------------------------------------------------
    // IS�f�o�b�K�o�i�[View���[�h�N��
    //-----------------------------------------------------
    if( pBootTitle == NULL ) {
		// �����`���[�p�����[�^�ɂ��_�C���N�g�u�[�g���Ȃ��ꍇ�̂ݔ���
        pBootTitle = SYSMi_CheckDebuggerBannerViewModeBoot();
    }
    
    //-----------------------------------------------------
    // �ʎY�H���p�V���[�g�J�b�g�L�[ or
    // �����J�[�h�N��
    //-----------------------------------------------------
    if( pBootTitle == NULL && // �R�R�܂Ń_�C���N�g�u�[�g���ݒ肳��Ă��Ȃ��ꍇ�̂ݔ���
        !( SYSMi_GetWork()->flags.common.isValidLauncherParam && SYSM_GetLauncherParamBody()->v1.flags.isLogoSkip ) )
        // �u�����`���[�ċN���w��i���ڋN���w�薳�� ���� �����`���p�����^�L�� ���� ���S�X�L�b�v�w��j�łȂ��v
    {
        pBootTitle = SYSMi_CheckShortcutBoot1();
    }
    
    //-----------------------------------------------------
    // ���̑��̃V���[�g�J�b�g�N��
    //-----------------------------------------------------
    if( pBootTitle == NULL ) {
		// �R�R�܂Ń_�C���N�g�u�[�g���ݒ肳��Ă��Ȃ��ꍇ�̂ݔ���
        pBootTitle = SYSMi_CheckShortcutBoot2();
    }

    return pBootTitle;
}


// �{�̐ݒ�f�[�^�Ȃǂ̃������W�J�B
static void SYSMi_CopyLCFGData( void )
{
	// �{�̐ݒ�f�[�^�AHW�m�[�}�����AHW�Z�L���A�����������ɓW�J���Ă���
	MI_CpuCopyFast( LCFGi_GetTSD(), (void *)HW_PARAM_TWL_SETTINGS_DATA, sizeof(LCFGTWLSettingsData) );
	MI_CpuCopyFast( LCFGi_GetHWN(), (void *)HW_PARAM_TWL_HW_NORMAL_INFO, sizeof(LCFGTWLHWNormalInfo) );
	MI_CpuCopyFast( LCFGi_GetHWS(), (void *)HW_HW_SECURE_INFO, HW_HW_SECURE_INFO_END - HW_HW_SECURE_INFO );
	
	// �{�̐ݒ�f�[�^��LauncherStatus�������N���A���Ă���
	{
		LCFGTWLSettingsData *pSettings = (LCFGTWLSettingsData *)HW_PARAM_TWL_SETTINGS_DATA;
		MI_CpuClear32( &pSettings->launcherStatus, sizeof(LCFGTWLLauncherStatus) );
	}
}



BOOL SYSM_IsLauncherHidden( void )
{
#ifdef DO_NOT_SHOW_LAUNCHER
	return TRUE;
#else
	return FALSE;
#endif
}


static TitleProperty *SYSMi_CheckDebuggerBannerViewModeBoot( void )
{
    MI_CpuClear8( &s_bootTitleBuf, sizeof(TitleProperty) );

    //-----------------------------------------------------
    // IS�f�o�b�K�o�i�[View���[�h�N��
    //-----------------------------------------------------
	//[TODO]������
#if 0
	if( SYSMi_IsDebuggerBannerViewMode() ) {
		return NULL;
	}
#endif

	return NULL;
}

// �V���[�g�J�b�g�N���̃`�F�b�N���̂P
static TitleProperty *SYSMi_CheckShortcutBoot1( void )
{
    MI_CpuClear8( &s_bootTitleBuf, sizeof(TitleProperty) );
	
    //-----------------------------------------------------
    // IS�f�o�b�K�N�� or
    // �ʎY�H���p�V���[�g�J�b�g�L�[ or
    // �����J�[�h�N��
    //-----------------------------------------------------
    if( SYSM_IsExistCard() ) {
        if( ( SYSM_IsRunOnDebugger() &&      // IS�f�o�b�K���L������JTAG���܂��L���łȂ���
              !( *(u8 *)( HW_SYS_CONF_BUF + HWi_WSYS09_OFFSET ) & HWi_WSYS09_JTAG_CPUJE_MASK ) ) ||
            SYSM_IsInspectCard() ||
            ( ( PAD_Read() & SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) ==
              SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT )
            ){
            s_bootTitleBuf.flags.isAppRelocate = TRUE;
            s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
            s_bootTitleBuf.flags.isInitialShortcutSkip = TRUE;         // ����N���V�[�P���X���΂�
            s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // ���S�f�����΂�
            s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_ROM;
            s_bootTitleBuf.flags.isValid = TRUE;
            // ROM�w�b�_�o�b�t�@�̃R�s�[
            {
                u16 id = (u16)OS_GetLockID();
                (void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );     // ARM7�Ɣr�����䂷��
                (void)SYSMi_CopyCardRomHeader();
                (void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );   // ARM7�Ɣr�����䂷��
                OS_ReleaseLockID( id );
            }
            s_bootTitleBuf.titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
            SYSM_SetLogoDemoSkip( s_bootTitleBuf.flags.isLogoSkip );
            return &s_bootTitleBuf;
        }
    }

    return NULL;                                                    // �u�u�[�g���e����v�Ń��^�[��
}

// �V���[�g�J�b�g�N���̃`�F�b�N���̂Q
static TitleProperty *SYSMi_CheckShortcutBoot2( void )
{
    MI_CpuClear8( &s_bootTitleBuf, sizeof(TitleProperty) );

    //-----------------------------------------------------
    // �X�^���h�A�����N�����A�V���[�g�J�b�g�L�[(select)
    // �������Ȃ���̋N���Ŗ{�̐ݒ�̒��ڋN��
    //-----------------------------------------------------
    if( ( PAD_Read() & SYSM_PAD_SHORTCUT_MACHINE_SETTINGS ) ==
		SYSM_PAD_SHORTCUT_MACHINE_SETTINGS )
    {
        s_bootTitleBuf.titleID = SYSMi_getTitleIdOfMachineSettings();
        if(s_bootTitleBuf.titleID != 0)
		{
            s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // �{�̐ݒ���N���ł��鎞�������S�f�����΂�
		}
        s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
        s_bootTitleBuf.flags.isValid = TRUE;
        s_bootTitleBuf.flags.isAppRelocate = FALSE;
        s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
        return &s_bootTitleBuf;
    }

	// �X�^���h�A�����N����
    // �����`���[��ʂ�\�����Ȃ��o�[�W�����̏ꍇ
    // �J�[�h���������Ă�����J�[�h���N������
    // �������Ă��Ȃ��ꍇ�͖{�̐ݒ���N��
#ifdef DO_NOT_SHOW_LAUNCHER
	if( SYSM_IsExistCard() )
	{
        s_bootTitleBuf.flags.isAppRelocate = TRUE;
        s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
        s_bootTitleBuf.flags.isInitialShortcutSkip = TRUE;         // ����N���V�[�P���X���΂�
        s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // ���S�f�����΂�
        s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_ROM;
        s_bootTitleBuf.flags.isValid = TRUE;
        // ROM�w�b�_�o�b�t�@�̃R�s�[
        {
            u16 id = (u16)OS_GetLockID();
            (void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );     // ARM7�Ɣr�����䂷��
            (void)SYSMi_CopyCardRomHeader();
            (void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );   // ARM7�Ɣr�����䂷��
            OS_ReleaseLockID( id );
        }
        s_bootTitleBuf.titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
        SYSM_SetLogoDemoSkip( s_bootTitleBuf.flags.isLogoSkip );
        return &s_bootTitleBuf;
	}else
	{
        s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // ���S�f�����΂�
        s_bootTitleBuf.titleID = SYSMi_getTitleIdOfMachineSettings();
        s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
        s_bootTitleBuf.flags.isValid = TRUE;
        s_bootTitleBuf.flags.isAppRelocate = FALSE;
        s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
        return &s_bootTitleBuf;
	}
#endif

    //-----------------------------------------------------
    // TWL�ݒ�f�[�^�����͎��̏���N���V�[�P���X�N��
    //-----------------------------------------------------
#if 0
#ifdef ENABLE_INITIAL_SETTINGS_
    if( !LCFG_TSD_IsFinishedInitialSetting() ) {
        s_bootTitleBuf.titleID = SYSMi_getTitleIdOfMachineSettings();
        if(s_bootTitleBuf.titleID != 0)
		{
            s_bootTitleBuf.flags.isLogoSkip = TRUE;                    // �{�̐ݒ���N���ł��鎞�������S�f�����΂�
		}
        s_bootTitleBuf.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
        s_bootTitleBuf.flags.isValid = TRUE;
        s_bootTitleBuf.flags.isAppRelocate = FALSE;
        s_bootTitleBuf.flags.isAppLoadCompleted = FALSE;
        return &s_bootTitleBuf;
    }
#endif // ENABLE_INITIAL_SETTINGS_
#endif

    return NULL;                                                    // �u�u�[�g���e����v�Ń��^�[��
}


// �N���[���u�[�g����
static void SYSMi_CheckCardCloneBoot( void )
{
#if 0
    u8  *buffp         = (u8 *)&pTempBuffer;
    u32 total_rom_size = SYSM_GetCardRomHeader()->rom_valid_size ? SYSM_GetCardRomHeader()->rom_valid_size : 0x01000000;
    u32 file_offset    = total_rom_size & 0xFFFFFE00;

    DC_FlushRange( buffp, BNR_IMAGE_SIZE );
    CARD_ReadRom( 4, (void *)file_offset, buffp, BNR_IMAGE_SIZE );

    buffp += total_rom_size & 0x000001FF;
    if( *buffp++ == 'a' && *buffp == 'c' ) {
        SYSMi_GetWork()->flags.common.cloneBootMode = CLONE_BOOT_MODE;
    }else {
        SYSMi_GetWork()->flags.common.cloneBootMode = OTHER_BOOT_MODE;
    }
#endif
}

// NAM_Init�����悤�ɂȂ����̂ŁANAM�Ŗ{�̐ݒ��ID�擾
// ����炵�����̂��C���X�g�[������Ă��Ȃ��ꍇ��0�iNULL�j�����^�[��
static OSTitleId SYSMi_getTitleIdOfMachineSettings( void )
{
	OSTitleId ret = NULL;
	int l;
	int getNum;
	int validNum = 0;
	NAMTitleId *pTitleIDList = NULL;
	
	// �C���X�g�[������Ă���^�C�g���̎擾
	getNum = NAM_GetNumTitles();
	pTitleIDList = SYSM_Alloc( sizeof(NAMTitleId) * getNum );
	if( pTitleIDList == NULL ) {
		OS_TPrintf( "%s: alloc error.\n", __FUNCTION__ );
		return 0;
	}
	(void)NAM_GetTitleList( pTitleIDList, (u32)getNum );
	
	// �擾�����^�C�g���ɖ{�̏���ID�����邩�`�F�b�N
	for( l = 0; l < getNum; l++ ) {
		char *code = ((char *)&pTitleIDList[l]) + 1;
		if( 0 == STD_CompareNString( code, "BNH", 3 ) )
		{
			ret = (OSTitleId)pTitleIDList[l];
			break;
		}
	}
	SYSM_Free( pTitleIDList );

	return ret;
}

//======================================================================
//  �f�o�b�O
//======================================================================
