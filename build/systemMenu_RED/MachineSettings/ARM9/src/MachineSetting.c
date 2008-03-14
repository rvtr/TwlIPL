/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mainMenu.c

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
	// キャンセルボタンLCD領域
#define CANCEL_BUTTON_TOP_X					( 12 * 8 )
#define CANCEL_BUTTON_TOP_Y					( 21 * 8 )
#define CANCEL_BUTTON_BOTTOM_X				( CANCEL_BUTTON_TOP_X + 5 * 8 )
#define CANCEL_BUTTON_BOTTOM_Y				( CANCEL_BUTTON_TOP_Y + 2 * 8 )
	// OKボタンLCD領域
#define OK_BUTTON_TOP_X						( 22 * 8 )
#define OK_BUTTON_TOP_Y						( 21 * 8 )
#define OK_BUTTON_BOTTOM_X					( OK_BUTTON_TOP_X + 2 * 8 )
#define OK_BUTTON_BOTTOM_Y					( OK_BUTTON_TOP_Y + 2 * 8 )


#define SETTING_MENU_ELEMENT_NUM			6						// メインメニューの項目数（※ピクトチャット起動テストは除いておく）

// extern data------------------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];

// function's prototype declaration---------------------

// global variable -------------------------------------

// static variable -------------------------------------
static u16 s_csr = 0;
static const u16 *s_pStrSetting[ SETTING_MENU_ELEMENT_NUM ];			// メインメニュー用文字テーブルへのポインタリスト

// const data  -----------------------------------------


//===============================================
// mainMenu.c
//===============================================
static const u16 *const s_pStrSettingElemTbl[ SETTING_MENU_ELEMENT_NUM ][ LCFG_TWL_LANG_CODE_MAX ] = {
	{
		(const u16 *)L"言語",
		(const u16 *)L"LANGUAGE",
		(const u16 *)L"LANGUAGE(F)",
		(const u16 *)L"LANGUAGE(G)",
		(const u16 *)L"LANGUAGE(I)",
		(const u16 *)L"LANGUAGE(S)",
		(const u16 *)L"LANGUAGE(C)",
		(const u16 *)L"LANGUAGE(K)",
	},
	{
		(const u16 *)L"日付 & 時刻",
		(const u16 *)L"DATE & TIME",
		(const u16 *)L"DATE & TIME(F)",
		(const u16 *)L"DATE & TIME(G)",
		(const u16 *)L"DATE & TIME(I)",
		(const u16 *)L"DATE & TIME(S)",
		(const u16 *)L"DATE & TIME(C)",
		(const u16 *)L"DATE & TIME(K)",
	},
	{
		(const u16 *)L"ユーザー情報",
		(const u16 *)L"USER INFORMATION",
		(const u16 *)L"USER INFORMATION(F)",
		(const u16 *)L"USER INFORMATION(G)",
		(const u16 *)L"USER INFORMATION(I)",
		(const u16 *)L"USER INFORMATION(S)",
		(const u16 *)L"USER INFORMATION(C)",
		(const u16 *)L"USER INFORMATION(K)",
	},
	{
		(const u16 *)L"タッチパネル補正",
		(const u16 *)L"TOUCH PANEL",
		(const u16 *)L"TOUCH PANEL(F)",
		(const u16 *)L"TOUCH PANEL(G)",
		(const u16 *)L"TOUCH PANEL(I)",
		(const u16 *)L"TOUCH PANEL(S)",
		(const u16 *)L"TOUCH PANEL(C)",
		(const u16 *)L"TOUCH PANEL(K)",
	},
	{
		(const u16 *)L"国設定",
		(const u16 *)L"COUNTRY",
		(const u16 *)L"COUNTRY(F)",
		(const u16 *)L"COUNTRY(G)",
		(const u16 *)L"COUNTRY(I)",
		(const u16 *)L"COUNTRY(S)",
		(const u16 *)L"COUNTRY(C)",
		(const u16 *)L"COUNTRY(K)",
	},
	{
		(const u16 *)L"ペアレンタルコントロール",
		(const u16 *)L"PARENTAL CONTROL",
		(const u16 *)L"PARENTAL CONTROL(F)",
		(const u16 *)L"PARENTAL CONTROL(G)",
		(const u16 *)L"PARENTAL CONTROL(I)",
		(const u16 *)L"PARENTAL CONTROL(S)",
		(const u16 *)L"PARENTAL CONTROL(C)",
		(const u16 *)L"PARENTAL CONTROL(K)",
	},
#if 0
	{
		(const u16 *)L"ピクトチャット起動テスト",
		(const u16 *)L"PICTOCHAT",
		(const u16 *)L"PICTOCHAT(F)",
		(const u16 *)L"PICTOCHAT(G)",
		(const u16 *)L"PICTOCHAT(I)",
		(const u16 *)L"PICTOCHAT(S)",
		(const u16 *)L"PICTOCHAT(C)",
		(const u16 *)L"PICTOCHAT(K)",
	},
#endif
};

