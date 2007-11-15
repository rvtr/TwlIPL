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
static TWLLangCode s_langCode;											// 言語コード

// const data  -----------------------------------------
static const u16 *const s_pStrLanguage[] = {
	(const u16 *)L"日本語",
	(const u16 *)L"English ",
	(const u16 *)L"Francais",
	(const u16 *)L"Deutsch ",
	(const u16 *)L"Italiano",
	(const u16 *)L"Espanol ",
};

static MenuPos s_languagePos[] = {
	{ TRUE,  4 * 8,   6 * 8 },
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
};

static const MenuParam langSel = {
	6,
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
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );

	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"LANGUAGE SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	if( g_initialSet ) {
		PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"Select language." );
	}
	
	if( ( SYSMi_GetWork()->isValidTSD ) ||
		( TSD_GetLanguage() >= TWL_LANG_CODE_MAX ) ) {
		s_langCode = TWL_LANG_ENGLISH;
	}else {
		s_langCode = TSD_GetLanguage();
	}
	
	DrawMenu( (u16)s_langCode, &langSel );
	
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
		if( ++s_langCode == TWL_LANG_CODE_MAX ) {
			s_langCode = (TWLLangCode)0;
		}
	}
	if( pad.trg & PAD_KEY_UP ) {
		if( --s_langCode < 0 ) {
			s_langCode = (TWLLangCode)( TWL_LANG_CODE_MAX - 1 );
		}
	}
	tp_select = SelectMenuByTP( (u16 *)&s_langCode, &langSel );
	DrawMenu( (u16)s_langCode, &langSel );
	
	// [CANCEL]ボタン押下チェック
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || tp_select ) {				// メニュー項目への分岐
		SYSMi_GetWork()->isValidTSD   		= 0;
		
		TSD_SetLanguage( s_langCode );
		TSD_SetFlagLanguage( TRUE );							// 言語入力フラグを立てる
		// ::::::::::::::::::::::::::::::::::::::::::::::
		// TWL設定データファイルへの書き込み
		// ::::::::::::::::::::::::::::::::::::::::::::::
		(void)SYSM_WriteTWLSettingsFile();
		
		MachineSettingInit();
		return 0;
	}else if( ( pad.trg & PAD_BUTTON_B ) || tp_cancel ) {
		MachineSettingInit();
		return 0;
	}
	
	return 0;
}


