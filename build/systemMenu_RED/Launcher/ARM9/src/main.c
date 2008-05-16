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
#include "launcher.h"
#include "misc.h"
#include "logoDemo.h"
#include "sound.h"
#include "loadWlanFirm.h"
#include "loadSharedFont.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static void INTR_VBlank( void );
static void deleteTmp();
void SYSM_DeleteTempDirectory( TitleProperty *pBootTitle );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static TitleProperty s_titleList[ LAUNCHER_TITLE_LIST_NUM ];

static u64 s_strmThreadStack[THREAD_STACK_SIZE / sizeof(u64)];
static OSThread s_strmThread;
static StreamInfo s_strm; // stream info

// const data------------------------------------------------------------------

const char filename[] = "data/fanfare.32.wav";

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
        AUTHENTICATE = 6,
        BOOT = 7,
        STOP = 8
    };
    u32 state = LOGODEMO_INIT;
    TitleProperty *pBootTitle = NULL;
    OSTick start, end = 0;
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
    
    OS_TPrintf( "SYSM_work size = 0x%x\n", sizeof(SYSM_work) );

    // 割り込み許可--------------------
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);

    // システムの初期化----------------
    InitAllocator();                                            // ※SYSM_Init以外のSYSMライブラリ関数を呼ぶ前に
                                                                //   Alloc, Freeで登録したメモリアロケータを初期化してください。


    // 各種パラメータの取得------------
    pBootTitle = SYSM_ReadParameters();                        // 本体設定データ、リセットパラメータのリード、検査用オート起動カード判定、量産ライン用キーショートカット起動判定等のリード

#ifdef DHT_TEST
    SYSMi_PrepareDatabase();
#endif

    if( SYSM_IsFatalError() ) {
        // FATALエラー処理
    }
    if( !LCFG_TSD_IsFinishedInitialSetting() ) {
        // 初回起動シーケンス判定
    }

    (void)SYSM_GetCardTitleList( s_titleList );                 // カードアプリリストの取得（カードアプリはs_titleList[0]に格納される）

	// TMPフォルダのクリーン
	SYSM_DeleteTmpDirectory( pBootTitle );

    // NANDタイトルリストの準備
    SYSM_InitNandTitleList();

    // 「ダイレクトブートでない」なら
    if( !pBootTitle ) {
        // NAND & カードアプリリスト取得
        (void)SYSM_GetNandTitleList( s_titleList, LAUNCHER_TITLE_LIST_NUM );    // NANDアプリリストの取得（内蔵アプリはs_titleList[1]から格納される）
    }

    // 「ダイレクトブートでない」もしくは
    // 「ダイレクトブートだが、ロゴデモ表示」の時、各種リソースのロード------------
    if( !pBootTitle ||
        ( pBootTitle && !SYSM_IsLogoDemoSkip() ) ) {
		u32 timestamp;
		if( !LoadSharedFontInit() ) {				// 共有フォントのロード
			SYSM_SetFatalError( TRUE );
		}
		timestamp = SFONT_GetFontTimestamp();
		if( timestamp > 0 ) OS_TPrintf( "SharedFont timestamp : %08x\n", timestamp );
	}

    // 開始ステートの判定--------------

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


    // 無線ファームウェアを無線モジュールにダウンロードする。
#ifndef DISABLE_WLFIRM_LOAD
    if( FALSE == InstallWlanFirmware( SYSM_IsHotStart() ) ) {
        OS_TPrintf( "ERROR: Wireless firmware download failed!\n" );
    }
#endif // DISABLE_WLFIRM_LOAD

    if( SYSM_IsFatalError() ) {
        // FATALエラー処理
    }

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
            SYSM_StartLoadTitle( pBootTitle );
            state = LOADING;

            start = OS_GetTick();

            break;
        case LOADING:
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
        case AUTHENTICATE:
            if( ( direct_boot || ( !direct_boot && LauncherFadeout( s_titleList ) ) ) &&
#ifndef DISABLE_WLFIRM_LOAD
                PollingInstallWlanFirmware( FALSE ) &&                 // アプリブート前に無線ファームのロードは完了しておく必要がある
#endif // DISABLE_WLFIRM_LOAD
                SYSM_IsAuthenticateTitleFinished() )
            {
                if( SYSM_IsFatalError() ) {
                    // FATALエラー処理
                }

                switch ( SYSM_TryToBootTitle( pBootTitle ) ) {   // アプリ認証結果取得orブート   成功時：never return
                case AUTH_RESULT_TITLE_LOAD_FAILED:
                case AUTH_RESULT_TITLE_POINTER_ERROR:
                case AUTH_RESULT_AUTHENTICATE_FAILED:
                case AUTH_RESULT_ENTRY_ADDRESS_ERROR:
                    state = STOP;
                    // [TODO:]クリアしたほうが良いデータ（鍵など）があれば消す
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
        // スリープモードへの遷移（蓋閉じ判定はデフォルトで行う）
        SYSM_GoSleepMode();
#endif // DISABLE_SLEEP
    }
}


// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);                              // Vブランク割込チェックのセット
}

