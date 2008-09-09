/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     strResource.c

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include "drawFunc.h"


// VERSION_MENU_SIZE、FONTMENU_SIZEだけコンテンツ数に応じて可変なのでconstではない
int s_numMenu[] = {
	OWNERMENU_SIZE,
	PARENTALMENU_SIZE,
	SECURE_USER_MENU_SIZE,
	OTHERMENU_SIZE,
	NORMAL_HW_MENU_SIZE,
	SECURE_HW_MENU_SIZE,
	SCFG_ARM7_MENU_SIZE,
	SCFG_ARM9_MENU_SIZE,
	SYSMENU_MENU_SIZE,
	FONTMENU_SIZE,
	WLMENU_SIZE,
	WHITEMENU_SIZE,
	VERSIONMENU_SIZE,
	0,
	0,
	0,
	ROOTMENU_SIZE
};

const char *s_strARM7RegisterName[] = {
	"ROM",
	"CLK",
	"JTAG",
	"EXP",
	"MC1",
	"MC2",
	"DSWL",
	"OPT",
};

const char *s_strARM9RegisterName[] = {
	"ROM",
	"CLK",
	"RST",
	"EXP",
};

const char *s_strSCFGViewMode[] = {
	"<Shared Area Data>",
	"<Register Data>"
};

const char *s_strRootMenu[] = {
	"＜Owner＞",
	"＜Parental control＞",
	"＜Secure User Info＞",
	"＜Other Info＞",
	"＜Normal Hardware Info＞",
	"＜Secure Hardware Info＞",
	"＜SCFG Info (ARM7 side)＞",
	"＜SCFG Info (ARM9 side)＞",
	"＜SystemMenu Version＞",
	"＜Shared Font＞",
	"＜Wireless Firmware＞",
	"＜Whitelist＞",
	"＜Installed Content Version＞",
	"Reset HW Setting to Default",
	"break HW Setting",
	"Reset RTC Data"
};

const char *s_strOwnerMenu[] = {
	"Language",
	"Favorite color",
	"Birthday",
	"Country",
	"Nickname",
	"Comment"
};

const char *s_strParentalMenu[] = {
	"Parental control",
	"Picto Chat",
	"ds Download",
	"browser",
	"Wii Point",
	"Photo Exchange",
	"UGC",
	"Organization",
	"Age",
	"Password",
	"Quastion id",
	"Answer"
};

const char *s_strSecureUserMenu[] = {
	"Finished Initial Setting(Setting)",
	"Finished Initial Setting(Launcher)",
	"Finished Broken Setting Sequence",
	"Installed Softbox Count",
	"Free Softbox Count",
	"Last Boot Soft Index",
	"Last Boot Soft Platform",
	"Last Boot Soft ID"
};

const char *s_strOtherMenu[] = {
	"Agree EULA",
	"EULA Version",
	"Wireless",
	"RTC Offset",
	"TP Calib raw x1",
	"TP Calib raw y1",
	"TP Calib dx1",
	"TP Calib dy1",
	"TP Calib raw x2",
	"TP Calib raw y2",
	"TP Calib dx2",
	"TP Calib dy2",
	"TP Calib RSV"
};

const char *s_strNormalHWMenu[] = {
	"RTC Adjustment",
	"Unique ID"
};

const char *s_strSecureHWMenu[] = {
	"Force Disable Wireless",
	"Region",
	"Serial No",
	"Language Bitmap",
	"Fuse Data",
	"Launcher TitleID Lo"
};

const char *s_strSCFGArm9Menu[] = {
	"ARM9 SecureROM",
	"Rom Status",

	"CPU Speed",
	"DSP Clock",	
	"Camera Clock",
	"WRAM Clock",
	"Camera CKI",
	
	"DSP Reset Flag",

	"Fixed DMA",
	"Fixed Geometry",
	"Fixed Renderer",
	"Fixed 2D Engine",
	"Fixed Divider",
	"Fixed Card I/F",
	"Expanded VRAM",
	"Expanded LCDC",
	"Expanded INTC",
	"PSRAM Boundary",
	"New DMA Access",
	"Camera Access",	
	"DSP Access",
	"MemoryCard I/F",
	"WRAM Access",
	"CFG Block Access",
	"All SCFG Check",
	"SCFG Check Err7",
	"SCFG Check Err9"
};

