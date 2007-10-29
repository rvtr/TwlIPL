/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     DS_Setting.c

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
#include "main.h"
#include "DS_Setting.h"

// define data-----------------------------------------------------------------

// function's prototype--------------------------------------------------------
int DS_SettingMain();
void VBlankIntr_bm(void);

// extern data-----------------------------------------------------------------

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------


// ============================================================================
// function's description
// ============================================================================

int DS_SettingMain()
{
	(void)OS_DisableIrq();
	
//	GXS_DispOff();													// LCDC OFF
	
//	reg_GX_POWCNT = 0x7fff;											// 表示画面を下LCDに切り替え
	
	OS_Printf("ARM9 bootMenu start.\n");
	
	//---- VRAM クリア
//	GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);							// VRAMクリアのために一時的にLCDCにVRAMを全て割り当てる。
//	MI_CpuClearFast((void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
//	(void)GX_DisableBankForLCDC();
	
	//---- OAMとパレットクリア
//	MI_CpuFillFast( (void*)HW_DB_OAM, 192, HW_DB_OAM_SIZE );
//	MI_CpuClearFast((void*)HW_DB_PLTT,     HW_PLTT_SIZE);
	
	//---- VRAMの割り当て
//	GX_SetBankForBG(GX_VRAM_BG_64_E); 								// VRAM割り当て
//	GX_SetBankForOBJ(GX_VRAM_OBJ_32_FG);
	
	//---- グラフィックス表示モードにする
//	GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
	
	//---- BG1の設定
//	G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf000,
//					 GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01 );
//	G2_SetBG1Priority(3);											// BGコントロール セット
	
	//---- OBJ,BG1の表示のみON
	GXS_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1);
	
	//---- OBJは2Dマップモードで使用
	GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);
	
	//---- データロード
//	MI_CpuCopyFast(myChar,	   (void *)HW_BG_VRAM,	sizeof(myChar));	//  BGキャラクタ セット
//	MI_CpuCopyFast(myChar,	   (void *)HW_OBJ_VRAM,	sizeof(myChar));	// OBJキャラクタ セット
//	MI_CpuCopyFast(myPlttData, (void *)HW_BG_PLTT,	sizeof(myPlttData));//  BGパレット   セット
//	MI_CpuCopyFast(myPlttData, (void *)HW_OBJ_PLTT,	sizeof(myPlttData));// OBJパレット   セット
	
	//----  Vブランク割込切り替え
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr_bm);
	(void)OS_EnableIrq();
	
	//---- 表示開始
	OS_WaitIrq(1, OS_IE_V_BLANK);
//	SVC_WaitVBlankIntr();
//	GXS_DispOn();
	
	//---- メインループ前処理
	SEQ_MainMenu_init();
	
	//================ メインループ
	while(1)
	{
		OS_WaitIrq(1, OS_IE_V_BLANK);
//		SVC_WaitVBlankIntr();										// Vブランク割込終了待ち
		ReadKeyPad();
		mf_KEYPAD_rapid();
		
		(void)nowProcess();											// メインプロセス実行
		
		if (PAD_DetectFold() == TRUE) {								// スリープモードへの遷移
			SYSM_GoSleepMode();
		}
		
		OS_PrintServer();											// ARM7からのプリントデバッグを処理する
	}
}

//=============================================================================
// 割り込みルーチン
//=============================================================================

// Vブランク割り込み
void VBlankIntr_bm(void)
{
	//---- OAM、BG-VRAMの更新
	DC_FlushRange(oamBakS, sizeof(oamBakS));
	MI_CpuCopyFast(oamBakS,(void*)HW_DB_OAM, sizeof(oamBakS));
	DC_FlushRange(bgBakS,  sizeof(bgBakS));
	MI_CpuCopyFast(bgBakS, (void*)(HW_DB_BG_VRAM+0xf000), sizeof(bgBakS));
	//---- 割り込みチェックフラグ
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}
