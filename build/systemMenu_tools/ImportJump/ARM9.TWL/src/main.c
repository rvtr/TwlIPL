/*---------------------------------------------------------------------------*
  Project:  ImportJump
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
#include <twl/lcfg.h>
#include <twl/os/common/format_rom.h>
#include <sysmenu/namut.h>

#include "kami_font.h"
#include "import.h"
#include "graphics.h"
#include "hwi.h"
#include "ImportJump.h"

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static NAMTitleId   titleId;
char sTadPath[FS_ENTRY_LONGNAME_MAX];

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
	NAMTitleId titleID;

	// OS_Initより前に実行する
	{
		// SRLの後方に配置したTADファイルにアクセス可能にするために
		// カードアクセスのハッシュチェックを無効化する
        ROM_Header_Short *th = (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF;
        ROM_Header_Short *dh = (ROM_Header_Short *)HW_ROM_HEADER_BUF;
        th->enable_signature = FALSE;
        dh->enable_signature = FALSE;
		// デバッガ情報を読み取るため拡張メモリを有効にする
		OS_EnableMainExArena();
	}

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

	// NAMライブラリ初期化
	NAM_Init( OS_AllocFromMain, OS_FreeToMain );
	NAMUT_Init( OS_AllocFromMain, OS_FreeToMain );

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

	// magicCodeが異なる場合は停止
	if (STD_CompareNString( (char *)&GetImportJumpSetting()->magicCode, "TWLD", 4 ))
	{
		OS_Warning(" Magic Code Wrong!\n");
		while(1){};
	}

	// HWInfo関連の前準備
	// InstalledSoftBoxCount, FreeSoftBoxCount の更新のために必要
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

	// 前回起動したソフトのplatformCodeがNITROだとアプリジャンプに失敗する。
	// デバッガ動作時はランチャーが前回起動したソフトのplatformCode & TitleID を
	// 更新しないためImportJump自身が更新しておく。NANDへの反映はkamiImportTad()
	// の中で行われる。
	LCFG_TSD_SetLastTimeBootSoftPlatform( PLATFORM_CODE_TWL_LIMITED );
	LCFG_TSD_SetLastTimeBootSoftTitleID( OS_GetTitleId() );

	// TADのインポート開始
	if (kamiImportTad(&titleID))
	{
		// アプリジャンプのためにジャンプ可能リストに直接追加する
    	OSTitleIDList *list  = ( OSTitleIDList * )HW_OS_TITLE_ID_LIST;
		list->TitleID[0]     = titleID;
		list->appJumpFlag[0] = 0x01;
		list->num = 1;

		// アプリジャンプ
		OS_DoApplicationJump( titleID, OS_APP_JUMP_NORMAL );
	}

	// アプリジャンプに成功したならここへは到達しない
	while(1)
	{
		kamiFontLoadScreenData();
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
