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
#include "launcher.h"
#include "misc.h"
#include "logoDemo.h"
#include "sound.h"
#include "loadWlanFirm.h"
#include "loadSharedFont.h"
#include "scanWDS.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------
#define INIT_DEVICES_LIKE_UIG_LAUNCHER

// デバッグ用時間計測スイッチ
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
	
    // システムメニュー初期化----------
    SYSM_Init( Alloc, Free );                       // OS_Initの前でコールする必要あり。
    OS_Init();
    SYSM_SetArena();                                // OS_Initの後でコールする必要あり。

    // OS初期化------------------------
    OS_InitTick();
    
	// start 時間計測total
    MEASURE_START(allstart);
    
    // start時間計測１
    MEASURE_START(start);
    
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

    ErrorLog_Init( Alloc, Free );
    
    // end時間計測１
	MEASURE_RESULT( start, "System Init Time 1: %dms\n" );
    
    // start時間計測１-b
    MEASURE_START(start);
                                                                    //   Alloc, Freeで登録したメモリアロケータを初期化してください。
#ifdef INIT_DEVICES_LIKE_UIG_LAUNCHER

    // カメラ初期化
//	CAMERA_Init();

    // end時間計測１-b
	MEASURE_RESULT( start, "Camera Init: %dms\n" );

