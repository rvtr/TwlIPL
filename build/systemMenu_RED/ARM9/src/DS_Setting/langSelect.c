/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     langSelect.c

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
void SEQ_LangSelect_init(void);
int  SEQ_LangSelect(void);

// global variable -------------------------------------

// static variable -------------------------------------
static NvLangCode langCode;											// 言語コード

// const data  -----------------------------------------
static const u8 *const str_language[] ATTRIBUTE_ALIGN(2) = {
	(const u8 *)"にほんご     ",
	(const u8 *)"English ",
	(const u8 *)"Francais",
	(const u8 *)"Deutsch ",
	(const u8 *)"Italiano",
	(const u8 *)"Espanol ",
};

static const MenuComponent langSel = {
	(u16)LANG_CODE_MAX,
	10,
	5,
	0,
	2,
	8,
	WHITE,
	HIGHLIGHT_Y,
	(const u8 **)&str_language,
};


//======================================================
// function's description
//======================================================

// 言語設定の初期化
void SEQ_LangSelect_init(void)
{
	GXS_SetVisiblePlane(GX_PLANEMASK_NONE);
	
	MI_CpuClearFast(bgBakS, sizeof(bgBakS));
	SVC_CpuClearFast(0xc0,  oamBakS, sizeof(oamBakS));
	
	ClearAllStringSJIS();
	
	(void)DrawStringSJIS( 1, 0, YELLOW, (const u8 *)"LANGUAGE SELECT");
	(void)DrawStringSJIS( CANCEL_BUTTON_LT_X, CANCEL_BUTTON_LT_Y,HIGHLIGHT_C, (const u8 *)" CANCEL ");
	if( initialSet ) {
		(void)DrawStringSJIS( 8, 18, RED, (const u8 *)"Select language.");
	}
	
	if((GetSYSMWork()->ncd_invalid) || (GetNCDWork()->option.language >= LANG_CODE_MAX)) {
		langCode = LANG_ENGLISH;
	}else {
		langCode = (NvLangCode)GetNCDWork()->option.language;
	}
	
	DrawMenu((u16)langCode, &langSel);
	SVC_CpuClear(0x0000,&tpd, sizeof(TpWork),16);
	
	GXS_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1);
}


// 言語選択
int SEQ_LangSelect(void)
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTpData();													// タッチパネル入力の取得
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if(pad.trg & PAD_KEY_DOWN){										// カーソルの移動
		if(++langCode == LANG_CODE_MAX)	langCode = (NvLangCode)0;
	}
	if(pad.trg & PAD_KEY_UP){
		if(--langCode < 0)				langCode = (NvLangCode)(LANG_CODE_MAX-1);
	}
	tp_select = SelectMenuByTp((u16 *)&langCode, &langSel);
	DrawMenu((u16)langCode, &langSel);
	
	// [CANCEL]ボタン押下チェック
	if(tpd.disp.touch) {
		tp_cancel = InRangeTp(CANCEL_BUTTON_LT_X*8, CANCEL_BUTTON_LT_Y*8-4,
							CANCEL_BUTTON_RB_X*8, CANCEL_BUTTON_RB_Y*8-4, &tpd.disp);
	}
	
	if((pad.trg & PAD_BUTTON_A) || (tp_select)) {					// メニュー項目への分岐
		GetSYSMWork()->ncd_invalid   		= 0;
		GetNCDWork()->option.input_language = 1;				// 言語入力フラグを立てる
		GetNCDWork()->option.language       = langCode;
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// NVRAMへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)NVRAMm_WriteNitroConfigData (GetNCDWork());
		SEQ_MainMenu_init();
		return 0;
	}else if((pad.trg & PAD_BUTTON_B) || (tp_cancel)) {
		SEQ_MainMenu_init();
		return 0;
	}
	
	ReadTpData();
	
	return 0;
}


