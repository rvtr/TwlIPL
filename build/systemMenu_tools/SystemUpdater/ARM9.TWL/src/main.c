/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     main.c

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

#include <stdlib.h>	// atoi
#include <twl.h>
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu/namut.h>
#include <twl/nam.h>
#include "kami_pxi.h"
#include "kami_font.h"
#include "kami_write_nandfirm.h"
#include "kami_copy_file.h"
#include "import.h"
#include "hw_info.h"
#include "graphics.h"
#include "hwi.h"
#include "keypad.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"

#define SCRAMBLE_MASK 0x00406000

extern const char *g_strIPLSvnRevision;
extern const char *g_strSDKSvnRevision;

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

typedef struct _SystemUpdaterLog
{
	int magic_code;
	int sdk_version;
	int ipl_version;
	int reserve[5];
} SystemUpdaterLog;

typedef struct _CopyFileList
{
	char* srcPath;
	char* dstPath;
} CopyFileList;

/*---------------------------------------------------------------------------*
    内部定数定義
 *---------------------------------------------------------------------------*/

#define SYSTEM_UPDATER_LOG_PATH     "nand:/sys/log/updater.log"
#define NAND_FIRM_PATH_IN_ROM       "rom:/data/menu_launcher.nand"

#define SYSTEM_UPDATER_MAGIC_CODE   44001111

// リトライ回数
#define MAX_RETRY_COUNT   2

static const char* ImportTadFileList[] =
{
	"rom:/data/HNAA.tad",
	"rom:/data/HNBA.tad",
	"rom:/data/HNCA.tad",
	"rom:/data/HNHA.tad",
	"rom:/data/HNLA.tad",
};

