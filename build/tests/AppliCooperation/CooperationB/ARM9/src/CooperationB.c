/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     CooperationB.c

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
#include "CooperationB.h"

// define data------------------------------------------
#define COPB_MENU_ELEMENT_NUM			2						// メニューの項目数

#define PARAM_LENGTH	10

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------

// static variable -------------------------------------
static u16 s_csr = 0;
static char s_parameter[ PARAM_LENGTH + 1 ];
static void(*s_pNowProcess)(void);

// const data  -----------------------------------------
static const char *s_pStrMenu[ COPB_MENU_ELEMENT_NUM ] = 
{
	"Launch Former App",
	"Return to Launcher",
};

static MenuPos s_menuPos[] = {
	{ FALSE,  3,   6 },
	{ TRUE,  3,  8 },
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
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "CooperationB");
	myDp_Printf( 1, 18, TXT_COLOR_BLACK, MAIN_SCREEN, "Received Paramater:");
	myDp_Printf( 3 , 19, TXT_COLOR_DARKLIGHTBLUE, MAIN_SCREEN, s_parameter );
	myDp_Printf( 1, 14, TXT_COLOR_BLACK, MAIN_SCREEN, "Former App:");
	myDp_Printf(3, 15, TXT_COLOR_BLACK, MAIN_SCREEN, "0x%llx",OS_IsValidDeliveryArgumentInfo() ? OS_GetTitleIdFromDeliveryArgumentInfo() : 0x0);
    // メニュー項目
	myDp_DrawMenu( s_csr, MAIN_SCREEN, &s_menuParam );
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
	
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "CooperationB");
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	s_pNowProcess = MenuScene;
	
	MI_CpuClear8(s_parameter, (PARAM_LENGTH+1));
	
	{
		if( OS_IsValidDeliveryArgumentInfo() )
		{
			s_menuPos[ 0 ].enable = TRUE;
			OS_DecodeDeliveryBuffer();
			if(OS_GetArgv(1) != NULL)
			{
				MI_CpuCopy8(OS_GetArgv(1), s_parameter, (PARAM_LENGTH+1));
			}
			else
			{
				s_parameter[0] = '\0';
			}
		}
	}
	
	DrawMenuScene();
	
	GX_DispOn();
	GXS_DispOn();
}

static void MenuScene(void)
{
	LauncherBootFlags tempflag = {TRUE, LAUNCHER_BOOTTYPE_NAND, TRUE, FALSE, FALSE, FALSE, 0};
	
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
					if(OS_IsValidDeliveryArgumentInfo())
					{
						u16 *maker_code_src_addr = (u16 *)(HW_TWL_ROM_HEADER_BUF + 0x10);
						u32 *game_code_src_addr = (u32 *)(HW_TWL_ROM_HEADER_BUF + 0xc);
						u64 targetApp = OS_GetTitleIdFromDeliveryArgumentInfo();
						// アプリ間パラメータの初期化
						OS_InitArgBufferForDelivery( OS_DELIVER_ARG_BUFFER_SIZE );
						// validフラグを立てる
						OS_SetValidDeliveryArgumentInfo( TRUE );
						// メーカーコードとゲームコードのセット(Launcher側でやるべき？)
						OS_SetMakerCodeToDeliveryArgumentInfo( *maker_code_src_addr );
						OS_SetGameCodeToDeliveryArgumentInfo( *game_code_src_addr );
						OS_SetTitleIdToDeliveryArgumentInfo( 0x00030004434f5042 );
						// アプリ専用引数のセット
						OS_SetDeliveryArgments( "-r" );
						
						//呼び出し元アプリ起動
						OS_SetLauncherParamAndResetHardware( 0, targetApp, &tempflag );
					}
					break;
				case 1:
					OS_SetLauncherParamAndResetHardware( 0, NULL, &tempflag );
					//再起動
					break;
			}
		}
	}
}

// 初期化
void CooperationBInit( void )
{
	s_parameter[0] = 0;
	MenuInit();
}

// メインループ
void CooperationBMain(void)
{
	s_pNowProcess();
}
