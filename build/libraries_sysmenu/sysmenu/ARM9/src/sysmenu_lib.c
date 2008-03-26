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
static TitleProperty *SYSMi_CheckShortcutBoot( void );
static void SYSMi_CheckCardCloneBoot( void );
void SYSMi_SendKeysToARM7( void );

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
    OS_TPrintf( "SYSM_Alloc : %08x  %xbytes\n", p, size );
    return p;
}


// ������Free
void SYSM_Free( void *ptr )
{
    OS_TPrintf( "SYSM_Free  : %08x\n", ptr );
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
		
		// [TODO]�܂��A�v���u�[�g����PlatformCode��ۑ����Ă��Ȃ��̂ŁA�R�����g�A�E�g
#if 0
		if( LCFG_TSD_GetLastTimeBootSoftPlatform() == PLATFORM_CODE_NTR ) {
		    // �O��u�[�g��NTR�Ȃ�A�����`���[�p�����[�^����
			SYSMi_GetWork()->flags.common.isValidLauncherParam = 0;
			MI_CpuClear32( &SYSMi_GetWork()->launcherParam, sizeof(LauncherParam) );
		}
#endif
		
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

    //-----------------------------------------------------
    // �ʎY�H���p�V���[�g�J�b�g�L�[ or
    // �����J�[�h�N��
    //-----------------------------------------------------
    if( pBootTitle == NULL ) {
		// �����`���[�p�����[�^�ɂ��_�C���N�g�u�[�g���Ȃ��ꍇ�̂ݔ���
        pBootTitle = SYSMi_CheckShortcutBoot();
    }

    return pBootTitle;
}

BOOL SYSM_IsLauncherHidden( void )
{
#ifdef DO_NOT_SHOW_LAUNCHER
	return TRUE;
#else
	return FALSE;
#endif
}

