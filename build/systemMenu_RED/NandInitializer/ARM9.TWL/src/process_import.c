/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
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
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_import.h"
#include "process_eticket.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"

#include "TWLHWInfo_api.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

typedef enum {
	TAD_WRITE_OPTION_OVERWRITE,		// 強制上書き
	TAD_WRITE_OPTION_NONEXISTENT,	// NANDに同プログラムが存在しない場合に限り書き込み
	TAD_WRITE_OPTION_USER			// ユーザーに選択させる
} TadWriteOption;

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define NUM_OF_MENU_SELECT               4
#define NUM_OF_MENU_SELECT_INDIVIDUALLY 17

#define DOT_OF_MENU_SPACE               16
#define DOT_OF_MENU_SPACE_INDIVIDUALLY   8

#define CHAR_OF_MENU_SPACE               2
#define CHAR_OF_MENU_SPACE_INDIVIDUALLY  1

#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      40


// 表示＆インポートできる.TADファイルは最大16個まで
// しかもＳＤカードのルートに存在するファイルのみというお手軽実装
#define FILE_NUM_MAX         16

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static s32 sMenuSelectNo;
static s32 sMenuSelectNoIndividually;

static LCFGReadResult (*s_pReadSecureInfoFunc)( void );

static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];

static u8 sFileNum;

static void* spStack;

static u32  sCurrentProgress;

static vu8 sNowImport = FALSE;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static BOOL ImportTad(char* file_name, TadWriteOption option);
static void ProgressThread(void* arg);
static void Destructor(void* arg);
static void DumpTadInfo(void);
static void MakeFullPathForSD(char* file_name, char* full_path);
static void ShowTitleinfoDifference( NAMTitleInfo* titleInfoNand, NAMTitleInfo* titleInfoSd);
void ProgessInit(void);
void ProgressDraw(f32 ratio);
static void* ImportProcessReturn0(void);
static void* ImportProcessReturn1(void);

static void* ImportIndividuallyProcess0(void);
static void* ImportIndividuallyProcess1(void);
static void* ImportIndividuallyProcess2(void);
static void* ImportIndividuallyProcess3(void);

static void* ImportAllOverwriteProcess0(void);
static void* ImportAllOverwriteProcess1(void);
static void* ImportAllOverwriteProcess2(void);
static void* ImportAllOverwriteProcess3(void);

