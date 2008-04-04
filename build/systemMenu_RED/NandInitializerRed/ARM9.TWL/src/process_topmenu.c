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
#ifdef    USE_WIRELESS_FORCE_DISABLE_SETTING
#include "process_wireless_setting.h"
#endif // USE_WIRELESS_FORCE_DISABLE_SETTING

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

enum {
	MENU_FORMAT = 0,
	MENU_HARDWARE_INFO,
#ifdef    USE_WIRELESS_FORCE_DISABLE_SETTING
	MENU_WIRELESS_SETTING,
#endif // USE_WIRELESS_FORCE_DISABLE_SETTING
#ifndef   NAND_INITIALIZER_LIMITED_MODE
	MENU_ETICKET,
	MENU_IMPORT_TAD,
	MENU_IMPORT_NANDFIRM,
	MENU_IMPORT_NORFIRM,
#endif // NAND_INITIALIZER_LIMITED_MODE
	NUM_OF_MENU_SELECT
};

typedef struct _MenuAndColor
{
	char* menu_name;
	u8    color;
} MenuAndColor;

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56
#define MENU_TOP_LINE         7
#define CHAR_OF_MENU_SPACE    2

static const MenuAndColor sMenuArray[] =
{
	{"    FORMAT NAND            ", BG_COLOR_BLUE   },
	{"    WRITE HARDWARE INFO    ", BG_COLOR_PURPLE },
#ifdef    USE_WIRELESS_FORCE_DISABLE_SETTING
	{"    WIRELESS FORCE SETTING ", BG_COLOR_YELLOW },
#endif // USE_WIRELESS_FORCE_DISABLE_SETTING
#ifndef   NAND_INITIALIZER_LIMITED_MODE
	{"    WRITE ETICKET SIGN     ", BG_COLOR_GRAY   },
	{"    INPORT TAD FROM SD     ", BG_COLOR_PINK   },
	{"    INPORT NANDFIRM FROM SD", BG_COLOR_GREEN  },
	{"    INPORT NORFIRM  FROM SD", BG_COLOR_VIOLET }
#endif // NAND_INITIALIZER_LIMITED_MODE
};

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;

#ifdef AUTO_FORMAT_MODE
static BOOL sAutoProcessFlag = TRUE;
#endif

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
#ifndef NAND_INITIALIZER_LIMITED_MODE
	kamiFontPrintf(7, 2, 0, "Nand Initializer RED");
#else
	kamiFontPrintf(8, 2, 0, "Nand Initializer");
#endif
	kamiFontPrintf(9, 4, 8, "<%s>", __DATE__);

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	for (i=0;i<NUM_OF_MENU_SELECT;i++)
	{
		// メニュー文字列描画
		kamiFontPrintf(3,  (s16)(MENU_TOP_LINE+CHAR_OF_MENU_SPACE*i), FONT_COLOR_BLACK, sMenuArray[i].menu_name);
		// 背景設定
		kamiFontFillChar( MENU_TOP_LINE+CHAR_OF_MENU_SPACE*i-1, BG_COLOR_NONE, sMenuArray[i].color );
		kamiFontFillChar( MENU_TOP_LINE+CHAR_OF_MENU_SPACE*i,   sMenuArray[i].color,  sMenuArray[i].color );
		kamiFontFillChar( MENU_TOP_LINE+CHAR_OF_MENU_SPACE*i+1, sMenuArray[i].color,  BG_COLOR_NONE );
	}

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


#ifndef NAND_INITIALIZER_LIMITED_MODE

#ifdef AUTO_FORMAT_MODE
	// sAutoProcessFlag でオート実行
	if (sAutoProcessFlag)
	{
		sAutoProcessFlag = FALSE;
		FADE_OUT_RETURN( AutoProcess0 );
	}
#endif

	// L&R同時押しでオート実行！
    if (kamiPadIsPress(PAD_BUTTON_L) && kamiPadIsPress(PAD_BUTTON_R))
	{
		FADE_OUT_RETURN( AutoProcess0 );
	}
#endif

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
	case MENU_FORMAT:
		FADE_OUT_RETURN( FormatProcess0 );
	case MENU_HARDWARE_INFO:
		FADE_OUT_RETURN( HWInfoProcess0 );
#ifdef    USE_WIRELESS_FORCE_DISABLE_SETTING
	case MENU_WIRELESS_SETTING:
		FADE_OUT_RETURN( WirelessSettingProcess0 );
#endif // USE_WIRELESS_FORCE_DISABLE_SETTING
#ifndef   NAND_INITIALIZER_LIMITED_MODE
	case MENU_ETICKET:
		FADE_OUT_RETURN( eTicketProcess0 );
	case MENU_IMPORT_TAD:
		FADE_OUT_RETURN( ImportProcess0 );
	case MENU_IMPORT_NANDFIRM:
		FADE_OUT_RETURN( NandfirmProcess0 );
	case MENU_IMPORT_NORFIRM:
		FADE_OUT_RETURN( NorfirmProcess0 );
#endif // NAND_INITIALIZER_LIMITED_MODE
	}

	return TopmenuProcess1;
}

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/
