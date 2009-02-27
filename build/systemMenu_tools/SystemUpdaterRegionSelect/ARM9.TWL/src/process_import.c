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
#include "sort_title.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define THREAD_STACK_SIZE (16*1024)

#define DIR_NUM           3
#define FULLPATH_LEN      ( FS_ENTRY_LONGNAME_MAX+6 )

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

typedef struct {
	u8 dirNameIndex;
	char fileName[FS_ENTRY_LONGNAME_MAX];
} ImportFileInfo;

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

static u32 CountTadInDir( char (*dir_path)[FULLPATH_LEN], u32 dir_num );
static BOOL MakeList( char (*dir)[FULLPATH_LEN], u32 dir_max, ImportFileInfo *info, TitleSortSet *sortset );
static BOOL ImportTadFromList( char (*dir)[FULLPATH_LEN], ImportFileInfo *info, TitleSortSet *sortset, u32 import_max );

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessImport

  Description:  

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
BOOL ProcessImport( void *(*alloc)(unsigned long), void (*free)(void *) )
{
	const s32 MAX_RETRY_COUNT = 2;
	BOOL result = TRUE;
	char directory[DIR_NUM][FULLPATH_LEN];
	s32 i=0;
	s32 j=0;
	u32 fileCount = 0;
	ImportFileInfo *importFileInfoList = NULL;
	TitleSortSet *titleSortSetList = NULL;
	void *sortBuf = NULL;

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

	STD_TSNPrintf(directory[0], sizeof(directory[0]), "rom:/data/%s/%s/", sDirectoryNameConsole[GetConsole()], sDirectoryNameRegion[gRegion]);
	STD_TSNPrintf(directory[1], sizeof(directory[1]), "rom:/data/common/%s/", sDirectoryNameRegion[gRegion]);
	STD_TSNPrintf(directory[2], sizeof(directory[2]), "rom:/data/common/");
	
	// ディレクトリ内の TAD ファイル数をカウントする
	fileCount = CountTadInDir(directory, DIR_NUM);

	// ファイル名+ディレクトリインデックス配列、TitleSortInfo 配列を確保
	importFileInfoList = alloc( sizeof(ImportFileInfo) * fileCount );
	titleSortSetList = alloc( sizeof(TitleSortSet) * fileCount );
	
	// 情報を読み込む
	result &= MakeList( directory, DIR_NUM, importFileInfoList, titleSortSetList);
	
	// TitleSortInfoをソート
	sortBuf = alloc( MATH_QSortStackSize( fileCount ) );
	SortTitle( titleSortSetList, fileCount, sortBuf );
	
	// インポート
	result &= ImportTadFromList( directory, importFileInfoList, titleSortSetList, fileCount );
	
	// もはや必要ないリストの解放
	free( importFileInfoList );
	free( titleSortSetList );

	while (!FadeOutTick())
	{
	    OS_WaitVBlankIntr();
	}

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         CountTadInDir

  Description:  リストで指定したディレクトリにある TAD の総数を返します。

  Arguments:    dir_list,dir_max

  Returns:      u32
 *---------------------------------------------------------------------------*/
static u32 CountTadInDir( char (*dir_list)[FULLPATH_LEN], u32 dir_max )
{
	int l;
	u32 count = 0;
	for( l=0; l<dir_max; l++ )
	{
		char *dirName = dir_list[ l ];
	    FSFile  dir;
	    FSDirectoryEntryInfo   info[1];

		FS_InitFile(&dir);
		if (!FS_OpenDirectory(&dir, dirName, FS_FILEMODE_R))
		{
			// 空ディレクトリはMakerom時に削除されるようなのでここでは飛ばす
			continue;
		}
		
	    while (FS_ReadDirectory(&dir, info))
	    {
	        if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
	        {
				char* pExtension;
				// 拡張子のチェック
				pExtension = STD_SearchCharReverse( info->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".tad") || !STD_CompareString( pExtension, ".TAD")  )
					{
						count++;
					}
				}
	        }
		}
		FS_CloseDirectory( &dir );
	}
	return count;
}

