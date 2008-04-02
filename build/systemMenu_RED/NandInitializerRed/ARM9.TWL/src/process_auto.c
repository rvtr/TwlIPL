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
#include "process_eticket.h"
#include "process_nandfirm.h"
#include "process_norfirm.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define NUM_OF_MENU_SELECT    6
#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56

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
	case 0:
		return FormatProcess0;
	case 1:
		return HWInfoProcess0;
		break;
	case 2:
		return eTicketProcess0;
	case 3:
		return ImportProcess0;
	case 4:
		return NandfirmProcess0;
	case 5:
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
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "    FORMAT NAND            ");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "    WRITE HARDWARE INFO    ");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "    WRITE ETICKET SIGN     ");
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "    INPORT TAD FROM SD     ");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "    INPORT NANDFIRM FROM SD");
#ifndef AUTO_FORMAT_MODE
	kamiFontPrintf(3, 22, FONT_COLOR_BLACK, " Button B : return to menu");
#endif

	for (i=0;i<sMenuSelectNo-1;i++)
	{
		kamiFontPrintf(3, (s16)(7+2*i), FONT_COLOR_GREEN, "OK");
	}

	// 失敗あり
	if (i<5)
	{
		kamiFontPrintf(3, (s16)(7+2*i), FONT_COLOR_RED, "NG");
		kamiFontPrintf(3, 18, FONT_COLOR_BLACK, "    Error Occured!");
		bg_color = BG_COLOR_RED;
	}
	// 失敗なし
	else
	{
		kamiFontPrintf(3, 18, FONT_COLOR_BLACK, "   Finished Successfully!");
		bg_color = BG_COLOR_GREEN;
	}

	// 背景上部
	kamiFontFillChar( 0, bg_color, bg_color );
	kamiFontFillChar( 1, bg_color, bg_color );
	kamiFontFillChar( 2, bg_color, BG_COLOR_TRANS );

	// 背景下部
	kamiFontFillChar(17, BG_COLOR_TRANS, bg_color );
	kamiFontFillChar(18, bg_color, bg_color );
	kamiFontFillChar(19, bg_color, BG_COLOR_TRANS );

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
#ifdef AUTO_FORMAT_MODE
	// 検査ソフトではオート処理が完了した段階でTerminateさせます。
	OS_Terminate();
#endif

    if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return AutoProcess3;
}


/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

