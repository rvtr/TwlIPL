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
#include <twl/dsp/ARM9/dsp_jpeg_dec.h>
#include <twl/camera.h>
#include "launcher.h"
#include "misc.h"
#include "logoDemo.h"
#include "sound.h"
#include "loadWlanFirm.h"
#include "loadSharedFont.h"

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
// デバグ用。SDに0x02ffc000から0x02ffe000までdump.datというダンプを吐く
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

// メイン
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
    // システムメニュー初期化----------
    SYSM_Init( Alloc, Free );                       // OS_Initの前でコールする必要あり。
    OS_Init();
    SYSM_SetArena();                                // OS_Initの後でコールする必要あり。

    // OS初期化------------------------
    OS_InitTick();
    
	// start 時間計測total
#if (MEASURE_TIME == 1)
    allstart = OS_GetTick();
#endif
    
    // start時間計測１
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    
    PM_Init();

    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    SYSM_InitPXI();                                 // 割り込み許可後にコールする必要あり。

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
    
     //NAMの初期化
    NAM_Init( Alloc, Free );
    
    OS_TPrintf( "SYSM_work size = 0x%x\n", sizeof(SYSM_work) );

    // 割り込み許可--------------------
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    
    // システムの初期化----------------
    InitAllocator();                                            // ※SYSM_Init以外のSYSMライブラリ関数を呼ぶ前に

    // end時間計測１