static const CopyFileList sCopyFileList[] =
{
	{ "rom:/data/TWLFontTable.dat", "nand:sys/TWLFontTable.dat" },
	{ "rom:/data/cert.sys",         "nand:/sys/cert.sys"        }
};

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static NAMTitleId   titleId;
static s16 printLine;
static vu8 sIsFormatFinish;
static u8 sFormatResult;
static s32       sLockId;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);
static BOOL IgnoreRemoval(void);
static void DrawWaitButtonA(void);
static void DrawInvalidConsole(void);
static void DrawCancel(void);
static void DrawAlready(SystemUpdaterLog* log);
static void DrawResult(BOOL result);
static void FormatCallback(KAMIResult result, void* arg);

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void 
TwlMain()
{
	BOOL result;
	BOOL hw_info_result;
	BOOL nand_firm_result;
	int tadNum;
	int i,j;

    // 製品ビルドランチャー＆デバッガ上での起動対応
    if ( OS_GetRunningConsoleType() & OS_CONSOLE_TWLDEBUGGER )
    {
        ROM_Header *dh = (void *)HW_ROM_HEADER_BUF;
        dh->s.game_cmd_param &= ~SCRAMBLE_MASK;
    }

    OS_Init();
	OS_InitThread();
	OS_InitTick();
	OS_InitAlarm();
    OS_InitArena();
    PXI_Init();
    OS_InitLock();
    OS_InitArenaEx();
    OS_InitIrqTable();
    OS_SetIrqStackChecker();
    MI_Init();
    OS_InitVAlarm();
    OSi_InitVramExclusive();
    OS_InitThread();
    OS_InitReset();
    GX_Init();
    FX_Init();
    SND_Init();
    TP_Init();
    RTC_Init();

    KamiPxiInit();   /* 独自PXI初期化 */

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // initialize file-system
	FS_Init(FS_DMA_NOT_USE);

	InitAllocation();

	// NAMライブラリ初期化
	NAM_Init( OS_AllocFromMain, OS_FreeToMain );
	NAMUT_Init( OS_AllocFromMain, OS_FreeToMain );	// SoftBoxCountの計算に必要

    // 表示関連初期化
    InitGraphics();
	kamiFontInit();

	// メインスレッドのカードロックID取得
	sLockId = OS_GetLockID();

    /* always preload FS table for faster directory access. */
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

	// コンソールチェック
	{
		u32 console = OS_GetRunningConsoleType();
		enum { IS_TWL_DEBUGGER=0, IS_TWL_CAPTURE, TWL, UNKNOWN };
		int running = UNKNOWN;
		BOOL isAdapter;
		u16 batLevel;

		// SystemUpdaterはデバッグ不可で作成されるためOS_CONSOLE_TWLが取得される
		// 赤箱にカードを挿してSystemUpdaterを実行した場合も同様（但しOS_CONSOLE_TWLTYPE_RETAILにはならない）
		// デバッガかどうかの判定はメモリサイズチェックにより行う
		// 念のためOS_CONSOLE_TWLTYPE_RETAILでないことも確認する

		if ((console & OS_CONSOLE_SIZE_MASK) == OS_CONSOLE_SIZE_32MB)
		{
			if ((console & OS_CONSOLE_TWLTYPE_MASK) != OS_CONSOLE_TWLTYPE_RETAIL)
			{
				IsToolType type;
				kamiGetIsToolType(&type);
				if (type == IS_TOOL_TYPE_DEBUGGER)
				{
					running = IS_TWL_DEBUGGER;
				}
				else if (type == IS_TOOL_TYPE_ERROR) // TSボードプラス + 旧仕様デバッガ
				{
					running = IS_TWL_DEBUGGER;
				}
				else if (type == IS_TOOL_TYPE_CAPTURE)
				{
					running = IS_TWL_CAPTURE;
				}
			}
		}
		else if ((console & OS_CONSOLE_MASK) == OS_CONSOLE_TWL)
		{
			IsToolType type;
			kamiGetIsToolType(&type);
			if (type == IS_TOOL_TYPE_CAPTURE)
			{
				running = IS_TWL_CAPTURE;
			}
			else
			{
				running = TWL;
			}
		}

		switch (running)
		{
		case IS_TWL_DEBUGGER:
			kamiFontPrintf( 0, printLine++, FONT_COLOR_GREEN, "Running on IS_TWL_DEBUGGER.");
			break;
		case IS_TWL_CAPTURE:
			kamiFontPrintf( 0, printLine++, FONT_COLOR_GREEN, "Running on IS_TWL_CAPTURE.");
			break;
		case TWL:
			kamiFontPrintf( 0, printLine++, FONT_COLOR_GREEN, "Running on TWL CONSOLE.");
			break;
		case UNKNOWN:
			kamiFontPrintf( 0, printLine++, FONT_COLOR_GREEN, "Running on UNKNOWN.");
			break;
		}

#ifdef SYSM_BUILD_FOR_DEBUGGER
		// デバッガ向けSystemUpdaterは実機とキャプチャでは動作させない
		if (running != IS_TWL_DEBUGGER)
		{
			kamiFontPrintfMain( 2,  9, 3, " Sorry,                     ");
			kamiFontPrintfMain( 2, 10, 3, " This SystemUpdater can not ");
			kamiFontPrintfMain( 2, 11, 3, " execute on TWL-CONSOLE.    ");
			DrawInvalidConsole();
		}
#else
	    // 実機向けSystemUpdaterはデバッガでは動作させない
		if (running == IS_TWL_DEBUGGER)
		{
			kamiFontPrintfMain( 2,  9, 3, " Sorry,                     ");
			kamiFontPrintfMain( 2, 10, 3, " This SystemUpdater can not ");
			kamiFontPrintfMain( 2, 11, 3, " execute on IS-TWL-DEBUGGER.");
			DrawInvalidConsole();
		}
#endif  // SYSM_BUILD_FOR_DEBUGGER

	    // UNKNOWNは動作させない
		if (running == UNKNOWN)
		{
			kamiFontPrintfMain( 2,  9, 3, " Sorry,                     ");
			kamiFontPrintfMain( 2, 10, 3, " This SystemUpdater can not ");
			kamiFontPrintfMain( 2, 11, 3, " execute on UNKNOWN CONSOLE.");
			DrawInvalidConsole();
		}

	    // 電池残量が少なければ動作させない
		while (PM_GetBatteryLevel( &batLevel ) != PM_RESULT_SUCCESS)
		{
			OS_Sleep(1);
		}
		while (PM_GetACAdapter( &isAdapter ) != PM_RESULT_SUCCESS)
		{
			OS_Sleep(1);
		}
		if (((batLevel <= 2) && ! isAdapter) ||
			 (batLevel <= 1))
		{
			kamiFontPrintfMain( 2,  9, 3, " Sorry,                     ");
			kamiFontPrintfMain( 2, 10, 3, " This SystemUpdater can not ");
			kamiFontPrintfMain( 2, 11, 3, " execute if battery is low. ");
			DrawInvalidConsole();
		}
	}

	// （更新可能条件）
	//  1.ログが存在しない
	//  2.ログが存在し、ログに記載のマジックコードが不正（初版SystemUpdater実行後の状態）
	//  2.ログが存在し、ログに記載のマジックコードが正しくかつログに記載の 
    //    SDK & IPL のバージョンが SystemUpdater のそれ以下である

#ifdef IGNORE_VERSION_CHECK
	if( 0 )
#endif // IGNORE_VERSION_CHECK
	{
		SystemUpdaterLog log;
		FSFile file;
		FS_InitFile( &file );

		if (FS_OpenFileEx(&file, SYSTEM_UPDATER_LOG_PATH, FS_FILEMODE_R) == TRUE)
		{
			DC_InvalidateRange(&log, sizeof(log));

			if (FS_ReadFile(&file, &log, sizeof(log)) == sizeof(log))
			{
				// ログリード成功
				OS_Printf("[%d, %d]\n", log.sdk_version, log.ipl_version);

				// 初版SystemUpdater実行状態でないことをマジックコードで判別する
				if (log.magic_code == SYSTEM_UPDATER_MAGIC_CODE)
				{
					// マジックコード、SDKバージョン、IPLバージョンの確認
					if (log.sdk_version > atoi(g_strSDKSvnRevision) || 
						log.ipl_version > atoi(g_strIPLSvnRevision))
					{
						// 更新不可
						DrawAlready(&log);
					}
				}
			}
			else
			{
				// ログリード失敗
				OS_Warning("Failure! FS_ReadFile");
			}

			FS_CloseFile(&file);
		}
	}

	// Ａボタン待ち
	DrawWaitButtonA();

	// TWLの更新処理を実行中です
	CARD_LockRom((u16)sLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_NOW_UPDATE);
	CARD_UnlockRom((u16)sLockId);

	// ISデバッガのハードウェアリセットを禁止する
    DEBUGGER_HwResetDisable();

	// HWInfo関連の前準備
	switch (HWI_Init( OS_AllocFromMain, OS_FreeToMain ))
	{
	case HWI_INIT_FAILURE:
		OS_Warning(" Fail! : HWI_INIT()");
		break;
	case HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE:
		break;
	case HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE:
		break;
	case HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE:
		break;
	}

	result = TRUE;

	// フォーマット実行
	sIsFormatFinish = FALSE;
    ExeFormatAsync(FORMAT_MODE_QUICK, FormatCallback);
	kamiFontPrintfMain( 7, 11, 8, "Now Format...");
	while(!sIsFormatFinish){};
	if (sFormatResult)
	{
		kamiFontPrintf( 0, printLine++, FONT_COLOR_GREEN, "NAND Format Success.");
	}
	else
	{
		kamiFontPrintf( 0, printLine++, FONT_COLOR_RED, "NAND Format Failure!");		
	}
	kamiFontPrintfMain( 7, 11, 8, "              ");

	// フォーマット後はESに必要なファイルがなくなっているため
	// ES_InitLibを呼び出すことで作成しておく
	NAM_End( NULL, NULL );
	NAM_Init( OS_AllocFromMain, OS_FreeToMain );

	// 全ハードウェア情報の更新
	for (i=0;i<MAX_RETRY_COUNT;i++)
	{
		hw_info_result = WriteHWInfoFile(OS_GetRegion(), OS_IsForceDisableWireless());
		if (hw_info_result)
		{
			kamiFontPrintf( 0, printLine++, FONT_COLOR_GREEN, "Write Hardware Info Success.");			
			break;
		}
		else
		{
			kamiFontPrintfConsole(CONSOLE_RED, "Write Hardware Info Retry!\n");
		}
	}
	if ( hw_info_result == FALSE)
	{
		result = FALSE;
		kamiFontPrintf( 0, printLine++, FONT_COLOR_RED, "Write Hardware Info Failure!");			
	}

	// 必要なファイルの書き込み
	for (i=0;i<sizeof(sCopyFileList)/sizeof(sCopyFileList[0]);i++)
	{
		if (kamiCopyFile(sCopyFileList[i].srcPath, sCopyFileList[i].dstPath))
		{
			kamiFontPrintf( 0, printLine++, FONT_COLOR_GREEN, "Write Data File %d Success.", i);			
		}
		else
		{
			result = FALSE;
			kamiFontPrintf( 0, printLine++, FONT_COLOR_RED, "Write Data File %d Failure!", i);
		}
	}

	// ダミーのラッピングデータ書き込み
	result &= kamiWriteWrapData();

	// TADのインポート開始
	tadNum = sizeof(ImportTadFileList)/sizeof(ImportTadFileList[0]);

	for (i=0; i<tadNum; i++)
	{
		s32  nam_result;
	
		// MAX_RETRY_COUNTまでリトライする
		for (j=0; j<MAX_RETRY_COUNT; j++)
		{	
			nam_result = kamiImportTad(i+1, tadNum, ImportTadFileList[i]);
			if (nam_result == NAM_OK)
			{
				break;
			}
			else
			{
				kamiFontPrintfConsole(CONSOLE_RED, "Import %d Retry!\n", i+1);
			}
		}

		if ( nam_result == NAM_OK)
		{
			kamiFontPrintf( 0, printLine++, FONT_COLOR_GREEN, "List : %d Update Success.", i+1 );			
		}
		else
		{
			kamiFontPrintf( 0, printLine++, FONT_COLOR_RED, "Error: %d : RetCode = %d", i+1, nam_result );
			result = FALSE;
		}
	}

	// NANDファームのインストール開始
	for (j=0;j<MAX_RETRY_COUNT;j++)
	{
		nand_firm_result = kamiWriteNandfirm(NAND_FIRM_PATH_IN_ROM, OS_AllocFromMain, OS_FreeToMain);
		if (nand_firm_result)
		{
			kamiFontPrintf( 0, printLine++, FONT_COLOR_GREEN, "Firm Update Success.");			
			break;
		}
		else
		{
			kamiFontPrintfConsole(CONSOLE_RED, "Write Firm Retry!\n");
		}
	}
	if ( nand_firm_result == FALSE)
	{
		result = FALSE;
		kamiFontPrintf( 0, printLine++, FONT_COLOR_RED, "Firm Update Failure!");
	}

	// 更新ログを作成して再実行を防ぐ
	if (result)
	{
		SystemUpdaterLog log;

		(void)FS_DeleteFile(SYSTEM_UPDATER_LOG_PATH);

		if (FS_CreateFileAuto(SYSTEM_UPDATER_LOG_PATH, FS_PERMIT_R | FS_PERMIT_W))
		{
			FSFile file;
			FS_InitFile( &file );
			if (FS_OpenFileEx(&file, SYSTEM_UPDATER_LOG_PATH, FS_FILEMODE_W))
			{
				log.magic_code  = SYSTEM_UPDATER_MAGIC_CODE;
				log.sdk_version = atoi(g_strSDKSvnRevision);
				log.ipl_version = atoi(g_strIPLSvnRevision);
				
				DC_FlushRange(&log, sizeof(log));

				if (FS_WriteFile(&file, (void*)&log, sizeof(log) ) == -1)
				{
					OS_Warning("Failure : FS_WriteFile\n");
				}
				FS_CloseFile(&file);
			}
		}
		else
		{
			OS_Warning("Failure : FS_CreateFileAuto\n");
		}
	}

	// ISデバッガのハードウェアリセットを許可する
    DEBUGGER_HwResetEnable();

	// TWLの更新処理が完了しました
	CARD_LockRom((u16)sLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_FINISHED);
	CARD_UnlockRom((u16)sLockId);

	// 結果表示
	while(1)
	{
		DrawResult(result);
	    OS_WaitVBlankIntr();
	};
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  VBlank割り込み処理

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void 
VBlankIntr(void)
{
	kamiFontLoadScreenData();
    OS_SetIrqCheckFlag(OS_IE_V_BLANK); // checking VBlank interrupt
}

/*---------------------------------------------------------------------------*
  Name:         InitAllocation

  Description:  ヒープの初期化.

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitAllocation(void)
{
    void   *tmp;
    OSHeapHandle hh;

    /* アリーナの初期化 */
    tmp = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tmp);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
        OS_Panic("ARM9: Fail to create heap...\n");
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

