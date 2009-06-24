/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_wireless_setting.c

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
#include <twl/lcfg.h>
#include "kami_font.h"
#include "process_topmenu.h"
#include "process_hw_info.h"
#include "process_wireless_setting.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "hwi.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

enum {
	MENU_WIRELESS_ENABLE,
	MENU_WIRELESS_FORCE_OFF,
	MENU_RETURN,
	NUM_OF_MENU_SELECT
};

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE    16
#define CHAR_OF_MENU_SPACE    2
#define MENU_TOP_LINE         5
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      40

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static BOOL WriteHWNormalInfoFile( void );
static BOOL WriteHWSecureInfoFile( u8 region );

const LCFGTWLHWNormalInfo *LCFG_THW_GetDefaultNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetDefaultSecureInfo( void );
const LCFGTWLHWNormalInfo *LCFG_THW_GetNormalInfo( void );
const LCFGTWLHWSecureInfo *LCFG_THW_GetSecureInfo( void );

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         WirelessSetting プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WirelessSettingProcess0(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Wireless Setting                ");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  5, FONT_COLOR_BLACK, "l  Wireless Enable   l    l");
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l  Wireless Force Offl    l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l  RETURN            l    l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+--------------------+----+");

	// 現在の無線強制OFF状態に"now"と表示
	kamiFontPrintf(26, (s16)(MENU_TOP_LINE+LCFG_THW_IsForceDisableWireless()*CHAR_OF_MENU_SPACE), FONT_COLOR_BLACK, "now");

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// 背景上部
	kamiFontFillChar( 0, BG_COLOR_YELLOW, BG_COLOR_YELLOW );
	kamiFontFillChar( 1, BG_COLOR_YELLOW, BG_COLOR_YELLOW );
	kamiFontFillChar( 2, BG_COLOR_YELLOW, BG_COLOR_TRANS );

	// カーソル除外
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( WirelessSettingProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         WirelessSetting プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WirelessSettingProcess1(void)
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
		return WirelessSettingProcess2;
	}
	// トップメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return WirelessSettingProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         WirelessSetting プロセス２

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WirelessSettingProcess2(void)
{
	BOOL result;

	switch( sMenuSelectNo )
	{
	case MENU_WIRELESS_ENABLE:
		result = WriteHWInfoFile( OS_GetRegion(), FALSE, FALSE );
		if ( result == TRUE )
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+MENU_WIRELESS_ENABLE*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK ");
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+MENU_WIRELESS_FORCE_OFF*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "   ");
		}
		else
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+MENU_WIRELESS_ENABLE*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "NG ");
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+MENU_WIRELESS_FORCE_OFF*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "   ");
		}
		break;

	case MENU_WIRELESS_FORCE_OFF:
		result = WriteHWInfoFile( OS_GetRegion(), TRUE, FALSE );
		if ( result == TRUE )
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+MENU_WIRELESS_ENABLE*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "   ");
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+MENU_WIRELESS_FORCE_OFF*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK ");
		}
		else
		{
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+MENU_WIRELESS_ENABLE*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "   ");
			kamiFontPrintf(26,  (s16)(MENU_TOP_LINE+MENU_WIRELESS_FORCE_OFF*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "NG ");
		}
		break;

	case MENU_RETURN:
		FADE_OUT_RETURN( TopmenuProcess0 );
		break;
	}

	return WirelessSettingProcess1;
}
