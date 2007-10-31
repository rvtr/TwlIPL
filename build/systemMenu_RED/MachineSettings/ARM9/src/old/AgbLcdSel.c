/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     AgbLcdSel.c

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
#include <sysmenu.h>
#include "misc.h"
#include "DS_Setting.h"

// define data------------------------------------------
#define CANCEL_BUTTON_LT_X					2
#define CANCEL_BUTTON_LT_Y					21
#define CANCEL_BUTTON_RB_X					(CANCEL_BUTTON_LT_X+8)
#define CANCEL_BUTTON_RB_Y					(CANCEL_BUTTON_LT_Y+2)

// extern data------------------------------------------

// function's prototype declaration---------------------
void SEQ_AgbLcdSelect_init(void);
int  SEQ_AgbLcdSelect(void);

// global variable -------------------------------------

// static variable -------------------------------------
static u16 agbLcd = 0;

// const data  -----------------------------------------
static const u8 *const str_lcd[] ATTRIBUTE_ALIGN(2) = {
	(const u8 *)"TOP    LCD",
	(const u8 *)"BOTTOM LCD",
};

static const MenuComponent agbLcdSel = {
	2,
	10,
	8,
	0,
	4,
	8,
	WHITE,
	HIGHLIGHT_Y,
	(const u8 **)&str_lcd,
};


//======================================================
// function's description
//======================================================

// AGBモード設定の初期化
void SEQ_AgbLcdSelect_init(void)
{
	GXS_SetVisiblePlane(GX_PLANEMASK_NONE);
	
	MI_CpuClearFast(bgBakS,sizeof(bgBakS));
	
	ClearAllStringSJIS();
	
	(void)DrawStringSJIS( 1, 0, YELLOW, (const u8 *)"AGB-MODE LCD SELECT");
	(void)DrawStringSJIS( CANCEL_BUTTON_LT_X, CANCEL_BUTTON_LT_Y,HIGHLIGHT_C, (const u8 *)" CANCEL ");
	
	if(GetSYSMWork()->ncd_invalid) {
		agbLcd = 0;
	}else {
		agbLcd = GetNCDWork()->option.agbLcd;
		if(agbLcd > 1) agbLcd = 1;
	}
	
	DrawMenu((u16)agbLcd, &agbLcdSel);
	SVC_CpuClear(0x0000, &tpd, sizeof(TpWork), 16);
	
	GXS_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1);
}


// AGBモード設定
int SEQ_AgbLcdSelect(void)
{
	BOOL tp_select;
	BOOL tp_cancel = FALSE;
	
	ReadTP();													// タッチパネル入力の取得
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if(pad.trg & (PAD_KEY_DOWN | PAD_KEY_UP)){						// カーソルの移動
		agbLcd ^= 0x01;
	}
	tp_select = SelectMenuByTp((u16 *)&agbLcd, &agbLcdSel);
	DrawMenu((u16)agbLcd, &agbLcdSel);
	
	// [CANCEL]ボタン押下チェック
	if(tpd.disp.touch) {
		tp_cancel = InRangeTp(CANCEL_BUTTON_LT_X*8, CANCEL_BUTTON_LT_Y*8,
							  CANCEL_BUTTON_RB_X*8, CANCEL_BUTTON_RB_Y*8, &tpd.disp);
	}
	
	if((pad.trg & PAD_BUTTON_A) || (tp_select)) {					// メニュー項目への分岐
		GetSYSMWork()->ncd_invalid  = 0;
		GetNCDWork()->option.agbLcd = agbLcd;
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAMへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData(GetNCDWork());
		SEQ_MainMenu_init();
		return 0;
	}else if((pad.trg & PAD_BUTTON_B) || (tp_cancel)) {
		SEQ_MainMenu_init();
		return 0;
	}
	
	ReadTP();
	
	return 0;
}


