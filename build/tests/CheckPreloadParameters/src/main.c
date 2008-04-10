/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - CheckPreloadParameters
  File:     main.c

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

void VBlankIntr(void);

OSOwnerInfoEx s_owner;
u8 s_cameraInfo[ OS_TWL_HWINFO_CAMERA_LEN ];
static char *s_strCountry[ 256 ];
static char *s_strRegion[ OS_TWL_REGION_MAX ];
static char *s_strUserColor[ OS_FAVORITE_COLOR_MAX ];
static char *s_strLanguage[ OS_LANGUAGE_CODE_MAX ];


/*---------------------------------------------------------------------------*
  Name:         TwlMain

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void TwlMain(void)
{
	OS_Init();

	//---- interrupt setting
	OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
	OS_EnableIrqMask(OS_IE_V_BLANK);
	OS_EnableIrq();
	GX_VBlankIntr(TRUE);

	OS_GetOwnerInfoEx( &s_owner );
	OS_TPrintf( "Language  : %02x %s\n", s_owner.language, s_strLanguage[ s_owner.language ] );
	OS_TPrintf( "UserColor : %02x %s\n", s_owner.favoriteColor, s_strUserColor[ s_owner.favoriteColor ] );
	OS_TPrintf( "Birthday  : %02d/%02d\n", s_owner.birthday.month, s_owner.birthday.day );
	{
		char string[ 256 ];
		int srcLen, dstLen;
		MI_CpuClear8( string, sizeof(string) );
		srcLen = (int)s_owner.nickNameLength;
		dstLen = sizeof(string);
		STD_ConvertStringUnicodeToSjis( string, &dstLen, s_owner.nickName, &srcLen, NULL );
		OS_TPrintf( "Nickname : %s\n", string );
		
		MI_CpuClear8( string, sizeof(string) );
		srcLen = (int)s_owner.commentLength;
		dstLen = sizeof(string);
		STD_ConvertStringUnicodeToSjis( string, &dstLen, s_owner.comment, &srcLen, NULL );
		OS_TPrintf( "Comment  : %s\n", string );
	}
	OS_TPrintf( "Country  : %s\n", s_strCountry[ s_owner.country ] );
	
	OS_TPrintf( "AvailableWireless : %s\n", OS_IsAvailableWireless() ? "TRUE" : "FALSE" );
	PMi_SetWirelessLED( OS_IsAvailableWireless() ? PM_WIRELESS_LED_ON : PM_WIRELESS_LED_OFF );
	{
		int i;
		OS_TPrintf( "CameraInfo :" );
		OS_GetCameraInfo( s_cameraInfo );
		for( i = 0; i < OS_TWL_HWINFO_CAMERA_LEN; i++ ) {
			if( ( i & 0x0f ) == 0 ) {
				OS_TPrintf( "\n" );
			}
			OS_TPrintf( " %02x,", s_cameraInfo[ i ] );
		}
		OS_TPrintf( "\n" );
	}
	OS_TPrintf( "ForceDisableWireless : %s\n", OS_IsForceDisableWireless() ? "TRUE" : "FALSE" );
	OS_TPrintf( "Region : %02x %s\n", OS_GetRegion(), ( OS_GetRegion() == 0xff ) ? "Invalid" : s_strRegion[ OS_GetRegion() ] );
	{
		u8 string[ 16 ];
		OS_GetSerialNo( string );
		OS_TPrintf( "SerialNo : %s\n", string );
	}
	
	OS_TPrintf("***End of demo\n");
	OS_Terminate();
}



/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  main

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}



static char *s_strRegion[] = {
	"JAPAN",
	"AMERICA",
	"EUROPA",
	"AUSTRALIA",
	"CHINA",
	"KOREA",
};

static char *s_strUserColor[] = {
	"GRAY      ",
	"BROWN     ",
	"RED       ",
	"PINK      ",
	"ORANGE    ",
	"YELLOW    ",
	"LIME_GREEN",
	"GREEN     ",
	"DARK_GREEN",
	"SEA_GREEN ",
	"TURQUOISE ",
	"BLUE      ",
	"DARK_BLUE ",
	"PURPLE    ",
	"VIOLET    ",
	"MAGENTA   ",
};

static char *s_strLanguage[] = {
	"JAPANESE",
	"ENGLISH",
	"FRENCH",
	"GERMAN",
	"ITALIAN",
	"SPANISH",
	"CHINESE",
	"KOREAN",
};

static char *s_strCountry[] = {
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

