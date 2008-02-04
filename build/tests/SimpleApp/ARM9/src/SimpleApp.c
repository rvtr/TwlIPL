/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SimpleApp.c

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
#include "misc_simple.h"
#include "SimpleApp.h"

// define data------------------------------------------

#define COPB_MENU_ELEMENT_NUM			1						// メニューの項目数

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------

// static variable -------------------------------------
static u16 s_csr = 0;
static void(*s_pNowProcess)(void);

// const data  -----------------------------------------
static const char *s_pStrMenu[ COPB_MENU_ELEMENT_NUM ] = 
{
	"return to launcher",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  3,   8 },
};

static const MenuParam s_menuParam = {
	COPB_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_menuPos[ 0 ],
	(const char **)&s_pStrMenu,
};
									
//======================================================
// アプリ連携テストプログラムB
//======================================================

static void DrawMenuScene( void )
{
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "SimpleApp");
    // メニュー項目
	myDp_DrawMenu( s_csr, MAIN_SCREEN, &s_menuParam );
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
	
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "SimpleApp");
	
	s_pNowProcess = MenuScene;

	DrawMenuScene();
	
	GX_DispOn();
	GXS_DispOn();
}

static void MenuScene(void)
{
	LauncherBootFlags tempflag = {TRUE, OS_BOOTTYPE_NAND, TRUE, FALSE, FALSE, FALSE, 0};
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( MYPAD_IS_TRIG(PAD_KEY_DOWN) ){									// カーソルの移動
		if( ++s_csr == COPB_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( MYPAD_IS_TRIG(PAD_KEY_UP) ){
		if( --s_csr & 0x80 ) {
			s_csr=COPB_MENU_ELEMENT_NUM - 1;
		}
	}
	
   	DrawMenuScene();
	
	if( MYPAD_IS_TRIG(PAD_BUTTON_A) ) {				// メニュー項目への分岐
		if( s_menuPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					OS_SetLauncherParamAndResetHardware( 0, NULL, &tempflag );
					//再起動
					break;
			}
		}
	}
}

// 初期化
void SimpleAppInit( void )
{
	MenuInit();
}

// メインループ
void SimpleAppMain(void)
{
	s_pNowProcess();
}
