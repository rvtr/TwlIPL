/*---------------------------------------------------------------------------*
  Project:  ImportJump
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
#include "TWLHWInfo_api.h"
#include "graphics.h"
#include "ImportJump.h"

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

static void* spStack;
static u32  sCurrentProgress;
static vu8 sNowImport = FALSE;
static ImportJump sImportJumpSetting;

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

BOOL kamiImportTad(NAMTitleId* pTitleID)
{
	NAMTadInfo tadInfo;
	NAMTitleInfo titleInfoTmp;
	OSThread thread;
	s32  nam_result;
	FSFile file;
	char savePublicPath[FS_ENTRY_LONGNAME_MAX];
	char savePrivatePath[FS_ENTRY_LONGNAME_MAX];
	char subBannerPath[FS_ENTRY_LONGNAME_MAX];
	int i;

	// 製品用CPUではインポート不可に
	if ( !((*(u8*)(OS_CHIPTYPE_DEBUGGER_ADDR) & OS_CHIPTYPE_DEBUGGER_MASK)) )
	{
		kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail : Production CPU\n");
		return FALSE;
    }

	// ファイル初期化
	FS_InitFile(&file);

	// CARD-ROM 領域を一時的なファイルとみなしそのファイルを開きます。
	if (!FS_CreateFileFromRom(&file, GetImportJumpSetting()->tadRomOffset, GetImportJumpSetting()->tadLength))
	{
		kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail : FS_CreateFileFromRom\n");
		return FALSE;
	}

	// tadファイルの情報取得
	if (NAM_ReadTadInfoWithFile(&tadInfo, &file) != NAM_OK)
	{
		kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! : NAM_ReadTadInfo\n");
		return FALSE;
	}

	// titleIDを保存しておく
	*pTitleID = tadInfo.titleInfo.titleId;

	// Data Only なら失敗
	if (tadInfo.titleInfo.titleId & TITLE_ID_DATA_ONLY_FLAG_MASK)
	{
		kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! : DATA_ONLY_FLAG is specified in rsf file\n");
		return FALSE;
	}

	// freeSoftBoxCountに空きがなければインポートしない
	{
		u8 installed, free;
		if (!NAMUT_GetSoftBoxCount(&installed, &free))
		{
			kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! : Can not get soft box count\n");
			return FALSE;
		}
		if (free == 0)
		{
			kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! : NAND FreeSoftBoxCount == 0\n");
			return FALSE;
		}
	}

	// TADファイルが更新されている場合に限りインポート処理を行う
	// NandInitializerによって消去されている可能性もあるので確認する
	if (GetImportJumpSetting()->importTad == 1 || NAM_ReadTitleInfo(&titleInfoTmp, tadInfo.titleInfo.titleId) != NAM_OK)
	{
/*
		// ESの仕様で古い e-ticket があると新しい e-ticket を使ったインポートができない
		// 暫定対応として該当タイトルを完全削除してからインポートする
		nam_result = NAM_DeleteTitleCompletely(tadInfo.titleInfo.titleId);
		if ( nam_result != NAM_OK )
		{
			kamiFontPrintfConsoleEx(CONSOLE_RED, "Fail! RetCode=%x\n", nam_result);
			return FALSE;
		}
*/
		for (i=0;i<2;i++)
		{
			// インポート開始フラグを立てる
			sNowImport = TRUE;

		    // 進捗スレッド作成
			spStack = OS_Alloc(THREAD_STACK_SIZE);
			MI_CpuClear8(spStack, THREAD_STACK_SIZE);
		    OS_CreateThread(&thread, ProgressThread, NULL,
		        (void*)((u32)spStack + THREAD_STACK_SIZE), THREAD_STACK_SIZE, OS_GetCurrentThread()->priority - 1);
			OS_SetThreadDestructor( &thread, Destructor );
		    OS_WakeupThreadDirect(&thread);

			// Import開始
			nam_result = NAM_ImportTadWithFile( &file );

			// 進捗スレッドの自力終了を待つ
			while (sNowImport){};

			if ( nam_result == NAM_OK )
			{
				break;
			}
			else
			{
				// SystemUpdaterでインポートしたタイトルの上書きインポートには失敗する（鍵が異なるため）
				// Tadのバージョンダウンは失敗する（rsfのRemasterVersion）
				// よってインポートに失敗した場合でも一旦消去してから再トライする（一度のみ）

				NAM_DeleteTitleCompletely(tadInfo.titleInfo.titleId);
			}
		}

		if ( nam_result == NAM_OK )
		{
			// InstalledSoftBoxCount, FreeSoftBoxCount の値を現在のNANDの状態に合わせて更新します。
			if (!NAMUT_UpdateSoftBoxCount())
			{
				kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! : Update Soft Box Count\n");
			}
		}
		else
		{
			kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! : NamInport Result Code = 0x%x\n", nam_result);
			return FALSE;
		}
	}

	// セーブデータクリア処理
	if (GetImportJumpSetting()->clearPublicSaveData || GetImportJumpSetting()->clearPrivateSaveData)
	{
		// セーブファイルパス取得
		if ( NAM_GetTitleSaveFilePath(savePublicPath, savePrivatePath, tadInfo.titleInfo.titleId) != NAM_OK )
		{
			kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! NAM_GetTitleSaveFilePath\n");
		}
		else
		{
			// publicセーブデータFFクリア＆フォーマット
			if (GetImportJumpSetting()->clearPublicSaveData && tadInfo.titleInfo.publicSaveSize > 0)
			{
				if (NAMUTi_ClearSavedataPublic(savePublicPath, tadInfo.titleInfo.titleId) == FALSE)
				{
					kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! NAMUTi_ClearSavedataPublic\n");
				}
			}

			// privateセーブデータFFクリア＆フォーマット
			if (GetImportJumpSetting()->clearPrivateSaveData && tadInfo.titleInfo.privateSaveSize > 0)
			{
				if (NAMUTi_ClearSavedataPrivate(savePrivatePath, tadInfo.titleInfo.titleId) == FALSE)
				{
					kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! NAMUTi_ClearSavedataPrivate\n");
				}
			}
		}
	}

	// サブバナークリア処理
	if (GetImportJumpSetting()->clearSubBannerFile)
	{
		// サブバナーパス取得
		if ( NAM_GetTitleBannerFilePath(subBannerPath, tadInfo.titleInfo.titleId) != NAM_OK )
		{
			kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! NAM_GetTitleBannerFilePath\n");
		}
		else
		{
			// サブバナー破壊
			if (NAMUTi_DestroySubBanner(subBannerPath) == FALSE)
			{
				kamiFontPrintfConsoleEx(CONSOLE_RED, " Fail! NAMUTi_DestroySubBanner\n");
			}
		}	
	}

	return TRUE;
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

	kamiFontPrintfMain( 4, 9, 8, "Now Importing...");
	kamiFontLoadScreenData();

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
  Name:         GetImportJumpSetting

  Description:  

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/

ImportJump* GetImportJumpSetting( void )
{
    static BOOL inited = FALSE;

    if ( ! inited )
    {
        // 開発用CPUでのみリード
        if ( *(u8*)(OS_CHIPTYPE_DEBUGGER_ADDR) & OS_CHIPTYPE_DEBUGGER_MASK )
        {
            u16 id = (u16)OS_GetLockID();
            CARD_LockRom( id );
            CARD_ReadRom( MI_DMA_NOT_USE, (void*)IMPORT_JUMP_SETTING_OFS, &sImportJumpSetting, sizeof(ImportJump) );
            CARD_UnlockRom( id );
        }
        inited = TRUE;
    }

    return &sImportJumpSetting;
}