static void* ImportAllNonexistentProcess0(void);
static void* ImportAllNonexistentProcess1(void);
static void* ImportAllNonexistentProcess2(void);
static void* ImportAllNonexistentProcess3(void);

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Import プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcess0(void)
{
    FATFSFileHandle fat_handle;
    FSFile    dir;
	int i;

	// F:sys/cert.sysが存在しないなら出直してもらう
    fat_handle = FATFS_OpenFile(E_TICKET_FILE_PATH_IN_NAND, "r");
    if (!fat_handle)
    {
		FATFS_CloseFile(fat_handle);
		return ImportProcessReturn0;
    }
	FATFS_CloseFile(fat_handle);

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
	kamiFontFillChar( 0, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 1, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar( 2, BG_COLOR_PINK, BG_COLOR_TRANS );

    // SDカードのルートディレクトリを検索
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
        kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory(sdmc:/)\n");
    }
    else
    {
        FSDirectoryEntryInfo   info[1];
        OS_Printf("[%s]:\n", "sdmc:/");

		kamiFontPrintfConsole(CONSOLE_ORANGE, "------ tad file List -----\n");

		// .dat .nand .nor を探してファイル名を保存しておく
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
					if (!STD_CompareString( pExtension, ".tad") )
					{
						char full_path[FS_ENTRY_LONGNAME_MAX+6];

						// フルパスを作成
						MakeFullPathForSD(info->longname, full_path);

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

//		DumpTadInfo();
    }

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( ImportProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         Import プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcess1(void)
{
	// オート実行用
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return ImportProcess2;
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
		return ImportProcess2;
	}
	// トップメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return ImportProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         Import プロセス２

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcess2(void)
{
	switch( sMenuSelectNo )
	{
	case 0:
		return ImportAllOverwriteProcess0;
		break;
	case 1:
		return ImportAllNonexistentProcess0;
		break;
	case 2:
		FADE_OUT_RETURN( ImportIndividuallyProcess0 );
	case 3:
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return ImportProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         ImportProcessReturn0

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcessReturn0(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();
	kamiFontPrintf(2,  10, FONT_COLOR_RED, "%s is not exist", E_TICKET_FILE_PATH_IN_NAND);
	kamiFontPrintf(2,  11, FONT_COLOR_RED, "You should write e-ticket", E_TICKET_FILE_PATH_IN_NAND);
	kamiFontPrintf(2,  12, FONT_COLOR_RED, "beforehand.", E_TICKET_FILE_PATH_IN_NAND);
	kamiFontPrintf(2,  22, FONT_COLOR_BLACK, "B Button : return to menu");

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import TAD from SD");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

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

	// フォントスクリーンデータロード
	kamiFontLoadScreenData();

	FADE_IN_RETURN( ImportProcessReturn1 );
}

/*---------------------------------------------------------------------------*
  Name:         ImportProcessReturn1

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportProcessReturn1(void)
{
	while(1)
	{
		kamiPadRead();
    	if (kamiPadIsTrigger(PAD_BUTTON_B)) { break; }
	}

	FADE_OUT_RETURN( TopmenuProcess0 );
}

/*---------------------------------------------------------------------------*
     全ファイルインポートプロセス（上書き）
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         全ファイルImport プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

static void* ImportAllOverwriteProcess0(void)
{
	int i;
	BOOL result = TRUE;

	kamiFontPrintf(25,  5, FONT_COLOR_BLACK, "WAIT");

	for (i=0;i<sFileNum;i++)
	{
		// 強制上書き
		if (ImportTad(sFilePath[i], TAD_WRITE_OPTION_OVERWRITE) == FALSE)
		{
			result = FALSE;
		}
	}

	if (result) { kamiFontPrintf(25,  5, FONT_COLOR_GREEN, " OK "); }
	else		{ kamiFontPrintf(25,  5, FONT_COLOR_RED,   " NG "); }

	// Auto用
	if (gAutoFlag)
	{
		if (result && sFileNum > 0) { FADE_OUT_RETURN( AutoProcess1 ); }
		else { FADE_OUT_RETURN( AutoProcess2 ); }
	}

	return ImportProcess1;
}

/*---------------------------------------------------------------------------*
    上書きインポートプロセス（既存ファイルは上書きしない）
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         重複しないファイルImport プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

static void* ImportAllNonexistentProcess0(void)
{
	int i;
	BOOL result = TRUE;

	kamiFontPrintf(25,  7, FONT_COLOR_BLACK, "WAIT");

	for (i=0;i<sFileNum;i++)
	{
		// 書き込みチャレンジ
		if (ImportTad(sFilePath[i], TAD_WRITE_OPTION_NONEXISTENT) == FALSE)
		{
			result = FALSE;
		}
	}

	if (result) { kamiFontPrintf(25,  7, FONT_COLOR_GREEN, " OK "); }
	else		{ kamiFontPrintf(25,  7, FONT_COLOR_RED,   " NG "); }

	return ImportProcess1;
}

/*---------------------------------------------------------------------------*
     個別インポートプロセス
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         個別Import プロセス０

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

static void* ImportIndividuallyProcess0(void)
{
	int i;

	// 文字列全クリア
	kamiFontClear();

	// バージョン表示
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import from SD Card ");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// メニュー一覧
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	for (i=0;i<sFileNum+1;i++)
	{
		kamiFontPrintf(3, (s16)(5+i), FONT_COLOR_BLACK, "l                    l    l");
	}
	kamiFontPrintf(3, (s16)(5+i), FONT_COLOR_BLACK, "+--------------------+----+");

	// tad ファイルリストを表示
	for (i=0;i<sFileNum; i++)
	{
		// ファイル名追加
		kamiFontPrintf(3,  (s16)(5+CHAR_OF_MENU_SPACE_INDIVIDUALLY*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sFilePath[i]);
	}

	// 最後にリターンを追加
	kamiFontPrintf(3, (s16)(5+CHAR_OF_MENU_SPACE_INDIVIDUALLY*sFileNum), FONT_COLOR_BLACK, "l   RETURN           l    l");

	DumpTadInfo();

	// カーソル消去
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( ImportIndividuallyProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         個別 Import プロセス１

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* ImportIndividuallyProcess1(void)
{
	// 選択メニューの変更
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNoIndividually < 0) sMenuSelectNoIndividually = sFileNum;
		DumpTadInfo();
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNoIndividually > sFileNum) sMenuSelectNoIndividually = 0;
		DumpTadInfo();
	}

	// カーソル配置
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNoIndividually * DOT_OF_MENU_SPACE_INDIVIDUALLY));

	// 決定
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return ImportIndividuallyProcess2;
	}
	// ひとつ前のメニューへ戻る
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( ImportProcess0 );
	}

	return ImportIndividuallyProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         個別Import プロセス２

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

#define THREAD_STACK_SIZE (16*1024)

void* ImportIndividuallyProcess2(void)
{
	BOOL ret;

	if (STD_GetStringLength(sFilePath[sMenuSelectNoIndividually]))
	{
		// 個別インポート
		ret = ImportTad(sFilePath[sMenuSelectNoIndividually], TAD_WRITE_OPTION_USER);
	}
	else
	{
		// リターン
		return ImportProcess0;
	}

	// 今回の結果を表示
	if ( ret == TRUE )
	{
		kamiFontPrintf(26, (s16)(5+sMenuSelectNoIndividually*CHAR_OF_MENU_SPACE_INDIVIDUALLY), FONT_COLOR_GREEN, "OK");
	}
	else
	{
		kamiFontPrintf(26, (s16)(5+sMenuSelectNoIndividually*CHAR_OF_MENU_SPACE_INDIVIDUALLY), FONT_COLOR_RED, "NG");
	}

	return ImportIndividuallyProcess1;
}

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ImportTad

  Description:  .tad ファイルインポート

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
static BOOL ImportTad(char* file_name, TadWriteOption option)
{
	NAMTadInfo tadInfo;
	NAMTitleInfo titleInfo;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];
	OSThread thread;
	BOOL ret = FALSE;
	s32  nam_result;

	// フルパスを作成
	MakeFullPathForSD(file_name, full_path);

	// tadファイルの情報取得
	if (NAM_ReadTadInfo(&tadInfo, full_path) != NAM_OK)
	{
		return FALSE;
	}

	// NANDの情報を取得
	if ( option != TAD_WRITE_OPTION_OVERWRITE && NAM_ReadTitleInfo(&titleInfo, tadInfo.titleInfo.titleId) == NAM_OK)
	{
		// NANDに既にインストールされているかどうか確認する
		if (tadInfo.titleInfo.titleId == titleInfo.titleId)
		{
			switch (option)
			{
			case TAD_WRITE_OPTION_NONEXISTENT:
				return TRUE;
			case TAD_WRITE_OPTION_USER:
				ShowTitleinfoDifference(&titleInfo, &tadInfo.titleInfo);

				kamiFontPrintfConsole(1, "The program has already existed.");
				kamiFontPrintfConsole(1, "Do you overwrite ?\n");
				kamiFontPrintfConsole(1, " <Yes: Push A>   <No: Push B>\n");

				// フォントスクリーンデータロード
				kamiFontLoadScreenData();

				while(1)
				{
					kamiPadRead();

				    if (kamiPadIsTrigger(PAD_BUTTON_A))
			    	{
						break;
					}
				    else if (kamiPadIsTrigger(PAD_BUTTON_B))
			    	{
						kamiFontPrintfConsole(CONSOLE_ORANGE, "Import was canceled.\n");
						return TRUE;
					}
				}
			}
		}
	}

	// インポート開始フラグを立てる
	sNowImport = TRUE;

    // 進捗スレッド作成
	spStack = OS_Alloc(THREAD_STACK_SIZE);
	MI_CpuClear8(spStack, THREAD_STACK_SIZE);
    OS_CreateThread(&thread, ProgressThread, NULL,
        (void*)((u32)spStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, OS_THREAD_PRIORITY_MAX);
	// デストラクタセット
	OS_SetThreadDestructor( &thread, Destructor );
    OS_WakeupThreadDirect(&thread);

	// Import開始
	OS_Printf( "Import %s Start.\n", full_path );
	kamiFontPrintfConsole(CONSOLE_ORANGE, "Import %s Start.\n", file_name );

	nam_result = NAM_ImportTad( full_path );

	// 進捗スレッドの自力終了を待つ
	while (sNowImport){ OS_Sleep(1); };

	if ( nam_result == NAM_OK )
	{
		kamiFontPrintfConsole(CONSOLE_ORANGE, "Import %s Sucess.\n", file_name );
		ret = TRUE;
	}
	else
	{
		kamiFontPrintfConsole(1, "Import %s Fail.\n", file_name );
	}

	return ret;
}

static void Destructor(void* /*arg*/)
{
	OS_Free(spStack);
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
    }

	sNowImport = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         DumpTadInfo

  Description:  .tad ファイルの情報を表示する

  Arguments:    arg -   使用しない。

  Returns:      None.
 *---------------------------------------------------------------------------*/

static void DumpTadInfo(void)
{
	NAMTadInfo info;

	// ファイル名の有無を確認
	if (STD_GetStringLength(sFilePath[sMenuSelectNoIndividually]))
	{
		char full_path[FS_ENTRY_LONGNAME_MAX+6];

		// フルパスを作成
		MakeFullPathForSD( sFilePath[sMenuSelectNoIndividually], full_path );

		// TADファイルの情報取得
		if (NAM_ReadTadInfo(&info, full_path) == NAM_OK)
		{
			char temp[100];
			u16 companyCode = MI_SwapEndian16(info.titleInfo.companyCode);
			u32 gameCode = (u32)(info.titleInfo.titleId & 0xffffffff);
			gameCode = MI_SwapEndian32(gameCode);

			kamiFontPrintfConsole(CONSOLE_ORANGE, "------ tad profile -----\n");

			// File Name
			kamiFontPrintfConsole(CONSOLE_ORANGE, "%s\n", sFilePath[sMenuSelectNoIndividually]);

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

			kamiFontPrintfConsole(CONSOLE_ORANGE, "--------------------------\n");
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         MakeFullPathForSD

  Description:  

  Arguments:    no

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void MakeFullPathForSD(char* file_name, char* full_path)
{
	// フルパスを作成
	STD_CopyString( full_path, "sdmc:/" );
	STD_ConcatenateString( full_path, file_name );
}

/*---------------------------------------------------------------------------*
  Name:         ShowTitleinfoDifference

  Description:  既存NANDプログラムとSDプログラムの比較情報を表示する

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ShowTitleinfoDifference( NAMTitleInfo* titleInfoNand, NAMTitleInfo* titleInfoSd)
{
	char tempOld[100];
	char tempNew[100];
	u16 companyCode;
	u32 gameCode;

	kamiFontPrintfConsole(CONSOLE_ORANGE, "+--------------+-------+-------+");
	kamiFontPrintfConsole(CONSOLE_ORANGE, "l              l NAND  l  SD   l");
	kamiFontPrintfConsole(CONSOLE_ORANGE, "+--------------+-------+-------+");
	
	// Company Code (nand)
	companyCode = MI_SwapEndian16(titleInfoNand->companyCode);
	MI_CpuCopy8( &companyCode, tempOld, sizeof(companyCode) );
	tempOld[sizeof(companyCode)] = NULL;

	// Company Code (sd)
	companyCode = MI_SwapEndian16(titleInfoSd->companyCode);
	MI_CpuCopy8( &companyCode, tempNew, sizeof(companyCode) );
	tempNew[sizeof(companyCode)] = NULL;
	kamiFontPrintfConsole(CONSOLE_ORANGE, "l Company Code l  %-2.2s   l  %-2.2s   l", tempOld, tempNew);

	// Game Code (nand)
	gameCode = (u32)(titleInfoNand->titleId & 0xffffffff);
	gameCode = MI_SwapEndian32(gameCode);
	MI_CpuCopy8( &gameCode, tempOld, sizeof(gameCode) );
	tempOld[sizeof(gameCode)] = NULL;

	// Game Code (sd)
	gameCode = (u32)(titleInfoSd->titleId & 0xffffffff);
	gameCode = MI_SwapEndian32(gameCode);
	MI_CpuCopy8( &gameCode, tempNew, sizeof(gameCode) );
	tempNew[sizeof(gameCode)] = NULL;
	kamiFontPrintfConsole(CONSOLE_ORANGE, "l Game Code    l  %-4.4s l  %-4.4s l", tempOld, tempNew);

	// Game Version
	kamiFontPrintfConsole(CONSOLE_ORANGE, "l Game Version l  %-4.4d l  %-4.4d l", titleInfoNand->version, titleInfoSd->version );
	kamiFontPrintfConsole(CONSOLE_ORANGE, "+--------------+-------+-------+");
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