static MenuPos s_settingPos[] = {
	{ TRUE,  4 * 8,   6 * 8 },
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE, 4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
	{ TRUE,  4 * 8,  18 * 8 },
};


static const MenuParam s_settingParam = {
	SETTING_MENU_ELEMENT_NUM,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_settingPos[ 0 ],
	(const u16 **)&s_pStrSetting,
};

//======================================================
// メインメニュー
//======================================================

// メインメニューの初期化
void MachineSettingInit( void )
{
	int i;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    
    // BGデータのロード処理
	GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
	GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));
	
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "MACHINE SETTINGS  IPL:%s SDK:%s", g_strIPLSvnRevision, g_strSDKSvnRevision );
	
	// NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
	for( i = 0; i < SETTING_MENU_ELEMENT_NUM; i++ ) {
		s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ LCFG_TSD_GetLanguage() ];
	}
	
	ChangeUserColor( LCFG_TSD_GetUserColor() );
	DrawMenu( s_csr, &s_settingParam );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
	
	g_pNowProcess = MachineSettingMain;
}

// メインメニュー
int MachineSettingMain( void )
{
	BOOL tp_select;
	ReadTP();
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.trg & PAD_KEY_DOWN ){									// カーソルの移動
		if( ++s_csr == SETTING_MENU_ELEMENT_NUM ) {
			s_csr=0;
		}
	}
	if( pad.trg & PAD_KEY_UP ){
		if( --s_csr & 0x80 ) {
			s_csr=SETTING_MENU_ELEMENT_NUM - 1;
		}
	}
	tp_select = SelectMenuByTP( &s_csr, &s_settingParam );
	DrawMenu( s_csr, &s_settingParam );

	if( pad.trg & PAD_BUTTON_START ) {
		OS_DoApplicationJump( NULL, OS_APP_JUMP_NORMAL );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
		if( s_settingPos[ s_csr ].enable ) {
			switch( s_csr ) {
				case 0:
					SelectLanguageInit();
					g_pNowProcess = SelectLanguageMain;
					break;
				case 1:
					SetRTCInit();
					g_pNowProcess = SetRTCMain;
					break;
				case 2:
					SetOwnerInfoInit();
					g_pNowProcess = SetOwnerInfoMain;
					break;
				case 3:
					TP_CalibrationInit();
					g_pNowProcess = TP_CalibrationMain;
					break;
				case 4:
					SelectCountryInit();
					g_pNowProcess = SelectCountryMain;
					break;
                case 5:
                    SetParentalControlInit();
                    g_pNowProcess = SetParentalControlMain;
                    break;
			}
		}
	}
	
	return 0;
}


// OK / CANCELボタンの描画
void DrawOKCancelButton(void)
{
	(void)PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL");
	(void)PutStringUTF16( OK_BUTTON_TOP_X,     OK_BUTTON_TOP_Y,     TXT_COLOR_CYAN, (const u16 *)L"OK");
}


// OK or CANCELボタン押下チェック
void CheckOKCancelButton(BOOL *tp_ok, BOOL *tp_cancel)
{
	*tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
								CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	*tp_ok     = WithinRangeTP( OK_BUTTON_TOP_X,     OK_BUTTON_TOP_Y,
								OK_BUTTON_BOTTOM_X,     OK_BUTTON_BOTTOM_Y, &tpd.disp );
}


//---------------------------------------------------------
//
// 設定終了
//
//---------------------------------------------------------


// 本体設定データのライト
BOOL MY_WriteTWLSettings( void )
{
	BOOL retval = FALSE;
	u8 *pBuffer = SYSM_Alloc( LCFG_WRITE_TEMP );
	if( pBuffer != NULL ) {
		retval = LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
		SYSM_Free( pBuffer );
	}
	return retval;
}
