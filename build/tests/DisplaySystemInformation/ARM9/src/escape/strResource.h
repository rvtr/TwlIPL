static const u8 s_numMenuK[] = {
	OWNERMENU_KSIZE,
	PARENTALMENU_KSIZE,
	OTHERMENU_KSIZE,
	SCFGMENU_KSIZE,
	FUSEMENU_KSIZE,
	0,
	0,
	0,
	0,
	0,
	ROOTMENU_KSIZE
	// !!! あとで残りの分も追加するよ
};

static const u8 s_numMenuV[] = {
	OWNERMENU_VSIZE,
	PARENTALMENU_VSIZE,
	OTHERMENU_VSIZE,
	SCFGMENU_VSIZE,
	FUSEMENU_VSIZE,
	0,
	0,
	0,
	0,
	0,
	ROOTMENU_VSIZE
	// !!! あとで残りの分も追加するよ
};

static const u16 *s_strRootMenu[] = {
	L"Owner",
	L"Parental control",
	L"Other machine setting",
	L"SCFG",
	L"Fuse rom"
};

static const char *s_strMenuName[] = {
	"Owner",
	"Parental control",
	"Other machine setting",
	"SCFG",
	"Fuse rom"
};


static const u16 *s_strOwnerMenu[] = {
	L"Language",
	L"Favorite color",
	L"Birthday",
	L"Country",
	L"Nickname",
	L"Comment"
};

static const u16 *s_strParentalMenu[] = {
	L"Parental control",
	L"Organization",
	L"Age",
	L"Password",
	L"Quastion id",
	L"Answer"
};

static const u16 *s_strOtherMenu[] = {
	L"Wireless",
	L"Force Disable Wireless",
	L"",
	L"Agree EULA",
	L"Eula Version",
	L"Region",
	L"",
	L"",
	L"Unique ID",
	L"",
	L"",
	
	L"Serial No"
};

static const u16 *s_strSCFGMenu[] = {
	L"UNDER CONSTRUCTION"
};

static const u16 *s_strFuseMenu[] = {
	L"UNDER CONSTRUCTION"	
};

static const u16 **s_strMetaMenu[] = {
	s_strOwnerMenu,
	s_strParentalMenu,
	s_strOtherMenu,
	s_strSCFGMenu,
	s_strFuseMenu,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	s_strRootMenu
};

static char *s_strEnable[] = {
	"DISABLED",
	"ENABLED"
};

static char *s_strBool[] = {
	"TRUE",
	"FALSE"
};

static char *s_strRatingOrg[] = {
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

static  char *s_strRegion[] = {
	"JAPAN",
	"AMERICA",
	"EUROPA",
	"AUSTRALIA",
	"CHINA",
	"KOREA",
};

static  char *s_strUserColor[] = {
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

static char s_strNA[] = {
	""
};	