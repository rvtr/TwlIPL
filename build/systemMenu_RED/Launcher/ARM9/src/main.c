/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     main.c

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
#include <twl/dsp.h>
#include <twl/dsp/common/g711.h>
#include <twl/camera.h>
#include <sysmenu/errorLog.h>
#include <nitro/crypto.h>
#include "launcher.h"
#include "misc.h"
#include "logoDemo.h"
#include "sound.h"
#include "loadWlanFirm.h"
#include "loadSharedFont.h"
#include "scanWDS.h"

#include <nitro/fs/sysarea.h>
#include <twl/na.h>
#include "getSysMenuVersion.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------
#define INIT_DEVICES_LIKE_UIG_LAUNCHER

// �f�o�b�O�p���Ԍv���X�C�b�`
#define MEASURE_TIME				1
#if ( MEASURE_TIME == 1 )
#define MEASURE_START(tgt)			( tgt = OS_GetTick() )
#define MEASURE_RESULT(tgt,str)		OS_TPrintf( str, OS_TicksToMilliSeconds( OS_GetTick() - tgt ) )
#else
#define MEASURE_START(tgt)			((void) 0)
#define MEASURE_RESULT(tgt,str)		((void) 0)
#endif

// function's prototype-------------------------------------------------------
static void INTR_VBlank( void );
static void deleteTmp();
void SYSM_DeleteTempDirectory( TitleProperty *pBootTitle );
static BOOL IsCommandSelected(void);
static void PrintPause(void);
static void PrintError(void);
static void PrintSystemMenuVersion( void );
static void CreateDummyWrapFile( void );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static TitleProperty *sp_titleList;

static u64 s_strmThreadStack[THREAD_STACK_SIZE / sizeof(u64)];
static OSThread s_strmThread;
static StreamInfo s_strm; // stream info

// const data------------------------------------------------------------------

const char filename[] = "data/fanfare.32.wav";

static const char *fatal_error_msg[FATAL_ERROR_MAX] = 
{
	"UNDEFINED",
	"NAND",
	"HWINFO_NORMAL",
	"HWINFO_SECURE",
	"TWLSETTINGS",
	"SHARED_FONT",
	"WLANFIRM_AUTH",
	"WLANFIRM_LOAD",
	"TITLE_LOAD_FAILED",
	"TITLE_POINTER_ERROR",
	"AUTHENTICATE_FAILED",
	"ENTRY_ADDRESS_ERROR",
	"TITLE_BOOTTYPE_ERROR",
	"SIGN_DECRYPTION_FAILED",
	"SIGN_COMPARE_FAILED",
	"HEADER_HASH_CALC_FAILED",
	"TITLEID_COMPARE_FAILED",
	"VALID_SIGN_FLAG_OFF",
	"CHECK_TITLE_LAUNCH_RIGHTS_FAILED",
	"MODULE_HASH_CHECK_FAILED",
	"MODULE_HASH_CALC_FAILED",
	"MEDIA_CHECK_FAILED",
	"DL_MAGICCODE_CHECK_FAILED",
	"DL_SIGN_DECRYPTION_FAILED",
	"DL_HASH_CALC_FAILED",
	"DL_SIGN_COMPARE_FAILED",
	"WHITELIST_INITDB_FAILED",
	"WHITELIST_NOTFOUND",
	"DHT_PHASE1_FAILED",
	"DHT_PHASE2_FAILED",
	"LANDING_TMP_JUMP_FLAG_OFF",
	"TWL_BOOTTYPE_UNKNOWN",
	"NTR_BOOTTYPE_UNKNOWN",
	"PLATFORM_UNKNOWN",
	"LOAD_UNFINISHED",
	"LOAD_OPENFILE_FAILED",
	"LOAD_MEMALLOC_FAILED",
	"LOAD_SEEKFILE_FAILED",
	"LOAD_READHEADER_FAILED",
	"LOAD_LOGOCRC_ERROR",
	"LOAD_READDLSIGN_FAILED",
	"LOAD_RELOCATEINFO_FAILED",
	"LOAD_READMODULE_FAILED",
    "NINTENDO_LOGO_CHECK_FAILED"
};

