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
#include <twl/dsp/common/shutter.h>
#include <twl/camera.h>
#include "launcher.h"
#include "misc.h"
#include "logoDemo.h"
#include "sound.h"
#include "loadWlanFirm.h"
#include "loadSharedFont.h"
#include "loadSysmVersion.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

#define INIT_DEVICES_LIKE_UIG_LAUNCHER

#define MEASURE_TIME     1

// function's prototype-------------------------------------------------------
static void INTR_VBlank( void );
static void deleteTmp();
void SYSM_DeleteTempDirectory( TitleProperty *pBootTitle );
static BOOL IsCommandSelected(void);
static void PrintPause(void);
static void PrintError(AuthResult res);

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static TitleProperty s_titleList[ LAUNCHER_TITLE_LIST_NUM ];

static u64 s_strmThreadStack[THREAD_STACK_SIZE / sizeof(u64)];
static OSThread s_strmThread;
static StreamInfo s_strm; // stream info

// const data------------------------------------------------------------------

const char filename[] = "data/fanfare.32.wav";

static const char *error_msg[AUTH_RESULT_MAX] = 
{
	"SUCCEEDED",
	"PROCESSING",
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
	"PLATFORM_UNKNOWN"
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
    OSTick allstart, start, end = 0;
    BOOL direct_boot = FALSE;

#ifdef DEBUG_LAUNCHER_DUMP
    // you should comment out to clear GX/G2/DMA/TM/PAD register in reboot.c to retreive valid boot time
    STD_TSPrintf((char*)0x02FFCFC0, "\nLauncher Boot Time: %lld usec\n", OS_TicksToMicroSeconds(reg_OS_TM3CNT_L * (1024/64)));
    STD_TSPrintf((char*)0x02FFCFF0, "HOTSTART(0x%08x): %02x\n", HW_NAND_FIRM_HOTSTART_FLAG, *(u8 *)HW_NAND_FIRM_HOTSTART_FLAG);
#endif
    // �V�X�e�����j���[������----------
    SYSM_Init( Alloc, Free );                       // OS_Init�̑O�ŃR�[������K�v����B
    OS_Init();
    SYSM_SetArena();                                // OS_Init�̌�ŃR�[������K�v����B

    // OS������------------------------
    OS_InitTick();
    
	// start ���Ԍv��total
#if (MEASURE_TIME == 1)
    allstart = OS_GetTick();
#endif
    
    // start���Ԍv���P
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    
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

    // end���Ԍv���P
#if (MEASURE_TIME == 1)
    OS_TPrintf( "System Init Time 1: %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif
    
    // start���Ԍv���P-b
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
                                                                    //   Alloc, Free�œo�^�����������A���P�[�^�����������Ă��������B
#ifdef INIT_DEVICES_LIKE_UIG_LAUNCHER
    // �J����������
    CAMERA_Init();

    // end���Ԍv���P-b
#if (MEASURE_TIME == 1)
    OS_TPrintf( "Camera Init: %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

#ifdef USE_HYENA_COMPONENT
    // DSP������
    {
        MIWramSize sizeB = MI_WRAM_SIZE_32KB;
        MIWramSize sizeC = MI_WRAM_SIZE_64KB;
        int slotB = CreateDspSlotBitmap( DSP_SLOT_B_COMPONENT_G711 );  // �P�X���b�g
        int slotC = CreateDspSlotBitmap( DSP_SLOT_C_COMPONENT_G711 );  // �Q�X���b�g

        MI_FreeWramSlot_B( 0, sizeB, MI_WRAM_ARM9 );
        MI_FreeWramSlot_C( 0, sizeC, MI_WRAM_ARM9 );
        MI_CancelWramSlot_B( 0, sizeB, MI_WRAM_ARM9 );
        MI_CancelWramSlot_C( 0, sizeC, MI_WRAM_ARM9 );
        if ( ! DSP_LoadShutter() )
        {
            OS_TPanic("failed to load Shutter DSP-component! (lack of WRAM-B/C)");
        }
        DSP_UnloadShutter();
    }
#endif // USE_HYENA_COMPONENT
#endif // INIT_DEVICES_LIKE_UIG_LAUNCHER

    // start���Ԍv���P-c
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    
    // �e��p�����[�^�̎擾------------
    pBootTitle = SYSM_ReadParameters();                        // �{�̐ݒ�f�[�^�A���Z�b�g�p�����[�^�̃��[�h�A�����p�I�[�g�N���J�[�h����A�ʎY���C���p�L�[�V���[�g�J�b�g�N�����蓙�̃��[�h
	
    // end���Ԍv���P-c
#if (MEASURE_TIME == 1)
    OS_TPrintf( "SYSM_ReadParameters: %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start���Ԍv���Q
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    
    // TP�L�����u���[�V����
	UTL_CaribrateTP( LCFG_TSD_GetTPCalibrationPtr() );
	
    if( SYSM_IsFatalError() ) {
        // FATAL�G���[����
    }
    if( !LCFG_TSD_IsFinishedInitialSetting() ) {
        // ����N���V�[�P���X����
    }
	
    (void)SYSM_GetCardTitleList( s_titleList );                 // �J�[�h�A�v�����X�g�̎擾�i�J�[�h�A�v����s_titleList[0]�Ɋi�[�����j
    // end���Ԍv���Q
#if (MEASURE_TIME == 1)
    OS_TPrintf( "GetCardTitleList Time : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start���Ԍv��3
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
	// TMP�t�H���_�̃N���[��
	SYSM_DeleteTmpDirectory( pBootTitle );
    // end���Ԍv��3
#if (MEASURE_TIME == 1)
    OS_TPrintf( "TmpClean : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start���Ԍv��4
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    // NAND�^�C�g�����X�g�̏���
    SYSM_InitNandTitleList();
    // end���Ԍv��4
#if (MEASURE_TIME == 1)
    OS_TPrintf( "InitNandTitleList : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start���Ԍv��5
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    // �u�_�C���N�g�u�[�g�łȂ��v�Ȃ�
    if( !pBootTitle ) {
        // NAND & �J�[�h�A�v�����X�g�擾
        (void)SYSM_GetNandTitleList( s_titleList, LAUNCHER_TITLE_LIST_NUM );    // NAND�A�v�����X�g�̎擾�i�����A�v����s_titleList[1]����i�[�����j
    }else
    {
		SYSM_GetNandTitleListMakerInfo();	// 	�A�v���Ɉ����n���^�C�g�����X�g�쐬�p���̎擾
	}
    // end���Ԍv��5
#if (MEASURE_TIME == 1)
    OS_TPrintf( "GetNandTitleList : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start���Ԍv��6
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    // �u�_�C���N�g�u�[�g�łȂ��v��������
    // �u�_�C���N�g�u�[�g�����A���S�f���\���v�̎��A�e�탊�\�[�X�̃��[�h------------
    if( !pBootTitle ||
        ( pBootTitle && !SYSM_IsLogoDemoSkip() ) ) {
		u32 timestamp;
		if( !LoadSharedFontInit() ) {				// ���L�t�H���g�̃��[�h
			SYSM_SetFatalError( TRUE );
		}
		timestamp = OS_GetSharedFontTimestamp();
		if( timestamp > 0 ) OS_TPrintf( "SharedFont timestamp : %08x\n", timestamp );
	}
    // end���Ԍv��6
#if (MEASURE_TIME == 1)
    OS_TPrintf( "GetSharedFont : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

	// �o�[�W�������̃��[�h
	LoadSysmVersion();
	OS_TPrintf( "Launcher Version = %d", GetSysmVersion() );

    // �J�n�X�e�[�g�̔���--------------

    // start���Ԍv��7
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
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
#if (MEASURE_TIME == 1)
    OS_TPrintf( "time 7 (etc...) : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif


    // start���Ԍv��8
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    // �����t�@�[���E�F�A�𖳐����W���[���Ƀ_�E�����[�h����B
#ifndef DISABLE_WLFIRM_LOAD
    if( FALSE == InstallWlanFirmware( SYSM_IsHotStart() ) ) {
        OS_TPrintf( "ERROR: Wireless firmware download failed!\n" );
    }
#endif // DISABLE_WLFIRM_LOAD
    // end���Ԍv��8
#if (MEASURE_TIME == 1)
    OS_TPrintf( "Load WlanFirm Time : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    if( SYSM_IsFatalError() ) {
        // FATAL�G���[����
    }

	// end ���Ԍv��total
#if (MEASURE_TIME == 1)
    OS_TPrintf( "Total Time : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - allstart ) );
#endif
    
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
            if( LogoMain() &&
				IsFinishedLoadSharedFont() ) {	// �t�H���g���[�h�I���������Ń`�F�b�N
#if 0
				if( SYSM_IsFatalError() ) {
					state = STOP;
				}else
#endif
				if( !direct_boot ) {
                    state = LAUNCHER_INIT;
                }else {
                    state = LOAD_START;
                }
            }
            break;
        case LAUNCHER_INIT:
            LauncherInit( s_titleList );
            state = LAUNCHER;
            break;
        case LAUNCHER:
            pBootTitle = LauncherMain( s_titleList );
            if( pBootTitle ) {
                state = LOAD_START;
            }
            break;
        case LOAD_START:
			if( IsFinishedLoadSharedFont() ) {		// �_�C���N�g�u�[�g�̎�������̂ŁA�t�H���g���[�h�I���������Ń`�F�b�N
	            SYSM_StartLoadTitle( pBootTitle );
    	        state = LOADING;

    	        start = OS_GetTick();
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
                (void)LauncherFadeout( s_titleList ); // �_�C���N�g�u�[�g�łȂ��Ƃ��̓t�F�[�h�A�E�g���s��
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
            if( ( direct_boot || ( !direct_boot && LauncherFadeout( s_titleList ) ) ) &&
#ifndef DISABLE_WLFIRM_LOAD
                PollingInstallWlanFirmware( FALSE ) &&                 // �A�v���u�[�g�O�ɖ����t�@�[���̃��[�h�͊������Ă����K�v������
#endif // DISABLE_WLFIRM_LOAD
                SYSM_IsAuthenticateTitleFinished() )
            {
				AuthResult res;
                if( SYSM_IsFatalError() ) {
                    // FATAL�G���[����
                }

				res = SYSM_TryToBootTitle( pBootTitle );
                switch ( res ) {   // �A�v���F�،��ʎ擾or�u�[�g   �������Fnever return
                case AUTH_RESULT_TITLE_LOAD_FAILED:
                case AUTH_RESULT_TITLE_POINTER_ERROR:
                case AUTH_RESULT_AUTHENTICATE_FAILED:
                case AUTH_RESULT_ENTRY_ADDRESS_ERROR:
                default:
                    state = STOP;
                    // [TODO:]�N���A�����ق����ǂ��f�[�^�i���Ȃǁj������Ώ���
                    
                    // �G���[�\��
                    PrintError(res);
					
                    break;
                }
            }
            break;
        case STOP:                                              // ��~
            break;
        }

        // �J�[�h�A�v�����X�g�̎擾�i�X���b�h�Ő����J�[�h�}����ʒm�������̂����C�����[�v�Ŏ擾�j
        (void)SYSM_GetCardTitleList( s_titleList );

        // �����t�@�[�����[�h�̃|�[�����O
		(void)PollingInstallWlanFirmware( pBootTitle ? FALSE : TRUE );

        // �R�}���h�t���b�V��
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

#ifndef DISABLE_SLEEP
        // �X���[�v���[�h�ւ̑J�ځi�W�J����Ԃƃf�o�b�K�ڑ����̃L�����Z���̓f�t�H���g�ōs���j
        UTL_GoSleepMode();
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
	LauncherInit( s_titleList );
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

static void PrintError(AuthResult res)
{
	LauncherInit( s_titleList );
	GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
	NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	G2_ChangeBlendAlpha( 0, 31 );
	PrintfSJIS( 1, 25, TXT_COLOR_RED,"LAUNCHER : ERROR OCCURRED! - %d\n",res );
	PrintfSJIS( 1, 40, TXT_COLOR_RED,"%s",error_msg[res] );
	// ����\��
	if(res == AUTH_RESULT_CHECK_TITLE_LAUNCH_RIGHTS_FAILED)
	{
		PrintfSJIS( 1, 55, TXT_COLOR_RED,"NAM result = %d", SYSMi_getCheckTitleLaunchRightsResult() );
	}
	GX_DispOn();
	GXS_DispOn();
}

// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);                              // V�u�����N�����`�F�b�N�̃Z�b�g
}

