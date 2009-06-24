/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_write_data.c

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
#include "process_write_data.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "common_utility.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

enum {
	MENU_FONT=0,
	NUM_OF_MENU_SELECT
};

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56
#define CURSOR_ORIGIN_Y_INDIVIDUALLY      40
#define CHAR_OF_MENU_SPACE    2
#define CHAR_OF_MENU_SPACE_INDIVIDUALLY  1

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

#define DOT_OF_MENU_SPACE               16
#define DOT_OF_MENU_SPACE_INDIVIDUALLY   8

#define FILE_NUM_MAX         256
#define VIEW_LINES_MAX        16

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;
static s32 sMenuSelectNoIndividually;

static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];

static u8 sFileNum;

static s32 sDatListViewOffset;

static s32 	sLines;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static BOOL WriteFontData(void);
static BOOL WriteDummyData(const char* nandpath);
static BOOL WriteCertData(void);
static void ShowDatList(void);

void* WriteDataProcess0(void);
void* WriteDataProcess1(void);
void* WriteDataProcess2(void);
void* WriteDataProcess3(void);
void* WriteDataProcess4(void);
void* WriteDataProcess5(void);

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         WriteData プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess0(void)
{
	int i;

	// 配列クリア
	MI_CpuClear8( sFilePath, sizeof(sFilePath) );

	// ファイル数初期化
	sFileNum = 0;

	// 表示オフセット初期化
	sDatListViewOffset = 0;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "FONT WRITER");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   Serch SD Card   l     l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+-------------------+-----+");

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

	FADE_IN_RETURN( WriteDataProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         WriteData プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess1(void)
{
	// カーソル配置
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// 決定
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		FADE_OUT_RETURN( WriteDataProcess2 );
	}

	return WriteDataProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         WriteData プロセス２

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess2(void)
{
    FSFile    dir;

    // SDカードのルートディレクトリを検索
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
        kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory(sdmc:/)\n");
    }
    else
    {
        FSDirectoryEntryInfo   info[1];
        OS_Printf("[%s]:\n", "sdmc:/");

		kamiFontPrintfConsole(CONSOLE_ORANGE, "------ font file List -----\n");

		// .dat を探してファイル名を保存しておく
        while (FS_ReadDirectory(&dir, info))
        {
            OS_Printf("  %s", info->longname);
            if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) != 0)
            {
                OS_Printf("/\n");
            }
            else
            {
				char* pExtension;
              OS_Printf(" (%d BYTEs)\n", info->filesize);

				// 拡張子のチェック
				pExtension = STD_SearchCharReverse( info->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".dat") || !STD_CompareString( pExtension, ".DAT")  )
					{
						char full_path[FS_ENTRY_LONGNAME_MAX+6];

						// フルパスを作成
						MakeFullPathForSD(info->longname, full_path);

						STD_CopyString( sFilePath[sFileNum], info->longname );
						kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", sFileNum, info->longname);

						if (++sFileNum >= FILE_NUM_MAX)
						{
							break;
						}
					}
				}
            }
        }
        (void)FS_CloseDirectory(&dir);

		kamiFontPrintfConsole(CONSOLE_ORANGE, "--------------------------\n");

//		DumpTadInfo();
    }
	ShowDatList();

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( WriteDataProcess3 );
}

/*---------------------------------------------------------------------------*
  Name:         WriteData プロセス３

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess3(void)
{

	// 選択メニューの変更
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNoIndividually < 0) 
		{
			sMenuSelectNoIndividually = sFileNum - 1;
			if (sFileNum > VIEW_LINES_MAX)
			{
				sDatListViewOffset = sFileNum - VIEW_LINES_MAX;
			}
			else
			{
				sDatListViewOffset = 0;
			}
		}
		if (sMenuSelectNoIndividually < sDatListViewOffset)
		{
			sDatListViewOffset--;
		}
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNoIndividually > sFileNum - 1) 
		{
			sMenuSelectNoIndividually = 0;
			sDatListViewOffset = 0;
		}
		if ((sMenuSelectNoIndividually - sDatListViewOffset) > VIEW_LINES_MAX - 1)
		{
			sDatListViewOffset++;
		}

	}

	// カーソル配置
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y_INDIVIDUALLY + (sMenuSelectNoIndividually - sDatListViewOffset) * DOT_OF_MENU_SPACE_INDIVIDUALLY));

	// 決定
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return WriteDataProcess4;
	}
	// ひとつ前のメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( WriteDataProcess0 );
	}

	return WriteDataProcess3;
}

/*---------------------------------------------------------------------------*
  Name:         WriteData プロセス4

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess4(void)
{
	BOOL ret;

	if (STD_GetStringLength(sFilePath[sMenuSelectNoIndividually]))
	{
		// 個別インポート
		ret = WriteFontData();
	}
	else
	{
		// リターン
		return WriteDataProcess0;
	}

	// 今回の結果を表示
	if ( ret == TRUE )
	{
		kamiFontPrintf(26, (s16)(5+sMenuSelectNoIndividually*CHAR_OF_MENU_SPACE_INDIVIDUALLY), FONT_COLOR_GREEN, "OK");
    	kamiFontPrintfConsoleEx(CONSOLE_GREEN, "Write Font Success!\n");
	}
	else
	{
		kamiFontPrintf(26, (s16)(5+sMenuSelectNoIndividually*CHAR_OF_MENU_SPACE_INDIVIDUALLY), FONT_COLOR_RED, "NG");
    	kamiFontPrintfConsoleEx(CONSOLE_RED, "Write Font Error!\n");
	}

	// フォントスクリーンデータロード
	kamiFontLoadScreenData();

	return WriteDataProcess3;
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
	char full_path[FS_ENTRY_LONGNAME_MAX+6];

	// フルパスを作成
	MakeFullPathForSD(sFilePath[sMenuSelectNoIndividually], full_path);

	// ROMファイルオープン
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, full_path);
	if (!open_is_ok)
	{
    	kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_OpenFile(\"%s\") ... ERROR!\n", full_path);
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
	    kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_ReadFile(\"%s\") ... ERROR!\n", full_path);
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

/*---------------------------------------------------------------------------*
  Name:         ShowDatList

  Description:  .dad のリストを表示する

  Arguments:    arg -   使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ShowDatList(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "FONT WRITER");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	if (sFileNum > 15)  { sLines = VIEW_LINES_MAX; }
	else                { sLines = sFileNum; }
	for (i=0;i<sLines;i++)
	{
		kamiFontPrintf(3, (s16)(5+i), FONT_COLOR_BLACK, "l                    l    l");
	}
	kamiFontPrintf(3, (s16)(5+sLines), FONT_COLOR_BLACK, "+--------------------+----+");

	// dat ファイルリストを表示
	for (i=0;i<sLines; i++)
	{
		// ファイル名追加
		kamiFontPrintf(3,  (s16)(5+CHAR_OF_MENU_SPACE_INDIVIDUALLY*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sFilePath[sDatListViewOffset+i]);
	}
}