/*---------------------------------------------------------------------------*
  Name:         DrawWaitButtonA

  Description:  Aボタン待ちを表示します

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawWaitButtonA(void)
{
	// 液晶を見てください。
	CARD_LockRom((u16)sLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_LOOK_SCREEN);
	CARD_UnlockRom((u16)sLockId);

	kamiFontPrintfMain( 5,  3, 8, "    System Updater    ");
	kamiFontPrintfMain( 4,  5, 8, " --- ver %s %s ---", g_strSDKSvnRevision, g_strIPLSvnRevision );

	kamiFontPrintfMain( 5,  9, 3, " A Button: Start  Update ");
	kamiFontPrintfMain( 5, 10, 3, " B Button: Cancel Update ");

	kamiFontPrintfMain( 3, 13, 1, "Do not turn off power");
	kamiFontPrintfMain( 3, 14, 1, "while update is processing");


	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		DrawQuad( 10,  54, 246, 150, GX_RGB(28, 28, 28));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

		kamiPadRead();
		if (kamiPadIsTrigger(PAD_BUTTON_A))
		{
			kamiFontClearMain();
			break;
		}
		else if (kamiPadIsTrigger(PAD_BUTTON_B))
		{
			kamiFontClearMain();
			DrawCancel();
		}

	    OS_WaitVBlankIntr();
	}

	G3X_Reset();
	G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
	OS_WaitVBlankIntr();
}

/*---------------------------------------------------------------------------*
  Name:         DrawInvalidConsole

  Description:  コンソール条件による失敗を表示します。

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawInvalidConsole(void)
{
	// キャンセルされました
	CARD_LockRom((u16)sLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_CANCELED);
	CARD_UnlockRom((u16)sLockId);

	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		DrawQuad( 10,  50, 246, 120, GX_RGB(28, 28, 28));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

	    OS_WaitVBlankIntr();
	}
}

/*---------------------------------------------------------------------------*
  Name:         DrawCancel

  Description:  Cancelを表示します

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawCancel(void)
{
	// キャンセルされました
	CARD_LockRom((u16)sLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_CANCELED);
	CARD_UnlockRom((u16)sLockId);

	kamiFontPrintfMain( 3,  9, 1, "--------------------------");
	kamiFontPrintfMain( 3, 10, 1, "    Update was Canceld.");
	kamiFontPrintfMain( 3, 11, 1, "--------------------------");

	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		DrawQuad( 10,  50, 246, 128, GX_RGB(28, 28, 28));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

	    OS_WaitVBlankIntr();
	}
}

/*---------------------------------------------------------------------------*
  Name:         DrawAlready

  Description:  Alreadyを表示します

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawAlready(SystemUpdaterLog* log)
{
	// 既にアップデート済み
	CARD_LockRom((u16)sLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_ALREADY);
	CARD_UnlockRom((u16)sLockId);

	kamiFontPrintfMain( 3,  8, 1, "--------------------------");
	kamiFontPrintfMain( 3,  9, 1, "This machine has already");
	kamiFontPrintfMain( 3, 10, 1, "been updated.");
	kamiFontPrintfMain( 3, 12, 1, "ver: %d %d", log->sdk_version, log->ipl_version );
	kamiFontPrintfMain( 3, 13, 1, "--------------------------");

	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		DrawQuad( 10,  50, 246, 128, GX_RGB(28, 28, 28));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

	    OS_WaitVBlankIntr();
	}
}

/*---------------------------------------------------------------------------*
  Name:         DrawResult

  Description:  処理結果を表示します。

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawResult(BOOL result)
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

/*---------------------------------------------------------------------------*
  Name:         FormatCallback

  Description:  フォーマットコールバック

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void FormatCallback(KAMIResult result, void* /*arg*/)
{
	if ( result == KAMI_RESULT_SUCCESS_TRUE )
	{
		sFormatResult = TRUE;
	}
	else
	{
		sFormatResult = FALSE;
	}

	sIsFormatFinish = TRUE;
}
