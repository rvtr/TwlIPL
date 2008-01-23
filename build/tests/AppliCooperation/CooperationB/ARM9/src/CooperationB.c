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
#include <sysmenu.h>
#include "misc.h"
#include "CooperationB.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

#define COPB_MENU_ELEMENT_NUM			2						// メニューの項目数

#define PARAM_LENGTH	10

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------
extern RTCDrawProperty g_rtcDraw;

// static variable -------------------------------------
static u16 s_csr = 0;
static u16 s_parameter[ PARAM_LENGTH + 1 ];
static void(*s_pNowProcess)(void);

// const data  -----------------------------------------
static const u16 *s_pStrMenu[ COPB_MENU_ELEMENT_NUM ] = 
{
	L"呼び出し元アプリを起動",
	L"ランチャーに戻る",
};

static MenuPos s_menuPos[] = {
	{ FALSE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
};

static const MenuParam s_menuParam = {
	COPB_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_menuPos[ 0 ],
	(const u16 **)&s_pStrMenu,
};
									
//======================================================
// アプリ連携テストプログラムB
//======================================================

static void DrawMenuScene( void )
{
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"CooperationB");
	PutStringUTF16( 1*8, 18*8, TXT_COLOR_BLACK,  (const u16 *)L"受け取ったパラメータ：");
	PutStringUTF16( 3 * 8 , 20*8, TXT_UCOLOR_G0, s_parameter );
	PutStringUTF16( 1*8, 14*8, TXT_COLOR_BLACK,  (const u16 *)L"呼び出し元アプリ：");
	
	PrintfSJIS(3*8, 16*8, TXT_COLOR_BLACK, "0x%llx",OS_IsValidDeliveryArgumentInfo() ? OS_GetTitleIdFromDeliveryArgumentInfo() : 0x0);
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
    // メニュー項目
	DrawMenu( s_csr, &s_menuParam );
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"CooperationB");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	s_pNowProcess = MenuScene;
	
	MI_CpuClear8(s_parameter, 2*(PARAM_LENGTH+1));
	
	{
		if( OS_IsValidDeliveryArgumentInfo() )
		{
			s_menuPos[ 0 ].enable = TRUE;
			OS_DecodeDeliveryBuffer();
			if(OS_GetArgv(1) != NULL)
			{
				MI_CpuCopy8(OS_GetArgv(1), s_parameter, 2*(PARAM_LENGTH+1));
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
	BOOL tp_select = FALSE;
	LauncherBootFlags tempflag = {TRUE, LAUNCHER_BOOTTYPE_NAND, TRUE, FALSE, FALSE, FALSE, 0};
	
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
		if( ++s_csr == COPB_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=COPB_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_menuParam );
	
   	DrawMenuScene();
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
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
	ChangeUserColor( TSD_GetUserColor() );
	s_parameter[0] = 0;
	MenuInit();
}

// メインループ
void CooperationBMain(void)
{
	s_pNowProcess();
}
