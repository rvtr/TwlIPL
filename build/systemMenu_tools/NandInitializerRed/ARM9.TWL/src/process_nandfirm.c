/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_nandfirm.c

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
#include <twl/nam.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_nandfirm.h"
#include "process_import.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "kami_write_nandfirm.h"
#include "common_utility.h"

#include "TWLHWInfo_api.h"
#include <firm/format/firm_common.h>
#include <../build/libraries/spi/ARM9/include/spi.h>

/*---------------------------------------------------------------------------*
    マクロ定義
 *---------------------------------------------------------------------------*/

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE		8
#define CHAR_OF_MENU_SPACE		1
#define CURSOR_ORIGIN_X			32
#define CURSOR_ORIGIN_Y			40

#define FILE_NUM_MAX			16

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s32 sMenuSelectNo;
static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
static u8 sFileNum;

/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/

void MakeFullPathForSD(char* file_name, char* full_path);

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Import プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess0(void)
{
    FSFile    dir;
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import NandFirm from SD");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// 配列クリア
	MI_CpuClear8( sFilePath, sizeof(sFilePath) );

	// ファイル数初期化
	sFileNum = 0;

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// 背景上部
	kamiFontFillChar( 0, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 1, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 2, BG_COLOR_GREEN, BG_COLOR_TRANS );

    // SDカードのルートディレクトリを検索
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
		kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory(sdmc:/)");
    }
    else
    {
        FSDirectoryEntryInfo   info[1];
        OS_Printf("[%s]:\n", "sdmc:/");

		kamiFontPrintfConsole(CONSOLE_ORANGE, "------ nand file list -----\n");

		// .nand を探してファイル名を保存しておく
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
					if (!STD_CompareString( pExtension, ".nand") || !STD_CompareString( pExtension, ".NAND"))
					{
						STD_CopyString( sFilePath[sFileNum], info->longname );
						kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", sFileNum, info->longname);

						// 最大16個で終了
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
    }

	// メニュー一覧
	kamiFontPrintf((s16)3, (s16)4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf((s16)3, (s16)(5+sFileNum+1), FONT_COLOR_BLACK, "+--------------------+----+");

	// tad ファイルリストを表示
	for (i=0;i<sFileNum; i++)
	{
		// ファイル名追加
		kamiFontPrintf((s16)3,  (s16)(5+CHAR_OF_MENU_SPACE*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sFilePath[i]);
	}

	// 最後にリターンを追加
	kamiFontPrintf((s16)3,  (s16)(5+CHAR_OF_MENU_SPACE*sFileNum), FONT_COLOR_BLACK, "l   RETURN           l    l");

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( NandfirmProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess1(void)
{
	// オート実行用
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return NandfirmProcess2;
	}

	// 選択メニューの変更
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = sFileNum;
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNo > sFileNum) sMenuSelectNo = 0;
	}

	// カーソル配置
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// 決定
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return NandfirmProcess2;
	}
	// トップメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return NandfirmProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         プロセス2

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess2(void)
{
	BOOL ret;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];

	if (STD_GetStringLength(sFilePath[sMenuSelectNo]))
	{
		// .nandのフルパスを作成
		MakeFullPathForSD(sFilePath[sMenuSelectNo], full_path);
		ret = kamiWriteNandfirm(full_path, OS_AllocFromMain, OS_FreeToMain);
	}
	else
	{
		if (gAutoFlag)	{ FADE_OUT_RETURN( AutoProcess2 ); 		}
		else 			{ FADE_OUT_RETURN( TopmenuProcess0 );	}
	}

	// 今回の結果を表示
	if ( ret == TRUE )
	{
		kamiFontPrintf((s16)26,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK");
	}
	else
	{
		kamiFontPrintf((s16)26,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_RED, "NG");
	}

	// Auto用
	if (gAutoFlag)
	{
		if (ret) 
		{
			gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_NANDFIRM] = AUTO_PROCESS_RESULT_SUCCESS;  
			FADE_OUT_RETURN( AutoProcess1 ); 
		}
		else 
		{ 
			gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_NANDFIRM] = AUTO_PROCESS_RESULT_FAILURE;
			FADE_OUT_RETURN( AutoProcess2 ); 
		}
	}

	return NandfirmProcess1;
}
