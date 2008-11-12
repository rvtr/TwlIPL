/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_import.c

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
#include <twl/lcfg.h>
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "hw_info.h"
#include "TWLHWInfo_api.h"
#include "graphics.h"
#include "kami_global.h"
#include "font.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define THREAD_STACK_SIZE (16*1024)

static const char* sDirectoryNameRegion[] =
{
	"japan",
	"america",
	"europe",
	"australia"
};

static const char* sDirectoryNameConsole[] =
{
	"debugger",	  // IS_TWL_DEBUGGER
	"standalone", // IS_TWL_CAPTURE
	"standalone", // TWL
	""			  // UNKNOWN
};

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static u32  sCurrentProgress;
static vu8 sNowImport = FALSE;
static vu8 sProgress  = FALSE;
static u8  sStack[THREAD_STACK_SIZE];

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static s32 kamiImportTad(const char* path, BOOL erase);
static void ProgressThread(void* arg);
static void Destructor(void* arg);
void ProgressDraw(f32 ratio);
BOOL ImportDirectoryTad(char* directory);

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessImport

  Description:  

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
BOOL ProcessImport(void)
{
	const s32 MAX_RETRY_COUNT = 2;
	BOOL result = TRUE;
	char directory[FS_ENTRY_LONGNAME_MAX+6];
	s32 i=0;
	s32 j=0;

	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas,  TXT_COLOR_WHITE, 0,  30, 256, 100);
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0, 130, 256,  62);
	OS_WaitVBlankIntr();
	NNS_G2dTextCanvasDrawText(&gTextCanvas, 40, 60,
		TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)L"Now Import..");

	while(!FadeInTick())
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);
		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
	    OS_WaitVBlankIntr();
	}

	STD_TSNPrintf(directory, sizeof(directory), "rom:/data/%s/%s/", sDirectoryNameConsole[GetConsole()], sDirectoryNameRegion[gRegion]);
	result &= ImportDirectoryTad(directory);

	STD_TSNPrintf(directory, sizeof(directory), "rom:/data/common/%s/", sDirectoryNameRegion[gRegion]);
	result &= ImportDirectoryTad(directory);

	STD_TSNPrintf(directory, sizeof(directory), "rom:/data/common/");
	result &= ImportDirectoryTad(directory);

	while (!FadeOutTick())
	{
	    OS_WaitVBlankIntr();
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         ImportDirectoryTad

  Description:  指定したディレクトリにあるTADを無条件にImportします。

  Arguments:    path

  Returns:      なし。
 *---------------------------------------------------------------------------*/
BOOL ImportDirectoryTad(char* directory)
{
    FSFile  dir;
    FSDirectoryEntryInfo   info[1];
	const s32 MAX_RETRY_COUNT = 2;
	BOOL result = TRUE;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];
	static s32 listNo=0;
	s32 j=0;

	FS_InitFile(&dir);
	if (!FS_OpenDirectory(&dir, directory, FS_FILEMODE_R))
	{
    	kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory()\n");
		return FALSE;
	}

	// tadファイルを検索してインポート
    while (FS_ReadDirectory(&dir, info))
    {
		s32  nam_result;
		char string1[256];
		u16  string2[256];

		MI_CpuClear8(string2, sizeof(string2));

        if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
        {
			char* pExtension;

			// 拡張子のチェック
			pExtension = STD_SearchCharReverse( info->longname, '.');
			if (pExtension)
			{
				if (!STD_CompareString( pExtension, ".tad") || !STD_CompareString( pExtension, ".TAD")  )
				{

					STD_TSPrintf(string1, "List %d ", ++listNo);
					STD_ConvertStringSjisToUnicode(string2, NULL, string1, NULL, NULL);

					NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_WHITE, 0, 60, 256, 20);
					NNS_G2dTextCanvasDrawText(&gTextCanvas, 40, 60,
						TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)L"Now Import..");
					NNS_G2dTextCanvasDrawText(&gTextCanvas, 135, 60,
						TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)string2);

					STD_TSNPrintf(full_path, sizeof(full_path), "%s/%s", directory, info->longname);
//            		kamiFontPrintfConsole(CONSOLE_GREEN, "  %s\n", full_path);

					// MAX_RETRY_COUNTまでリトライする
					for (j=0; j<MAX_RETRY_COUNT; j++)
					{	
						nam_result = kamiImportTad(full_path, j);
						if (nam_result == NAM_OK)
						{
							break;
						}
						else
						{
							kamiFontPrintfConsole(CONSOLE_GREEN, "Import %d Retry!\n", listNo);
						}
					}

					if ( nam_result == NAM_OK)
					{
						kamiFontPrintfConsole(FONT_COLOR_GREEN, "List : %d Import Success.\n", listNo);			
					}
					else
					{
						kamiFontPrintfConsole(FONT_COLOR_RED, "Error: %d : RetCode = %d\n", listNo, nam_result );
						result = FALSE;
					}

					kamiFontLoadScreenData();
				}
			}
        }
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         kamiImportTad

  Description:  .tad ファイルインポート

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
static s32 kamiImportTad(const char* path, BOOL erase)
{
	NAMTadInfo tadInfo;
	OSThread thread;
	s32  nam_result;

	// tadファイルの情報取得
	nam_result = NAM_ReadTadInfo(&tadInfo, path);
	if ( nam_result != NAM_OK )
	{
		return nam_result;
	}

	// ESの仕様で古い e-ticket があると新しい e-ticket を使ったインポートができない
	// 暫定対応として該当タイトルを完全削除してからインポートする
	if (erase)
	{
		nam_result = NAM_DeleteTitleCompletely(tadInfo.titleInfo.titleId);
		if ( nam_result != NAM_OK )
		{
			kamiFontPrintfConsole(CONSOLE_RED, "Fail! RetCode=%x\n", nam_result);
			return FALSE;
		}
	}

	// インポート開始フラグを立てる
	sNowImport = TRUE;

    // 進捗スレッド作成
	MI_CpuClear8(sStack, THREAD_STACK_SIZE);
    OS_CreateThread(&thread, ProgressThread, NULL,
        (void*)((u32)sStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, OS_GetCurrentThread()->priority - 1);
    OS_WakeupThreadDirect(&thread);

	// Import開始
	nam_result = NAM_ImportTad( path );

	// インポート開始フラグを下げる
	sNowImport = FALSE;

	// 進捗スレッドの自力終了を待つ
	while (sProgress){};

	// きちんと表示する
	G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
    OS_WaitVBlankIntr();

	// InstalledSoftBoxCount, FreeSoftBoxCount の値を現在のNANDの状態に合わせて更新します。
	(void)NAMUT_UpdateSoftBoxCount();

	return nam_result;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressThread

  Description:  .tad ファイルインポートの進捗を表示するスレッド。
				進捗が100%に達すると処理を抜ける。

  Arguments:    arg -   使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void ProgressThread(void* /*arg*/)
{
    u32  currentSize;
    u32  totalSize   = 0;
	u32  totalSizeBk = 0;

	sProgress = TRUE;

    while (sNowImport)
    {
        NAM_GetProgress(&currentSize, &totalSize);

		if ((totalSize > 0 && totalSize == currentSize) || totalSizeBk > totalSize)
		{
			// 既にインポートが終了
			ProgressDraw((f32)1.0);
			break;	
		}
		else if (totalSize > 0)
		{
			ProgressDraw((f32)currentSize/totalSize);
		}

		totalSizeBk = totalSize;

		// Vブランク待ち
		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
        OS_WaitVBlankIntr();

		// 3D初期化
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);
    }

	sProgress = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressDraw

  Description:  インポートの進捗を表示します

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/

void ProgressDraw(f32 ratio)
{
	s16 x = (s16)(30 + (226 - 30)*ratio);

	// グリーンバー
	DrawQuadWithColors( 30,  86,   x,  95, GX_RGB(22, 31, 22), GX_RGB(12, 25, 12));

	// グレーバー
	DrawQuad( 30,  86, 226,  95, GX_RGB(28, 28, 28));
}
