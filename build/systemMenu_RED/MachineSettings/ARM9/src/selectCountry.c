/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SelectCountry.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-19#$
  $Rev: 215 $
  $Author: yosiokat $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "misc.h"
#include "MachineSetting.h"

// define data------------------------------------------
#define CANCEL_BUTTON_TOP_X					( 2 * 8 )
#define CANCEL_BUTTON_TOP_Y					( 21 * 8 )
#define CANCEL_BUTTON_BOTTOM_X				( (CANCEL_BUTTON_TOP_X + 8 ) * 8 )
#define CANCEL_BUTTON_BOTTOM_Y				( (CANCEL_BUTTON_TOP_Y + 2 ) * 8 )

#define MENU_DISPLAY_SIZE 7

// extern data------------------------------------------

// function's prototype declaration---------------------

// global variable -------------------------------------

// static variable -------------------------------------
static TWLCountryCode s_countryCode;										// 国コード
static TWLRegion s_regionCode;											// リージョン

static u16 list_start, list_end;
static u16 s_menu_display_start;

static const u16 *s_pStrCountry[MENU_DISPLAY_SIZE];

// const data  -----------------------------------------
static const u16 *const s_pStrCountryName[] = {
	(const u16 *)L"UNDEFINED",
	(const u16 *)L"JAPAN",					// 日本
	
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	
	// USAリージョン
	(const u16 *)L"Anguilla",        // アンギラ
	(const u16 *)L"ANTIGUA_AND_BARBUDA",    // アンティグア・バーブーダ
	(const u16 *)L"ARGENTINA",		// アルゼンチン
	(const u16 *)L"ARUBA",                  // アルバ
	(const u16 *)L"BAHAMAS",                // バハマ
	(const u16 *)L"BARBADOS",               // バルバドス
	(const u16 *)L"BELIZE",                 // ベリーズ
	(const u16 *)L"BOLIVIA",                // ボリビア
	(const u16 *)L"BRAZIL",                 // ブラジル
	(const u16 *)L"BRITISH_VIRGIN_ISLANDS", // 英領ヴァージン諸島
	(const u16 *)L"CANADA",                 // カナダ
	(const u16 *)L"CAYMAN_ISLANDS",         // ケイマン諸島
	(const u16 *)L"CHILE",       // チリ
	(const u16 *)L"COLOMBIA",               // コロンビア
	(const u16 *)L"COSTA_RICA",             // コスタリカ
	(const u16 *)L"DOMINICA",               // ドミニカ国
	(const u16 *)L"DOMINICAN_REPUBLIC",     // ドミニカ共和国
	(const u16 *)L"ECUADOR",                // エクアドル
	(const u16 *)L"EL_SALVADOR",            // エルサルバドル
	(const u16 *)L"FRENCH_GUIANA",          // フランス領ギアナ
	(const u16 *)L"GRENADA",                // グレナダ
	(const u16 *)L"GUADELOUPE",             // グアドループ
	(const u16 *)L"GUATEMALA",       // グアテマラ
	(const u16 *)L"GUYANA",                 // ガイアナ
	(const u16 *)L"HAITI",                  // ハイチ
	(const u16 *)L"HONDURAS",               // ホンジュラス
	(const u16 *)L"JAMAICA",                // ジャマイカ
	(const u16 *)L"MARTINIQUE",             // マルティニーク
	(const u16 *)L"MEXICO",                 // メキシコ
	(const u16 *)L"MONTSERRAT",             // モントセラト
	(const u16 *)L"NETHERLANDS_ANTILLES",   // オランダ領アンティル
	(const u16 *)L"NICARAGUA",              // ニカラグア
	(const u16 *)L"PANAMA",       // パナマ
	(const u16 *)L"PARAGUAY",               // パラグアイ
	(const u16 *)L"PERU",                   // ペルー
	(const u16 *)L"ST_KITTS_AND_NEVIS",     // セントキッツ・ネイビス
	(const u16 *)L"ST_LUCIA",               // セントルシア
	(const u16 *)L"ST_VINCENT_AND_THE_GRENADINES",  // セントビンセント・グレナディーン
	(const u16 *)L"SURINAME",               // スリナム
	(const u16 *)L"TRINIDAD_AND_TOBAGO",    // トリニダード・トバゴ
	(const u16 *)L"TURKS_AND_CAICOS_ISLANDS",   // タークス・カイコス諸島
	(const u16 *)L"UNITED_STATES",          // アメリカ
	(const u16 *)L"URUGUAY",       // ウルグアイ
	(const u16 *)L"US_VIRGIN_ISLANDS",      // 米領バージン諸島
	(const u16 *)L"VENEZUELA",              // ベネズエラ
	
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",

    // EUR", NAL リージョン
	(const u16 *)L"ALBANIA",       // アルバニア
	(const u16 *)L"AUSTRALIA",              // オーストラリア
	(const u16 *)L"AUSTRIA",                // オーストリア
	(const u16 *)L"BELGIUM",                // ベルギー
	(const u16 *)L"BOSNIA_AND_HERZEGOVINA", // ボスニア・ヘルツェゴビナ
	(const u16 *)L"BOTSWANA",               // ボツワナ
	(const u16 *)L"BULGARIA",       // ブルガリア
	(const u16 *)L"CROATIA",                // クロアチア
	(const u16 *)L"CYPRUS",                 // キプロス
	(const u16 *)L"CZECH_REPUBLIC",         // チェコ
	(const u16 *)L"DENMARK",                // デンマーク
	(const u16 *)L"ESTONIA",                // エストニア
	(const u16 *)L"FINLAND",                // フィンランド
	(const u16 *)L"FRANCE",                 // フランス
	(const u16 *)L"GERMANY",                // ドイツ
	(const u16 *)L"GREECE",                 // ギリシャ
	(const u16 *)L"HUNGARY",       // ハンガリー
	(const u16 *)L"ICELAND",                // アイスランド
	(const u16 *)L"IRELAND",                // アイルランド
	(const u16 *)L"ITALY",                  // イタリア
	(const u16 *)L"LATVIA",                 // ラトビア
	(const u16 *)L"LESOTHO",                // レソト
	(const u16 *)L"LIECHTENSTEIN",          // リヒテンシュタイン
	(const u16 *)L"LITHUANIA",              // リトアニア
	(const u16 *)L"LUXEMBOURG",             // ルクセンブルク
	(const u16 *)L"MACEDONIA",              // マケドニア
	(const u16 *)L"MALTA",       // マルタ
	(const u16 *)L"MONTENEGRO",             // モンテネグロ
	(const u16 *)L"MOZAMBIQUE",             // モザンビーク
	(const u16 *)L"NAMIBIA",                // ナミビア
	(const u16 *)L"NETHERLANDS",            // オランダ
	(const u16 *)L"NEW_ZEALAND",            // ニュージーランド
	(const u16 *)L"NORWAY",                 // ノルウェー
	(const u16 *)L"POLAND",                 // ポーランド
	(const u16 *)L"PORTUGAL",               // ポルトガル
	(const u16 *)L"ROMANIA",                // ルーマニア
	(const u16 *)L"RUSSIA",      // ロシア
	(const u16 *)L"SERBIA",                 // セルビア
	(const u16 *)L"SLOVAKIA",               // スロバキア
	(const u16 *)L"SLOVENIA",               // スロベニア
	(const u16 *)L"SOUTH_AFRICA",           // 南アフリカ
	(const u16 *)L"SPAIN",                  // スペイン
	(const u16 *)L"SWAZILAND",              // スワジランド
	(const u16 *)L"SWEDEN",                 // スウェーデン
	(const u16 *)L"SWITZERLAND",            // スイス
	(const u16 *)L"TURKEY",                 // トルコ
	(const u16 *)L"UNITED_KINGDOM",   // イギリス
	(const u16 *)L"ZAMBIA",                 // ザンビア
	(const u16 *)L"ZIMBABWE",               // ジンバブエ

	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",

    // TWNリージョン
    (const u16 *)L"TAIWAN",      // 台湾
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // KORリージョン
    (const u16 *)L"SOUTH_KOREA",      // 韓国
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // HKGリージョン（Wiiの国リストに存在）
    (const u16 *)L"HONG_KONG",      // ホンコン
    (const u16 *)L"MACAU",                  // マカオ
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // ASIリージョン（Wiiの国リストに存在）
    (const u16 *)L"INDONESIA",      // インドネシア
    
    // USAリージョン
    (const u16 *)L"SINGAPORE",      // シンガポール
    
    // ASIリージョン（再び）
    (const u16 *)L"THAILAND",      // タイ
    (const u16 *)L"PHILIPPINES",            // フィリピン
    (const u16 *)L"MALAYSIA",               // マレーシア

	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // 未定義リージョン（IQueリージョン？）
    (const u16 *)L"CHINA",      // 中国
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
    
    // USAリージョン
    (const u16 *)L"UAE",      // アラブ首長国連邦
    
    // 未定義リージョン
    (const u16 *)L"INDIA",      // インド
    (const u16 *)L"EGYPT",      // エジプト
    (const u16 *)L"OMAN",                   // オマーン
    (const u16 *)L"QATAR",                  // カタール
    (const u16 *)L"KUWAIT",                 // クウェート
    (const u16 *)L"SAUDI_ARABIA",           // サウジアラビア
    (const u16 *)L"SYRIA",                  // シリア
    (const u16 *)L"BAHRAIN",                // バーレーン
    (const u16 *)L"JORDAN",                 // ヨルダン
    
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//180
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//190
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//200
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//210
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//220
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//230
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//240
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",//250
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",
	(const u16 *)L"DUMMY",

    (const u16 *)L"OTHERS",
    (const u16 *)L"UNKNOWN"
};

