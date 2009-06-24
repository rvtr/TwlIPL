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
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_eticket.h"
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

static BOOL MakeETicketFile(void);

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         eTicket プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* eTicketProcess0(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Write eTicket Sign");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   WRITE E-TICKET  l     l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l   RETURN          l     l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+-------------------+-----+");

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// 背景上部
	kamiFontFillChar( 0, BG_COLOR_GRAY, BG_COLOR_GRAY );
	kamiFontFillChar( 1, BG_COLOR_GRAY, BG_COLOR_GRAY );
	kamiFontFillChar( 2, BG_COLOR_GRAY, BG_COLOR_TRANS );

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( eTicketProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         eTicket プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* eTicketProcess1(void)
{
	// オート実行用
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return eTicketProcess2;
	}

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
		return eTicketProcess2;
	}
	// トップメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return eTicketProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         eTicket プロセス２

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* eTicketProcess2(void)
{
	BOOL result;

	switch( sMenuSelectNo )
	{
	case 0:
		result = MakeETicketFile();
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

	// Auto用
	if (gAutoFlag)
	{
		if (result) { FADE_OUT_RETURN( AutoProcess1 ); }
		else { FADE_OUT_RETURN( AutoProcess2); }
	}

	return eTicketProcess1;
}

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

static BOOL MakeETicketFile(void)
{
    FSFile  file;
    FATFSFileHandle fat_handle;
	
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	void* pTempBuf;
	u32 file_size;
	u32 alloc_size;
	BOOL result = TRUE;
		
	// F:sys/cert.sysが既に存在するなら何もしない
    fat_handle = FATFS_OpenFile(E_TICKET_FILE_PATH_IN_NAND, "r");
    if (fat_handle)
    {
        kamiFontPrintfConsoleEx(CONSOLE_RED, "%s already exist\n", E_TICKET_FILE_PATH_IN_NAND);
		kamiFontPrintf(2,  20, FONT_COLOR_RED, "%s already exist", E_TICKET_FILE_PATH_IN_NAND);
		FATFS_CloseFile(fat_handle);
		return FALSE;
    }

	// ROMファイルオープン
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, E_TICKET_FILE_PATH_IN_ROM);
	if (!open_is_ok)
	{
    	OS_Printf("FS_OpenFile(\"%s\") ... ERROR!\n", E_TICKET_FILE_PATH_IN_ROM);
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
	    kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_ReadFile(\"%s\") ... ERROR!\n", E_TICKET_FILE_PATH_IN_ROM);
		FS_CloseFile(&file);
		OS_Free(pTempBuf);
		return FALSE;
	}

	// ROMファイルクローズ
	FS_CloseFile(&file);

	// F:sys/cert.sys作成
    if (!FATFS_CreateFile(E_TICKET_FILE_PATH_IN_NAND, TRUE, "rwxrwxrwx"))
    {
        kamiFontPrintfConsoleEx(CONSOLE_RED, "FATFS_CreateFile(%s) failed.\n", E_TICKET_FILE_PATH_IN_NAND);
		result = FALSE;
    }
    else
    {
		// F:sys/cert.sysオープン
        fat_handle = FATFS_OpenFile(E_TICKET_FILE_PATH_IN_NAND, "w");
        if (!fat_handle)
        {
            kamiFontPrintfConsoleEx(CONSOLE_RED, "FATFS_OpenFile(%s) failed.\n", E_TICKET_FILE_PATH_IN_NAND);
			result = FALSE;
        }
		// F:sys/cert.sys書き込み
        else if (FATFS_WriteFile(fat_handle, pTempBuf, (s32)file_size) == -1)
        {
            kamiFontPrintfConsoleEx(CONSOLE_RED, "FATFS_WritFile() failed.\n");
			result = FALSE;
        }
        (void)FATFS_CloseFile(fat_handle);
    }

	OS_Free(pTempBuf);

	return result;
}

