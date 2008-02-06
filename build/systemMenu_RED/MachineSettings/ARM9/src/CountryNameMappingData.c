#include "misc.h"

const u16 *const s_pStrCountryName[] = {
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
	(const u16 *)L"ST_VINCENT(略)GRENADINES",  // セントビンセント・グレナディーン
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

const u32 region_country_mapping[LCFG_TWL_REGION_MAX] = 
{
	LCFG_TWL_COUNTRY_MAPPING_JAPAN,
	LCFG_TWL_COUNTRY_MAPPING_AMERICA,
	LCFG_TWL_COUNTRY_MAPPING_EUROPE,
	LCFG_TWL_COUNTRY_MAPPING_AUSTRALIA,
	LCFG_TWL_COUNTRY_MAPPING_CHINA,
	LCFG_TWL_COUNTRY_MAPPING_KOREA
};