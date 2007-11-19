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
static TitleProperty *CheckShortcutBoot( TitleProperty *pTitleList );
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
	SYSM_ReadParameters();										// 本体設定データ等のリード
	(void)SYSM_GetNandTitleList( pTitleList, LAUNCHER_TITLE_LIST_NUM );	// NANDアプリリストの取得（内蔵アプリはpTitleList[1]から格納される）
	(void)SYSM_GetCardTitleList( pTitleList );					// カードアプリリストの取得（カードアプリはpTitleList[0]に格納される）
	
	// リセットパラメータ＆ショートカットチェック----------
	if( SYSM_GetResetParamBody()->v1.bootTitleID ) {			// アプリ直接起動の指定があったらロゴデモを飛ばして指定アプリ起動
		pBootTitle = (TitleProperty *)&SYSM_GetResetParamBody()->v1;
	}else {
		pBootTitle = CheckShortcutBoot( pTitleList );
	}
	
	// ダイレクトブートでロゴデモスキップでない時、各種リソースのロード------------
	if( !( pBootTitle && !pBootTitle->flags.isLogoSkip ) ) {
//		FS_ReadContentFile( ContentID );						// タイトル内リソースファイルのリード
//		FS_ReadSharedContentFile( ContentID );					// 共有コンテントファイルのリード
	}
	
	// 開始ステートの判定--------------
	if( pBootTitle ) {
		// ダイレクト起動タイトルの指定があるなら、ロゴ、ランチャーを飛ばして起動
		if( pBootTitle->flags.isAppLoadCompleted ) {
			// ロード済み状態なら、直接認証へ
			state = AUTHENTICATE;
		}else {
			// さもなくば、ロード開始
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
			SYSM_StartLoadTitle( pBootTitle );
			state = LOADING;
			break;
		case LOADING:
			LauncherLoading( pTitleList );
			if( SYSM_IsLoadTitleFinished() )
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
		
		// カードアプリリストの取得（スレッドで随時カード挿抜を通知されるものをメインループで取得）
		(void)SYSM_GetCardTitleList( pTitleList );
	}
}


// ショートカット起動のチェック
static TitleProperty *CheckShortcutBoot( TitleProperty *pTitleList )
{
#if 0	// ※未実装
	TitleProperty *pTgt;
	
	ReadKeyPad();													// キー入力の取得
	
	//-----------------------------------------------------
	// TWL設定データ未入力時の初回起動シーケンス起動
	//-----------------------------------------------------
#ifdef ENABLE_INITIAL_SETTINGS_
	if( !TSD_IsSetTP() ||
		!TSD_IsSetLanguage() ||
		!TSD_IsSetDateTime() ||
		!TSD_IsSetUserColor() ||
		!TSD_IsSetNickname() ) {
		return SYSM_GetTitleProperty( TITLE_ID_MACHINE_SETTINGS, pTitleList );	// ※未実装
	}
#endif // ENABLE_INITIAL_SETTINGS_
	
	//-----------------------------------------------------
	// 量産工程用ショートカットキー or
	// 検査カード起動
	//-----------------------------------------------------
	if( ( SYSM_IsExistCard() &&
		  ( ( pad.cont & PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) == PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) ) ||
		SYSM_IsInspectCard() ) {
		pTgt = SYSM_GetTitleProperty();	// ※未実装
		if( pTgt ) {
			pTgt->flags.isLogoSkip = TRUE;							// ロゴデモを飛ばす
			pTgt->flags.isInitialShortcutSkip = TRUE;				// 初回起動シーケンスを飛ばす
		}
		return pTgt;
	}
#endif	// 0
	return NULL;													// 「ブート内容未定」でリターン
}


// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// Vブランク割込チェックのセット
}

