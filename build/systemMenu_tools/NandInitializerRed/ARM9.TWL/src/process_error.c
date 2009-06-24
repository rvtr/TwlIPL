/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_eticket.c

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
#include "kami_pxi.h"
#include "process_error.h"
#include "process_fade.h"
#include "cursor.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         eTicket プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* errorProcess(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "ERROR");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "The region specified in");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "nandinitializer.ini is ");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "not equal to the region");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "of this machine.       ");

	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "          or           ");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "nandinitializer.ini is ");
	kamiFontPrintf(3, 16, FONT_COLOR_BLACK, "not exist.");

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_RED, BG_COLOR_RED );
	}

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	return errorProcess;
}