const char *s_strSCFGArm7Menu[] = {
	
	// rom制御
	"ARM9 SecureROM",
	"ARM9 ROM Type",
	"ARM7 SecureROM",
	"ARM7 ROM Type",
	"ARM7 FuseRom",
	"Write Rom Area",
	
	// 新規ブロッククロック
	"SD1 I/F Clock",
	"SD2 I/F Clock",
	"AES Clock",
	"WRAM Clock",
	"SND Clock",
	
	// JTAG
	"Chain ARM7 to CPU JTAG",
	"CPU JTAG",
	"DSP JTAG",
	
	// 拡張機能
	"Fixed A7-DMAC1",
	"Fixed Sound DMA",
	"Fixed Sound",
	"Fixed Memory Card",
	"Expanded A7-INTC",
	"Expanded SPI",
	"Expanded Sound DMA",
	"Expanded SIO",
	"Expanded LCDC",
	"Expanded VRAM",
	"PSRAM Boundary",
	"A7-DMAC2 Block",
	"AES Block",
	"SD1 I/F Block",
	"SD2 I/F Block",
	"Mic Block",
	"I2S I/F Block",
	"I2C I/F Block",
	"GPIO Block",
	"MemoryCard I/F",
	"Shared WRAM",	
	"PU Resistance",
	"ALL SCFG Block",
	
	// メモリカード I/F
	"MC Slot1 DET",
	"MC Slot1 Mode",
	"MC Slot2 DET",
	"MC Slot2 Mode",
	"Swap MC1 MC2",
	"Chatter. Count",
	"MC Counter Data",
	
	// 旧無線
	"Old Wireless",
	
	// オプション端子読み出し
	"Option Form",
	"Option App for"
};

const char *s_strSystemMenu[] = {
	"Timestamp",
	"Version (numeric)",
	"Version (string)",
	"EULA URL",
	"NUP Hostname",

	".twl-nup-cert.der" ,
	".twl-nup-prvkey.der",
	".twl-shop-cert.der",
	".twl-shop-prvkey.der",
	"NintendoCA-G2.der"
};

const char *s_strFontMenu[] = {
	"Timestamp",
};

const char *s_strWLMenu[] = {
	"Version",
	"Num of FW",
	"FW1 type",
	"FW1 version",
	"FW2 type",
	"FW2 version"
};

const char *s_strWhiteMenu[] = {
	"Num of Entry",
	"Tmd Hash"
};

const char **s_strMetaMenu[] = {
	s_strOwnerMenu,
	s_strParentalMenu,
	s_strSecureUserMenu,
	s_strOtherMenu,
	s_strNormalHWMenu,
	s_strSecureHWMenu,
	s_strSCFGArm7Menu,
	s_strSCFGArm9Menu,
	s_strSystemMenu,
	s_strFontMenu,
	s_strWLMenu,
	s_strWhiteMenu,
	NULL,
	NULL,
	NULL,
	NULL,
	s_strRootMenu
};

char *s_strAccess[] = {
	"Inaccessible",
	"Accessible"
};

char *s_strJoint[] = {
	"Jointed",
	"DisJointed"
};

char *s_strSupply[] = {
	"STOPPED",
	"SUPPLIED"
};

char *s_strPSRAM[] = {
	"4MB",
	"16MB",
	"32MB"
};

char *s_strRomMode[] = {
	"TWL 64KB ROM",
	"NITRO 8KB ROM"

};

char *s_strCpuSpeed[] = {
	"67.03 MHz",
	"134.06 MHz"
};

char *s_strRomForm[] = {
	"Included ROM",
	"Downloaded PROM"
};

char *s_strRomApp[] = {
	"Mass Product",
	"Development 1",
	"Development 2",
	"PROM Download"
};

