/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     ExecTmpApp.c

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
#include "ExecTmpApp.h"

// define data------------------------------------------
#define RETURN_BUTTON_TOP_X					2
#define RETURN_BUTTON_TOP_Y					21
#define RETURN_BUTTON_BOTTOM_X				( RETURN_BUTTON_TOP_X + 8 )
#define RETURN_BUTTON_BOTTOM_Y				( RETURN_BUTTON_TOP_Y + 2 )

#define COPB_MENU_ELEMENT_NUM			2						// メニューの項目数

// extern data------------------------------------------

// function's prototype declaration---------------------

static void MenuScene( void );

// global variable -------------------------------------
extern RTCDrawProperty g_rtcDraw;

// static variable -------------------------------------
static u16 s_csr = 0;
static void(*s_pNowProcess)(void);

// const data  -----------------------------------------
static const u16 *s_pStrMenu[ COPB_MENU_ELEMENT_NUM ] = 
{
	L"tmpフォルダにアプリを保存し起動",
	L"ランチャーに戻る",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  4 * 8,   8 * 8 },
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
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"ExecTmpApp");
	
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
    // メニュー項目
	DrawMenu( s_csr, &s_menuParam );
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 1 * 8, 0 * 8, TXT_COLOR_BLUE,  (const u16 *)L"ExecTmpApp");
	GetAndDrawRTCData( &g_rtcDraw, TRUE );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	s_pNowProcess = MenuScene;

	DrawMenuScene();
	
	GX_DispOn();
	GXS_DispOn();
}

static void MenuScene(void)
{
	BOOL tp_select = FALSE;
	LauncherBootFlags tempflag = {TRUE, LAUNCHER_BOOTTYPE_TEMP, TRUE, FALSE, FALSE, FALSE, 0};
	
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
			u64 targetApp = 0x00030004534d504c;// SMPL
			static char destfilename[256];
			s32 len = 0;
			s32 llen;
			BOOL success = TRUE;
			FSFile src,dest;
			void *buf;
			switch( s_csr ) {
				case 0:
					STD_TSNPrintf( destfilename, 31, "nand:/tmp/%.16llx.srl", targetApp );
					// tmpに保存
					FS_DeleteFile(destfilename);
					FS_CreateFile(destfilename, FS_PERMIT_R | FS_PERMIT_W);
					FS_InitFile( &src );
					FS_InitFile( &dest );
					if ( !FS_OpenFileEx( &src, "rom:/data/simple.srl", FS_FILEMODE_R ) ) success = FALSE;
					len = (int)FS_GetFileLength( &src );

					buf = (void *)0x02400000;// 適当にあいてるところ
					llen = FS_ReadFile( &src, buf, len );
					if ( !FS_CloseFile( &src ) ) success = FALSE;
					if (len != llen) success = FALSE;

					if ( !FS_OpenFileEx( &dest, destfilename, FS_FILEMODE_W ) ) success = FALSE;
					llen = FS_WriteFile( &dest, buf, len );
					if ( !FS_CloseFile( &dest ) ) success = FALSE;
					if (len != llen) success = FALSE;

					if( !success ) break;
					//アプリ起動
					OS_SetLauncherParamAndResetHardware( 0, targetApp, &tempflag );
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
void ExecTmpAppInit( void )
{
	ChangeUserColor( LCFG_TSD_GetUserColor() );
	MenuInit();
}

// メインループ
void ExecTmpAppMain(void)
{
	s_pNowProcess();
}