#ifdef USE_HYENA_COMPONENT
    // DSP初期化
    {
        FSFile  file[1];
        MIWramSize sizeB = MI_WRAM_SIZE_32KB;
        MIWramSize sizeC = MI_WRAM_SIZE_64KB;
        int slotB = CreateDspSlotBitmap( DSP_SLOT_B_COMPONENT_G711 );  // １スロット
        int slotC = CreateDspSlotBitmap( DSP_SLOT_C_COMPONENT_G711 );  // ２スロット

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

    // start時間計測１-c
    MEASURE_START(start);
    
    // 各種パラメータの取得------------
    pBootTitle = SYSM_ReadParameters();        // 本体設定データ、HW情報リード
											   // アプリジャンプ、検査用カード起動、生産工程用ショートカット、デバッガ起動、初回起動シーケンス、TP設定ショートカットの判定
    
    // TPキャリブレーション
	UTL_CaribrateTP( LCFG_TSD_GetTPCalibrationPtr() );
	
    if( UTL_IsFatalError() ) {
        // FATALエラー処理
        PrintError(); // エラー表示
        state = STOP;
        goto MAIN_LOOP_START; // state を STOP にして強制的にメインループ開始
    }
    
    if( !LCFG_TSD_IsFinishedInitialSetting_Launcher() ) {
        // ランチャー内での初回起動シーケンス中なら、写真撮影を実行するようにする。
		// ※本体設定内での初会起動シーケンス中の場合は、SYSM_ReadParameters 内のチェックで検出されて、本体設定が起動されるようになっています。
    }

    // end時間計測１-c
	MEASURE_RESULT( start, "SYSM_ReadParameters: %dms\n" );
    
    // start時間計測4
    MEASURE_START(start);
	
    // タイトルリストの準備
    SYSM_InitTitleList();
	
    // end時間計測4
	MEASURE_RESULT( start, "InitNandTitleList : %dms\n" );

    // start時間計測２
    MEASURE_START(start);
	
    sp_titleList = SYSM_GetCardTitleList(NULL);                 // カードアプリリストの取得（カードアプリはsp_titleList[0]に格納される）
	
    // end時間計測２
	MEASURE_RESULT( start, "GetCardTitleList Time : %dms\n" );

    // start時間計測3
    MEASURE_START(start);
	
	// TMPフォルダのクリーン
	SYSM_DeleteTmpDirectory( pBootTitle );
	
    // end時間計測3
	MEASURE_RESULT( start, "TmpClean : %dms\n" );

    // start時間計測5
    MEASURE_START(start);
	
    // 「ダイレクトブートでない」なら
    if( !pBootTitle ) {
        // NAND & カードアプリリスト取得
        if( !SYSM_IsLogoDemoSkip() )
        {
        	SYSM_MakeNandTitleListAsync();    // NANDアプリリストの作成（取得はしていないので注意）
        }else
        {
			sp_titleList = SYSM_GetNandTitleList();
		}
    }else
    {
		if( !pBootTitle->flags.isLogoSkip )
		{
			SYSM_MakeNandTitleListMakerInfoAsync();	// 	アプリに引き渡すタイトルリスト作成用情報の作成
		}else
		{
			SYSM_MakeNandTitleListMakerInfo();
		}
	}
    // end時間計測5
	MEASURE_RESULT( start, "GetNandTitleList : %dms\n" );

    // start時間計測6
    MEASURE_START(start);
	
    // 「ダイレクトブートでない」もしくは
    // 「ダイレクトブートだが、ロゴデモ表示」の時、各種リソースのロード------------
    if( !pBootTitle ||
        ( pBootTitle && !SYSM_IsLogoDemoSkip() ) ) {
		u32 timestamp;
		if( !LoadSharedFontInit() ) {				// 共有フォントのロード
			UTL_SetFatalError( FATAL_ERROR_SHARED_FONT );
		}
		timestamp = OS_GetSharedFontTimestamp();
		if( timestamp > 0 ) OS_TPrintf( "SharedFont timestamp : %08x\n", timestamp );
	}
	
    // end時間計測6
	MEASURE_RESULT( start, "GetSharedFont : %dms\n" );

    // 開始ステートの判定--------------

    // start時間計測7
    MEASURE_START(start);
	
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
	MEASURE_RESULT( start, "time 7 (etc...) : %dms\n" );

    // start時間計測8
    MEASURE_START(start);
	
    // 無線ファームウェアを無線モジュールにダウンロードする。
#ifndef DISABLE_WLFIRM_LOAD
    if( FALSE == InstallWlanFirmware( SYSM_IsHotStart() ) ) {
        OS_TPrintf( "ERROR: Wireless firmware download failed!\n" );
    }
#endif // DISABLE_WLFIRM_LOAD
	
    // end時間計測8
	MEASURE_RESULT( start, "Load WlanFirm Time : %dms\n" );

    if( UTL_IsFatalError() ) {
        // FATALエラー処理
        PrintError(); // エラー表示
        state = STOP;
        goto MAIN_LOOP_START; // state を STOP にして強制的にメインループ開始
    }

	// end 時間計測total
	MEASURE_RESULT( allstart, "Total Time : %dms\n" );
    
MAIN_LOOP_START:
    
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
            if( IsFinishedLoadSharedFont() &&					// 通常ブート時は、フォントロード終了をここでチェック
				LogoMain() &&
            	SYSM_isNandTitleListReady()						// NANDタイトル取得完了かどうかチェック
				) {
				if( !direct_boot ) {
					sp_titleList = SYSM_GetTitlePropertyList();// TitlePropertyListの取得
                    state = LAUNCHER_INIT;
                }else {
                    state = LOAD_START;
                }
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
			if( IsFinishedLoadSharedFont()						// ダイレクトブートの時は、フォントロード終了をここでチェック
#ifndef DISABLE_WLFIRM_LOAD										// アプリブート前に無線ファームのロードは完了しておく
                && PollingInstallWlanFirmware()
#endif // DISABLE_WLFIRM_LOAD
#ifndef DISABLE_WDS_SCAN										// アプリブート前にWDSスキャンは終了しておく必要がある
			    && ( WDS_WrapperStopScan() != WDSWRAPPER_ERRCODE_OPERATING )
#endif // DISABLE_WLFIRM_LOAD
				) {
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
                (void)LauncherFadeout( sp_titleList ); // ダイレクトブートでないときはフェードアウトも行う
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
            if( ( direct_boot || ( !direct_boot && LauncherFadeout( sp_titleList ) ) ) &&
                SYSM_IsAuthenticateTitleFinished()
				) {
				// メインループ開始から検証終了までの間に起きたFATALの処理
                if( UTL_IsFatalError() ) {
                    // FATALエラー処理
                    // [TODO:]クリアしたほうが良いデータ（鍵など）があれば消す
                    PrintError(); // エラー表示
                    state = STOP;
                    break; // state を STOP にして break し、 Boot させない
                }
				
#ifndef DISABLE_WDS_SCAN
				// Nintendoスポットブート時は、アプリ間パラメータにビーコン情報をセットする。
				if( STD_CompareNString( (char *)&pBootTitle->titleID + 1, "JNH", 3 ) == 0 )
				{
					(void)WDS_WrapperSetArgumentParam();
				}
#endif // DISABLE_WDS_SCAN
				
				state = BOOT;
			}
			break;
		case BOOT:
#ifndef DISABLE_WDS_SCAN
			// アプリブート前にWDSスキャンは終了しておく必要がある
			if( ( WDS_WrapperCleanup() != WDSWRAPPER_ERRCODE_OPERATING ) &&
				IsClearnupWDSWrapper() )
#endif // DISABLE_WDS_SCAN
			{
				SYSM_TryToBootTitle( pBootTitle ); // never return.
            }
            break;
        case STOP:                                              // 停止
            break;
        }

        // カードアプリリストの取得（スレッドで随時カード挿抜を通知されるものをメインループで取得）
        {
	        BOOL changed;
	        sp_titleList = SYSM_GetCardTitleList( &changed );
	        if( changed )
	        {
				OS_TPrintf( "Change CARD status.\n" );
			}
		}

        // 無線ファームロードのポーリング
		if( PollingInstallWlanFirmware() &&
			( GetWlanFirmwareInstallFinalResult() == WLANFIRM_RESULT_SUCCESS )			// ロード成功
			) {
			// 下記条件を満たすなら、WDSスキャン開始
#ifndef DISABLE_WDS_SCAN
			if( !isStartScanWDS &&														// WDSスキャン開始済みでない
				!direct_boot &&															// ダイレクトブートでない
				!LCFG_THW_IsForceDisableWireless() &&									// 無線強制OFFでない
				LCFG_TSD_IsAvailableWireless() 											// 無線ON
				) {
				InitializeWDS();		// 初期化と動作開始を兼ねている。（失敗しても止まりはしないので、気にしない）
				isStartScanWDS = TRUE;
			}
#endif // DISABLE_WDS_SCAN
		}

        // コマンドフラッシュ
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

#ifndef DISABLE_SLEEP
        // スリープモードへの遷移
        //（無線ファームのロード完了はアプリ側でチェックしてもらう方針）
        //（蓋開き状態とデバッガ接続中のキャンセルはデフォルトで行う）
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
	ErrorLog_WriteErrorLog(error_code);
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

// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);                              // Vブランク割込チェックのセット
}

