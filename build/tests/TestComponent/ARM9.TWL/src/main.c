/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
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
#include <twl/os/common/format_rom.h>
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "graphics.h"
#include "keypad.h"
#include "kami_pxi.h"

#define SCRAMBLE_MASK 0x00406000

extern void HWInfoWriterInit( void );

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/
typedef void*  (*Process)(void);

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static Process sProcess;
static FSEventHook  sSDHook;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);

static void* DummyProcess0(void)
{
    return DummyProcess0;
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
    // 製品ビルドランチャー＆デバッガ上での起動対応
    if ( OS_GetRunningConsoleType() & OS_CONSOLE_TWLDEBUGGER )
    {
        ROM_Header *dh = (void *)HW_ROM_HEADER_BUF;
        dh->s.game_cmd_param &= ~SCRAMBLE_MASK;
    }

    OS_Init();
	OS_InitTick();
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

#ifndef NAND_INITIALIZER_LIMITED_MODE
    KamiPxiInit();   /* 独自PXI初期化 */
#endif

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
	
    // initialize file-system
	FS_Init(FS_DMA_NOT_USE);

	// FATドライバを初期化します。
	// NANDにアクセスするために必要です。
	// この関数はTwlSDK_branchUIGには含まれていません。
	FS_InitFatDriver();

	// SDカードの挿抜イベント監視コールバック設定
//  FS_RegisterEventHook("sdmc", &sSDHook, SDEvents, NULL);

	// FS_Initの後の方が良い模様
	InitAllocation();

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
    
	// 初期シーケンス設定
	sProcess = DummyProcess0;

	kamiFontPrintfConsole( CONSOLE_ORANGE, "This is Test Component\n");

#ifdef TWL_CAPTURE_VERSION
	// memory-launcher経由で立ち上がるTWLCaptureSystemWriterでは
	// RED_LAUNCHER_VERが0でフォーマットに失敗するので強制的に書き換える
	MI_StoreLE8((void*)HW_TWL_RED_LAUNCHER_VER, 1);
#endif

#ifdef AUTO_FORMAT_MODE
//  検査工程ではNANDが初期化されていないがその状態でFATにアクセスすると
//  問題があるため強制的にフォーマットを行う 2008/06/18
//	ExeFormat(FORMAT_MODE_QUICK);
//
//  Secure情報を読み込む前にフォーマットしてしまうとSecure情報が消えてしまう。
//  そうするとシリアルNoが書き換わってしまうのでこの処理はなくす。
//  検査工程と異なりNandInitializerが実行される段階ではフォーマット済みなはず。
//  仮に未フォーマットであったとしてもBadFormat扱いとなる。2009/03/05
#endif

	// NAMライブラリ初期化
	NAM_Init( OS_AllocFromMain, OS_FreeToMain);
	NAMUT_Init( OS_AllocFromMain, OS_FreeToMain);

/*
{
	u8 temp;
	kamiCodecRead( 1, 31, &temp);
	OS_Printf("temp = %x\n", temp);
	kamiCodecWrite( 1, 31, (temp & 0x7f));
	kamiCodecRead( 1, 31, &temp);
	OS_Printf("temp = %x\n", temp);

}
*/

    while (1)
    {
		kamiPadRead();

        // コマンドフラッシュ
//      (void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

		// Vブランク待ち
        OS_WaitVBlankIntr();

        // ＡＲＭ７コマンド応答受信
//      while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
//      {
//      }

		// フォントスクリーンデータロード
		kamiFontLoadScreenData();

		sProcess = sProcess();
    }
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

