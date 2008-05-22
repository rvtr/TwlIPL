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
#include "import.h"
#include "hw_info.h"
#include "TWLHWInfo_api.h"
#include "graphics.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define THREAD_STACK_SIZE (16*1024)

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

static void ProgressThread(void* arg);
static void Destructor(void* arg);
void ProgressDraw(f32 ratio);

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         kamiImportTad

  Description:  .tad ファイルインポート

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/

s32 kamiImportTad(int no, int total, const char* path)
{
	NAMTadInfo tadInfo;
	OSThread thread;
	s32  nam_result;

	kamiFontPrintfMain( 4, 9, 8, "Now Updating...  %d / %d", no, total );
	kamiFontLoadScreenData();

	// tadファイルの情報取得
	nam_result = NAM_ReadTadInfo(&tadInfo, path);
	if ( nam_result != NAM_OK )
	{
		return nam_result;
	}

	// ESの仕様で古い e-ticket があると新しい e-ticket を使ったインポートができない
	// 暫定対応として該当タイトルを完全削除してからインポートする
	nam_result = NAM_DeleteTitleCompletely(tadInfo.titleInfo.titleId);
	if ( nam_result != NAM_OK )
	{
		kamiFontPrintfConsole(CONSOLE_RED, "Fail! RetCode=%x\n", nam_result);
		return FALSE;
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
        OS_WaitVBlankIntr();
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

	// 3D初期化
	G3X_Reset();
	G3_Identity();
	G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

	// グリーンバー
	DrawQuad( 30,  90,   x,  95, GX_RGB(12, 25, 12));

	// グレーバー
	DrawQuad( 30,  90, 226,  95, GX_RGB(28, 28, 28));

	// グレーダイアログ
	DrawQuad( 20,  60, 236, 110, GX_RGB(25, 25, 25));

	// 3Dスワップ
	G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

/*---------------------------------------------------------------------------*
  Name:         DrawResult

  Description:  処理結果を表示します。

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/

void DrawResult(BOOL result)
{
	// 3D初期化
	G3X_Reset();
	G3_Identity();
	G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

	// "Now Updating.." を消去
	kamiFontPrintfMain( 0, 9, 7, "                                ");

	if (result)
	{
		kamiFontPrintfMain( 9, 10, 7, "Update Success!");
		// グリーンダイアログ
		DrawQuad( 50,  50, 206, 120, GX_RGB(12, 25, 12));
	}
	else
	{
		kamiFontPrintfMain( 9, 10, 7, "Update Failure!");
		// レッドダイアログ
		DrawQuad( 50,  50, 206, 120, GX_RGB(31,  0,  0));
	}

	kamiFontLoadScreenData();

	// 3Dスワップ
	G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
}