//#define DEBUG_LAUNCHER_DUMP
#ifdef DEBUG_LAUNCHER_DUMP
// �f�o�O�p�BSD��0x02ffc000����0x02ffe000�܂�dump.dat�Ƃ����_���v��f��
static void debugWriteToSD( void )
{
    FSFile dest;
    FS_InitFile( &dest );
    (void)FS_CreateFile("sdmc:/dump.dat", FS_PERMIT_W | FS_PERMIT_R);
    if ( !FS_OpenFileEx( &dest, "sdmc:/dump.dat", FS_FILEMODE_W ) ) return;
    FS_WriteFile( &dest, (void *)0x02ffc000, 0x2000 );
    if ( !FS_CloseFile( &dest ) ) return;
    OS_TPrintf( "debugWriteToSD:ok\n");
}
#endif

#ifdef INIT_DEVICES_LIKE_UIG_LAUNCHER

static int CreateDspSlotBitmap(int slot_num)
{
    int i, bitmap;
    bitmap = 0;
    
    for (i=0; i<slot_num; i++)
    {
        bitmap += (1 << i);
    }
    
    return bitmap;
}

#endif // INIT_DEVICES_LIKE_UIG_LAUNCHER

// ���C��
void TwlMain( void )
{
    enum {
        LOGODEMO_INIT = 0,
        LOGODEMO = 1,
        LAUNCHER_INIT = 2,
        LAUNCHER = 3,
        LOAD_START = 4,
        LOADING = 5,
        LOAD_PAUSE = 6,
        AUTHENTICATE = 7,
        BOOT = 8,
        STOP = 9
    };
    u32 state = LOGODEMO_INIT;
    TitleProperty *pBootTitle = NULL;
#if ( MEASURE_TIME == 1 )
    OSTick allstart;
#endif
    OSTick start, end = 0;
    BOOL direct_boot = FALSE;
	BOOL isStartScanWDS = FALSE;
	
#ifdef DEBUG_LAUNCHER_DUMP
    // you should comment out to clear GX/G2/DMA/TM/PAD register in reboot.c to retreive valid boot time
    STD_TSPrintf((char*)0x02FFCFC0, "\nLauncher Boot Time: %lld usec\n", OS_TicksToMicroSeconds(reg_OS_TM3CNT_L * (1024/64)));
#endif
	
    // �V�X�e�����j���[������----------
    SYSM_Init( Alloc, Free );                       // OS_Init�̑O�ŃR�[������K�v����B
    OS_Init();
    SYSM_SetArena();                                // OS_Init�̌�ŃR�[������K�v����B
	
    // CRYPTO���C�u����������---------- 2008.07.24 ES���C�u������CRYPTO���g�p����悤�ɂȂ����̂ŁA���̏������K�v�B
	CRYPTO_SetAllocator( Alloc, Free );

	// ColdStart���́A���S�f�����I���܂ł́AHW���Z�b�g�{�^���ɂ��HotBoot�t���O�Z�b�g��}������B
	// �i�u���N�ƈ��S�v��ʂ�K���\�����邽�߁j
	if( !SYSM_IsHotStart() ) {
		OSi_SetEnableHotBoot( FALSE );
	}
	
    // OS������------------------------
    OS_InitTick();
    
	// start ���Ԍv��total
    MEASURE_START(allstart);
    
    // start���Ԍv���P
    MEASURE_START(start);
    
    PM_Init();

    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    SYSM_InitPXI();                                 // ���荞�݋���ɃR�[������K�v����B

    FS_Init( FS_DMA_NOT_USE );

#ifdef DEBUG_LAUNCHER_DUMP
    // debug
    debugWriteToSD();
#endif

    GX_Init();
    PM_Init();
    TP_Init();
    RTC_Init();
    SND_Init();// sound init
#ifdef USE_WRAM_LOAD
	HOTSW_Init();
#endif
    
     //NAM�̏�����
    NAM_Init( Alloc, Free );
    
    OS_TPrintf( "SYSM_work size = 0x%x\n", sizeof(SYSM_work) );

    // ���荞�݋���--------------------
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    
    // �V�X�e���̏�����----------------
    InitAllocator();                                            // ��SYSM_Init�ȊO��SYSM���C�u�����֐����ĂԑO��

    ERRORLOG_Init( Alloc, Free );
    
    // end���Ԍv���P
	MEASURE_RESULT( start, "System Init Time 1: %dms\n" );
    
    // start���Ԍv���P-b
    MEASURE_START(start);
                                                                    //   Alloc, Free�œo�^�����������A���P�[�^�����������Ă��������B
#ifdef INIT_DEVICES_LIKE_UIG_LAUNCHER

    // �J����������
//	CAMERA_Init();

    // end���Ԍv���P-b
	MEASURE_RESULT( start, "Camera Init: %dms\n" );

#ifdef USE_HYENA_COMPONENT
    // DSP������
    {
        FSFile  file[1];
        MIWramSize sizeB = MI_WRAM_SIZE_32KB;
        MIWramSize sizeC = MI_WRAM_SIZE_64KB;
        int slotB = CreateDspSlotBitmap( DSP_SLOT_B_COMPONENT_G711 );  // �P�X���b�g
        int slotC = CreateDspSlotBitmap( DSP_SLOT_C_COMPONENT_G711 );  // �Q�X���b�g

        MI_FreeWramSlot_B( 0, sizeB, MI_WRAM_ARM9 );
        MI_FreeWramSlot_C( 0, sizeC, MI_WRAM_ARM9 );
        MI_CancelWramSlot_B( 0, sizeB, MI_WRAM_ARM9 );
        MI_CancelWramSlot_C( 0, sizeC, MI_WRAM_ARM9 );
        FS_InitFile(file);
        DSPi_OpenStaticComponentG711Core(file);
        if ( ! DSP_LoadG711(file, slotB, slotC) )
        {
            OS_TPanic("failed to load G.711 DSP-component! (lack of WRAM-B/C)");
        }
        DSP_UnloadG711();
    }
#endif // USE_HYENA_COMPONENT
#endif // INIT_DEVICES_LIKE_UIG_LAUNCHER

    // start���Ԍv���P-c
    MEASURE_START(start);
    
    // �e��p�����[�^�̎擾------------
    pBootTitle = SYSM_ReadParameters();        // �{�̐ݒ�f�[�^�AHW��񃊁[�h
											   // �A�v���W�����v�A�����p�J�[�h�N���A���Y�H���p�V���[�g�J�b�g�A�f�o�b�K�N���A����N���V�[�P���X�ATP�ݒ�V���[�g�J�b�g�̔���
    
    // TP�L�����u���[�V����
	UTL_CaribrateTP( LCFG_TSD_GetTPCalibrationPtr() );
	
    if( UTL_IsFatalError() ) {
        // FATAL�G���[����
        PrintError(); // �G���[�\��
        state = STOP;
        goto MAIN_LOOP_START; // state �� STOP �ɂ��ċ����I�Ƀ��C�����[�v�J�n
    }
    
    if( !LCFG_TSD_IsFinishedInitialSetting_Launcher() ) {
        // �����`���[���ł̏���N���V�[�P���X���Ȃ�A�ʐ^�B�e�����s����悤�ɂ���B
		// ���{�̐ݒ���ł̏���N���V�[�P���X���̏ꍇ�́ASYSM_ReadParameters ���̃`�F�b�N�Ō��o����āA�{�̐ݒ肪�N�������悤�ɂȂ��Ă��܂��B
    }

    // end���Ԍv���P-c
	MEASURE_RESULT( start, "SYSM_ReadParameters: %dms\n" );
    
    // start���Ԍv��4
    MEASURE_START(start);
	
    // �^�C�g�����X�g�̏���
    SYSM_InitTitleList();
	
    // end���Ԍv��4
	MEASURE_RESULT( start, "InitNandTitleList : %dms\n" );

    // start���Ԍv���Q
    MEASURE_START(start);
	
    sp_titleList = SYSM_GetCardTitleList(NULL);                 // �J�[�h�A�v�����X�g�̎擾�i�J�[�h�A�v����sp_titleList[0]�Ɋi�[�����j
	
    // end���Ԍv���Q
	MEASURE_RESULT( start, "GetCardTitleList Time : %dms\n" );

    // start���Ԍv��3
    MEASURE_START(start);
	
	// TMP�t�H���_�̃N���[��
	SYSM_DeleteTmpDirectory( pBootTitle );
	
	// �_�~�[��DS���j���[���b�s���O�p�t�@�C���쐬�iUIG�����`���[������Ă�����́j
	CreateDummyWrapFile();
	
    // end���Ԍv��3
	MEASURE_RESULT( start, "TmpClean : %dms\n" );

    // start���Ԍv��5
    MEASURE_START(start);
	
    // �u�_�C���N�g�u�[�g�łȂ��v�Ȃ�
    if( !pBootTitle ) {
        // NAND & �J�[�h�A�v�����X�g�擾
        if( !SYSM_IsLogoDemoSkip() )
        {
        	SYSM_MakeNandTitleListAsync();    // NAND�A�v�����X�g�̍쐬�i�擾�͂��Ă��Ȃ��̂Œ��Ӂj
        }else
        {
			sp_titleList = SYSM_GetNandTitleList();
		}
    }else
    {
		if( !pBootTitle->flags.isLogoSkip )
		{
			SYSM_MakeNandTitleListMakerInfoAsync();	// 	�A�v���Ɉ����n���^�C�g�����X�g�쐬�p���̍쐬
		}else
		{
			SYSM_MakeNandTitleListMakerInfo();
		}
	}
    // end���Ԍv��5
	MEASURE_RESULT( start, "GetNandTitleList : %dms\n" );

    // start���Ԍv��6
    MEASURE_START(start);
	
    // �u�_�C���N�g�u�[�g�łȂ��v��������
    // �u�_�C���N�g�u�[�g�����A���S�f���\���v�̎��A�e�탊�\�[�X�̃��[�h------------
#ifndef SYSM_BUILD_FOR_PRODUCTION_TEST
    if( !pBootTitle ||
        ( pBootTitle && !SYSM_IsLogoDemoSkip() ) ) {
		u32 timestamp;
		if( !LoadSharedFontInit() ) {				// ���L�t�H���g�̃��[�h
			UTL_SetFatalError( FATAL_ERROR_SHARED_FONT );
		}
		timestamp = OS_GetSharedFontTimestamp();
		if( timestamp > 0 ) OS_TPrintf( "SharedFont timestamp : %08x\n", timestamp );
	}
#endif // SYSM_BUILD_FOR_PRODUCTION_TEST
	
    // end���Ԍv��6
	MEASURE_RESULT( start, "GetSharedFont : %dms\n" );

    // �J�n�X�e�[�g�̔���--------------

    // start���Ԍv��7
    MEASURE_START(start);
	
    if( pBootTitle ) {
        // �_�C���N�g�u�[�g�Ȃ�A���S�A�����`���[���΂��ă��[�h�J�n
        if( pBootTitle->flags.isLogoSkip ) {
            state = LOAD_START;
        }else {
            state = LOGODEMO_INIT;
        }
        direct_boot = TRUE;
    }else if( SYSM_IsLogoDemoSkip() ) {
        // ���S�f���X�L�b�v���w�肳��Ă�����A�����`���[�N��
        state = LAUNCHER_INIT;
    }else {
        // �����Ȃ��Ȃ�A���S�f���N��
        state = LOGODEMO_INIT;
    }

// �����`���[��ʂ��Ε\�����Ȃ��o�[�W����
    if( SYSM_IsLauncherHidden() )
    {
        if(direct_boot == FALSE)
        {
            state = STOP;
        }else
        {
            state = LOAD_START;
        }
    }

#ifdef SYSM_BUILD_FOR_PRODUCTION_TEST
    if( !pBootTitle ||
        ( pBootTitle && ( pBootTitle->flags.bootType != LAUNCHER_BOOTTYPE_ROM ) )
	) {
		state = STOP;
	}
#endif // SYSM_BUILD_FOR_PRODUCTION_TEST

    // �`�����l�������b�N����
    SND_LockChannel((1 << L_CHANNEL) | (1 << R_CHANNEL), 0);

    /* �X�g���[���X���b�h�̋N�� */
    OS_CreateThread(&s_strmThread,
                    StrmThread,
                    NULL,
                    s_strmThreadStack + THREAD_STACK_SIZE / sizeof(u64),
                    THREAD_STACK_SIZE, STREAM_THREAD_PRIO);
    OS_WakeupThreadDirect(&s_strmThread);
	
    // end���Ԍv��7
	MEASURE_RESULT( start, "time 7 (etc...) : %dms\n" );

    // start���Ԍv��8
    MEASURE_START(start);
	
    // �����t�@�[���E�F�A�𖳐����W���[���Ƀ_�E�����[�h����B
    if( FALSE == InstallWlanFirmware( SYSM_IsHotStart() ) ) {
        OS_TPrintf( "ERROR: Wireless firmware download failed!\n" );
    }
	
    // end���Ԍv��8
	MEASURE_RESULT( start, "Load WlanFirm Time : %dms\n" );

    if( UTL_IsFatalError() ) {
        // FATAL�G���[����
        PrintError(); // �G���[�\��
        state = STOP;
        goto MAIN_LOOP_START; // state �� STOP �ɂ��ċ����I�Ƀ��C�����[�v�J�n
    }

	// end ���Ԍv��total
	MEASURE_RESULT( allstart, "Total Time : %dms\n" );
    
MAIN_LOOP_START:
    
    // ���C�����[�v--------------------
    while( 1 ) {
        OS_WaitIrq(1, OS_IE_V_BLANK);                           // V�u�����N���荞�ݑ҂�

        // �`�q�l�V�R�}���h������M
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }

        ReadKeyPad();                                           // �L�[���͂̎擾
        ReadTP();                                               // TP���͂̎擾

        switch( state ) {
        case LOGODEMO_INIT:
            LogoInit();
            // ���炷�e�X�g
            FS_InitFile(&s_strm.file);
            s_strm.isPlay = FALSE;
            PlayStream(&s_strm, filename);

            state = LOGODEMO;
            break;
        case LOGODEMO:
            if( IsFinishedLoadSharedFont() &&					// �ʏ�u�[�g���́A�t�H���g���[�h�I���������Ń`�F�b�N
				LogoMain() &&
            	SYSM_isNandTitleListReady()						// NAND�^�C�g���擾�������ǂ����`�F�b�N
				) {
				// ::::::::::::::::::::::::::::::::::::::::::::::
				// SystemMenu�o�[�W����etc.�̓ǂݍ���
				// ::::::::::::::::::::::::::::::::::::::::::::::
				
				// NAND�^�C�g�����X�g�擾�������������_�ŁA���̏������Ƃ�SystemMenuVersion�f�[�^�ɃA�N�Z�X���邽�߂̐�������Z�b�g����B
				SYSM_SetSystemMenuVersionControlData();
				PrintSystemMenuVersion();
				
				if( !direct_boot ) {
					sp_titleList = SYSM_GetTitlePropertyList();// TitlePropertyList�̎擾
                    state = LAUNCHER_INIT;
                }else {
                    state = LOAD_START;
                }
				// ���S�f�����I�������ꍇ�́AHotBoot�t���O�}������������B
				OSi_SetEnableHotBoot( TRUE );
			}
            break;
        case LAUNCHER_INIT:
            LauncherInit( NULL );
            state = LAUNCHER;
            break;
        case LAUNCHER:
            pBootTitle = LauncherMain( sp_titleList );
            if( pBootTitle ) {
                state = LOAD_START;
            }
            break;
        case LOAD_START:
			if( IsFinishedLoadSharedFont()						// �_�C���N�g�u�[�g�̎��́A�t�H���g���[�h�I���������Ń`�F�b�N
                && PollingInstallWlanFirmware()
#ifndef SYSM_DISABLE_WDS_SCAN										// �A�v���u�[�g�O��WDS�X�L�����͏I�����Ă����K�v������
			    && ( WDS_WrapperStopScan() != WDSWRAPPER_ERRCODE_OPERATING )
#endif // SYSM_DISABLE_WDS_SCAN
				) {
	            SYSM_StartLoadTitle( pBootTitle );
    	        state = LOADING;
    	        start = OS_GetTick();
				// �O�̂��ߕK���ʂ�p�X�ł��V���[�g�J�b�g�N���AHotBoot�t���O�}�����������Ă����B
				OSi_SetEnableHotBoot( TRUE );
			}
            break;
        case LOADING:
            // �����Ń��[�h�O�z���C�g���X�g�`�F�b�N���s���b�Z�[�W���|�[�����O���A�󂯎������state��LOAD_PAUSE��
            if( SYSM_IsLoadTitlePaused() )
            {
				state = LOAD_PAUSE;
				PrintPause();
			}
            if( SYSM_IsLoadTitleFinished() ) {
                SYSM_StartAuthenticateTitle( pBootTitle );
                state = AUTHENTICATE;
            }
            if( !direct_boot )
            {
                (void)LauncherFadeout( sp_titleList ); // �_�C���N�g�u�[�g�łȂ��Ƃ��̓t�F�[�h�A�E�g���s��
            }
            if( ( end == 0 ) &&
                SYSM_IsLoadTitleFinished() ) {
                end = OS_GetTick();
                OS_TPrintf( "Load Time : %dms\n", OS_TicksToMilliSeconds( end - start ) );
            }
            break;
        case LOAD_PAUSE:
            // ���[�h�O�z���C�g���X�g�`�F�b�N���s�ňꎞ��~��
            // �����N�����邩���~���邩�̑I�������ꂽ��LOADING�ɖ߂�
            if( IsCommandSelected() )
            {
				state = LOADING;
			}
            break;
        case AUTHENTICATE:
            if( ( direct_boot || ( !direct_boot && LauncherFadeout( sp_titleList ) ) ) &&
                SYSM_IsAuthenticateTitleFinished()
				) {
				// ���C�����[�v�J�n���猟�؏I���܂ł̊ԂɋN����FATAL�̏���
                if( UTL_IsFatalError() ) {
                    // FATAL�G���[����
                    // [TODO:]�N���A�����ق����ǂ��f�[�^�i���Ȃǁj������Ώ���
                    PrintError(); // �G���[�\��
                    state = STOP;
                    break; // state �� STOP �ɂ��� break ���A Boot �����Ȃ�
                }
				
#ifndef SYSM_DISABLE_WDS_SCAN
				// Nintendo�X�|�b�g�u�[�g���́A�A�v���ԃp�����[�^�Ƀr�[�R�������Z�b�g����B
				if( STD_CompareNString( (char *)&pBootTitle->titleID + 1, "JNH", 3 ) == 0 )
				{
					(void)WDS_WrapperSetArgumentParam();
				}
#endif // SYSM_DISABLE_WDS_SCAN
				
				state = BOOT;
			}
			break;
		case BOOT:
#ifndef SYSM_DISABLE_WDS_SCAN
			// �A�v���u�[�g�O��WDS�X�L�����͏I�����Ă����K�v������
			if( ( WDS_WrapperCleanup() != WDSWRAPPER_ERRCODE_OPERATING ) &&
				IsClearnupWDSWrapper() )
#endif // SYSM_DISABLE_WDS_SCAN
			{
				SYSM_TryToBootTitle( pBootTitle ); // never return.
            }
            break;
        case STOP:                                              // ��~
			OS_Terminate();
            break;
        }

        // �J�[�h�A�v�����X�g�̎擾�i�X���b�h�Ő����J�[�h�}����ʒm�������̂����C�����[�v�Ŏ擾�j
        {
	        BOOL changed;
	        sp_titleList = SYSM_GetCardTitleList( &changed );
	        if( changed )
	        {
				OS_TPrintf( "Change CARD status.\n" );
			}
		}

        // �����t�@�[�����[�h�̃|�[�����O
		if( PollingInstallWlanFirmware() &&
			( GetWlanFirmwareInstallFinalResult() == WLANFIRM_RESULT_SUCCESS )			// ���[�h����
			) {
			// ���L�����𖞂����Ȃ�AWDS�X�L�����J�n
#ifndef SYSM_DISABLE_WDS_SCAN
			if( !isStartScanWDS &&														// WDS�X�L�����J�n�ς݂łȂ�
				!direct_boot &&															// �_�C���N�g�u�[�g�łȂ�
				!LCFG_THW_IsForceDisableWireless() &&									// ��������OFF�łȂ�
				LCFG_TSD_IsAvailableWireless() 											// ����ON
				) {
				InitializeWDS();		// �������Ɠ���J�n�����˂Ă���B�i���s���Ă��~�܂�͂��Ȃ��̂ŁA�C�ɂ��Ȃ��j
				isStartScanWDS = TRUE;
			}
#endif // SYSM_DISABLE_WDS_SCAN
		}

        // �R�}���h�t���b�V��
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

#ifndef DISABLE_SLEEP
        // �X���[�v���[�h�ւ̑J��
        //�i�����t�@�[���̃��[�h�����̓A�v�����Ń`�F�b�N���Ă��炤���j�j
        //�i�W�J����Ԃƃf�o�b�K�ڑ����̃L�����Z���̓f�t�H���g�ōs���j
        if ( PollingInstallWlanFirmware() )
        {
	        UTL_GoSleepMode();
        }
#endif // DISABLE_SLEEP
    }
}

