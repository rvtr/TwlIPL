/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_write_font.c

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
#include "font.h"
#include "graphics.h"
#include "keypad.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"
#include "kami_global.h"
#include "kami_copy_file.h"
#include "kami_font.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

typedef struct _CopyFileList
{
	char* srcPath;
	char* dstPath;
} CopyFileList;

/*---------------------------------------------------------------------------*
    グローバル変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部定数定義
 *---------------------------------------------------------------------------*/
/*
static const CopyFileList sCopyFileList[] =
{
	{ "rom:/data/TWLFontTable.dat", "nand:sys/TWLFontTable.dat" },
	{ "rom:/data/cert.sys",         "nand:/sys/cert.sys"        }
};
*/
/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessWriteFont

  Description:  

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
BOOL ProcessWriteFont(void)
{
    FSFile  dir;
    FSDirectoryEntryInfo   info[1];
	char full_path[FS_ENTRY_LONGNAME_MAX+6];
	BOOL find = FALSE;
	BOOL result = TRUE;

/*
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas,  TXT_COLOR_WHITE, 0,  30, 256, 100);
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0, 130, 256,  62);
	OS_WaitVBlankIntr();

	NNS_G2dTextCanvasDrawText(&gTextCanvas, 84, 60,
		TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)
		L"Write Files.."
	);
*/

	// 適切なディレクトリを開く
	STD_TSNPrintf(full_path, sizeof(full_path), "rom:/data/common/");

	FS_InitFile(&dir);
	if (!FS_OpenDirectory(&dir, full_path, FS_FILEMODE_R))
	{
    	kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory()\n");
		return FALSE;
	}

	// .datファイルを検索
    while (FS_ReadDirectory(&dir, info))
    {
        if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
        {
			char* pExtension;

			// 拡張子のチェック
			pExtension = STD_SearchCharReverse( info->longname, '.');
			if (pExtension)
			{
				if (!STD_CompareString( pExtension, ".dat") || !STD_CompareString( pExtension, ".DAT")  )
				{
					// 日米欧豪
					if (gRegion < OS_TWL_REGION_CHINA && 
						!STD_SearchString( info->longname, "_CN_" ) && 
						!STD_SearchString( info->longname, "_KR_" )) 
					{
						STD_TSNPrintf(full_path, sizeof(full_path), "rom:/data/common/%s", info->longname);
						find = TRUE;
						break;
					}
					// 中
					else if (gRegion == OS_TWL_REGION_CHINA && STD_SearchString( info->longname, "_CN_" )) 
					{
						STD_TSNPrintf(full_path, sizeof(full_path), "rom:/data/common/%s", info->longname);
						find = TRUE;
						break;
					}
					// 韓
					else if (gRegion == OS_TWL_REGION_KOREA && STD_SearchString( info->longname, "_KR_" )) 
					{
						STD_TSNPrintf(full_path, sizeof(full_path), "rom:/data/common/%s", info->longname);
						find = TRUE;
						break;
					}

				}
			}
        }
	}

	if (find)
	{
		if (kamiCopyFile(full_path, "nand:sys/TWLFontTable.dat"))
		{
			kamiFontPrintfConsole(FONT_COLOR_GREEN, "Write Data1 Success.\n");			
		}
		else
		{
			result = FALSE;
			kamiFontPrintfConsole(FONT_COLOR_RED, "Write Data1 Failure!\n");
		}
	}

	OS_WaitVBlankIntr();
	kamiFontLoadScreenData();
	return result;
}
