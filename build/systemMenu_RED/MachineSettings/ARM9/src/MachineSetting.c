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
#include "spi.h"

// define data------------------------------------------
	// キャンセルボタンLCD領域
#define CANCEL_BUTTON_LT_X					( 12 * 8 )
#define CANCEL_BUTTON_LT_Y					( 21 * 8 )
#define CANCEL_BUTTON_RB_X					( (CANCEL_BUTTON_LT_X + 8) * 8 )
#define CANCEL_BUTTON_RB_Y					( (CANCEL_BUTTON_LT_Y + 2) * 8 )
	// OKボタンLCD領域
#define OK_BUTTON_LT_X						( 22 * 8 )
#define OK_BUTTON_LT_Y						( 21 * 8 )
#define OK_BUTTON_RB_X						( (OK_BUTTON_LT_X + 8) * 8 )
#define OK_BUTTON_RB_Y						( (OK_BUTTON_LT_Y + 2) * 8 )


#define SETTING_MENU_ELEMENT_NUM			4						// メインメニューの項目数

// extern data------------------------------------------

// function's prototype declaration---------------------
static BOOL InitialSetting( void );
static void InitialSettingFinalizeInit( void );
static int  InitialSettingFinalizeMain( void );

// global variable -------------------------------------
BOOL g_initialSet = FALSE;

// static variable -------------------------------------
static u16 s_csr = 0;
static const u16 *s_pStrSetting[ SETTING_MENU_ELEMENT_NUM ];			// メインメニュー用文字テーブルへのポインタリスト

// const data  -----------------------------------------


//===============================================
// mainMenu.c
//===============================================
static const u16 *const s_pStrSettingElemTbl[ SETTING_MENU_ELEMENT_NUM ][ LANG_CODE_MAX ] = {
	{
		(const u16 *)L"言語",
		(const u16 *)L"LANGUAGE",
		(const u16 *)L"LANGUAGE(F)",
		(const u16 *)L"LANGUAGE(G)",
		(const u16 *)L"LANGUAGE(I)",
		(const u16 *)L"LANGUAGE(S)",
	},
	{
		(const u16 *)L"日付 & 時刻",
		(const u16 *)L"DATE & TIME",
		(const u16 *)L"DATE & TIME(F)",
		(const u16 *)L"DATE & TIME(G)",
		(const u16 *)L"DATE & TIME(I)",
		(const u16 *)L"DATE & TIME(S)",
	},
	{
		(const u16 *)L"ユーザー情報",
		(const u16 *)L"USER INFORMATION",
		(const u16 *)L"USER INFORMATION(F)",
		(const u16 *)L"USER INFORMATION(G)",
		(const u16 *)L"USER INFORMATION(I)",
		(const u16 *)L"USER INFORMATION(S)",
	},
	{
		(const u16 *)L"タッチパネル補正",
		(const u16 *)L"TOUCH PANEL",
		(const u16 *)L"TOUCH PANEL(F)",
		(const u16 *)L"TOUCH PANEL(G)",
		(const u16 *)L"TOUCH PANEL(I)",
		(const u16 *)L"TOUCH PANEL(S)",
	},
};

static MenuPos s_settingPos[] = {
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ FALSE, 4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
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
	
	// 初回起動シーケンス
	if( InitialSetting() ) {
		return;
	}
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"MACHINE SETTINGS" );
#ifdef __NCD_CLEAR_ENABLE
	(void)PutStringUTF16( 18 * 8, 21 * 8, TXT_COLOR_BLACK, (const u16 *)L"[START]:NCD clear.");
#endif	/* __NCD_CLEAR_ENABLE */
	
	// NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
	for( i = 0; i < SETTING_MENU_ELEMENT_NUM; i++ ) {
		s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ GetNCDWork()->option.language ];
	}
	
	DrawMenu( s_csr, &s_settingParam );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 );
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
	
	g_pNowProcess = MachineSettingMain;
}


// メインメニュー
int MachineSettingMain( void )
{
	BOOL tp_select;
	
	GetAndDrawRtcData();
	ReadTpData();
	
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
//					SetOwnerInfoInit();
//					g_pNowProcess = SetOwnerInfoMain;
					break;
				case 3:
					TP_CalibrationInit();
					g_pNowProcess = TP_CalibrationMain;
					break;
			}
		}
	}
	