static MenuPos s_countryPos[MENU_DISPLAY_SIZE] = {
	{ TRUE,  4 * 8,   6 * 8 },
	{ TRUE,  4 * 8,   8 * 8 },
	{ TRUE,  4 * 8,  10 * 8 },
	{ TRUE,  4 * 8,  12 * 8 },
	{ TRUE,  4 * 8,  14 * 8 },
	{ TRUE,  4 * 8,  16 * 8 },
	{ TRUE,  4 * 8,  18 * 8 },
};

static MenuParam countrySel = {
	MENU_DISPLAY_SIZE,
	TXT_COLOR_BLACK,
	TXT_COLOR_GREEN,
	TXT_COLOR_RED,
	&s_countryPos[ 0 ],
	(const u16 **)&s_pStrCountry,
};

static u32 region_country_mapping[TWL_REGION_MAX] = 
{
	TWL_COUNTRY_MAPPING_JAPAN,
	TWL_COUNTRY_MAPPING_AMERICA,
	TWL_COUNTRY_MAPPING_EUROPE,
	TWL_COUNTRY_MAPPING_AUSTRALIA,
	TWL_COUNTRY_MAPPING_CHINA,
	TWL_COUNTRY_MAPPING_KOREA
};

//======================================================
// function's description
//======================================================

