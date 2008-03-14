/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SelectLanguage.c

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
#include "misc.h"
#include "MachineSetting.h"

// define data------------------------------------------
#define CANCEL_BUTTON_TOP_X					( 2 * 8 )
#define CANCEL_BUTTON_TOP_Y					( 21 * 8 )
#define CANCEL_BUTTON_BOTTOM_X				( (CANCEL_BUTTON_TOP_X + 8 ) * 8 )
#define CANCEL_BUTTON_BOTTOM_Y				( (CANCEL_BUTTON_TOP_Y + 2 ) * 8 )

// extern data------------------------------------------

// function's prototype declaration---------------------

// global variable -------------------------------------

// static variable -------------------------------------
static int s_lang;											// 言語選択肢の何番目を選択しているか
static LCFGTWLRegion s_regionCode;											// リージョンコード

static const u16* s_pStrLanguage[LCFG_TWL_LANG_CODE_MAX];
static LCFGTWLLangCode s_langCodeList[LCFG_TWL_LANG_CODE_MAX];

// const data  -----------------------------------------
static const u16 region_lang_Mapping[LCFG_TWL_REGION_MAX] =
{
	LCFG_TWL_LANG_BITMAP_JAPAN,
	LCFG_TWL_LANG_BITMAP_AMERICA,
	LCFG_TWL_LANG_BITMAP_EUROPE,
	LCFG_TWL_LANG_BITMAP_AUSTRALIA,
	LCFG_TWL_LANG_BITMAP_CHINA,
	LCFG_TWL_LANG_BITMAP_KOREA
};

static const u16 *const s_pStrLanguageData[LCFG_TWL_LANG_CODE_MAX] = {
	(const u16 *)L"日本語",
	(const u16 *)L"English ",
	(const u16 *)L"Francais",
	(const u16 *)L"Deutsch ",
	(const u16 *)L"Italiano",
	(const u16 *)L"Espanol ",
	(const u16 *)L"中国語（仮）",
	(const u16 *)L"韓国語（仮）"
};

static MenuPos s_languagePos[LCFG_TWL_LANG_CODE_MAX] = {
	{ TRUE,  4 * 8,   6 * 8 },
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
	{ TRUE,  4 * 8,  18 * 8 },
	{ TRUE,  4 * 8,  20 * 8 }
};

static MenuParam langSel = {
	LCFG_TWL_LANG_CODE_MAX,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_languagePos[ 0 ],
	(const u16 **)&s_pStrLanguage,
};


//======================================================
// function's description
//======================================================

// 言語設定の初期化
void SelectLanguageInit( void )
{
    int l;
    u16 temp_langCode = 0;
    BOOL in_list_flag = FALSE;
	int lang_count = 0;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );

	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"LANGUAGE SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// あらかじめTWL設定データファイルから読み込み済みの設定を取得
	// :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
	// リージョンの取得
	s_regionCode = (LCFGTWLRegion)LCFG_THW_GetRegion();
	
	// 言語の取得
	if( !g_isValidTSD ||
		( LCFG_TSD_GetLanguage() >= LCFG_TWL_LANG_CODE_MAX ) ) {
		temp_langCode = LCFG_TWL_LANG_ENGLISH;
	}else {
		temp_langCode = LCFG_TSD_GetLanguage();
	}
	
	// リージョン-言語マッピング情報から、現在のリージョンで選択できる言語をリストアップ
	s_lang = 0;
	for(l=0; l<LCFG_TWL_LANG_CODE_MAX; l++)
	{
		if( ( 0x0001 << l ) & region_lang_Mapping[s_regionCode] )
		{
			s_pStrLanguage[lang_count] = s_pStrLanguageData[l];
			s_langCodeList[lang_count] = (LCFGTWLLangCode)l;
			if(temp_langCode == l)
			{
				s_lang = lang_count;
			}
			lang_count++;
		}
	}
	
	langSel.num = lang_count;
	
	DrawMenu( (u16)s_lang, &langSel );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// 言語選択
int SelectLanguageMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	
	ReadTP();												// TP入力の取得
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ) {								// カーソルの移動
		if( ++s_lang == langSel.num ) {
			s_lang = (LCFGTWLLangCode)0;
		}
	}
	if( pad.trg & PAD_KEY_UP ) {
		if( --s_lang < 0 ) {
			s_lang = (LCFGTWLLangCode)( langSel.num - 1 );
		}
	}
	tp_select = SelectMenuByTP( (u16 *)&s_lang, &langSel );
	DrawMenu( (u16)s_lang, &langSel );
	
	// [CANCEL]ボタン押下チェック
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || tp_select ) {				// メニュー項目への分岐
		LCFG_TSD_SetLanguage( s_langCodeList[s_lang] );
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// TWL設定データファイルへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		if( !MY_WriteTWLSettings() ) {
			OS_TPrintf( "TWL settings write failed.\n" );
		}
		
		MachineSettingInit();
		return 0;
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		MachineSettingInit();
		return 0;
	}
	
	return 0;
}


