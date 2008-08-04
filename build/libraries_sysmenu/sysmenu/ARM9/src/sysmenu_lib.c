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
#include <sysmenu/namut.h>
#include <firm/format/from_firm.h>
#include <firm/hw/ARM9/mmap_firm.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
extern void LCFG_VerifyAndRecoveryNTRSettings( void );

// function's prototype-------------------------------------------------------
static void SYSMi_CopyLCFGDataHWInfo( u32 dst_addr );
static void SYSMi_CopyLCFGDataSettings( void );
static TitleProperty *SYSMi_CheckDebuggerBannerViewModeBoot( void );
static TitleProperty *SYSMi_CheckShortcutBoot1( void );
static TitleProperty *SYSMi_CheckShortcutBoot2( void );
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
static u8 s_lcfgBuffer[ HW_PARAM_TWL_SETTINGS_DATA_SIZE								// 0x01fc
					  + HW_PARAM_WIRELESS_FIRMWARE_DATA_SIZE						// 0x0004
					  + HW_PARAM_TWL_HW_NORMAL_INFO_SIZE ] ATTRIBUTE_ALIGN(32);		// 0x1000

static TitleProperty s_bootTitleBuf;

// const data------------------------------------------------------------------

// ============================================================================
//
// ������
//
// ============================================================================

#if 1
#include    <twl/code32.h>
void _start_AutoloadDoneCallback(void* argv[]);
// AutoloadDoneCallback�́AARM�łȂ��Ɠ��삵�Ȃ��BIS�f�o�b�K���u���[�N�|�C���g�����̂��߂ɏ��肵�Ă��邪�A��������̃��^�[�����������B
// AutoloadDoneCallback�𗘗p���Č��������n��
void _start_AutoloadDoneCallback(void* argv[])
{
#pragma unused(argv)
    // ARM7�Ŏg�p���镪�̌���n��
    SYSMi_SendKeysToARM7();
}
#include    <twl/codereset.h>
#endif