char *s_strWLFWType[] = {
	"2in1 module 1.1",
	"2in1 module 2.0",
	"One Chip (M&M)"
};

char *s_strMCMode[] = {
	"00",
	"01",
	"10",
	"11"
};

char *s_strEnable[] = {
	"DISABLED",
	"ENABLED"
};

char *s_strResult[] = {
	"Failed.",
	"Succeeded.",	
	""
};


char *s_strCorrect[] = {
	"Incorrect",
	"Correct"
};

char *s_strSysMenuKey[] = {
	"Incorrect",
	"correct: dev",
	"correct: prod"
};

char *s_strBool[] = {
	"FALSE",
	"TRUE"
};

char *s_strOK[] = {
	"CANCEL",
	"OK"
};

char *s_strRatingOrg[] = {
	"CERO",
	"ESRB",
	"BBFC",
	"USK",
	"PEGI general",
	"PEGI Finland",
	"PEGI Portugal",
	"PEGI and BBFC Great Briten",
	"OFLC",
	"GRB"
};

 char *s_strRegion[] = {
	"JAPAN",
	"AMERICA",
	"EUROPA",
	"AUSTRALIA",
	"CHINA",
	"KOREA",
};

 char *s_strUserColor[] = {
	"GRAY      ",
	"BROWN     ",
	"RED       ",
	"PINK      ",
	"ORANGE    ",
	"YELLOW    ",
	"LIME GREEN",
	"GREEN     ",
	"DARK GREEN",
	"SEA GREEN ",
	"TURQUOISE ",
	"BLUE      ",
	"DARK BLUE ",
	"PURPLE    ",
	"VIOLET    ",
	"MAGENTA   ",
};

char *s_strLanguage[] = {
	"JAPANESE",
	"ENGLISH",
	"FRENCH",
	"GERMAN",
	"ITALIAN",
	"SPANISH",
	"CHINESE",
	"KOREAN",
};

