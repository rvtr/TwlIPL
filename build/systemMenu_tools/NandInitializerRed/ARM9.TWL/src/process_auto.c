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
#include "process_write_data.h"
#include "process_nandfirm.h"
#include "process_norfirm.h"
#include "process_auto.h"
#include "process_fade.h"
#include "process_mcu.h"
#include "cursor.h"
#include "keypad.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    グローバル変数定義
 *---------------------------------------------------------------------------*/

BOOL gAutoFlag = FALSE;
AutoProcessResult gAutoProcessResult[AUTO_PROCESS_MENU_NUM];

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
	s32 i;

	// オートフラグセット
	gAutoFlag = TRUE;

	// 処理結果初期化
	for (i=0;i<AUTO_PROCESS_MENU_NUM; i++)
	{
		gAutoProcessResult[i] = AUTO_PROCESS_RESULT_SKIP;
	}

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
	case AUTO_PROCESS_MENU_FORMAT:
		return FormatProcess0;

	case AUTO_PROCESS_MENU_HARDWARE_INFO:
		return HWInfoProcess0;

#ifdef    USE_WRITE_VARIOUS_DATA
	case AUTO_PROCESS_MENU_VARIOUS_DATA:
		return WriteDataProcess0;	
#endif // USE_WRITE_VARIOUS_DATA

	case AUTO_PROCESS_MENU_IMPORT_TAD:
		return ImportProcess0;

	case AUTO_PROCESS_MENU_IMPORT_NANDFIRM:
		return NandfirmProcess0;

	case AUTO_PROCESS_MENU_MCU:
		return mcuProcess0;

#ifdef    MARIOCLUB_VERSION
	case AUTO_PROCESS_MENU_MACHINE_INITIALIZE:
		return FormatProcess0;
#endif // MARIOCLUB_VERSION
		
	case AUTO_PROCESS_MENU_NUM:
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
	s8 line = 3;
	u8 bg_color;
	BOOL totalResult = TRUE;

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
#ifdef    USE_WRITE_VARIOUS_DATA
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    WRITE VARIOUS DATA     ");
#endif // USE_WRITE_VARIOUS_DATA
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    INPORT TAD FROM SD     ");
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    INPORT NANDFIRM FROM SD");
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    WRITE MCU FIRM         ");
#ifdef    MARIOCLUB_VERSION
	kamiFontPrintf(3, line += 2, FONT_COLOR_BLACK, "    MACHINE INITIALIZE     ");
#endif // MARIOCLUB_VERSION

/*
	for (i=0;i<sMenuSelectNo-1;i++)
	{
		kamiFontPrintf(3, (s16)(7+2*i), FONT_COLOR_GREEN, "OK");
	}
*/

	for (i=0;i<AUTO_PROCESS_MENU_NUM;i++)
	{
		switch (gAutoProcessResult[i])
		{
			case AUTO_PROCESS_RESULT_SUCCESS:
				kamiFontPrintf(3, (s16)(5+2*i), FONT_COLOR_GREEN, "OK");
				break;
			case AUTO_PROCESS_RESULT_FAILURE:
				totalResult = FALSE;
				kamiFontPrintf(3, (s16)(5+2*i), FONT_COLOR_RED, "NG");
				break;
			case AUTO_PROCESS_RESULT_SKIP:
				kamiFontPrintf(2, (s16)(5+2*i), FONT_COLOR_PURPLE, "SKIP");
				break;				
		}
	}

	// 失敗なし
	if (totalResult)
	{
		kamiFontPrintf(3, 21, FONT_COLOR_BLACK, "   Finished Successfully!");
		bg_color = BG_COLOR_GREEN;
	}
	// 失敗あり
	else
	{
		kamiFontPrintf(3, 21, FONT_COLOR_BLACK, "    Error Occured!");
		bg_color = BG_COLOR_RED;
	}

	// 背景上部
	kamiFontFillChar( 0, bg_color, bg_color );
	kamiFontFillChar( 1, bg_color, bg_color );
	kamiFontFillChar( 2, bg_color, BG_COLOR_TRANS );

	// 背景下部
	kamiFontFillChar(20, BG_COLOR_TRANS, bg_color );
	kamiFontFillChar(21, bg_color, bg_color );
	kamiFontFillChar(22, bg_color, BG_COLOR_TRANS );

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
	if (gAutoProcessResult[AUTO_PROCESS_MENU_MCU] == AUTO_PROCESS_RESULT_SKIP)
	{
	    if (kamiPadIsTrigger(PAD_BUTTON_B))
		{
			FADE_OUT_RETURN( TopmenuProcess0 );
		}
	}
#endif

	return AutoProcess3;
}

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

