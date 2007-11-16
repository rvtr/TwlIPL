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

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static BOOL CheckBootStatus( void );
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------

// メイン
void TwlMain( void )
{
	enum {
		START = 0,
		LOGODEMO = 1,
		LAUNCHER_INIT = 2,
		LAUNCHER = 3,
		LOAD_START = 4,
		LOADING = 5,
		AUTHENTICATE = 6,
		BOOT = 7,
		STOP = 8
	};
	u32 state = START;
	TitleProperty *pBootTitle = NULL;
	TitleProperty pTitleList[ LAUNCHER_TITLE_LIST_NUM ];
	OSThread *thread;
	
	// システムメニュー初期化----------
	SYSM_Init( Alloc, Free );											// OS_Initの前でコール。
	
	// OS初期化------------------------
    OS_Init();
    PM_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
	FS_Init( FS_DMA_NOT_USE );
    GX_Init();
	PM_Init();
	TP_Init();
	RTC_Init();
    
	// 割り込み許可--------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// システムの初期化----------------
	InitAllocator();											// ※SYSM_Init以外のSYSMライブラリ関数を呼ぶ前に
																//   Alloc, Freeで登録したメモリアロケータを初期化してください。
	// 各種パラメータの取得------------
	SYSM_ReadParameters();
	if( SYSM_GetResetParamBody()->v1.bootTitleID ) {			// アプリ直接起動の指定があったらロゴデモを飛ばして指定アプリ起動
		pBootTitle = (TitleProperty *)&SYSM_GetResetParamBody()->v1;
	}
	
	// 各種リソースのロード------------
	if( pBootTitle == NULL ) {
//		FS_ReadContentFile( ContentID );						// タイトル内リソースファイルのリード
//		FS_ReadSharedContentFile( ContentID );					// 共有コンテントファイルのリード
		
		// NANDアプリリストの取得
		(void)SYSM_GetNandTitleList( pTitleList, LAUNCHER_TITLE_LIST_NUM );
	}
	
	// 開始ステートの判定--------------
	if( pBootTitle ) {
		// リセットパラメータでダイレクト起動タイトルの指定があるなら、ロゴ、ランチャーを飛ばして起動
		if( SYSM_GetResetParamBody()->v1.flags.isAppLoadCompleted ) {
			state = AUTHENTICATE;
		}else {
			state = LOAD_START;
		}
	}else if( SYSM_IsLogoDemoSkip() ) {
		// リセットパラメータでロゴデモスキップが指定されていたら、ランチャー起動
		state = LAUNCHER_INIT;
	}else {
		// 何もないなら、ロゴデモ起動
		state = START;
	}
	
	// メインループ--------------------
	while( 1 ) {
		OS_WaitIrq(1, OS_IE_V_BLANK);							// Vブランク割り込み待ち
		
		ReadKeyPad();											// キー入力の取得
		ReadTP();												// TP入力の取得
		
		(void)SYSM_GetCardTitleList( pTitleList );				// カードアプリリストの取得（スレッドで随時カード挿抜を通知されるものをメインループで取得）
		
		switch( state ) {
		case START:
			state = LOGODEMO;
			break;
		case LOGODEMO:
			if( LogoMain() ) {
				state = LAUNCHER_INIT;
			}
			break;
		case LAUNCHER_INIT:
			InitBG();										// BG初期化
			LauncherInit( pTitleList );
			state = LAUNCHER;
			break;
		case LAUNCHER:
			pBootTitle = LauncherMain( pTitleList );
			if( pBootTitle ) {
				state = LOAD_START;
			}
			break;
		case LOAD_START:
			thread = SYSM_StartLoadTitle( pBootTitle );
			state = LOADING;
			break;
		case LOADING:
			LauncherLoading( pTitleList );
			if(OS_IsThreadTerminated( thread ))
			{
				GX_DispOff();
				GXS_DispOff();
				state = AUTHENTICATE;
			}
			break;
		case AUTHENTICATE:
			switch ( SYSM_AuthenticateTitle( pBootTitle ) ) {	// アプリ認証＆ブート	成功時：never return
			case AUTH_PROCESSING:
				break;
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
	}
}


// ブート状態を確認し、ロゴ表示有無を判断する-------
static BOOL CheckBootStatus(void)
{
#if 0
	BOOL boot_decision		= FALSE;								// 「ブート内容未定」に
	BOOL other_shortcut_off	= FALSE;
	
	//-----------------------------------------------------
	// デバッグ用コンパイルスイッチによる挙動
	//-----------------------------------------------------
	{
		
#ifdef __LOGO_SKIP													// ※デバッグ用ロゴスキップ
		SetLogoEnable( FALSE );										// ロゴ表示スキップ
#endif /* __LOGO_SKIP */
	}
	
	
	//-----------------------------------------------------
	// NITRO設定データ未入力時の設定メニューショートカット起動
	//-----------------------------------------------------
#ifdef __DIRECT_BOOT_BMENU_ENABLE									// ※NITRO設定データ未入力時のブートメニュー直接起動スイッチがONか？
	if( !TSD_IsSetTP() ||
		!TSD_IsSetLanguage() ||
		!TSD_IsSetDateTime() ||
		!TSD_IsSetUserColor() ||
		!TSD_IsSetNickname() ) {									// TP,言語,RTC,ニックネームがセットされていなければ、ロゴ表示もゲームロードも行わず、ブートメニューをショートカット起動。
		
		if( ( pad.cont & PAD_PRODUCTION_NITRO_SHORTCUT ) == PAD_PRODUCTION_NITRO_SHORTCUT ) {
			other_shortcut_off = TRUE;								// 量産工程用のキーショートカットが押されていたら、設定メニュー起動はなし。
		}else if( !SYSM_IsInspectNITROCard() )  {					// 但し、量産用のキーショートカットが押されている時か、NITRO検査カードがささっている時は、ブートメニューへのショートカット起動は行わない。
			SYSM_SetBootFlag( BFLG_BOOT_BMENU );
			SetLogoEnable( FALSE );
			return TRUE;											// 「ブート内容決定」でリターン
		}
	}
#endif /* __DIRECT_BOOT_BMENU_ENABLE */
	
	
	//-----------------------------------------------------
	// キーショートカット起動
	//-----------------------------------------------------
	if( !other_shortcut_off
//		&& !TSD_IsAutoBoot()
		) {
																	// 他ショートカットONかつオート起動OFFの時
		u32 nowBootFlag = 0;
		
		if(pad.cont & PAD_BUTTON_R){								// Rボタン押下起動なら、ロゴ表示なしでAGBゲームへ
			SetLogoEnable( FALSE );
			nowBootFlag = BFLG_BOOT_AGB;
		}else if(pad.cont & PAD_BUTTON_L){							// Lボタン押下起動なら、ロゴ表示後にNITROゲームへ
			nowBootFlag = BFLG_BOOT_NITRO;
		}else if(pad.cont & PAD_BUTTON_B){							// Bボタン押下起動なら、ロゴ表示後にブートメニューへ
			nowBootFlag = BFLG_BOOT_BMENU;
		}
		if( nowBootFlag ) {
			SYSM_SetBootFlag( nowBootFlag );
			return TRUE;											// 「ブート内容決定」でリターン
		}
	}
	
	
	//-----------------------------------------------------
	// 自動起動オプション有効時の挙動
	//-----------------------------------------------------
#ifndef __SYSM_DEBUG
//	if( TSD_IsAutoBoot() ) {
	if( 0 ) {
		if ( SYSM_IsExistCard() ) {									// NITROカードのみの時はNITRO起動
			SYSM_SetBootFlag( BFLG_BOOT_NITRO );
			return TRUE;											// 「ブート内容決定」でリターン
		}
	}
#endif /* __SYSM_DEBUG */
#endif
	return FALSE;													// 「ブート内容未定」でリターン
}


// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// Vブランク割込チェックのセット
}

