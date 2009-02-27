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

#include <twl.h>
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu/namut.h>
#include <twl/nam.h>
#include "kami_pxi.h"
#include "kami_font.h"
#include "kami_copy_file.h"
#include "graphics.h"
#include "hwi.h"
#include "keypad.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"
#include "font.h"
#include "kami_global.h"

#define SCRAMBLE_MASK 0x00406000

extern void InitFont(void);

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

s32       gLockId;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);
static BOOL IgnoreRemoval(void);

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
	SNDEX_Init();
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
	kamiFontPrintfConsole(FONT_COLOR_GREEN, "Log Window:\n");

	// メインスレッドのカードロックID取得
	gLockId = OS_GetLockID();

    /* always preload FS table for faster directory access. */
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

	// フォント初期化
	InitFont();

	// コンソールチェック
	ProcessCheckConsole();

	// ログ情報確認
	ProcessCheckLog();

	// リージョン選択
	ProcessSelectRegion();

	// Note表示
	ProcessNote();

	// TWLの更新処理を実行中です
	CARD_LockRom((u16)gLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_NOW_UPDATE);
	CARD_UnlockRom((u16)gLockId);

	// ISデバッガのハードウェアリセットを禁止する
    DEBUGGER_HwResetDisable();

	// HWInfo関連の前準備
	switch (HWI_Init( OS_AllocFromMain, OS_FreeToMain ))
	{
	case HWI_INIT_FAILURE:
		kamiFontPrintfConsole(FONT_COLOR_RED, " Fail! : HWI_INIT()\n");
		break;
	case HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE:
		break;
	case HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE:
		break;
	case HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE:
		break;
	}

	result = TRUE;

	// NANDのフォーマットが古ければフォーマット
	result &= ProcessFormat();

	// 全ハードウェア情報の更新
	result &= ProcessHwinfo();

	// フォントの書き込み
	result &= ProcessWriteFont();

	// cert.sysの書き込み
	result &= ProcessWriteCert();
	
	// ダミーファイルの生成
	result &= ProcessWriteDummy();

	// TADのインポート開始
	result &= ProcessImport( OS_AllocFromMain, OS_FreeToMain );

	// 選択リージョン以外のSystemMenuの消去を行う
	result &= ProcessDeleteOtherResionSysmenu();

	// NANDファームのインストール開始
	result &= ProcessNandfirm();

	// 本体初期化を行う
	result &= ProcessNamutFormat();

	// 更新ログを作成してVersionDownを防ぐ
	if (result)
	{
		ProcessLog();
	}

	// ISデバッガのハードウェアリセットを許可する
    DEBUGGER_HwResetEnable();

	// 結果表示
	ProcessFinish( result );
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
//	kamiFontLoadScreenData();
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