/*---------------------------------------------------------------------------*
  Name:         MakeList

  Description:  リストで指定したディレクトリから ImportTadFromList 
                を行うのに必要なリストを作成します。

  Arguments:    dir_list, dir_max, info, sortset

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
static BOOL MakeList( char (*dir_list)[FULLPATH_LEN], u32 dir_max, ImportFileInfo *info, TitleSortSet *sortset )
{
	int l;
	u32 count = 0;
	BOOL result = TRUE;
	
	for( l=0; l<dir_max; l++ )
	{
		char *dirName = dir_list[ l ];
	    FSFile  dir;
	    FSDirectoryEntryInfo   fsinfo[1];

		FS_InitFile(&dir);
		if (!FS_OpenDirectory(&dir, dirName, FS_FILEMODE_R))
		{
			// 空ディレクトリはMakerom時に削除されるようなのでここでは飛ばす
			continue;
		}
		
	    while (FS_ReadDirectory(&dir, fsinfo))
	    {
	        if ((fsinfo->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
	        {
				char* pExtension;
				// 拡張子のチェック
				pExtension = STD_SearchCharReverse( fsinfo->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".tad") || !STD_CompareString( pExtension, ".TAD")  )
					{
						NAMTadInfo tadInfo;
						char fullPath[FULLPATH_LEN];
						
						// フルパス作成
						STD_TSNPrintf(fullPath, sizeof(fullPath), "%s/%s", dirName, fsinfo->longname);

						// TAD情報取得
						// tadファイルの情報取得
						if (NAM_ReadTadInfo(&tadInfo, fullPath) != NAM_OK)
						{
							// 失敗したらエラーを表示して次のファイルへ、結果はFalse
							kamiFontPrintfConsole(CONSOLE_RED, "Error NAM_ReadTadInfo()\n");
							kamiFontPrintfConsole(CONSOLE_RED, "file : %s\n",fsinfo->longname);
							result = FALSE;
							continue;
						}

						// ImportFileInfo と TitleSortSet に情報を転載
						info[count].dirNameIndex = (u8)l;
						STD_TSNPrintf(info[count].fileName, FS_ENTRY_LONGNAME_MAX, fsinfo->longname);
						sortset[count].index = count;
						sortset[count].titleID = tadInfo.titleInfo.titleId;

						count++;
					}
				}
	        }
		}
	}
	return result;
}

/*---------------------------------------------------------------------------*
  Name:         ImportTadFromList

  Description:  MakeList で得たリストの情報に従って TAD をインポートします。

  Arguments:    dir_list, info, sortset, import_max

  Returns:      BOOL
 *---------------------------------------------------------------------------*/
static BOOL ImportTadFromList( char (*dir_list)[FULLPATH_LEN], ImportFileInfo *info, TitleSortSet *sortset, u32 import_max )
{
	int l;
	int j;
	BOOL result = TRUE;
	s32 listNo=0;
	
	for( l=0; l<import_max; l++ )
	{
		char *longName = info[ sortset[ l ].index ].fileName;
		char *dirName = dir_list[ info[ sortset[ l ].index ].dirNameIndex ];
		char fullPath[FULLPATH_LEN];
		char string1[256];
		u16  string2[256];
		const s32 MAX_RETRY_COUNT = 2;
		s32  nam_result;
		char *tlo = (char *)( &sortset[ l ].titleID );
		
		// フルパス作成
		STD_TSNPrintf(fullPath, sizeof(fullPath), "%s/%s", dirName, longName);
		
		// インポート
		STD_TSPrintf(string1, "List %d ", ++listNo);
		MI_CpuClear8(string2, sizeof(string2));
		STD_ConvertStringSjisToUnicode(string2, NULL, string1, NULL, NULL);

		NNS_G2dCharCanvasClearArea(&gCanvas, TXT_COLOR_WHITE, 0, 60, 256, 20);
		NNS_G2dTextCanvasDrawText(&gTextCanvas, 40, 60,
			TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)L"Now Import..");
		NNS_G2dTextCanvasDrawText(&gTextCanvas, 135, 60,
			TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)string2);

		// MAX_RETRY_COUNTまでリトライする
		for (j=0; j<MAX_RETRY_COUNT; j++)
		{	
			nam_result = kamiImportTad(fullPath, j);
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
			// kamiFontPrintfConsole(FONT_COLOR_GREEN, "List : %d(%c%c%c%c) Import Success.\n", listNo, tlo[3], tlo[2], tlo[1], tlo[0] );
			kamiFontPrintfConsole(FONT_COLOR_GREEN, "List : %d Import Success.\n", listNo );
		}
		else
		{
			// kamiFontPrintfConsole(FONT_COLOR_RED, "Error: %d(%c%c%c%c) : RetCode = %d\n", listNo, tlo[3], tlo[2], tlo[1], tlo[0], nam_result );
			kamiFontPrintfConsole(FONT_COLOR_RED, "Error: %d : RetCode = %d\n", listNo, nam_result );
			result = FALSE;
		}

		kamiFontLoadScreenData();

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
	char full_path[FULLPATH_LEN];
	static s32 listNo=0;
	s32 j=0;

	FS_InitFile(&dir);
	if (!FS_OpenDirectory(&dir, directory, FS_FILEMODE_R))
	{
		// 空ディレクトリはMakerom時に削除されるようなのでここではTRUEを返す
//    	kamiFontPrintfConsole(CONSOLE_GREEN, "%s can not Open.\n", directory);
		return TRUE;
	}

	// tadファイルを検索してインポート
	// [TODO:]先にfull_pathのリストを作って、NAM_ReadTadInfoで取れるTitleID_loの値でソートしてから
	// 順番にインポートするように変更する。
	// この関数を分けるイメージで、各フォルダの名称リストを先に作成する
	// 
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