static BOOL IsCommandSelected(void)
{
	static BOOL left = FALSE;
	if( !( pad.cont & ( PAD_BUTTON_A | PAD_BUTTON_B ) ) )
	{
		left = TRUE;
	}
	
	if( left && ( pad.trg & PAD_BUTTON_A ) )
	{
		NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
		PrintfSJIS( 1, 25, TXT_COLOR_RED,"Resume Loading....\n" );
		SYSM_ResumeLoadingThread( TRUE );
		left = FALSE;
		return TRUE;
	}else if( left && ( pad.trg & PAD_BUTTON_B ) )
	{
		NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
		PrintfSJIS( 1, 25, TXT_COLOR_RED,"Please Wait....\n" );
		SYSM_ResumeLoadingThread( FALSE );
		left = FALSE;
		return TRUE;
	}
	return FALSE;
}

static void PrintPause(void)
{
	LauncherInit( NULL );
	GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	G2_ChangeBlendAlpha( 0, 31 );
	PrintfSJIS( 1, 25, TXT_COLOR_RED,"WhiteList Check Failed.\n" );
	PrintfSJIS( 1, 40, TXT_COLOR_RED,"Prease Select Command." );
	PrintfSJIS( 1, 55, TXT_COLOR_RED,"A : Force to Launch" );
	PrintfSJIS( 1, 70, TXT_COLOR_RED,"        or" );
	PrintfSJIS( 1, 85, TXT_COLOR_RED,"B : Stop And Show Error Message" );
	GX_DispOn();
	GXS_DispOn();
}