// 国名設定の初期化
void SelectCountryInit( void )
{
    int l;
    BOOL in_list_flag = FALSE;
	
	GX_DispOff();
	GXS_DispOff();
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );

	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COUNTRY SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	if( g_initialSet ) {
		PutStringUTF16( 8 * 8, 18 * 8, TXT_COLOR_RED, (const u16 *)L"Select country." );
	}
	
	// 設定済みリージョンと国名コードの取得
	if( !SYSM_IsValidTSD() ||
		( TSD_GetRegion() >= TWL_REGION_MAX ) ) {
		s_regionCode = (TWLRegion)TWL_DEFAULT_REGION;
	}else {
		s_regionCode = (TWLRegion)TSD_GetRegion();
	}
	
	if( !SYSM_IsValidTSD() ||
		( TSD_GetCountry() >= TWL_COUNTRY_MAX ) ) {
		s_countryCode = (TWLCountryCode)0;
	}else {
		s_countryCode = TSD_GetCountry();
	}

	// メニューに表示する国名リスト全体の最初と最後をマッピングデータから取得
	list_start = (u16)(region_country_mapping[s_regionCode] >> 16);
	list_end = (u16)(region_country_mapping[s_regionCode]);
	if(list_start > list_end) OS_Panic("selectCountry.c:list_start>list_end!");
	
	// 画面に表示する最大項目数よりも、国名リストが小さいか？
	countrySel.num = (MENU_DISPLAY_SIZE < list_end - list_start + 1) ? MENU_DISPLAY_SIZE : list_end - list_start + 1 ;
	
	// 設定されていた国名コードがリスト範囲に入っていなければデフォルト値にする
	if(s_countryCode < list_start || list_end < s_countryCode)
	{
		s_countryCode = (TWLCountryCode)list_start;
	}
	
	// 実際に表示する範囲の調整
	s_menu_display_start = s_countryCode;
	if(s_countryCode + countrySel.num - 1 > list_end)
	{
		s_menu_display_start = (u16)(list_end + 1 - countrySel.num);
	}
	
	// 実際に表示する国名のみリスト化
	for(l=0; l<countrySel.num;l++)
	{
		s_pStrCountry[l] = s_pStrCountryName[s_menu_display_start + l];
	}
	
	DrawMenu( (u16)(s_countryCode - s_menu_display_start), &countrySel );
	
	SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );
	
	GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
	GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
	GX_DispOn();
	GXS_DispOn();
}