// SystemMenu�̏�����
void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) )
{
#ifdef SYSM_DEBUG_
    pSysm = SYSMi_GetWork();
    pRomHeader = (ROM_Header_Short *)0x027fc000;
#endif /* SYSM_DEBUG_ */

    // ARM7�Ŏg�p���镪�̌���n��
    //SYSMi_SendKeysToARM7();

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
    MI_CpuCopyFast( &((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->ds_blowfish, (void *)&GetDeliverBROM9KeyAddr()->ds_blowfish, sizeof(BLOWFISH_CTX) );
    // AES��0��ARM7�֓n��
//    MI_CpuCopyFast( &((OSFromFirm9Buf *)HW_FIRM_FROM_FIRM_BUF)->aes_key[ 0 ], (void *)&GetDeliverBROM9KeyAddr()->aes_key[ 0 ], AES_KEY_SIZE );
    DC_FlushRange( (void *)HW_WRAM_0, sizeof(DeliverBROM9Key) );
    MI_SetWramBank(MI_WRAM_ARM7_ALL);

#ifdef INITIAL_KEYTABLE_PRELOAD
    SYSMi_GetWork()->flags.hotsw.isKeyTableLoadReady = TRUE;
#endif
}


// nand��tmp�f�B���N�g���̒��g������
void SYSM_DeleteTmpDirectory( TitleProperty *pBootTitle )
{
    // bootType��LAUNCHER_BOOTTYPE_TEMP�łȂ��ꍇ�Atmp�t�H���_���̃f�[�^������
	if( !pBootTitle || pBootTitle->flags.bootType != LAUNCHER_BOOTTYPE_TEMP ) {
		if( NAMUT_DeleteNandDirectory( "nand:/tmp" ) ) {
	        OS_TPrintf( "\"nand:/tmp\" delete succeeded.\n" );
		}else {
	        OS_TPrintf( "\"nand:/tmp\" delete failed.\n" );
		}
	}
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
	
    //-----------------------------------------------------
    // FATAL�G���[�`�F�b�N
    //-----------------------------------------------------
	if( SYSMi_GetWork()->flags.common.isNANDFatalError ) {
		UTL_SetFatalError( FATAL_ERROR_NAND );
	}

    //-----------------------------------------------------
    // HW���̃��[�h
    //-----------------------------------------------------
    // �m�[�}����񃊁[�h
    if( !LCFG_ReadHWNormalInfo() ) {
#ifndef SYSM_IGNORE_RESULT_HWINFO
        OS_TPrintf( "HW Normal Info Broken!\n" );
        UTL_SetFatalError( FATAL_ERROR_HWINFO_NORMAL );
#endif // SYSM_IGNORE_RESULT_HWINFO
    }
    // �Z�L���A��񃊁[�h
    if( !LCFG_ReadHWSecureInfo() ) {
#ifndef SYSM_IGNORE_RESULT_HWINFO
        OS_TPrintf( "HW Secure Info Broken!\n" );
        UTL_SetFatalError( FATAL_ERROR_HWINFO_SECURE );
#endif // SYSM_IGNORE_RESULT_HWINFO
    }

	//-----------------------------------------------------
    // �V�X�e���̈��HWInfo���R�s�[
    //-----------------------------------------------------
	// NTR�J�[�h�A�v��ARM9�R�[�h�̃��[�h�̈�ƃ������������������A�擪0x4000�̓Z�L���A�̈�ŕʃo�b�t�@�Ɋi�[�����̂ŁA
	// �����ł����̃p�����[�^�����[�h���Ă����v�B
	SYSMi_CopyLCFGDataHWInfo( (u32)s_lcfgBuffer );
	
	//-----------------------------------------------------
    // �{�̐ݒ�f�[�^�̃��[�h�i���K��HWSecureInfor���[�h��Ɏ��s���邱�ƁBLanguageBitmap�𔻒�Ɏg�����߁j
    //-----------------------------------------------------
    {
        u8 *pBuffer = SYSM_Alloc( LCFG_READ_TEMP );
        if( pBuffer ) {
			// NAND����TWL�{�̐ݒ�f�[�^�����[�h
			BOOL isRead = LCFG_ReadTWLSettings( (u8 (*)[LCFG_READ_TEMP])pBuffer );
			
			// ���[�h���s�t�@�C�������݂���ꍇ�́A�t�@�C�������J�o��
			if( LCFG_RecoveryTWLSettings() ) {
				if( isRead ) {
					// �~���[�f�[�^�̂����A��������[�h�ł��Ă����Ȃ牽�����Ȃ��B
				}else {
					// ���[�h�Ɋ��S�Ɏ��s���Ă����ꍇ�́A�t���b�V�����V�[�P���X�ցB
					LCFG_TSD_SetFlagFinishedBrokenTWLSettings( FALSE );
					(void)LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );	// LCFG_READ_TEMP > LCFG_WRITE_TEMP �Ȃ̂ŁApBuffer�����̂܂ܗ��p
				}
			}else {
				// ���J�o�����s���́AFALTAL�G���[
		        UTL_SetFatalError( FATAL_ERROR_TWLSETTINGS );
			}
            SYSM_Free( pBuffer );
        }else {
			// �������m�ۂ��ł��Ȃ��������́AFATAL�G���[
	        UTL_SetFatalError( FATAL_ERROR_TWLSETTINGS );
		}
	    LCFG_VerifyAndRecoveryNTRSettings();  		                          	// NTR�ݒ�f�[�^��ǂݏo���āATWL�ݒ�f�[�^�ƃx���t�@�C���A�K�v�Ȃ烊�J�o��
    }
	
	//-----------------------------------------------------
    // �V�X�e���̈�ɖ{�̐ݒ���R�s�[
    //-----------------------------------------------------
	// NTR�J�[�h�A�v��ARM9�R�[�h�̃��[�h�̈�ƃ������������������A�擪0x4000�̓Z�L���A�̈�ŕʃo�b�t�@�Ɋi�[�����̂ŁA
	// �����ł����̃p�����[�^�����[�h���Ă����v�B
	SYSMi_CopyLCFGDataSettings();
	
    //-----------------------------------------------------
    // ����ON/OFF�t���O�����ƂɁALED��ݒ肷��B
    //-----------------------------------------------------
	{
		PMWirelessLEDStatus enable;
		if( LCFG_THW_IsForceDisableWireless() ) {
			enable = PM_WIRELESS_LED_OFF;
		}else {
			enable = LCFG_TSD_IsAvailableWireless() ? PM_WIRELESS_LED_ON : PM_WIRELESS_LED_OFF;
		}
		PMi_SetWirelessLED( enable );
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
		
		if( !SYSM_IsRunOnDebugger() && LCFG_TSD_GetLastTimeBootSoftPlatform() == PLATFORM_CODE_NTR ) {
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
    // �����炩����NTR�J�[�h�̃Z�L���A�̈��ޔ������ɒ���0x2000000���烍�[�h���Ă���ꍇ���e�͂Ȃ������̂Œ���
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
    if( pBootTitle == NULL ) {
		// �R�R�܂Ń_�C���N�g�u�[�g���ݒ肳��Ă��Ȃ��ꍇ�̂ݔ���
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


// HWInfo�̃������W�J�B
static void SYSMi_CopyLCFGDataHWInfo( u32 dst_addr )
{
	// HotStart���ɂ��ێ�����K�v�̂���f�[�^�������`���[�p�Ɉړ�����v�����[�h�p�����[�^�o�b�t�@�ɃR�s�[�B
	MI_CpuCopy8( (void *)HW_PARAM_WIRELESS_FIRMWARE_DATA, (void *)(dst_addr + HW_PARAM_TWL_SETTINGS_DATA_SIZE),
                 HW_PARAM_WIRELESS_FIRMWARE_DATA_SIZE );	// �����t�@�[���p
	
	// �v�����[�h�p�����[�^�A�h���X�������`���[�����ɕύX�B
	*(u32 *)HW_PRELOAD_PARAMETER_ADDR = dst_addr;
	
	// HW�m�[�}�����AHW�Z�L���A�����������ɓW�J���Ă���
	MI_CpuCopyFast( LCFGi_GetHWN(), (void *)HW_PARAM_TWL_HW_NORMAL_INFO, sizeof(LCFGTWLHWNormalInfo) );
	MI_CpuCopyFast( LCFGi_GetHWS(), (void *)HW_HW_SECURE_INFO, HW_HW_SECURE_INFO_END - HW_HW_SECURE_INFO );
}


// �{�̐ݒ�f�[�^�̃������W�J�B
static void SYSMi_CopyLCFGDataSettings( void )
{
	// �{�̐ݒ�f�[�^
	MI_CpuCopyFast( LCFGi_GetTSD(), (void *)HW_PARAM_TWL_SETTINGS_DATA, sizeof(LCFGTWLSettingsData) );
	
	// �{�̐ݒ�f�[�^��LauncherStatus�������N���A���Ă���
	{
		LCFGTWLSettingsData *pSettings = (LCFGTWLSettingsData *)HW_PARAM_TWL_SETTINGS_DATA;
		MI_CpuClear32( &pSettings->launcherStatus, sizeof(LCFGTWLLauncherStatus) );
	}
	
	// NTR�{�̐ݒ�f�[�^���������ɓW�J���Ă���
	{
		LCFG_NSD_SetLanguage( LCFG_NSD_GetLanguageEx() );
		MI_CpuCopy8( LCFGi_GetNSD(), OS_GetSystemWork()->nvramUserInfo, sizeof(LCFGNTRSettingsData) );
	}
}



BOOL SYSM_IsLauncherHidden( void )
{
#ifdef SYSM_DO_NOT_SHOW_LAUNCHER
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
            ( ( PAD_Read() == SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) && 
              ( !LCFG_TSD_IsFinishedBrokenTWLSettings() || !LCFG_TSD_IsFinishedInitialSetting() || !LCFG_TSD_IsFinishedInitialSetting_Launcher() ) )
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
	BOOL isSetArgument = FALSE;
	BOOL isBootMSET = FALSE;
	u16 argument = 0;
	
    MI_CpuClear8( &s_bootTitleBuf, sizeof(TitleProperty) );

#ifndef SYSM_DISABLE_INITIAL_SETTINGS
    //-----------------------------------------------------
    // TWL�ݒ�f�[�^�j�����̃t���b�V�����V�[�P���X�N��
    //-----------------------------------------------------
	if( !LCFG_TSD_IsFinishedBrokenTWLSettings() ) {
		argument      = 100;		// �t���b�V�����V�[�P���X�N��
		isSetArgument = TRUE;
		isBootMSET    = TRUE;
    }else 
#endif
    //-----------------------------------------------------
    // L+R+Start�{�^�������N���ŁA�{�̐ݒ�̃^�b�`�p�l���ݒ���N��
    //-----------------------------------------------------
    if( ( PAD_Read() & SYSM_PAD_SHORTCUT_TP_CALIBRATION ) ==
		SYSM_PAD_SHORTCUT_TP_CALIBRATION ) {
		argument      = 101;
		isSetArgument = TRUE;
		isBootMSET    = TRUE;
    }
#ifndef SYSM_DISABLE_INITIAL_SETTINGS
    //-----------------------------------------------------
    // TWL�ݒ�f�[�^���ݒ莞�̏���N���V�[�P���X�N��
    //-----------------------------------------------------
    else if( !LCFG_TSD_IsFinishedInitialSetting() ) {
		argument      = 0;
		isSetArgument = FALSE;
		isBootMSET    = TRUE;
    }
#endif
	
    //-----------------------------------------------------
    // �����`���[��ʂ�\�����Ȃ��o�[�W�����̏ꍇ
    // �J�[�h���������Ă�����J�[�h���N������
    // �������Ă��Ȃ��ꍇ�͖{�̐ݒ���N��
    //-----------------------------------------------------
#ifdef SYSM_DO_NOT_SHOW_LAUNCHER
	else if( SYSM_IsExistCard() )
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
		argument      = 0;
		isSetArgument = FALSE;
		isBootMSET    = TRUE;
	}
#endif

	// �u�A�v���ԃp�����[�^�Z�b�g�v�L�����́A�p�����[�^���Z�b�g
	if( isSetArgument ) {
        OSDeliverArgInfo argInfo;
        int result;
		
        OS_InitDeliverArgInfo(&argInfo, 0);
        OS_DecodeDeliverArg();
        OSi_SetDeliverArgState( OS_DELIVER_ARG_BUF_ACCESSIBLE | OS_DELIVER_ARG_BUF_WRITABLE );
        result = OS_SetSysParamToDeliverArg( (u16)argument );
        
        if(result != OS_DELIVER_ARG_SUCCESS )
        {
            OS_Warning("Failed to Set DeliverArgument.");
            return FALSE;
        }
        OS_EncodeDeliverArg();
    }
	
	// �u�{�̐ݒ�u�[�g�v�L�����́A�{�̐ݒ�v�[�g����
	if( isBootMSET ) {
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
	
    return NULL;                                                    // �u�u�[�g���e����v�Ń��^�[��
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
	char machine_setting_code[4];
	ROM_Header_Short *header = ( ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
	
	// �C���X�g�[������Ă���^�C�g���̎擾
	getNum = NAM_GetNumTitles();
	pTitleIDList = SYSM_Alloc( sizeof(NAMTitleId) * getNum );
	if( pTitleIDList == NULL ) {
		OS_TPrintf( "%s: alloc error.\n", __FUNCTION__ );
		return 0;
	}
	(void)NAM_GetTitleList( pTitleIDList, (u32)getNum );
	
	// �{�̏���TitleID��4�o�C�g�ڂ̓����`���[��TitleID��4�o�C�g�ڂƓ���
	STD_TSNPrintf( machine_setting_code, 4, "BN%c", header->titleID_Lo[3]);
	
	// �擾�����^�C�g���ɖ{�̏���ID�����邩�`�F�b�N
	for( l = 0; l < getNum; l++ ) {
		char *code = ((char *)&pTitleIDList[l]) + 1;
		if( 0 == STD_CompareNString( code, machine_setting_code, 3 ) )
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
