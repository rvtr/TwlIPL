/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

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
#include <sysmenu/namut.h>
#include "misc.h"
#include "HWInfoWriter.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------


// ============================================================================
// function's description
// ============================================================================
void TwlMain(void)
{
	// 初期化----------------------------------
    OS_Init();
	OS_InitTick();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
    GX_Init();
	GX_SetPower(GX_POWER_ALL);										// 各ロジック パワーON
	
	// 割り込み許可----------------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// デバイス初期化-------------------------------
	(void)RTC_Init();
	
	// システムの初期化------------------
	InitAllocator();
	
	// SoftBoxCountを調べるのにNAM/NAMUTを使用する
	NAM_Init( Alloc, Free );
	NAMUT_Init( Alloc, Free );

	// メインループ----------------------------
	HWInfoWriterInit();
	while(1){
		OS_WaitIrq(1, OS_IE_V_BLANK);								// Vブランク割り込み待ち
		ReadKeyPad();												// キー入力の取得
		
		HWInfoWriterMain();
	}
}


// ============================================================================
// 割り込み処理
// ============================================================================

// Vブランク割り込み
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// Vブランク割込チェックのセット
}

