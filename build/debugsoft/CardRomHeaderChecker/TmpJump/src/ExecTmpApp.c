/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandApp - demos - launcher_param - ExecTmpApp
  File:     ExecTmpApp.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

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
#include "misc_simple.h"
#include "ExecTmpApp.h"

// define data------------------------------------------

#define COPB_MENU_ELEMENT_NUM			2						// メニューの項目数

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
	"save app to tmp and restart",
	"return to launcher",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  3,   6 },
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
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "ExecTmpCardRomHeaderChecker");
    // メニュー項目
	myDp_DrawMenu( s_csr, MAIN_SCREEN, &s_menuParam );
}

static void MenuInit( void )
{
	FS_Init(3);
	GX_DispOff();
 	GXS_DispOff();
	
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "ExecTmpCardRomHeaderChecker");
	
	s_pNowProcess = MenuScene;

	DrawMenuScene();
	
	GX_DispOn();
	GXS_DispOn();
}

static void MenuScene(void)
{
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
			u64 targetApp = 0x00030004534d504c;// SMPL
			s32 len = 0;
			s32 llen;
			BOOL success = TRUE;
			FSFile src,dest;
			void *buf;
			switch( s_csr ) {
				case 0:

					// tmpに保存
					FS_DeleteFile(OS_TMP_APP_PATH);
					FS_CreateFile(OS_TMP_APP_PATH, FS_PERMIT_R | FS_PERMIT_W);
					FS_InitFile( &src );
					FS_InitFile( &dest );
					if ( !FS_OpenFileEx( &src, "rom:/CardRomHeaderChecker.nand.srl", FS_FILEMODE_R ) ) success = FALSE;
					len = (int)FS_GetFileLength( &src );
					
					buf = (void *)0x2400000;
					for(llen = 0; llen < len; )
					{
						int rd;
						rd = FS_ReadFile( &src, buf, len );
						if(rd == -1)
						{
							success = FALSE;
							break;
						}
						buf = (void *)((u32)buf + rd);
						llen += rd;
					}
					buf = (void *)0x2400000;
					if ( !FS_CloseFile( &src ) ) success = FALSE;
					if (len != llen) success = FALSE;

					if ( !FS_OpenFileEx( &dest, OS_TMP_APP_PATH, FS_FILEMODE_W ) ) success = FALSE;
					llen = FS_WriteFile( &dest, buf, len );
					if ( !FS_CloseFile( &dest ) ) success = FALSE;
					if (len != llen) success = FALSE;

					if( !success ) break;
					
					//アプリ起動
					OS_DoApplicationJump( targetApp, OS_APP_JUMP_TMP );
					break;
				case 1:
					OS_DoApplicationJump( NULL, OS_APP_JUMP_NORMAL );
					//再起動
					break;
			}
		}
	}
}

// 初期化
void ExecTmpAppInit( void )
{
	MenuInit();
}

// メインループ
void ExecTmpAppMain(void)
{
	s_pNowProcess();
}
