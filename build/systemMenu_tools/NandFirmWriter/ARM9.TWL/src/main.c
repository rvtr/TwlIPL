/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2009-04-16#$
  $Rev: 2809 $
  $Author: kamikawa $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <twl/nam.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu/namut.h>
#include "kami_pxi.h"
#include "common.h"
#include "screen.h"
#include "kami_write_nandfirm.h"
#include "hwi.h"

#define SCRAMBLE_MASK 0x00406000

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static FSEventHook  sSDHook;

// キー入力
static KeyInfo  gKey;

// 書き込み結果
static BOOL gProc;
static BOOL gResult;

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);
static void InitAllocation(void);
static void DrawScene(void);

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

	// SDカードの挿抜イベント監視コールバック設定
//  FS_RegisterEventHook("sdmc", &sSDHook, SDEvents, NULL);

	// FS_Initの後の方が良い模様
	InitAllocation();

    InitScreen();

    GX_DispOn();
    GXS_DispOn();

	ClearScreen();

    /* always preload FS table for faster directory access. */
    {
        u32     need_size = FS_GetTableSize();
        void   *p_table = OS_Alloc(need_size);
        SDK_ASSERT(p_table != NULL);
        (void)FS_LoadTable(p_table, need_size);
    }

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

#ifndef TWL_CAPTURE_VERSION
	// HWInfo関連の前準備
	switch (HWI_Init( OS_AllocFromMain, OS_FreeToMain ))
	{
	case HWI_INIT_FAILURE:
        OS_PutString("HWI_INIT() Failure!\n");
		break;
	case HWI_INIT_SUCCESS_PRO_SIGNATURE_MODE:
		OS_PutString("[PRO Signature MODE]\n");
		break;
	case HWI_INIT_SUCCESS_DEV_SIGNATURE_MODE:
        OS_PutString("[DEV Signature MODE]\n");
		break;
	case HWI_INIT_SUCCESS_NO_SIGNATRUE_MODE:
        OS_PutString("[No Signature MODE]\n");
		break;
	}
#endif

    gResult = FALSE;
    gProc = FALSE;

    while (1)
    {
        // キー入力情報取得
		ReadKey(&gKey);

		if (gKey.trg & PAD_BUTTON_A)
        {
            OS_PutString("A\n");
            
    		gResult = FALSE;
    		gProc = FALSE;
            
			// 書き込み関数
            gResult = NandfirmProcess();
            
            gProc = TRUE;
        }
        
        DrawScene();

        // コマンドフラッシュ
      	(void)SND_FlushCommand(SND_COMMAND_NOBLOCK);

		// Vブランク待ち
        OS_WaitVBlankIntr();

        // ＡＲＭ７コマンド応答受信
      	while (SND_RecvCommandReply(SND_COMMAND_NOBLOCK) != NULL)
      	{
      	}

        // 画面クリア
        ClearScreen();
    }
}


static void DrawScene(void)
{
    PutMainScreen( 5, 2, 0xf8, " ------------------- ");
    PutMainScreen( 5, 3, 0xf8, " -                 - ");
	PutMainScreen( 5, 4, 0xf8, " - NandFirm Writer - ");
    PutMainScreen( 5, 5, 0xf8, " -                 - ");
	PutMainScreen( 5, 6, 0xf8, " ------------------- ");

	PutMainScreen( 3, 9, 0xff, "Please Insert SD Card and");
	PutMainScreen( 3,11, 0xff, "Push A Button.");
    
    if( gProc )
    {
		if( gResult )
    	{
			PutMainScreen( 4, 15, 0xf2, "NandFirm Write Successed!!");
    	}
    	else
	    {
			PutMainScreen( 4, 15, 0xf1, "NandFirm Write Failed...");
    	}
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
    // テキスト表示を更新
    UpdateScreen();
    
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

