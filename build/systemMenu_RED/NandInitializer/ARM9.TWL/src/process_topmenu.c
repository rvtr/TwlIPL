/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_topmenu.c

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

void* TopmenuProcess0(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(4, 2, 0, "Nand Initializer ver 0.1");

	// メニュー一覧
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "    FORMAT NAND            ");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "    WRITE HARDWARE INFO    ");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "    WRITE ETICKET SIGN     ");
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "    INPORT TAD FROM SD     ");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "    INPORT NANDFIRM FROM SD");
	kamiFontPrintf(3, 17, FONT_COLOR_BLACK, "    INPORT NORFIRM FROM SD");

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// 背景設定
	kamiFontFillChar( 6, BG_COLOR_TRANS, BG_COLOR_BLUE );
	kamiFontFillChar( 7, BG_COLOR_BLUE,  BG_COLOR_BLUE );
	kamiFontFillChar( 8, BG_COLOR_BLUE,  BG_COLOR_TRANS );

	kamiFontFillChar( 8, BG_COLOR_NONE,  BG_COLOR_PURPLE );
	kamiFontFillChar( 9, BG_COLOR_PURPLE, BG_COLOR_PURPLE );
	kamiFontFillChar(10, BG_COLOR_PURPLE, BG_COLOR_TRANS );

	kamiFontFillChar(10, BG_COLOR_NONE,  BG_COLOR_GRAY );
	kamiFontFillChar(11, BG_COLOR_GRAY, BG_COLOR_GRAY );
	kamiFontFillChar(12, BG_COLOR_GRAY, BG_COLOR_TRANS );

	kamiFontFillChar(12, BG_COLOR_NONE,  BG_COLOR_PINK );
	kamiFontFillChar(13, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar(14, BG_COLOR_PINK, BG_COLOR_TRANS );

	kamiFontFillChar(14, BG_COLOR_NONE,  BG_COLOR_GREEN );
	kamiFontFillChar(15, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar(16, BG_COLOR_GREEN, BG_COLOR_TRANS );

	kamiFontFillChar(16, BG_COLOR_NONE,  BG_COLOR_VIOLET );
	kamiFontFillChar(17, BG_COLOR_VIOLET, BG_COLOR_VIOLET );
	kamiFontFillChar(18, BG_COLOR_VIOLET, BG_COLOR_TRANS );

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN(TopmenuProcess1);
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* TopmenuProcess1(void)
{
	// 選択メニューの変更
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = NUM_OF_MENU_SELECT -1;
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNo >= NUM_OF_MENU_SELECT) sMenuSelectNo = 0;
	}

	// カーソル配置
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// 決定
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return TopmenuProcess2;
	}

	// L&R同時押しでオート実行！
    if (kamiPadIsPress(PAD_BUTTON_L) && kamiPadIsPress(PAD_BUTTON_R))
	{
		FADE_OUT_RETURN( AutoProcess0 );
	}

	return TopmenuProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu プロセス２

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* TopmenuProcess2(void)
{
	switch ( sMenuSelectNo )
	{
	case 0:
		FADE_OUT_RETURN( FormatProcess0 );
	case 1:
		FADE_OUT_RETURN( HWInfoProcess0 );
	case 2:
		FADE_OUT_RETURN( eTicketProcess0 );
	case 3:
		FADE_OUT_RETURN( ImportProcess0 );
	case 4:
		FADE_OUT_RETURN( NandfirmProcess0 );
	case 5:
		FADE_OUT_RETURN( NorfirmProcess0 );
	}

	return TopmenuProcess1;
}

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

