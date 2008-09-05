/*---------------------------------------------------------------------------*
  Project:  TwlSDK - nandfirmVersionChecker
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
#include <twl/os/common/format_rom.h>
#include <firm/format/nandfirm.h>
#include "kami_font.h"
#include "graphics.h"
#include "keypad.h"
#include "kami_pxi.h"

#define SCRAMBLE_MASK     0x00406000
#define NAND_BLOCK_SIZE   0x200

extern void HWInfoWriterInit( void );

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static FSEventHook  sSDHook;
static u8 tempBuf[NAND_BLOCK_SIZE];
/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);

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

    KamiPxiInit();   /* 独自PXI初期化 */

    // Vブランク割り込み設定
    OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)OS_EnableIrqMask(OS_IE_FIFO_RECV);
    (void)OS_EnableIrq();
    (void)GX_VBlankIntr(TRUE);
	
    // initialize file-system
	FS_Init(FS_DMA_NOT_USE);

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

	kamiFontPrintfConsole( CONSOLE_ORANGE, "+------------------------------+");
	kamiFontPrintfConsole( CONSOLE_ORANGE, "l   nandfirm Version Checker   l");
	kamiFontPrintfConsole( CONSOLE_ORANGE, "+------------------------------+\n");

	DC_FlushRange(tempBuf, sizeof(tempBuf));

	// ブロック単位、バイト単位、ブロック単位
	if (kamiNandRead(1, tempBuf, 1) == KAMI_RESULT_SEND_ERROR)
	{
	    kamiFontPrintfConsoleEx(1, "kamiNandRead ... %s!\n", "ERROR");
	}
	DC_FlushRange(tempBuf, NAND_BLOCK_SIZE);

	{
		NANDHeaderLow* header = (NANDHeaderLow *)tempBuf;
		u32 offsetbyte  = header->sub_rom_offset + header->sub_size;
		u32 offsetblock = offsetbyte / NAND_BLOCK_SIZE;

		// ブロック単位、バイト単位、ブロック単位
		if (kamiNandRead(offsetblock, tempBuf, 1) == KAMI_RESULT_SEND_ERROR)
		{
		    kamiFontPrintfConsoleEx(1, "kamiNandRead ... %s!\n", "ERROR");
		}
		DC_FlushRange(tempBuf, NAND_BLOCK_SIZE);

		kamiFontPrintfConsole( CONSOLE_ORANGE, "%s\n", tempBuf);
	}

    while (1)
    {
		kamiPadRead();

		// Vブランク待ち
        OS_WaitVBlankIntr();

		// フォントスクリーンデータロード
		kamiFontLoadScreenData();
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