static void PrintError( void )
{
	u64 error_code;
	int l;
	int count = 0;
	LauncherInit( NULL );
	GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	G2_ChangeBlendAlpha( 0, 31 );
	error_code = UTL_GetFatalError();
	PrintfSJIS( 2, 25, TXT_COLOR_RED,"ERROR! - 0x%0.16x\n", error_code );
	ERRORLOG_Write(error_code);
	for(l=0;l<64;l++)
	{
		if( error_code & 0x1 )
		{
			PrintfSJIS( 2, 50+count*13, TXT_COLOR_RED,"%s\n", fatal_error_msg[l] );
			count++;
		}
		error_code = error_code >> 1;
	}
	GX_DispOn();
	GXS_DispOn();
}


// �V�X�e�����j���[�o�[�W�����̃v�����g
static void PrintSystemMenuVersion( void )
{
	u8 *pBuffer = SYSM_Alloc( NA_VERSION_DATA_WORK_SIZE );
	
	if( pBuffer &&
		ReadSystemMenuVersionData( pBuffer, NA_VERSION_DATA_WORK_SIZE ) ) {
		// ���[�h����
	}else {
		// FATAL�G���[
		UTL_SetFatalError( FATAL_ERROR_TWLSETTINGS );
	}
	SYSM_Free( pBuffer );
	
	// �o�[�W�������̕\��
	{
		char str_ver[ TWL_SYSMENU_VER_STR_LEN / sizeof(u16) ];
		int len = sizeof(str_ver);
		MI_CpuClear8( str_ver, sizeof(str_ver) );
		OS_TPrintf( "SystemMenuVersionData\n" );
		// ������
		if( STD_ConvertStringUnicodeToSjis( str_ver, &len, GetSystemMenuVersionString(), NULL, NULL ) == STD_RESULT_SUCCESS ) {
			OS_TPrintf( "  Version(str)       : %s\n", str_ver );
		}
		// ���l
		OS_TPrintf( "  Version(num)       : %d.%d\n", GetSystemMenuMajorVersion(), GetSystemMenuMinorVersion() );
		// ���[�U�[�̈�MAX�T�C�Y�̕\��
		OS_TPrintf( "  TotalUserAreadSize : 0x%08x\n", FSi_GetTotalUserAreaSize() );
		// EULA URL�̕\��
		OS_TPrintf( "  EULA URL           : %s\n", GetEULA_URL() );
		// NUP HostName�̕\��
		OS_TPrintf( "  NUP HostName       : %s\n", GetNUP_HostName() );
		// SystemMenuVersion���̃^�C���X�^���v�̎擾
		OS_TPrintf( "  Timestamp          : %08x\n", GetSystemMenuVersionTimeStamp() );
	}
}


// �_�~�[��DS���j���[���b�s���O�p�t�@�C���쐬�iUIG�����`���[������Ă�����́j
static void CreateDummyWrapFile( void )
{
	const char *path = "nand:/shared2/launcher/wrap.bin";
	
	if( FS_CreateFileAuto( path, FS_PERMIT_R | FS_PERMIT_W ) ) {
		FSFile file;
		if( FS_OpenFileEx( &file, path, FS_FILEMODE_RW ) ) {
			(void)FS_SetFileLength( &file, 16 * 1024 );
			FS_CloseFile( &file );
		}
	}
}


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);                              // V�u�����N�����`�F�b�N�̃Z�b�g
}