char *s_strCountry[] = {
    	"UNDEFINED  ",        // 未設定
    	"JAPAN      ",        // 日本
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"Anguilla   ",        // アンギラ
    	"ANTIGUA_AND_BARBUDA",   // アンティグア・バーブーダ
    	"ARGENTINA   ",      // アルゼンチン
    	"ARUBA",                 // アルバ
    	"BAHAMAS",               // バハマ
    	"BARBADOS",              // バルバドス
    	"BELIZE",                // ベリーズ
    	"BOLIVIA",               // ボリビア
    	"BRAZIL",                // ブラジル
    	"BRITISH_VIRGIN_ISLANDS",    // 英領ヴァージン諸島
    	"CANADA",                // カナダ
    	"CAYMAN_ISLANDS",        // ケイマン諸島
    	"CHILE       ",      // チリ
    	"COLOMBIA",              // コロンビア
    	"COSTA_RICA",            // コスタリカ
    	"DOMINICA",              // ドミニカ国
    	"DOMINICAN_REPUBLIC",    // ドミニカ共和国
    	"ECUADOR",               // エクアドル
    	"EL_SALVADOR",           // エルサルバドル
    	"FRENCH_GUIANA",         // フランス領ギアナ
    	"GRENADA",               // グレナダ
    	"GUADELOUPE",            // グアドループ
    	"GUATEMALA   ",      // グアテマラ
    	"GUYANA",                // ガイアナ
    	"HAITI",                 // ハイチ
    	"HONDURAS",              // ホンジュラス
    	"JAMAICA",               // ジャマイカ
    	"MARTINIQUE",            // マルティニーク
    	"MEXICO",                // メキシコ
    	"MONTSERRAT",            // モントセラト
    	"NETHERLANDS_ANTILLES",  // オランダ領アンティル
    	"NICARAGUA",             // ニカラグア
    	"PANAMA      ",      // パナマ
    	"PARAGUAY",              // パラグアイ
    	"PERU",                  // ペルー
    	"ST_KITTS_AND_NEVIS",    // セントキッツ・ネイビス
    	"ST_LUCIA",              // セントルシア
    	"ST_VINCENT_AND_THE_GRENADINES", // セントビンセント・グレナディーン
    	"SURINAME",              // スリナム
    	"TRINIDAD_AND_TOBAGO",   // トリニダード・トバゴ
    	"TURKS_AND_CAICOS_ISLANDS",  // タークス・カイコス諸島
    	"UNITED_STATES",         // アメリカ
    	"URUGUAY     ",      // ウルグアイ
    	"US_VIRGIN_ISLANDS",     // 米領バージン諸島
    	"VENEZUELA",             // ベネズエラ
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"ALBANIA     ",      // アルバニア
    	"AUSTRALIA",             // オーストラリア
    	"AUSTRIA",               // オーストリア
    	"BELGIUM",               // ベルギー
    	"BOSNIA_AND_HERZEGOVINA",    // ボスニア・ヘルツェゴビナ
    	"BOTSWANA",              // ボツワナ
    	"BULGARIA    ",      // ブルガリア
    	"CROATIA",               // クロアチア
    	"CYPRUS",                // キプロス
    	"CZECH_REPUBLIC",        // チェコ
    	"DENMARK",               // デンマーク
    	"ESTONIA",               // エストニア
    	"FINLAND",               // フィンランド
    	"FRANCE",                // フランス
    	"GERMANY",               // ドイツ
    	"GREECE",                // ギリシャ
    	"HUNGARY     ",      // ハンガリー
    	"ICELAND",               // アイスランド
    	"IRELAND",               // アイルランド
    	"ITALY",                 // イタリア
    	"LATVIA",                // ラトビア
    	"LESOTHO",               // レソト
    	"LIECHTENSTEIN",         // リヒテンシュタイン
    	"LITHUANIA",             // リトアニア
    	"LUXEMBOURG",            // ルクセンブルク
    	"MACEDONIA",             // マケドニア
    	"MALTA       ",      // マルタ
    	"MONTENEGRO",            // モンテネグロ
    	"MOZAMBIQUE",            // モザンビーク
    	"NAMIBIA",               // ナミビア
    	"NETHERLANDS",           // オランダ
    	"NEW_ZEALAND",           // ニュージーランド
    	"NORWAY",                // ノルウェー
    	"POLAND",                // ポーランド
    	"PORTUGAL",              // ポルトガル
    	"ROMANIA",               // ルーマニア
    	"RUSSIA      ",     // ロシア
    	"SERBIA",                // セルビア
    	"SLOVAKIA",              // スロバキア
    	"SLOVENIA",              // スロベニア
    	"SOUTH_AFRICA",          // 南アフリカ
    	"SPAIN",                 // スペイン
    	"SWAZILAND",             // スワジランド
    	"SWEDEN",                // スウェーデン
    	"SWITZERLAND",           // スイス
    	"TURKEY",                // トルコ
    	"UNITED_KINGDOM ",  // イギリス
    	"ZAMBIA",                // ザンビア
    	"ZIMBABWE",              // ジンバブエ
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"TAIWAN      ",     // 台湾
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"SOUTH_KOREA ",     // 韓国
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"HONG_KONG   ",     // ホンコン
    	"MACAU",                 // マカオ
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"INDONESIA   ",     // インドネシア
    	"SINGAPORE   ",     // シンガポール
    	"THAILAND    ",     // タイ
    	"PHILIPPINES",           // フィリピン
    	"MALAYSIA",              // マレーシア
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"CHINA       ",     // 中国
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"UAE         ",     // アラブ首長国連邦
    	"INDIA       ",     // インド
    	"EGYPT       ",     // エジプト
    	"OMAN",                  // オマーン
    	"QATAR",                 // カタール
    	"KUWAIT",                // クウェート
    	"SAUDI_ARABIA",          // サウジアラビア
    	"SYRIA",                 // シリア
    	"BAHRAIN",               // バーレーン
    	"JORDAN",                // ヨルダン
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
    	"OTHERS      ",
    	"UNKNOWN     ",
};

char s_strNA[] = {
	"N/A"
};	

