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

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static TitleProperty s_titleList[ LAUNCHER_TITLE_LIST_NUM ];

static u64 strmThreadStack[THREAD_STACK_SIZE / sizeof(u64)];
static OSThread strmThread;

static StreamInfo strm; // stream info

// const data------------------------------------------------------------------

const char filename[] = "data/fanfare.32.wav";

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
	
	// システムメニュー初期化----------
	SYSM_Init( Alloc, Free );									// OS_Initは、本関数内でコールしているので、コールする必要なし。
	
	// OS初期化------------------------
	OS_InitTick();
    PM_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
	FS_Init( FS_DMA_NOT_USE );
	
	// debug
	debugWriteToSD();
	
    GX_Init();
	PM_Init();
	TP_Init();
	RTC_Init();
    SND_Init();// sound init
	
	OS_TPrintf( "SYSM_work size = 0x%x\n", sizeof(SYSM_work) );
	
	// 割り込み許可--------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// システムの初期化----------------
	InitAllocator();											// ※SYSM_Init以外のSYSMライブラリ関数を呼ぶ前に
																//   Alloc, Freeで登録したメモリアロケータを初期化してください。
	// 各種パラメータの取得------------
	pBootTitle = SYSM_ReadParameters();							// 本体設定データ、リセットパラメータ、
																// 初回起動シーケンス判定、
																// 検査用オート起動カード判定、量産ライン用キーショートカット起動判定等のリード
	
	(void)SYSM_GetCardTitleList( s_titleList );					// カードアプリリストの取得（カードアプリはs_titleList[0]に格納される）
	
	// 「ダイレクトブートでない」なら
	if( !pBootTitle ) {
		// アプリ間パラメタをクリア
		// TODO:あらかじめNTRカードのセキュア領域を退避せずに直接0x2000000からロードしている場合も容赦なく消すので注意
		MI_CpuClearFast((void *)HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_SIZE);
		
		// NAND & カードアプリリスト取得
		(void)SYSM_GetNandTitleList( s_titleList, LAUNCHER_TITLE_LIST_NUM );	// NANDアプリリストの取得（内蔵アプリはs_titleList[1]から格納される）
	}
	
	// 「ダイレクトブートでない」もしくは
	// 「ダイレクトブートだが、ロゴデモ表示」の時、各種リソースのロード------------
	if( !pBootTitle ||
		( pBootTitle && !SYSM_IsLogoDemoSkip() ) ) {
//		FS_ReadContentFile( ContentID );						// タイトル内リソースファイルのリード
//		FS_ReadSharedContentFile( ContentID );					// 共有コンテントファイルのリード
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
	
    // チャンネルをロックする
    SND_LockChannel((1 << L_CHANNEL) | (1 << R_CHANNEL), 0);

    /* ストリームスレッドの起動 */
    OS_CreateThread(&strmThread,
                    StrmThread,
                    NULL,
                    strmThreadStack + THREAD_STACK_SIZE / sizeof(u64),
                    THREAD_STACK_SIZE, STREAM_THREAD_PRIO);
    OS_WakeupThreadDirect(&strmThread);
	
	// メインループ--------------------
	while( 1 ) {
		OS_WaitIrq(1, OS_IE_V_BLANK);							// Vブランク割り込み待ち
		
        // ＡＲＭ７コマンド応答受信
        while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
        {
        }
        
		ReadKeyPad();											// キー入力の取得
		ReadTP();												// TP入力の取得
		
		switch( state ) {
		case LOGODEMO_INIT:
			LogoInit();
			// 音鳴らすテスト
			FS_InitFile(&strm.file);
			strm.isPlay = FALSE;
			PlayStream(&strm, filename);

			state = LOGODEMO;
			break;
		case LOGODEMO:
			if( LogoMain() ) {
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
			if( ( direct_boot || ( !direct_boot && LauncherFadeout( s_titleList ) ) ) &&
				SYSM_IsLoadTitleFinished( pBootTitle ) ) {
				state = AUTHENTICATE;
			}
			
			if( ( end == 0 ) &&
				SYSM_IsLoadTitleFinished( pBootTitle ) ) {
				end = OS_GetTick();
				OS_TPrintf( "Load Time : %dms\n", OS_TicksToMilliSeconds( end - start ) );
			}
			break;
		case AUTHENTICATE:
			switch ( SYSM_AuthenticateTitle( pBootTitle ) ) {	// アプリ認証＆ブート	成功時：never return
			case AUTH_RESULT_TITLE_LOAD_FAILED:
			case AUTH_RESULT_TITLE_POINTER_ERROR:
			case AUTH_RESULT_AUTHENTICATE_FAILED:
			case AUTH_RESULT_ENTRY_ADDRESS_ERROR:
				state = STOP;
				break;
			}
			break;
		case STOP:												// 停止
			break;
		}
		
		// カードアプリリストの取得（スレッドで随時カード挿抜を通知されるものをメインループで取得）
		(void)SYSM_GetCardTitleList( s_titleList );
		
        // コマンドフラッシュ
        (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);
	}
}


// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// Vブランク割込チェックのセット
}