#if (MEASURE_TIME == 1)
    OS_TPrintf( "System Init Time 1: %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif
    
    // start時間計測１-b
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
                                                                    //   Alloc, Freeで登録したメモリアロケータを初期化してください。
#ifdef INIT_DEVICES_LIKE_UIG_LAUNCHER
    // カメラ初期化
    CAMERA_Init();

    // end時間計測１-b
#if (MEASURE_TIME == 1)
    OS_TPrintf( "Camera Init: %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // DSP初期化
    {
        FSFile file[1];
        MIWramSize sizeB = MI_WRAM_SIZE_128KB;
        MIWramSize sizeC = MI_WRAM_SIZE_128KB;
        int slotB = CreateDspSlotBitmap( DSP_SLOT_B_COMPONENT_JPEGDECODER );  // ２スロット
        int slotC = CreateDspSlotBitmap( DSP_SLOT_C_COMPONENT_JPEGDECODER );  // ４スロット

        FS_InitFile( file );
        DSP_OpenStaticComponentJpegDecoder( file );
        MI_FreeWramSlot_B( 0, sizeB, MI_WRAM_ARM9 );
        MI_FreeWramSlot_C( 0, sizeC, MI_WRAM_ARM9 );
        MI_CancelWramSlot_B( 0, sizeB, MI_WRAM_ARM9 );
        MI_CancelWramSlot_C( 0, sizeC, MI_WRAM_ARM9 );
        if ( ! DSP_LoadJpegDecoder( file, slotB, slotC ) )
        {
            OS_TPanic("failed to load JpegDecoder DSP-component! (lack of WRAM-B/C)");
        }
        DSP_UnloadJpegDecoder();
    }
#endif // INIT_DEVICES_LIKE_UIG_LAUNCHER

    // start時間計測１-c
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    
    // 各種パラメータの取得------------
    pBootTitle = SYSM_ReadParameters();                        // 本体設定データ、リセットパラメータのリード、検査用オート起動カード判定、量産ライン用キーショートカット起動判定等のリード
	
    // end時間計測１-c
#if (MEASURE_TIME == 1)
    OS_TPrintf( "SYSM_ReadParameters: %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start時間計測２
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    
    // TPキャリブレーション
	UTL_CaribrateTP( LCFG_TSD_GetTPCalibrationPtr() );
	
    if( SYSM_IsFatalError() ) {
        // FATALエラー処理
    }
    if( !LCFG_TSD_IsFinishedInitialSetting() ) {
        // 初回起動シーケンス判定
    }
    
    (void)SYSM_GetCardTitleList( s_titleList );                 // カードアプリリストの取得（カードアプリはs_titleList[0]に格納される）
    // end時間計測２
#if (MEASURE_TIME == 1)
    OS_TPrintf( "GetCardTitleList Time : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start時間計測3
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
	// TMPフォルダのクリーン
	SYSM_DeleteTmpDirectory( pBootTitle );
    // end時間計測3
#if (MEASURE_TIME == 1)
    OS_TPrintf( "TmpClean : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start時間計測4
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    // NANDタイトルリストの準備
    SYSM_InitNandTitleList();
    // end時間計測4
#if (MEASURE_TIME == 1)
    OS_TPrintf( "InitNandTitleList : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start時間計測5
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    // 「ダイレクトブートでない」なら
    if( !pBootTitle ) {
        // NAND & カードアプリリスト取得
        (void)SYSM_GetNandTitleList( s_titleList, LAUNCHER_TITLE_LIST_NUM );    // NANDアプリリストの取得（内蔵アプリはs_titleList[1]から格納される）
    }
    // end時間計測5
#if (MEASURE_TIME == 1)
    OS_TPrintf( "GetNandTitleList : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // start時間計測6
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    // 「ダイレクトブートでない」もしくは
    // 「ダイレクトブートだが、ロゴデモ表示」の時、各種リソースのロード------------
    if( !pBootTitle ||
        ( pBootTitle && !SYSM_IsLogoDemoSkip() ) ) {
		u32 timestamp;
		if( !LoadSharedFontInit() ) {				// 共有フォントのロード
			SYSM_SetFatalError( TRUE );
		}
		timestamp = OS_GetSharedFontTimestamp();
		if( timestamp > 0 ) OS_TPrintf( "SharedFont timestamp : %08x\n", timestamp );
	}
    // end時間計測6
#if (MEASURE_TIME == 1)
    OS_TPrintf( "GetSharedFont : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    // 開始ステートの判定--------------

    // start時間計測7
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    if( pBootTitle ) {
        // ダイレクトブートなら、ロゴ、ランチャーを飛ばしてロード開始
        if( pBootTitle->flags.isLogoSkip ) {
            state = LOAD_START;
        }else {
            state = LOGODEMO_INIT;
        }
        direct_boot = TRUE;
    }else if( SYSM_IsLogoDemoSkip() ) {
        // ロゴデモスキップが指定されていたら、ランチャー起動
        state = LAUNCHER_INIT;
    }else {
        // 何もないなら、ロゴデモ起動
        state = LOGODEMO_INIT;
    }

// ランチャー画面を絶対表示しないバージョン
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

    // チャンネルをロックする
    SND_LockChannel((1 << L_CHANNEL) | (1 << R_CHANNEL), 0);

    /* ストリームスレッドの起動 */
    OS_CreateThread(&s_strmThread,
                    StrmThread,
                    NULL,
                    s_strmThreadStack + THREAD_STACK_SIZE / sizeof(u64),
                    THREAD_STACK_SIZE, STREAM_THREAD_PRIO);
    OS_WakeupThreadDirect(&s_strmThread);
    // end時間計測7
#if (MEASURE_TIME == 1)
    OS_TPrintf( "time 7 (etc...) : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif


    // start時間計測8
#if (MEASURE_TIME == 1)
    start = OS_GetTick();
#endif
    // 無線ファームウェアを無線モジュールにダウンロードする。
#ifndef DISABLE_WLFIRM_LOAD
    if( FALSE == InstallWlanFirmware( SYSM_IsHotStart() ) ) {
        OS_TPrintf( "ERROR: Wireless firmware download failed!\n" );
    }
#endif // DISABLE_WLFIRM_LOAD
    // end時間計測8
#if (MEASURE_TIME == 1)
    OS_TPrintf( "Load WlanFirm Time : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
#endif

    if( SYSM_IsFatalError() ) {
        // FATALエラー処理
    }

	// end 時間計測total
#if (MEASURE_TIME == 1)
    OS_TPrintf( "Total Time : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - allstart ) );
#endif
    
    // メインループ--------------------
    while( 1 ) {
        OS_WaitIrq(1, OS_IE_V_BLANK);                           // Vブランク割り込み待ち

        // ＡＲＭ７コマンド応答受信
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }

        ReadKeyPad();                                           // キー入力の取得
        ReadTP();                                               // TP入力の取得

        switch( state ) {
        case LOGODEMO_INIT:
            LogoInit();
            // 音鳴らすテスト
            FS_InitFile(&s_strm.file);
            s_strm.isPlay = FALSE;
            PlayStream(&s_strm, filename);

            state = LOGODEMO;
            break;
        case LOGODEMO:
            if( LogoMain() &&
				IsFinishedLoadSharedFont() ) {	// フォントロード終了をここでチェック
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
			if( IsFinishedLoadSharedFont() ) {		// ダイレクトブートの時があるので、フォントロード終了をここでチェック
	            SYSM_StartLoadTitle( pBootTitle );
    	        state = LOADING;

    	        start = OS_GetTick();
			}
            break;
        case LOADING:
            // ここでロード前ホワイトリストチェック失敗メッセージをポーリングし、受け取ったらstateをLOAD_PAUSEに
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
                (void)LauncherFadeout( s_titleList ); // ダイレクトブートでないときはフェードアウトも行う
            }
            if( ( end == 0 ) &&
                SYSM_IsLoadTitleFinished() ) {
                end = OS_GetTick();
                OS_TPrintf( "Load Time : %dms\n", OS_TicksToMilliSeconds( end - start ) );
            }
            break;
        case LOAD_PAUSE:
            // ロード前ホワイトリストチェック失敗で一時停止中
            // 強制起動するか中止するかの選択がされたらLOADINGに戻る
            if( IsCommandSelected() )
            {
				state = LOADING;
			}
            break;
        case AUTHENTICATE:
            if( ( direct_boot || ( !direct_boot && LauncherFadeout( s_titleList ) ) ) &&
#ifndef DISABLE_WLFIRM_LOAD
                PollingInstallWlanFirmware( FALSE ) &&                 // アプリブート前に無線ファームのロードは完了しておく必要がある
#endif // DISABLE_WLFIRM_LOAD
                SYSM_IsAuthenticateTitleFinished() )
            {
				AuthResult res;
                if( SYSM_IsFatalError() ) {
                    // FATALエラー処理
                }

				res = SYSM_TryToBootTitle( pBootTitle );
                switch ( res ) {   // アプリ認証結果取得orブート   成功時：never return
                case AUTH_RESULT_TITLE_LOAD_FAILED:
                case AUTH_RESULT_TITLE_POINTER_ERROR:
                case AUTH_RESULT_AUTHENTICATE_FAILED:
                case AUTH_RESULT_ENTRY_ADDRESS_ERROR:
                default:
                    state = STOP;
                    // [TODO:]クリアしたほうが良いデータ（鍵など）があれば消す
                    
                    // エラー表示
                    PrintError(res);
					
                    break;
                }
            }
            break;
        case STOP:                                              // 停止
            break;
        }

        // カードアプリリストの取得（スレッドで随時カード挿抜を通知されるものをメインループで取得）
        (void)SYSM_GetCardTitleList( s_titleList );

        // 無線ファームロードのポーリング
		(void)PollingInstallWlanFirmware( pBootTitle ? FALSE : TRUE );

        // コマンドフラッシュ
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

#ifndef DISABLE_SLEEP
        // スリープモードへの遷移（蓋開き状態とデバッガ接続中のキャンセルはデフォルトで行う）
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
	// 特殊表示
	if(res == AUTH_RESULT_CHECK_TITLE_LAUNCH_RIGHTS_FAILED)
	{
		PrintfSJIS( 1, 55, TXT_COLOR_RED,"NAM result = %d", SYSMi_getCheckTitleLaunchRightsResult() );
	}
	GX_DispOn();
	GXS_DispOn();
}

// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);                              // Vブランク割込チェックのセット
}

