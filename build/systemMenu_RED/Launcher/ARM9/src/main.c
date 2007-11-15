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
		AUTHENTICATE = 4,
		BOOT = 5,
		STOP = 6
	};
	u32 state = START;
	TitleProperty *pBootTitle = NULL;
	TitleProperty pTitleList[ LAUNCHER_TITLE_LIST_NUM ];
	
	// システムメニュー初期化----------
	SYSM_Init( Alloc, Free );											// OS_Initの前でコール。
	
	// OS初期化------------------------
    OS_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
	FS_Init( FS_DMA_NOT_USE );
    GX_Init();
	TP_Init();
	RTC_Init();
    
	// 割り込み許可--------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// システムの初期化----------------
	InitAllocator();
	
	// 各種パラメータの取得--------
	SYSM_ReadParameters();
	if( SYSM_GetResetParam()->flags.isLogoSkip ) {
		if( SYSM_GetResetParam()->bootTitleID ) {							// アプリ直接起動の指定があったらロゴデモを飛ばして指定アプリ起動
			pBootTitle = (TitleProperty *)SYSM_GetResetParam();
			state = AUTHENTICATE;
		}else {																// それ以外の場合は、ロゴデモを飛ばしてランチャー起動
			state = LAUNCHER_INIT;
		}
	}
	
	// コンテント（リソース）ファイルのリード
//	FS_ReadContentFile( ContentID );
	
	// 共有コンテントファイルのリード
//	FS_ReadSharedContentFile( ContentID );
	
	// NANDアプリリストの取得----------
	(void)SYSM_GetNandTitleList( pTitleList, LAUNCHER_TITLE_LIST_NUM );
	
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
				state = AUTHENTICATE;
			}
			break;
		case AUTHENTICATE:
			switch ( SYSM_LoadAndAuthenticateTitle( pBootTitle ) ) {	// アプリロード＆認証	成功時：never return
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


// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// Vブランク割込チェックのセット
}