#ifdef __NCD_CLEAR_ENABLE
	if( pad.trg & PAD_BUTTON_START ) {
		SVC_CpuClearFast( 0x0000, GetNCDWork(), sizeof(NitroConfigData) );
		(void)SPI_NvramWriteEnable();
		SVC_WaitVBlankIntr();
		(void)SPI_NvramPageErase( 0x3fe00 );
		SVC_WaitVBlankIntr();
		(void)SPI_NvramWriteEnable();
		SVC_WaitVBlankIntr();
		(void)SPI_NvramPageErase( 0x3ff00 );
		SVC_WaitVBlankIntr();
		(void)SPI_NvramWriteDisable();
		OS_TPrintf("NitroConfigData zero clear!!\n");
	}
#endif	/* __NCD_CLEAR_ENABLE */
	
	return 0;
}


// OK / CANCELボタンの描画
void DrawOKCancelButton(void)
{
	(void)PutStringUTF16( CANCEL_BUTTON_LT_X, CANCEL_BUTTON_LT_Y, TXT_COLOR_CYAN, (const u16 *)L" CANCEL ");
	(void)PutStringUTF16( OK_BUTTON_LT_X,     OK_BUTTON_LT_Y,     TXT_COLOR_CYAN, (const u16 *)L"   OK   ");
}


// OK or CANCELボタン押下チェック
void CheckOKCancelButton(BOOL *tp_ok, BOOL *tp_cancel)
{
	*tp_cancel = InRangeTp( CANCEL_BUTTON_LT_X, CANCEL_BUTTON_LT_Y - 4,
						    CANCEL_BUTTON_RB_X, CANCEL_BUTTON_RB_Y - 4, &tpd.disp );
	*tp_ok     = InRangeTp( OK_BUTTON_LT_X,     OK_BUTTON_LT_Y - 4,
						    OK_BUTTON_RB_X,     OK_BUTTON_RB_Y - 4, &tpd.disp );
}


//---------------------------------------------------------
//
// 設定終了
//
//---------------------------------------------------------

// 初回起動シーケンス
static BOOL InitialSetting( void )
{
#if 0
	if(GetNCDWork()->option.input_language == 0) {			// 言語設定がまだ。
		g_initialSet = TRUE;
		s_csr = 2;
		SelectLangageInit();
		g_pNowProcess	= SelectLanguageMain;
		return TRUE;
	}else if(GetNCDWork()->option.input_tp == 0) {			// TPキャリブレーションがまだ。
		g_initialSet = TRUE;
		s_csr = 3;
		TP_CalibrationInit();
		g_pNowProcess	= TP_CalibrationMain;
		return TRUE;
	}else if(GetNCDWork()->option.input_rtc == 0) {			// RTC設定がまだ。
		ClearRTC();
		g_initialSet = TRUE;
		s_csr = 1;
		SetRTCInit();
		g_pNowProcess	= SetRTCMain;
		return TRUE;
	}else if( (GetNCDWork()->option.input_nickname == 0)	// ニックネームまたは好きな色入力がまだ。
		  ||  (GetNCDWork()->option.input_favoriteColor == 0) ) {
/*		g_initialSet = TRUE;
		s_csr = 0;
		SetOwnerInfoInit();
		g_pNowProcess	= SetOwnerInfoMain;
		return TRUE;
*/	}
	
	if( g_initialSet ) {
		InitialSettingFinalizeInit();
		g_pNowProcess = InitialSettingFinalizeMain();
		return TRUE;
	}
#endif
	return FALSE;
}


static void InitialSettingFinalizeInit( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_WHITE );
	(void)PutStringUTF16( 6 * 8, 10 * 8, TXT_COLOR_BLACK, (const u16 *)L" Initial setting completed.");
	(void)PutStringUTF16( 6 * 8, 12 * 8, TXT_COLOR_BLACK, (const u16 *)L"      Please reboot.");
}


static int InitialSettingFinalizeMain( void )
{
	return 0;
}