// 国名選択
int SelectCountryMain( void )
{
	BOOL tp_select,tp_cancel = FALSE;
	static u16 selecteditem;
	int l;
	static int padcount = 0;
	BOOL padcont = FALSE;
	
	ReadTP();												// TP入力の取得
	
	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( pad.cont & PAD_KEY_DOWN ) {								// カーソルの移動
		if(padcount == 0 || (padcount>29 && padcount%5==0))
			if( s_countryCode < list_end ) s_countryCode++;
		padcont = TRUE;
	}
	if( pad.cont & PAD_KEY_UP ) {
		if(padcount == 0 || (padcount>29 && padcount%5==0))
			if( s_countryCode > list_start ) s_countryCode--;
		padcont = TRUE;
	}
	if( padcont ) padcount++;
	else padcount = 0;
	padcont = FALSE;
	
	// キー入力後、表示される項目の調整
	if( s_countryCode < s_menu_display_start ) s_menu_display_start = s_countryCode;
	else if( s_menu_display_start + countrySel.num - 1 < s_countryCode ) s_menu_display_start = (u16)(s_countryCode - countrySel.num + 1);
	
	// 簡易スクロールバーによるスクロール
	
	
	// タッチパッドによるスクロール後、表示される項目の調整
	if( s_countryCode < s_menu_display_start ) s_countryCode = (TWLCountryCode)s_menu_display_start;
	else if( s_menu_display_start + countrySel.num - 1 < s_countryCode ) s_countryCode = (TWLCountryCode)(s_menu_display_start + countrySel.num - 1);
	
	// 実際に表示する国名のみリスト化
	for(l=0; l<countrySel.num;l++)
	{
		s_pStrCountry[l] = s_pStrCountryName[s_menu_display_start + l];
	}
	
	// タッチパッドによるメニュー項目の選択
	selecteditem = (u16)(s_countryCode - s_menu_display_start);
	tp_select = SelectMenuByTP( (u16 *)&selecteditem, &countrySel );
	s_countryCode = (TWLCountryCode)(s_menu_display_start + selecteditem);
	
	// 描画
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
	// 簡易スクロールバー表示
	{
		int list_size = list_end - list_start + 1;
		int bar_height = 107 - (list_size - countrySel.num);
		double dots_per_item = 1;
		if(bar_height<11){
			bar_height = 11;
			dots_per_item = (double)(107-11)/(list_size - countrySel.num);
		}
		PutStringUTF16( 10, 48-11, TXT_UCOLOR_G0, (const u16 *)L"□" );
		for(l=0; l<bar_height-11;l+=11)
		{
			PutStringUTF16( 10, (int)(l+48+dots_per_item * (s_menu_display_start - list_start)), TXT_UCOLOR_G2, (const u16 *)L"■" );
		}
		PutStringUTF16( 10, (int)(bar_height-11+48+dots_per_item * (s_menu_display_start - list_start)), TXT_UCOLOR_G2, (const u16 *)L"■" );
		PutStringUTF16( 10, 48+107, TXT_UCOLOR_G0, (const u16 *)L"□" );
	}
	PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"COUNTRY SELECT" );
	PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_COLOR_CYAN, (const u16 *)L"CANCEL" );
	DrawMenu( (u16)(s_countryCode - s_menu_display_start), &countrySel );
	
	// [CANCEL]ボタン押下チェック
	if( tpd.disp.touch ) {
		tp_cancel = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
							   CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, &tpd.disp );
	}
	
	if( ( pad.trg & PAD_BUTTON_A ) || tp_select ) {				// メニュー項目への分岐
		TSD_SetCountry( s_countryCode );
		//TSD_SetFlagCountry( TRUE );							// 国名入力フラグを立てる
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


