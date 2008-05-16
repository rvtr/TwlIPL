/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_font.c

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
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_font.h"
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

#define NUM_OF_MENU_SELECT    2
#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static BOOL WriteFontData(void);

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         font プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* fontProcess0(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Write Font Data");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   WRITE FONT DATA l     l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l   RETURN          l     l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+-------------------+-----+");

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// 背景上部
	kamiFontFillChar( 0, BG_COLOR_BROWN, BG_COLOR_BROWN );
	kamiFontFillChar( 1, BG_COLOR_BROWN, BG_COLOR_BROWN );
	kamiFontFillChar( 2, BG_COLOR_BROWN, BG_COLOR_TRANS );

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( fontProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         font プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* fontProcess1(void)
{
#ifndef NAND_INITIALIZER_LIMITED_MODE
	// オート実行用
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return fontProcess2;
	}
#endif

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
		return fontProcess2;
	}
	// トップメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return fontProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         font プロセス２

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* fontProcess2(void)
{
	BOOL result;

	switch( sMenuSelectNo )
	{
	case 0:
		result = WriteFontData();
		if (result)
		{
			kamiFontPrintf(25, 7, FONT_COLOR_GREEN, "OK");
		}
		else
		{
			kamiFontPrintf(25, 7, FONT_COLOR_RED, "NG");
		}
		break;
	case 1:
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

#ifndef NAND_INITIALIZER_LIMITED_MODE
	// Auto用
	if (gAutoFlag)
	{
		if (result) { FADE_OUT_RETURN( AutoProcess1 ); }
		else { FADE_OUT_RETURN( AutoProcess2); }
	}
#endif

	return fontProcess1;
}

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

static BOOL WriteFontData(void)
{
    FSFile  file;	
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	void* pTempBuf;
	u32 file_size;
	u32 alloc_size;
	BOOL result = TRUE;

	// ROMファイルオープン
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, FONT_DATA_FILE_PATH_IN_ROM);
	if (!open_is_ok)
	{
    	OS_Printf("FS_OpenFile(\"%s\") ... ERROR!\n", FONT_DATA_FILE_PATH_IN_ROM);
		return FALSE;
	}

	// ROMファイルリード
	file_size  = FS_GetFileLength(&file) ;
	alloc_size = ROUND_UP(file_size, 32) ;
	pTempBuf = OS_Alloc( alloc_size );
	SDK_NULL_ASSERT(pTempBuf);
	DC_InvalidateRange(pTempBuf, alloc_size);
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
	if (!read_is_ok)
	{
	    kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_ReadFile(\"%s\") ... ERROR!\n", FONT_DATA_FILE_PATH_IN_ROM);
		FS_CloseFile(&file);
		OS_Free(pTempBuf);
		return FALSE;
	}

	// ROMファイルクローズ
	FS_CloseFile(&file);

	// 一旦フォントデータを削除する
	(void)FS_DeleteFile(FONT_DATA_FILE_PATH_IN_NAND);

	// nand:sys/TWLFontTable.dat作成
    if (!FS_CreateFile(FONT_DATA_FILE_PATH_IN_NAND, FS_PERMIT_R | FS_PERMIT_W))
    {
        kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_CreateFile(%s) failed.\n", FONT_DATA_FILE_PATH_IN_NAND);
		result = FALSE;
    }
    else
    {
		// nand:sys/TWLFontTable.datオープン
		FS_InitFile(&file);
        open_is_ok = FS_OpenFileEx(&file, FONT_DATA_FILE_PATH_IN_NAND, FS_FILEMODE_W);
        if (!open_is_ok)
        {
            kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_OpenFile(%s) failed.\n", FONT_DATA_FILE_PATH_IN_NAND);
			result = FALSE;
        }
		// nand:sys/TWLFontTable.dat書き込み
        else if (FS_WriteFile(&file, pTempBuf, (s32)file_size) == -1)
        {
            kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_WritFile() failed.\n");
			result = FALSE;
        }
        (void)FS_CloseFile(&file);
    }

	OS_Free(pTempBuf);

	return result;
}

