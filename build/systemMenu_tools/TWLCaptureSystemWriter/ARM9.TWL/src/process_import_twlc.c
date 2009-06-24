/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_import_twlc.c

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
#include <twl/nam.h>
#include <sysmenu/namut.h>
#include "kami_pxi.h"
#include "kami_font.h"
#include "process_topmenu.h"
#include "process_import_twlc.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "common_utility.h"

#define NUM_IMPORT_TAD 5

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

// tad格納先のシンボル
extern void *hnaa_begin, *hnaa_end, *hnba_begin, *hnba_end, *hnca_begin, *hnca_end,
				*hnha_begin, *hnha_end, *hnla_begin, *hnla_end;

static char* sFilename[] = {"HNAA", "HNBA", "HNCA", "HNHA", "HNLA" };
static void* spFileBegin[] = {  &hnaa_begin, &hnba_begin, &hnca_begin, &hnha_begin, &hnla_begin};
static void* spFileEnd[] = {  &hnaa_end, &hnba_end, &hnca_end, &hnha_end, &hnla_end};

static vu8 sNowImport = FALSE;
static void* spStack;
static u32  sCurrentProgress;
/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/


BOOL ImportTadTWLC(void* symbolBegin, void* symbolEnd);
static void ProgressThread(void*);
static void Destructor(void* /*arg*/);
void ProgressInit(void);
void ProgressDraw(f32 ratio);
static BOOL DumpTadInfo(FSFile *pInfo);


void* ImportProcessTWLC0(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import TAD from SD");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  5, FONT_COLOR_BLACK, "l  OVERWRITE ALL     l    l");
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l  WRITE NONEXISTENT l    l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l  SELECT FILE >>    l    l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "l  RETURN            l    l");
	kamiFontPrintf(3, 12, FONT_COLOR_BLACK, "+--------------------+----+");

	// 背景全クリア
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// 背景上部
	kamiFontFillChar( 0, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 1, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 2, BG_COLOR_PINK, BG_COLOR_TRANS );

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( ImportProcessTWLC1 );
}

void* ImportProcessTWLC1(void)
{
	// オート実行用
	if (gAutoFlag)
	{
		return ImportProcessTWLC2;
	}

	// can't reach
	return ImportProcessTWLC1;
}

static void* ImportProcessTWLC2(void)
{
	int i;
	BOOL result = TRUE;

	kamiFontPrintf(25,  5, FONT_COLOR_BLACK, "WAIT");

	for (i=0;i<NUM_IMPORT_TAD;i++)
	{
		kamiFontPrintfConsole(CONSOLE_ORANGE, "import begin: %s\n", sFilename[i]) ;
		
		// 強制上書き
		if (ImportTadTWLC(spFileBegin[i], spFileEnd[i]) == FALSE)
		{
			result = FALSE;
		}
	}

	if (result) { kamiFontPrintf(25,  5, FONT_COLOR_GREEN, " OK "); }
	else		{ kamiFontPrintf(25,  5, FONT_COLOR_RED,   " NG "); }

	// Auto用
	if (gAutoFlag)
	{
		if (result) 
		{
			gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_TAD] = AUTO_PROCESS_RESULT_SUCCESS;  
			FADE_OUT_RETURN( AutoProcess1 ); 
		}
		else 
		{ 
			gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_TAD] = AUTO_PROCESS_RESULT_FAILURE;  
			FADE_OUT_RETURN( AutoProcess2 ); 
		}
	}

	return ImportProcessTWLC1;
}


#define THREAD_STACK_SIZE (16*1024)

