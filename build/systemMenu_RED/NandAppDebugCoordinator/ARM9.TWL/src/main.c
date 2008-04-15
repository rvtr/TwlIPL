/*---------------------------------------------------------------------------*
  Project:  NandAppDebugCoordinator
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
#include <nitro/card.h>
#include <twl/nam.h>
#include "kami_font.h"
#include "import.h"
#include "graphics.h"
#include "hwi.h"


#define DEBUG_TARGET_TAD_FILE_PATH   "hostio:/c:/TwlIPL/build/systemMenu_RED/MachineSettings/ARM9/bin/ARM9-TS.LTD/Release/HNBA.Release.tad"


extern void HWInfoWriterInit( void );
extern void FS_MountHostIO(const char *basepath);

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static NAMTitleId   titleId;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);

// 現時点ではカード割り込みをHIO通知に使用しているようなので
// カード抜け誤検出を無視するようにしてテストを作成している。
static BOOL IgnoreRemoval(void)
{
    OS_TWarning("detected CARD-removal!(miss-notification from debugger Host-I/O)\n");
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void 
TwlMain()
{
    OS_Init();
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

	InitAllocation();

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);

    // initialize file-system
	FS_Init(FS_DMA_NOT_USE);

    // HostI/Oをマウント。
    {
        CARD_SetPulledOutCallback(IgnoreRemoval);
        FS_MountHostIO("c:");
    }

	// NAMライブラリ初期化
	NAM_Init( OS_AllocFromMain, OS_FreeToMain);

    // 表示関連初期化
    InitGraphics();
	kamiFontInit();

    /* always preload FS table for faster directory access. */
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

	// HWInfo関連の前準備
	// InstalledSoftBoxCount, FreeSoftBoxCount の更新のために必要
	switch (HWI_Init( OS_AllocFromMain, OS_FreeToMain ))
	{
	case HWI_INIT_FAILURE:
//		kamiFontPrintfConsoleEx(CONSOLE_RED, "HWI_INIT() Failure!\n" );
		OS_Warning(" Fail! : HWI_INIT()");
		break;
	case HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE:
//		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, "[PRO Signature MODE]\n" );
		break;
	case HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE:
//		kamiFontPrintfConsoleEx(CONSOLE_ORANGE, "[DEV Signature MODE]\n" );
		break;
	case HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE:
//		kamiFontPrintfConsoleEx(CONSOLE_RED, "[No Signature MODE]\n" );
		break;
	}

	// TADのインポート開始
	if (kamiImportTad(DEBUG_TARGET_TAD_FILE_PATH, &titleId))
	{
		// インポートに成功したならアプリジャンプ
		OS_DoApplicationJump( titleId, OS_APP_JUMP_NORMAL );
	}

	while(1){};
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

