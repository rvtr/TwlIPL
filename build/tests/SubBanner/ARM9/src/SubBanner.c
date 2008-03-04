/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SubBanner.c

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
#include <twl/os/common/banner.h>
#include "misc_simple.h"
#include "SubBanner.h"

// define data------------------------------------------

#define COPB_MENU_ELEMENT_NUM			3						// メニューの項目数

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
	"write sub banner file",
	"delete sub banner file",
	"return to launcher",
};

static MenuPos s_menuPos[] = {
	{ TRUE,  3,   8 },
	{ TRUE,  3,   9 },
	{ TRUE,  3,   10 },
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
// サブバナーテストプログラム
//======================================================

static void DrawMenuScene( void )
{
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "SubBanner");
    // メニュー項目
	myDp_DrawMenu( s_csr, MAIN_SCREEN, &s_menuParam );
}

static void MenuInit( void )
{
	GX_DispOff();
 	GXS_DispOff();
	FS_Init(3);
	
	myDp_Printf( 1, 0, TXT_COLOR_BLUE, MAIN_SCREEN, "SubBanner");
	
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
				TWLSubBannerFile subBanner;
				FSFile file[1];
				s32 readLen;
				case 0:
					//subbanner書き込み
					FS_InitFile(file);
					if (FS_OpenFileEx(file, "rom:/data/sub_banner.bnr", FS_FILEMODE_R) )
					{
						readLen = FS_ReadFile(file, &subBanner, sizeof(TWLSubBannerFile));
						FS_CloseFile(file);
						if( readLen == sizeof(TWLSubBannerFile) )
						{
							// 成功
							if (FS_OpenFileEx(file, "nand:/<banner>", FS_FILEMODE_W) )
							{
								if( sizeof(TWLSubBannerFile) == FS_WriteFile(file, &subBanner, sizeof(TWLSubBannerFile)) )
								{
									myDp_Printf( 1, 3, TXT_COLOR_RED, MAIN_SCREEN, "write succeed.          ");
								}else
								{
									myDp_Printf( 1, 3, TXT_COLOR_RED, MAIN_SCREEN, "write failed.           ");
								}
								FS_CloseFile(file);
							}else
							{
								myDp_Printf( 1, 3, TXT_COLOR_RED, MAIN_SCREEN, "banner file open failed.");
							}
						}else
						{
							myDp_Printf( 1, 3, TXT_COLOR_RED, MAIN_SCREEN, "read romfile failed.    ");
						}
					}else
					{
						myDp_Printf( 1, 3, TXT_COLOR_RED, MAIN_SCREEN, "open romfile failed.    ");
					}
					break;
				case 1:
					//subbanner消去
					FS_InitFile(file);
					MI_CpuClear8( &subBanner, sizeof(TWLSubBannerFile) );
					if (FS_OpenFileEx(file, "nand:/<banner>", FS_FILEMODE_W) )
					{
						if( sizeof(TWLSubBannerFile) == FS_WriteFile(file, &subBanner, sizeof(TWLSubBannerFile)) )
						{
							myDp_Printf( 1, 3, TXT_COLOR_RED, MAIN_SCREEN, "write succeed.          ");
						}else
						{
							myDp_Printf( 1, 3, TXT_COLOR_RED, MAIN_SCREEN, "write failed.           ");
						}
						FS_CloseFile(file);
					}else
					{
						myDp_Printf( 1, 3, TXT_COLOR_RED, MAIN_SCREEN, "banner file open failed.");
					}
					break;
				case 2:
					OS_SetLauncherParamAndResetHardware( NULL, &tempflag );
					//再起動
					break;
			}
		}
	}
}

// 初期化
void SubBannerInit( void )
{
	MenuInit();
}

// メインループ
void SubBannerMain(void)
{
	s_pNowProcess();
}
