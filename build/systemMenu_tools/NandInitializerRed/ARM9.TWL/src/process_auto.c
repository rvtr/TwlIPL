/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_auto.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include "kami_font.h"
#include "process_topmenu.h"
#include "process_format.h"
#include "process_hw_info.h"
#include "process_import.h"
#include "process_font.h"
#include "process_nandfirm.h"
#include "process_norfirm.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

enum {
	MENU_FORMAT = 0,
	MENU_HARDWARE_INFO,
#ifdef    USE_WRITE_FONT_DATA
	MENU_FONT_DATA,
#endif // USE_WRITE_FONT_DATA
	MENU_IMPORT_TAD,
#ifndef   MARIOCLUB_VERSION
	MENU_IMPORT_NANDFIRM,
#endif // MARIOCLUB_VERSION
	MENU_END
};

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    グローバル変数定義
 *---------------------------------------------------------------------------*/

BOOL gAutoFlag = FALSE;

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Top Menu プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* AutoProcess0(void)
{
	// オートフラグセット
	gAutoFlag = TRUE;

	// メニュー初期化
	sMenuSelectNo = 0;

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	return AutoProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* AutoProcess1(void)
{
	switch ( sMenuSelectNo++ )
	{
	case MENU_FORMAT:
		return FormatProcess0;
	case MENU_HARDWARE_INFO:
		return HWInfoProcess0;

#ifdef    USE_WRITE_FONT_DATA
	case MENU_FONT_DATA:
		return fontProcess0;	
#endif // USE_WRITE_FONT_DATA

	case MENU_IMPORT_TAD:
		return ImportProcess0;

#ifndef   MARIOCLUB_VERSION
	case MENU_IMPORT_NANDFIRM:
		return NandfirmProcess0;
#endif // MARIOCLUB_VERSION

	case MENU_END:
		return AutoProcess2;
	}

	// never reach
	return TopmenuProcess0;
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu プロセス2

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* AutoProcess2(void)
{
	int i;
	s8 line = 5;
	u8 bg_color;

	// 文字列全クリア
	kamiFontClear();

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Auto Nand Initialization");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    FORMAT NAND            "); 
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    WRITE HARDWARE INFO    ");
#ifdef    USE_WRITE_FONT_DATA
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    WRITE FONT DATA        ");
#endif // USE_WRITE_FONT_DATA
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    INPORT TAD FROM SD     ");
#ifndef MARIOCLUB_VERSION
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    INPORT NANDFIRM FROM SD");
#endif //MARIOCLUB_VERSION
#ifndef AUTO_FORMAT_MODE
	kamiFontPrintf(3, 22, FONT_COLOR_BLACK, " Button B : return to menu");
#endif //AUTO_FORMAT_MODE

	for (i=0;i<sMenuSelectNo-1;i++)
	{
		kamiFontPrintf(3, (s16)(7+2*i), FONT_COLOR_GREEN, "OK");
	}

	// 失敗あり
	if (i<MENU_END)
	{
		kamiFontPrintf(3, (s16)(7+2*i), FONT_COLOR_RED, "NG");
		kamiFontPrintf(3, 19, FONT_COLOR_BLACK, "    Error Occured!");
		bg_color = BG_COLOR_RED;
	}
	// 失敗なし
	else
	{
		kamiFontPrintf(3, 19, FONT_COLOR_BLACK, "   Finished Successfully!");
		bg_color = BG_COLOR_GREEN;
	}

	// 背景上部
	kamiFontFillChar( 0, bg_color, bg_color );
	kamiFontFillChar( 1, bg_color, bg_color );
	kamiFontFillChar( 2, bg_color, BG_COLOR_TRANS );

	// 背景下部
	kamiFontFillChar(18, BG_COLOR_TRANS, bg_color );
	kamiFontFillChar(19, bg_color, bg_color );
	kamiFontFillChar(20, bg_color, BG_COLOR_TRANS );

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	// オートフラグクリア
	gAutoFlag = FALSE;

	FADE_IN_RETURN( AutoProcess3 );
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu プロセス3

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* AutoProcess3(void)
{
	// NandInitializerProdectはオート処理が完了した段階で終了です。
#ifndef AUTO_FORMAT_MODE
    if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}
#endif

	return AutoProcess3;
}


/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