BOOL ImportTadTWLC(void* symbolBegin, void* symbolEnd)
{
	NAMTadInfo tadInfo;
	FSFile file;
	u32 filesize = (u32)symbolEnd - (u32)symbolBegin;
	s32 nam_result = NAM_OK;
	OSThread thread;
	
	FS_InitFile(&file);
	
	// メモリ上からtadとしてオープンするためにメモリ上にcreateして中身をコピー
	kamiFontPrintfConsole(CONSOLE_ORANGE, "tad open from memory.\n") ;
	
	FS_CreateFileFromMemory(&file, symbolBegin, filesize);
	if( !DumpTadInfo(&file))
	{
		goto error_proc;
	}
	
	
	// 同titleIDのtadが既にインポートされているかチェック
	// そんざいするばあいは、まずそれを削除してからインポートする
	kamiFontPrintfConsole(CONSOLE_ORANGE, "imported tad check.\n") ;
	// 情報を取得
	nam_result = NAM_ReadTadInfoWithFile(&tadInfo, &file);
	if(nam_result != NAM_OK)
	{
		goto error_proc;
	}
	
	nam_result = NAM_WasTitleInstalled(tadInfo.titleInfo.titleId);
	if(nam_result == NAM_INSTALLED )
	{
		kamiFontPrintfConsole(CONSOLE_ORANGE, "matched tad is found.\n") ;
		
		// tad削除
		kamiFontPrintfConsole(CONSOLE_ORANGE, "delete tad completely.\n") ;
		nam_result = NAM_DeleteTitleCompletely(tadInfo.titleInfo.titleId);
		if( nam_result != NAM_OK ) 
		{
			goto error_proc;
		}
	}
	else if( nam_result == NAM_NOT_INSTALLED )
	{
		kamiFontPrintfConsole(CONSOLE_ORANGE, "matched tad isn't found.\n") ;
	}
	else
	{
		// INSTALLEDとNOT_INSTALLED以外はエラー
		goto error_proc;
	}
	
	// 進捗管理用スレッドを立てる
	spStack = OS_Alloc(THREAD_STACK_SIZE);
	OS_CreateThread(&thread, ProgressThread, NULL, 
					(void*)((u32)spStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, 
					OS_GetCurrentThread()->priority -1);
	 
	OS_SetThreadDestructor(&thread, Destructor);
	OS_WakeupThreadDirect(&thread);
	
	// Import開始
	OS_Printf( "Import Start.\n" );
	kamiFontPrintfConsole(CONSOLE_ORANGE, "Import Start.\n");
	nam_result = NAM_ImportTadWithFile(&file);

	// インポートが終わるまで待つ
	while(sNowImport){};
	OS_WaitVBlankIntr();	
	
	if(nam_result != NAM_OK)
	{
		goto error_proc;
	}
	

	kamiFontPrintfConsole(CONSOLE_ORANGE, "Import finished.\n");
	FS_CloseFile(&file);
	NAMUT_UpdateSoftBoxCount();

	return TRUE;
	
error_proc:
	// エラーの事後処理
	kamiFontPrintfConsole(CONSOLE_RED, "\nfail! errorcode: %d.\n", nam_result) ;
	FS_CloseFile(&file);
	NAMUT_UpdateSoftBoxCount();
	return FALSE;
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

	ProgressInit();

    while (TRUE)
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
    

	sNowImport = FALSE;
}
static void Destructor(void* /*arg*/)
{
	OS_Free(spStack);
}

/*---------------------------------------------------------------------------*
  Name:         ProgressInit

  Description:  インポートの進捗を表示します

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ProgressInit(void)
{
	sCurrentProgress = 0;
}

/*---------------------------------------------------------------------------*
  Name:         ProgressDraw

  Description:  インポートの進捗を表示します

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ProgressDraw(f32 ratio)
{
	char square[2] = { 0x01, 0x00 };
	u32 temp;
	s32 i;

	temp = (u32)(32 * ratio);
	if (temp > sCurrentProgress)
	{
		s32 diff = (s32)(temp - sCurrentProgress);
		for (i=0;i<diff;i++)
		{
			kamiFontPrintfConsole(2, square);
		}
	}
	sCurrentProgress = temp;

	// フォントスクリーンデータロード
	kamiFontLoadScreenData();
}

/*---------------------------------------------------------------------------*
  Name:         DumpTadInfo

  Description:  .tad ファイルの情報を表示する

  Arguments:    arg -   使用しない。

  Returns:      成功したらTRUE,失敗したらFALSEを返します.
 *---------------------------------------------------------------------------*/

static BOOL DumpTadInfo(FSFile *pFile)
{
	NAMTadInfo info;
	s32 nam_result;

	// TADファイルの情報取得
	if ((nam_result = NAM_ReadTadInfoWithFile(&info, pFile)) == NAM_OK)
	{
		char temp[100];
		u16 companyCode = MI_SwapEndian16(info.titleInfo.companyCode);
		u32 gameCode = (u32)(info.titleInfo.titleId & 0xffffffff);
		gameCode = MI_SwapEndian32(gameCode);

		kamiFontPrintfConsole(CONSOLE_ORANGE, "------ tad profile -----\n");

		// File Size
		kamiFontPrintfConsole(CONSOLE_ORANGE, "fileSize      = %d Byte\n", info.fileSize);

		// Company Code
		MI_CpuCopy8( &companyCode, temp, sizeof(companyCode) );
		temp[sizeof(companyCode)] = NULL;
		kamiFontPrintfConsole(CONSOLE_ORANGE, "Company Code  = %s\n", temp);

		// Game Code
		MI_CpuCopy8( &gameCode, temp, sizeof(gameCode) );
		temp[sizeof(gameCode)] = NULL;
		kamiFontPrintfConsole(CONSOLE_ORANGE, "GameCode Code = %s\n", temp);

		// Game Version
		kamiFontPrintfConsole(CONSOLE_ORANGE, "GameVersion   = %d\n", info.titleInfo.version);

		// Public Save Size
		kamiFontPrintfConsole(CONSOLE_ORANGE, "PublicSaveSize = %d\n", info.titleInfo.publicSaveSize);

		// Private Save Size
		kamiFontPrintfConsole(CONSOLE_ORANGE, "PrivateSaveSize = %d\n", info.titleInfo.privateSaveSize);

		kamiFontPrintfConsole(CONSOLE_ORANGE, "--------------------------\n");
		return TRUE;
	}
	else
	{
		kamiFontPrintfConsole(CONSOLE_RED, "Fail! retcode=%x\n", nam_result);
		return FALSE;
	}
}
