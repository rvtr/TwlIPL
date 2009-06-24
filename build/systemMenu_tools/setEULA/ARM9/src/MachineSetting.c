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

// メインメニューの項目数（※ピクトチャット起動テストは除いておく）
#ifdef OUTSIDE_UI
#define SETTING_MENU_ELEMENT_NUM			5						// 社外用
#else  // !OUTSIDE_UI
#define SETTING_MENU_ELEMENT_NUM			10						// 社内用
#endif // OUTSIDE_UI

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
		(const u16 *)L"EULA",
		(const u16 *)L"EULA",
		(const u16 *)L"EULA(F)",
		(const u16 *)L"EULA(G)",
		(const u16 *)L"EULA(I)",
		(const u16 *)L"EULA(S)",
		(const u16 *)L"EULA(C)",
		(const u16 *)L"EULA(K)",
	},
};

static MenuPos s_settingPos[] = {
	{ TRUE,  4 * 8,   2 * 8 },
	{ TRUE,  4 * 8,   4 * 8 },
	{ TRUE,  4 * 8,   6 * 8 },
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE, 4 * 8,   10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
	{ TRUE,  4 * 8,  18 * 8 },
	{ TRUE,  4 * 8,  20 * 8 },
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
	
	PrintfSJIS( 0, 0, TXT_COLOR_BLUE, "Set EULA", g_strIPLSvnRevision, g_strSDKSvnRevision );
	
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

	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// メニュー項目への分岐
		if( s_settingPos[ s_csr ].enable ) {
/*
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
                    SetWirelessInit();
                    g_pNowProcess = SetWirelessMain;
                    break;
                case 5:
                    CleanupMachineInit();
                    g_pNowProcess = CleanupMachineMain;
                    break;
				case 6:
					SelectCountryInit();
					g_pNowProcess = SelectCountryMain;
					break;
                case 7:
                    SetParentalControlInit();
                    g_pNowProcess = SetParentalControlMain;
                    break;
                case 8:
                    SetEULAInit();
                    g_pNowProcess = SetEULAMain;
                    break;

                case 9:
                    SetFreeSoftBoxInit();
                    g_pNowProcess = SetFreeSoftBoxMain;
                    break;
			}
*/
		}
	}
	
	// とりあえずバックライト輝度変更をここで確認。
	if( pad.trg & PAD_BUTTON_R) {
		u8 brightness;
		(void)UTL_GetBacklightBrightness( &brightness );
		if( ++brightness > BACKLIGHT_BRIGHTNESS_MAX ) {
			brightness = BACKLIGHT_BRIGHTNESS_MAX;
		}
		(void)UTL_SetBacklightBrightness( brightness );
	}
	if( pad.trg & PAD_BUTTON_L ) {
		u8 brightness;
		(void)UTL_GetBacklightBrightness( &brightness );
		if( --brightness < 0 ) {
			brightness = 0;
		}
		(void)UTL_SetBacklightBrightness( brightness );
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
	u8 *pBuffer = Alloc( LCFG_WRITE_TEMP );
	if( pBuffer != NULL ) {
		// ***********************************************************
		// RED本体設定では、初回起動シーケンスもフラッシュ壊れシーケンスもないので、とりあえず何でも設定したらシーケンスを終了するようにする。
		LCFG_TSD_SetFlagFinishedInitialSetting( TRUE );
		LCFG_TSD_SetFlagFinishedInitialSetting_Launcher( TRUE );
		LCFG_TSD_SetFlagFinishedBrokenTWLSettings( TRUE );
		// ***********************************************************
		
		retval = LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
		Free( pBuffer );
	}
	return retval;
}