// �V���[�g�J�b�g�N���̃`�F�b�N
static TitleProperty *SYSMi_CheckShortcutBoot( void )
{
    static TitleProperty s_bootTitle;

    MI_CpuClear8( &s_bootTitle, sizeof(TitleProperty) );

    //-----------------------------------------------------
    // IS�f�o�b�K�o�i�[View���[�h�N��
    //-----------------------------------------------------
	//[TODO]������
#if 0
	if( SYSMi_IsDebuggerBannerViewMode() ) {
		return NULL;
	}
#endif
	
    //-----------------------------------------------------
    // IS�f�o�b�K�N�� or
    // �ʎY�H���p�V���[�g�J�b�g�L�[ or
    // �����J�[�h�N��
    //-----------------------------------------------------
    if( SYSM_IsExistCard() && !SYSM_GetLauncherParamBody()->v1.flags.isLogoSkip ) { 
    	// �u�J�[�h���݁v���u�����`���[�ċN���w��i�����S�X�L�b�v���^�C�g�����ڋN���w�薳���j�łȂ��v
        if( ( SYSMi_GetWork()->flags.hotsw.isOnDebugger &&      // IS�f�o�b�K���L������JTAG���܂��L���łȂ���
              !( *(u8 *)( HW_SYS_CONF_BUF + HWi_WSYS09_OFFSET ) & HWi_WSYS09_JTAG_CPUJE_MASK ) ) ||
            SYSM_IsInspectCard() ||
            ( ( PAD_Read() & SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) ==
              SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT )
            ){
            s_bootTitle.flags.isAppRelocate = TRUE;
            s_bootTitle.flags.isAppLoadCompleted = TRUE;
            s_bootTitle.flags.isInitialShortcutSkip = TRUE;         // ����N���V�[�P���X���΂�
            s_bootTitle.flags.isLogoSkip = TRUE;                    // ���S�f�����΂�
            s_bootTitle.flags.bootType = LAUNCHER_BOOTTYPE_ROM;
            s_bootTitle.flags.isValid = TRUE;
            // ROM�w�b�_�o�b�t�@�̃R�s�[
            {
                u16 id = (u16)OS_GetLockID();
                (void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );     // ARM7�Ɣr�����䂷��
                (void)SYSMi_CopyCardRomHeader();
                (void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );   // ARM7�Ɣr�����䂷��
                OS_ReleaseLockID( id );
            }
            s_bootTitle.titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
            SYSM_SetLogoDemoSkip( s_bootTitle.flags.isLogoSkip );
            return &s_bootTitle;
        }
    }

    //-----------------------------------------------------
    // �X�^���h�A�����N�����A�V���[�g�J�b�g�L�[(select)
    // �������Ȃ���̋N���Ŗ{�̐ݒ�̒��ڋN��
    //-----------------------------------------------------
    if( ( PAD_Read() & SYSM_PAD_SHORTCUT_MACHINE_SETTINGS ) ==
		SYSM_PAD_SHORTCUT_MACHINE_SETTINGS )
    {
        s_bootTitle.flags.isLogoSkip = TRUE;                    // ���S�f�����΂�
        s_bootTitle.titleID = TITLE_ID_MACHINE_SETTINGS;
        s_bootTitle.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
        s_bootTitle.flags.isValid = TRUE;
        s_bootTitle.flags.isAppRelocate = FALSE;
        s_bootTitle.flags.isAppLoadCompleted = FALSE;
        return &s_bootTitle;
    }

	// �X�^���h�A�����N����
    // �����`���[��ʂ�\�����Ȃ��o�[�W�����̏ꍇ
    // �J�[�h���������Ă�����J�[�h���N������
    // �������Ă��Ȃ��ꍇ�͖{�̐ݒ���N��
#ifdef DO_NOT_SHOW_LAUNCHER
	if( SYSM_IsExistCard() )
	{
        s_bootTitle.flags.isAppRelocate = TRUE;
        s_bootTitle.flags.isAppLoadCompleted = TRUE;
        s_bootTitle.flags.isInitialShortcutSkip = TRUE;         // ����N���V�[�P���X���΂�
        s_bootTitle.flags.isLogoSkip = TRUE;                    // ���S�f�����΂�
        s_bootTitle.flags.bootType = LAUNCHER_BOOTTYPE_ROM;
        s_bootTitle.flags.isValid = TRUE;
        // ROM�w�b�_�o�b�t�@�̃R�s�[
        {
            u16 id = (u16)OS_GetLockID();
            (void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );     // ARM7�Ɣr�����䂷��
            (void)SYSMi_CopyCardRomHeader();
            (void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );   // ARM7�Ɣr�����䂷��
            OS_ReleaseLockID( id );
        }
        s_bootTitle.titleID = *(u64 *)( &SYSM_GetCardRomHeader()->titleID_Lo );
        SYSM_SetLogoDemoSkip( s_bootTitle.flags.isLogoSkip );
        return &s_bootTitle;
	}else
	{
        s_bootTitle.flags.isLogoSkip = TRUE;                    // ���S�f�����΂�
        s_bootTitle.titleID = TITLE_ID_MACHINE_SETTINGS;
        s_bootTitle.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
        s_bootTitle.flags.isValid = TRUE;
        s_bootTitle.flags.isAppRelocate = FALSE;
        s_bootTitle.flags.isAppLoadCompleted = FALSE;
        return &s_bootTitle;
	}
#endif

    //-----------------------------------------------------
    // TWL�ݒ�f�[�^�����͎��̏���N���V�[�P���X�N��
    //-----------------------------------------------------
#if 0
#ifdef ENABLE_INITIAL_SETTINGS_
    if( !LCFG_TSD_IsFinishedInitialSetting() ) {
        s_bootTitle.flags.isLogoSkip = TRUE;                    // ���S�f�����΂�
        s_bootTitle.titleID = TITLE_ID_MACHINE_SETTINGS;
        s_bootTitle.flags.bootType = LAUNCHER_BOOTTYPE_NAND;
        s_bootTitle.flags.isValid = TRUE;
        s_bootTitle.flags.isAppRelocate = FALSE;
        s_bootTitle.flags.isAppLoadCompleted = FALSE;
        return &s_bootTitle;
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


//======================================================================
//  �f�o�b�O
//======================================================================
