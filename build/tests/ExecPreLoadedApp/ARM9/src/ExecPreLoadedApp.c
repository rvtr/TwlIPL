/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     ExecPreLoadedApp.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-10-31#$
  $Rev: 91 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include "misc.h"
#include "ExecPreLoadedApp.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

#define MLEP_MENU_ELEMENT_NUM			7						// メニューの項目数

#define PARAM_LENGTH	10

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------
extern RTCDrawProperty g_rtcDraw;

// static variable -------------------------------------
static u16 s_csr = 0;
static void(*s_pNowProcess)(void);

// const data  -----------------------------------------
static const u16 *s_pStrMenu[ MLEP_MENU_ELEMENT_NUM ] = 
{
	L"再配置チェッカ0（再配置無し）",
	L"再配置チェッカ1（エラー）",
	L"再配置チェッカ2（正順コピー1）",
	L"再配置チェッカ3（正順コピー2）",
	L"再配置チェッカ4（エラー）",
	L"再配置チェッカ5（WRAMへ配置）",
	L"ランチャーに戻る",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
	{ TRUE,  4 * 8,  18 * 8 },
	{ TRUE,  4 * 8,  20 * 8 },
};

static const MenuParam s_menuParam = {
	MLEP_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_menuPos[ 0 ],
	(const u16 **)&s_pStrMenu,
};
									
//======================================================
// テストプログラム
//======================================================

// ============================================================================
//
// アプリ起動
//
// ============================================================================

static void DrawMenuScene( void )
{
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"ExecPreLoadedApp");
    // メニュー項目
	DrawMenu( s_csr, &s_menuParam );
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"ExecPreLoadedApp");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	s_pNowProcess = MenuScene;

	DrawMenuScene();
	
	GX_DispOn();
	GXS_DispOn();
}


static BOOL LoadTitle( NAMTitleId bootTitleID )
{
	// ロード
    char path[256];
    FSFile  file[1];
    BOOL bSuccess;
    BOOL isTwlApp = TRUE;
    NAM_GetTitleBootContentPath(path, bootTitleID);

	if( !OS_SetRelocateInfoAndLoadApplication( path ) )
	{
		return FALSE;
	}
	
	SYSMi_GetWork()->flags.common.isLoadSucceeded = TRUE;
    return TRUE;
}

static void MenuScene(void)
{
	BOOL tp_select = FALSE;
	LauncherBootFlags tempflag = {TRUE, LAUNCHER_BOOTTYPE_NAND, TRUE, FALSE, TRUE, FALSE, 0};
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
		if( ++s_csr == MLEP_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=MLEP_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_menuParam );
	
   	DrawMenuScene();
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
		if( s_menuPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					//アプリ起動
					if(LoadTitle(0x0003000452434b30))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b30, &tempflag ); // RCK0
					break;
				case 1:
					//アプリ起動
					if(LoadTitle(0x0003000452434b31))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b31, &tempflag ); // RCK1
					break;
				case 2:
					//アプリ起動
					if(LoadTitle(0x0003000452434b32))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b32, &tempflag ); // RCK2
					break;
				case 3:
					//アプリ起動
					if(LoadTitle(0x0003000452434b33))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b33, &tempflag ); // RCK3
					break;
				case 4:
					//アプリ起動
					if(LoadTitle(0x0003000452434b34))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b34, &tempflag ); // RCK4
					break;
				case 5:
					//アプリ起動
					if(LoadTitle(0x0003000452434b35))
						OS_SetLauncherParamAndResetHardware( 0, 0x0003000452434b35, &tempflag ); // RCK5
					break;
				case 6:
					OS_SetLauncherParamAndResetHardware( 0, NULL, &tempflag );
					//再起動
					break;
			}
		}
	}
}

// 初期化
void ExecPreLoadedAppInit( void )
{
	ChangeUserColor( LCFG_TSD_GetUserColor() );
	MenuInit();
}

// メインループ
void ExecPreLoadedAppMain(void)
{
	s_pNowProcess();
}
