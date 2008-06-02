/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     setParentalControl.c

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

//
// <処理>
// 以下のペアレンタルコントロール情報の設定
// ++ レーティング制限のON/OFF
// ++ レーティング年齢
// ++ レーティング団体
// ++ 暗証番号(4桁の数字:asciiで格納)
// ++ 秘密の質問ID (質問内容が決定していないのでIDで指定)
// ++ 秘密の質問への回答(日本リージョンのみ文字数制限が半分になる:日本語3--32文字/その他6--64文字)
//
// <例外処理>
// ++ 暗証番号はasciiなので初期状態ではascii以外が格納されている可能性がある
//    -> 各桁がasciiの範囲外のときメニュー画面/設定画面では '*' を表示
//       -- 設定画面では4桁すべてに正当な値が入力されるまでOKボタンを押せない(キャンセルは可能)
// ++ 秘密の質問への回答は日本リージョンだけ文字数制限が小さいので
//    文字数制限(32文字)以上の文字列が格納されている場合がある(他のリージョンから日本へリージョン変更したときなど)
//    -> 設定画面では33文字目以降を切り捨てて32文字(制限ぎりぎり)の文字列とする
//       -- 設定画面で決定ボタンが押されたときに33文字目以降を0クリアして登録する
//       -- 設定画面でキャンセルされたときは元の文字列のままである
//

// テスト表示
#if 1
#define DEBUGPRINT              OS_TPrintf
#else
#define DEBUGPRINT(...)         ((void)0)
#endif

// define data----------------------------------

// ソフトウェアキーボードLCD領域
#define CLIST_LT_X                          23
#define CLIST_LT_Y                          50
#define CLIST_MARGIN                        14
#define CLIST_KEY_PER_SEGMENT               5
#define CLIST_SEGMENT_INTERVAL              7

// キャンセルボタン領域
#define CANCEL_BUTTON_TOP_X                 ( 2 * 8 )
#define CANCEL_BUTTON_TOP_Y                 ( 21 * 8 )
#define CANCEL_BUTTON_BOTTOM_X              ( CANCEL_BUTTON_TOP_X + (8 * 8) )
#define CANCEL_BUTTON_BOTTOM_Y              ( CANCEL_BUTTON_TOP_Y + (2 * 8) )

// OKボタン領域
#define OK_BUTTON_TOP_X                     ( 26 * 8 )
#define OK_BUTTON_TOP_Y                     ( 21 * 8 )
#define OK_BUTTON_BOTTOM_X                  ( OK_BUTTON_TOP_X + (4 * 8) )
#define OK_BUTTON_BOTTOM_Y                  ( OK_BUTTON_TOP_Y + (2 * 8) )

// 改ページボタン領域
#define PREVPAGE_BUTTON_TOP_X               ( 2 * 8 )
#define PREVPAGE_BUTTON_TOP_Y               ( 2 * 8 )
#define PREVPAGE_BUTTON_BOTTOM_X            ( PREVPAGE_BUTTON_TOP_X + (2 * 8) + (9 * 8) )
#define PREVPAGE_BUTTON_BOTTOM_Y            ( PREVPAGE_BUTTON_TOP_Y + (2 * 8) )
#define SUCCPAGE_BUTTON_TOP_X               ( 18 * 8 )
#define SUCCPAGE_BUTTON_TOP_Y               (  2 * 8 )
#define SUCCPAGE_BUTTON_BOTTOM_X            ( SUCCPAGE_BUTTON_TOP_X + (2 * 8) + (9 * 8) )
#define SUCCPAGE_BUTTON_BOTTOM_Y            ( SUCCPAGE_BUTTON_TOP_Y + (2 * 8) )

// ON/OFFボタン領域
#define ON_BUTTON_TOP_X                     ( 8 * 8 )
#define ON_BUTTON_TOP_Y                     ( 8 * 8 )
#define ON_BUTTON_BOTTOM_X                  ( ON_BUTTON_TOP_X + (2 * 8) )
#define ON_BUTTON_BOTTOM_Y                  ( ON_BUTTON_TOP_Y + (2 * 8) )
#define OFF_BUTTON_TOP_X                    ( 18 * 8 )
#define OFF_BUTTON_TOP_Y                    (  8 * 8 )
#define OFF_BUTTON_BOTTOM_X                 ( OFF_BUTTON_TOP_X + (3 * 8) )
#define OFF_BUTTON_BOTTOM_Y                 ( OFF_BUTTON_TOP_Y + (2 * 8) )

// UP/DOWNボタン領域
#define UP_BUTTON_TOP_X                     ( 8 * 8 )
#define UP_BUTTON_TOP_Y                     ( 6 * 8 )
#define UP_BUTTON_BOTTOM_X                  ( UP_BUTTON_TOP_X + (2 * 8) + (2 * 8) )
#define UP_BUTTON_BOTTOM_Y                  ( UP_BUTTON_TOP_Y + (2 * 8) )
#define DOWN_BUTTON_TOP_X                   (  8 * 8 )
#define DOWN_BUTTON_TOP_Y                   ( 14 * 8 )
#define DOWN_BUTTON_BOTTOM_X                ( DOWN_BUTTON_TOP_X + (2 * 8) + (4 * 8) )
#define DOWN_BUTTON_BOTTOM_Y                ( DOWN_BUTTON_TOP_Y + (2 * 8) )

// 複数のUP/DOWNボタン領域
#define MULTI_UP_BUTTON_OFFSET_X            ( 6 * 8 )   // 左端からのスペース
#define MULTI_UP_BUTTON_OFFSET_Y            ( 6 * 8 )
#define MULTI_UP_BUTTON_WIDTH_X             ( 2 * 8 )   // ボタン幅
#define MULTI_UP_BUTTON_WIDTH_Y             ( 2 * 8 )
#define MULTI_UP_BUTTON_INTERVAL_X          ( 4 * 8 )   // 左右のボタン間隔(横に並べるのでXだけでよい)
#define MULTI_DOWN_BUTTON_OFFSET_X          (MULTI_UP_BUTTON_OFFSET_X)
#define MULTI_DOWN_BUTTON_OFFSET_Y          (14 * 8 )
#define MULTI_DOWN_BUTTON_WIDTH_X           ( 2 * 8 )
#define MULTI_DOWN_BUTTON_WIDTH_Y           ( 2 * 8 )
#define MULTI_DOWN_BUTTON_INTERVAL_X        (MULTI_UP_BUTTON_INTERVAL_X)

// RESTRICT/NOT RESTRICTボタン領域
#define RESTRICT_BUTTON_TOP_X               (  8 * 8 )
#define RESTRICT_BUTTON_TOP_Y               ( 10 * 8 )
#define RESTRICT_BUTTON_BOTTOM_X            ( RESTRICT_BUTTON_TOP_X + (8 * 8) )
#define RESTRICT_BUTTON_BOTTOM_Y            ( RESTRICT_BUTTON_TOP_Y + (2 * 8) )
#define NOTRESTRICT_BUTTON_TOP_X            (  8 * 8 )
#define NOTRESTRICT_BUTTON_TOP_Y            (  6 * 8 )
#define NOTRESTRICT_BUTTON_BOTTOM_X         ( NOTRESTRICT_BUTTON_TOP_X + (12 * 8) )
#define NOTRESTRICT_BUTTON_BOTTOM_Y         ( NOTRESTRICT_BUTTON_TOP_Y + ( 2 * 8) )


// ページ数
#define MS_PARENTAL_NUMOF_PAGES             3

// 項目の総数
#ifdef BROADON_UI
#define MS_PARENTAL_NUMOF_ELEMENTS          10
#else   // BROADON_UI
#define MS_PARENTAL_NUMOF_ELEMENTS          7
#endif  // BROADON_UI

// ソフトウェアキーボードのパラメータ
#define CHAR_LIST_CHAR_NUM                  120
#define CHAR_LIST_MODE_NUM                  3

// 特殊キーコード
#define EOM_                                (u16)0xe050
#define CODE_BUTTON_TOP_                    (u16)0xe051
#define DEL_BUTTON_                         (u16)0xe051
#define SPACE_BUTTON_                       (u16)0xe052
#define VAR_BUTTON1_                        (u16)0xe053
#define VAR_BUTTON2_                        (u16)0xe054
#define OK_BUTTON_                          (u16)0xe055
#define CANCEL_BUTTON_                      (u16)0xe056
#define CODE_BUTTON_BOTTOM_                 (u16)0xe057

#define CHAR_USCORE     L'＿'
#define KEY_PER_LINE    11

#define KEY_START       109	//ソフトウェアキーのカーソルデフォルト位置はキャンセルキー

#define KEY_OK          0xffff
#define KEY_CANCEL      0xfffe
#define KEY_PREVPAGE    0xfffd
#define KEY_SUCCPAGE    0xfffc
#define KEY_ON          0xfffb
#define KEY_OFF         0xfffa
#define KEY_UP          0xfff9
#define KEY_DOWN        0xfff8
#define KEY_RESTRICT    0xfff7
#define KEY_NOTRESTRICT 0xfff6
#define MULTI_KEY_UP    0xffe0
#define MULTI_KEY_DOWN  0xffd0
#define MASK_MULTI_KEY  0xfff0

// 改行が必要なときの一行での表示文字数/何回改行するか
#define  MS_PARENTAL_ANS_COL            16
#define  MS_PARENTAL_ANS_NUMOF_ROWS     (LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX / MS_PARENTAL_ANS_COL - 1)

// ソフトウェアキーボードを使用するモード
typedef enum eUseSoftKey
{
    USE_SOFTKEY_COMMON,     // 不使用
    USE_SOFTKEY_ANSWER      // 秘密の質問に対する回答
}eUseSoftKey;

// extern data----------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_birth_scr_data[32 * 32];

// function's prototype-------------------------

static void SetRatingOgnInit( void );
static int  SetRatingOgnMain( void );
static void SetRatingLockInit( void );
static int  SetRatingLockMain( void );
static void SetRatingAgeInit( void );
static int  SetRatingAgeMain( void );
static void SetSecretQuestionIDInit( void );
static int  SetSecretQuestionIDMain( void );
static void SetAnswerInit( void );
static int  SetAnswerMain( void );
static void SetPasswordInit( void );
static int  SetPasswordMain( void );
static u8   MY_StrLen( const u16 *pStr );
static void SetDrawnPageElement( void );
static void SetBroadOnSettingInit( void );
static int  SetBroadOnSettingMain( void );

// static variable------------------------------
// 一時的にしか使わない物をstaticにしているので
// 少しでもダイエットしたい時はWork扱いにしてAlloc→Freeしましょう

// メニューで使用
static u16    sCursorMenu = 0;

// 流用
static int    s_char_mode = 0;      // ソフトウェアキーボードの文字種別(かな/カナ/英数)
static u16    s_key_csr = 0;        // 現在のキー位置

// 特定のモードでのみ使用
static u8     sCurrentPage = 0;             // ページ番号
static BOOL   sbInitPage = TRUE;            // メニューに戻るときに1ページ目に戻すかどうか
static BOOL   sbRatingLock = FALSE;         // レーティング制限
static u8     sRatingAge=0;                 // レーティング年齢
static u16    sRatingOgnFirstIndex=0;       // 団体リストのオフセット(テーブルの何番目から団体リストとして使用するか)
static u16    sCursorRatingOgn=0;           // オフセットからの位置(リストから何番目の団体が選択されているか)
static u8     sRegion=0;                    // 本体のリージョン
static u16    spBufAnswer[ LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1 ];    // 秘密の質問に対する回答
static u8     sCurrentLenAnswer;
static u8     sBufSizeAnswer;
static u8     sMinLenAnswer;                                                // -- 文字制限
static u8     sMaxLenAnswer;
static u8     spBufPassword[ LCFG_TWL_PCTL_PASSWORD_LENGTH + 1 ];           // パスワード
static u16    sCursorPassword=0;
static BOOL   sbValidPassword=FALSE;
static u8     sSecretQuestionID=0;
static BOOL   sbEnableOKButton = FALSE;
static u8     sBroadOnMenu = 0;
static BOOL   sbBroadOnSetting = FALSE;

// const data-----------------------------------

// ++ メインメニュー

static const u16 *s_pStrSetting[ MS_PARENTAL_NUMOF_ELEMENTS ];          // メインメニュー用文字テーブルへのポインタリスト

static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM];      // ソフトウェアキーボードの文字配列

static const u16 *const s_pStrSettingElemTbl[ MS_PARENTAL_NUMOF_ELEMENTS ][ LCFG_TWL_LANG_CODE_MAX ] = {

    // ページ1
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
    {
        (const u16 *)L"レーティング対象年齢",
        (const u16 *)L"RATING TARGET AGE",
        (const u16 *)L"RATING TARGET AGE(F)",
        (const u16 *)L"RATING TARGET AGE(G)",
        (const u16 *)L"RATING TARGET AGE(I)",
        (const u16 *)L"RATING TARGET AGE(S)",
        (const u16 *)L"RATING TARGET AGE(C)",
        (const u16 *)L"RATING TARGET AGE(K)",
    },
    {
        (const u16 *)L"レーティング団体",
        (const u16 *)L"RATING ORGANIZATION",
        (const u16 *)L"RATING ORGANIZATION(F)",
        (const u16 *)L"RATING ORGANIZATION(G)",
        (const u16 *)L"RATING ORGANIZATION(I)",
        (const u16 *)L"RATING ORGANIZATION(S)",
        (const u16 *)L"RATING ORGANIZATION(C)",
        (const u16 *)L"RATING ORGANIZATION(K)",
    },
//    {
//        (const u16 *)L"Wi-Fi制限",
//        (const u16 *)L"Wi-Fi LOCK",
//        (const u16 *)L"Wi-Fi LOCK(F)",
//        (const u16 *)L"Wi-Fi LOCK(G)",
//        (const u16 *)L"Wi-Fi LOCK(I)",
//        (const u16 *)L"Wi-Fi LOCK(S)",
//        (const u16 *)L"Wi-Fi LOCK(C)",
//        (const u16 *)L"Wi-Fi LOCK(K)",
//    },
//    {
//        (const u16 *)L"すれちがい通信制限",
//        (const u16 *)L"Pass-by LOCK",
//        (const u16 *)L"Pass-by LOCK(F)",
//        (const u16 *)L"Pass-by LOCK(G)",
//        (const u16 *)L"Pass-by LOCK(I)",
//        (const u16 *)L"Pass-by LOCK(S)",
//        (const u16 *)L"Pass-by LOCK(C)",
//        (const u16 *)L"Pass-by LOCK(K)",
//    },
    
    // ページ2
    {
        (const u16 *)L"暗証番号",
        (const u16 *)L"PASSWORD",
        (const u16 *)L"PASSWORD(F)",
        (const u16 *)L"PASSWORD(G)",
        (const u16 *)L"PASSWORD(I)",
        (const u16 *)L"PASSWORD(S)",
        (const u16 *)L"PASSWORD(C)",
        (const u16 *)L"PASSWORD(K)",
    },
    {
        (const u16 *)L"秘密の質問ID",
        (const u16 *)L"SECRET QUESTION ID",
        (const u16 *)L"SECRET QUESTION ID(F)",
        (const u16 *)L"SECRET QUESTION ID(G)",
        (const u16 *)L"SECRET QUESTION ID(I)",
        (const u16 *)L"SECRET QUESTION ID(S)",
        (const u16 *)L"SECRET QUESTION ID(C)",
        (const u16 *)L"SECRET QUESTION ID(K)",
    },
    {
        (const u16 *)L"秘密の質問への回答",
        (const u16 *)L"ANSWER FOR SECRET QUESTION",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(F)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(G)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(I)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(S)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(C)",
        (const u16 *)L"ANSWER FOR SECRET QUESTION(K)",
    },
    
    // ページ3
#ifdef BROADON_UI
    {
        (const u16 *)L"ピクトチャット起動",
        (const u16 *)L"PictoChat Boot",
        (const u16 *)L"PictoChat Boot(F)",
        (const u16 *)L"PictoChat Boot(G)",
        (const u16 *)L"PictoChat Boot(I)",
        (const u16 *)L"PictoChat Boot(S)",
        (const u16 *)L"PictoChat Boot(C)",
        (const u16 *)L"PictoChat Boot(K)",
    },
    {
        (const u16 *)L"DSダウンロード起動",
        (const u16 *)L"DSDownload Boot",
        (const u16 *)L"DSDownload Boot(F)",
        (const u16 *)L"DSDownload Boot(G)",
        (const u16 *)L"DSDownload Boot(I)",
        (const u16 *)L"DSDownload Boot(S)",
        (const u16 *)L"DSDownload Boot(C)",
        (const u16 *)L"DSDownload Boot(K)",
    },
    {
        (const u16 *)L"ブラウザ起動",
        (const u16 *)L"Browser Boot",
        (const u16 *)L"Browser Boot(F)",
        (const u16 *)L"Browser Boot(G)",
        (const u16 *)L"Browser Boot(I)",
        (const u16 *)L"Browser Boot(S)",
        (const u16 *)L"Browser Boot(C)",
        (const u16 *)L"Browser Boot(K)",
    },
    {
        (const u16 *)L"Wiiポイント",
        (const u16 *)L"Wii Point",
        (const u16 *)L"Wii Point(F)",
        (const u16 *)L"Wii Point(G)",
        (const u16 *)L"Wii Point(I)",
        (const u16 *)L"Wii Point(S)",
        (const u16 *)L"Wii Point(C)",
        (const u16 *)L"Wii Point(K)",
    },
    {
        (const u16 *)L"写真交換",
        (const u16 *)L"Photo Exchange",
        (const u16 *)L"Photo Exchange(F)",
        (const u16 *)L"Photo Exchange(G)",
        (const u16 *)L"Photo Exchange(I)",
        (const u16 *)L"Photo Exchange(S)",
        (const u16 *)L"Photo Exchange(C)",
        (const u16 *)L"Photo Exchange(K)",
    },
    {
        (const u16 *)L"UGC",
        (const u16 *)L"UGC",
        (const u16 *)L"UGC(F)",
        (const u16 *)L"UGC(G)",
        (const u16 *)L"UGC(I)",
        (const u16 *)L"UGC(S)",
        (const u16 *)L"UGC(C)",
        (const u16 *)L"UGC(K)",
    },
#else  // BROADON_UI
    {
        (const u16 *)L"その他の設定(仮)",
        (const u16 *)L"OTHER SETTINGS(None)",
        (const u16 *)L"OTHER SETTINGS(None)(F)",
        (const u16 *)L"OTHER SETTINGS(None)(G)",
        (const u16 *)L"OTHER SETTINGS(None)(I)",
        (const u16 *)L"OTHER SETTINGS(None)(S)",
        (const u16 *)L"OTHER SETTINGS(None)(C)",
        (const u16 *)L"OTHER SETTINGS(None)(K)",
    },
#endif // BROADON_UI
};

// 表示位置
static MenuPos s_settingPos[] = {
    // ページ1
    { TRUE,  4 * 8,   6 * 8 },
    { TRUE,  4 * 8,   8 * 8 },
    { TRUE,  4 * 8,  10 * 8 },
//    { TRUE,  4 * 8,  12 * 8 },
//    { TRUE,  4 * 8,  14 * 8 },

    // ページ2
    { TRUE,  3 * 8,   6 * 8 },
    { TRUE,  3 * 8,   8 * 8 },
    { TRUE,  3 * 8,  10 * 8 },

    // ページ3
#ifdef BROADON_UI
    { TRUE,  2 * 8,   6 * 8 },
    { TRUE,  2 * 8,   8 * 8 },
    { TRUE,  2 * 8,  10 * 8 },
    { TRUE,  2 * 8,  12 * 8 },
#else  // BROADON_UI
    { FALSE,  4 * 8,   6 * 8 },
#endif // BROADON_UI
};

// 各ページの表示項目数
static const int sNumOfPageElements[] =
{
    3,
    3,
#ifdef BROADON_UI
    4,
#else  // BROADON_UI
    1,
#endif // BROADON_UI
};

// 表示パラメータ
static MenuParam s_settingParam = 
{
    3,
    TXT_COLOR_BLACK,
    TXT_COLOR_GREEN,
    TXT_COLOR_RED,
    &s_settingPos[0],
    (const u16 **)&s_pStrSetting,
};

// ++ ソフトウェアキーボード

static const u16 *str_button_char[CHAR_LIST_MODE_NUM] = 
{
    L"かな",
    L"カナ",
    L"英数",
};

static u16 next_char_mode[CHAR_LIST_MODE_NUM-1];

static const u16  str_button_del[]    = L"DEL";
static const u16  str_button_space[]  = L"SPACE";
static const u16  str_button_ok[]     = L"OK";
static const u16  str_button_cancel[] = L"CANCEL";

static const u16 *str_button[] = 
{
    (const u16 *)str_button_del,
    (const u16 *)str_button_space,
    NULL,
    NULL,
    (const u16 *)str_button_ok,
    (const u16 *)str_button_cancel,
};

// ++ レーティング団体の選択

// リージョンの文字列
static const u16 *sppRegionCharTable[] =
{
    (const u16*)L"JAPAN",
    (const u16*)L"AMERICA",
    (const u16*)L"EUROPE",
    (const u16*)L"AUSTRALIA",
    (const u16*)L"CHINA",
    (const u16*)L"KOREA",
};

// レーティングリストのオフセット(リストの先頭にUNDEFINEDがあるため)
static const u16 sRatingOgnIndexOffset = 0;     // 1がリストの先頭index

// レーティング団体
static const u16 *sppRatingOgnCharTable[] =
{
    (const u16*)L"CERO",            //LCFG_TWL_PCTL_OGN_CERO        = 0,  // 日本
    (const u16*)L"ESRB",            //LCFG_TWL_PCTL_OGN_ESRB        = 1,  // アメリカ
    (const u16*)L"RESERVED2",       //LCFG_TWL_PCTL_OGN_RESERVED2   = 2,  // 
    (const u16*)L"USK",             //LCFG_TWL_PCTL_OGN_USK         = 3,  // ドイツ
    (const u16*)L"PEGI GENERAL",    //LCFG_TWL_PCTL_OGN_PEGI_GEN    = 4,  // 欧州
    (const u16*)L"RESERVED5",       //LCFG_TWL_PCTL_OGN_RESERVED5   = 5,  // 
    (const u16*)L"PEGI PORTUGAL",   //LCFG_TWL_PCTL_OGN_PEGI_PRT    = 6,  // ポルトガル
    (const u16*)L"PEGI BBFC",       //LCFG_TWL_PCTL_OGN_PEGI_BBFC   = 7,  // イギリス
    (const u16*)L"OFLC",            //LCFG_TWL_PCTL_OGN_OFLC        = 8,  // オーストラリア、ニュージーランド
    (const u16*)L"GRB",             //LCFG_TWL_PCTL_OGN_GRB         = 9,  // 韓国
};

// 表示位置のリスト
static MenuPos spRatingOgnPosTable[] =
{
    { TRUE, 10 * 8,  8 * 8 },   // 日本
    { TRUE, 10 * 8,  8 * 8 },   // アメリカ
    { TRUE, 10 * 8,  8 * 8 },
    { TRUE, 10 * 8, 10 * 8 },
    { TRUE, 10 * 8, 12 * 8 },   // 欧州
    { TRUE, 10 * 8, 14 * 8 },
    { TRUE, 10 * 8, 16 * 8 },
    { TRUE, 10 * 8, 18 * 8 },
    { TRUE, 10 * 8,  8 * 8 },   // オーストラリア
    { TRUE, 10 * 8,  8 * 8 },   // 韓国
};

// リージョンごとの団体数
static const int spNumOfRatingOgnsTable[] =
{
    1,  // 日本
    1,  // アメリカ
    6,  // 欧州
    1,  // オーストラリア
    0,  // 中国
    1,  // 韓国
};

static const int sIndexFirstOgnTable[] = {
	0,
	1,
	2,
	8,
	0,
	9,
};

// ここに表示したい(選択させたい)リストをコピーする
static const u16 *sppRatingOgnCharList[ LCFG_TWL_PCTL_OGN_MAX ];

// 表示/選択関数に渡すパラメータ
static MenuParam sRatingOgnMenuParam =
{
    1,
    TXT_COLOR_BLACK,
    TXT_COLOR_GREEN,
    TXT_COLOR_RED,
    &spRatingOgnPosTable[0],
    (const u16 **)&sppRatingOgnCharList,
};

// ++ BroadOn用の設定項目
enum
{
    MS_BROADON_PICTOCHAT      = 0,
    MS_BROADON_DS_DOWNLOAD    = 1,
    MS_BROADON_BROWSER        = 2,
    MS_BROADON_WIIPOINT       = 3,
    MS_BROADON_PHOTO_EXCHANGE = 4,
    MS_BROADON_UGC            = 5
};

//=========================================================
//
// ボタンのタッチ処理
//
//=========================================================

static void SetSoftKeyboardButton(int mode)
{
    int l;
    int count = 0;
    for(l=0; l<CHAR_LIST_MODE_NUM ;l++)
    {
        if(l != mode){
            str_button[2+count]=str_button_char[l];
            next_char_mode[count] = (u16)l;
            count++;
        }
    }
    s_char_mode = mode;
}

// キャンセルボタン専用SelectSomethingFuncの実装
static BOOL SelectCancelFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y,
                         CANCEL_BUTTON_BOTTOM_X, CANCEL_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_CANCEL;
    return ret;
}

// OKボタン専用SelectSomethingFuncの実装
static BOOL SelectOKFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y,
                         OK_BUTTON_BOTTOM_X, OK_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_OK;
    return ret;
}

// 改ページ専用SelectSomethingFuncの実装
static BOOL SelectPrevPageFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( PREVPAGE_BUTTON_TOP_X, PREVPAGE_BUTTON_TOP_Y,
                         PREVPAGE_BUTTON_BOTTOM_X, PREVPAGE_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_PREVPAGE;
    return ret;
}
static BOOL SelectSuccPageFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( SUCCPAGE_BUTTON_TOP_X, SUCCPAGE_BUTTON_TOP_Y,
                         SUCCPAGE_BUTTON_BOTTOM_X, SUCCPAGE_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_SUCCPAGE;
    return ret;
}

// ON/OFFボタン専用SelectSomethingFuncの実装
static BOOL SelectONFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( ON_BUTTON_TOP_X, ON_BUTTON_TOP_Y,
                         ON_BUTTON_BOTTOM_X, ON_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_ON;
    return ret;
}
static BOOL SelectOFFFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( OFF_BUTTON_TOP_X, OFF_BUTTON_TOP_Y,
                         OFF_BUTTON_BOTTOM_X, OFF_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_OFF;
    return ret;
}
/*
// UP/DOWNボタン専用SelectSomethingFuncの実装
static BOOL SelectUPFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( UP_BUTTON_TOP_X, UP_BUTTON_TOP_Y,
                         UP_BUTTON_BOTTOM_X, UP_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_UP;
    return ret;
}
static BOOL SelectDOWNFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( DOWN_BUTTON_TOP_X, DOWN_BUTTON_TOP_Y,
                         DOWN_BUTTON_BOTTOM_X, DOWN_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_DOWN;
    return ret;
}
*/
// UP/DOWNボタンの長押しとトリガを検出する
static BOOL DetectTouchUD( u16 *csr )
{
    BOOL         curr[2]  = {FALSE, FALSE};     // 0:UP/1:DOWN
    static BOOL  prev[2]  = {FALSE, FALSE};     // トリガ検出のために前の状態を記憶させる
    BOOL         trg[2]   = {FALSE, FALSE};
    BOOL         rep[2]   = {FALSE, FALSE};     // 長押し
    static u8    count[2] = {0, 0};             // 何フレーム連続で押されているか
    BOOL         ret = FALSE;
    u16          i;

    for( i=0; i < 2; i++ )
    {
        switch(i)
        {
            case 0:
                curr[i] = WithinRangeTP( UP_BUTTON_TOP_X, UP_BUTTON_TOP_Y,
                                         UP_BUTTON_BOTTOM_X, UP_BUTTON_BOTTOM_Y, &tpd.disp );
            break;
            case 1:
                curr[i] = WithinRangeTP( DOWN_BUTTON_TOP_X, DOWN_BUTTON_TOP_Y,
                                         DOWN_BUTTON_BOTTOM_X, DOWN_BUTTON_BOTTOM_Y, &tpd.disp );
            break;
            default:
            break;
        }

        // はじめて押されたかどうか
        if( !prev[i] && curr[i] )
        {
            trg[i] = TRUE;
        }
        // 長押しカウント
        if( curr[i] )
        {
            if( trg[i] )
            {
                count[i] = 1;
            }
            else if( count[i] > 25 )
            {
                count[i] = 25 - 10;
                rep[i] = TRUE;
            }
            else
            {
                (count[i])++;
            }
        }
        else        // 押されていないとき
        {
            count[i] = 0;
        }
        prev[i] = curr[i];        // 状態を記憶
    }

    if(trg[0] || rep[0])
    {
        *csr = KEY_UP;
        ret  = TRUE;
    }
    else if(trg[1] || rep[1])
    {
        *csr = KEY_DOWN;
        ret  = TRUE;
    }
    else
    {
        ret  = FALSE;
    }
    return ret;
}
/*
// UP/DOWN複数個ボタン専用SelectSomethingFuncの実装
static BOOL SelectMultiUPDOWNFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    int  topX, topY, bottomX, bottomY;
    int  i;
    for( i=0; i < 4; i++ )      // 左から順に
    {
        // UPボタン
        topX    = MULTI_UP_BUTTON_OFFSET_X + (MULTI_UP_BUTTON_WIDTH_X * i) + (MULTI_UP_BUTTON_INTERVAL_X * i);
        bottomX = topX + MULTI_UP_BUTTON_WIDTH_X;
        topY    = MULTI_UP_BUTTON_OFFSET_Y;
        bottomY = topY + MULTI_UP_BUTTON_WIDTH_Y;
        ret = WithinRangeTP( topX, topY, bottomX, bottomY, tgt );
        if(ret)
        {
            *csr = (u16)(MULTI_KEY_UP | i);        // ボタンの種類 + 番号
            break;
        }

        // DOWNボタン
        topX    = MULTI_DOWN_BUTTON_OFFSET_X + (MULTI_DOWN_BUTTON_WIDTH_X * i) + (MULTI_DOWN_BUTTON_INTERVAL_X * i);
        bottomX = topX + MULTI_DOWN_BUTTON_WIDTH_X;
        topY    = MULTI_DOWN_BUTTON_OFFSET_Y;
        bottomY = topY + MULTI_DOWN_BUTTON_WIDTH_Y;
        ret = WithinRangeTP( topX, topY, bottomX, bottomY, tgt );
        if(ret)
        {
            *csr = (u16)(MULTI_KEY_DOWN | i);
            break;
        }
    }
    return ret;
}
*/
// UP/DOWN複数個ボタンの長押しとトリガを検出する
static BOOL DetectTouchMultiUD( u16 *csr )
{
    BOOL         curr[8]  = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};     // 0--3:UP/4--8:DOWN
    static BOOL  prev[8]  = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
    BOOL         trg[8]   = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
    BOOL         rep[8]   = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
    static u8    count[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    BOOL         ret = FALSE;
    int          topX, topY, bottomX, bottomY;
    int          i;

    for( i=0; i < 8; i++ )
    {
        if( i < 4 )
        {
            // UPボタン
            topX     = MULTI_UP_BUTTON_OFFSET_X + (MULTI_UP_BUTTON_WIDTH_X * i) + (MULTI_UP_BUTTON_INTERVAL_X * i);
            bottomX  = topX + MULTI_UP_BUTTON_WIDTH_X;
            topY     = MULTI_UP_BUTTON_OFFSET_Y;
            bottomY  = topY + MULTI_UP_BUTTON_WIDTH_Y;
            curr[i] = WithinRangeTP( topX, topY, bottomX, bottomY, &tpd.disp );
        }
        else
        {
            int  pos = i - 4;

            // DOWNボタン
            topX     = MULTI_DOWN_BUTTON_OFFSET_X + (MULTI_DOWN_BUTTON_WIDTH_X * pos) + (MULTI_DOWN_BUTTON_INTERVAL_X * pos);
            bottomX  = topX + MULTI_DOWN_BUTTON_WIDTH_X;
            topY     = MULTI_DOWN_BUTTON_OFFSET_Y;
            bottomY  = topY + MULTI_DOWN_BUTTON_WIDTH_Y;
            curr[i] = WithinRangeTP( topX, topY, bottomX, bottomY, &tpd.disp );
        }

        // はじめて押されたかどうか
        if( !prev[i] && curr[i] )
        {
            trg[i] = TRUE;
        }
        // 長押しカウント
        if( curr[i] )
        {
            if( trg[i] )
            {
                count[i] = 1;
            }
            else if( count[i] > 25 )
            {
                count[i] = 25 - 10;
                rep[i] = TRUE;
            }
            else
            {
                (count[i])++;
            }
        }
        else        // 押されていないとき
        {
            count[i] = 0;
        }
        prev[i] = curr[i];        // 状態を記憶
    }

    ret = FALSE;
    for( i=0; i < 8; i++ )
    {
        if( (i<4) && (trg[i] || rep[i]) )
        {
            *csr = (u16)(MULTI_KEY_UP | i);
            ret  = TRUE;
            break;
        }
        else if( (i>=4) && (trg[i] || rep[i]) )
        {
            *csr = (u16)(MULTI_KEY_DOWN | (i-4));
            ret  = TRUE;
            break;
        }
    }
    return ret;
}

// RESTRICT/NOT RESTRICTボタン専用SelectSomethingFuncの実装
static BOOL SelectRESTRICTFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( RESTRICT_BUTTON_TOP_X, RESTRICT_BUTTON_TOP_Y,
                         RESTRICT_BUTTON_BOTTOM_X, RESTRICT_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_RESTRICT;
    return ret;
}
static BOOL SelectNOTRESTRICTFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( NOTRESTRICT_BUTTON_TOP_X, NOTRESTRICT_BUTTON_TOP_Y,
                         NOTRESTRICT_BUTTON_BOTTOM_X, NOTRESTRICT_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_NOTRESTRICT;
    return ret;
}

// パッドのキーの長押しを検出(ReadPad()を呼び出しているループ内で呼ばれる必要がある)
static u16 DetectPadRepeat( void )
{
    static u8 repcount[12];       // 各キーが長押しされているフレーム数
    u16       rep = 0;
    int       i;

    for( i=0; i < 12; i++ )     // 全部のキーについて
    {
        if( pad.trg & ((u16)(0x0001 << i)) )     // 押されたらカウントし始める
        {
            repcount[i] = 1;
        }
        else if( pad.cont & ((u16)(0x0001 << i)) )
        {
            if( repcount[i] > 25 )  // ある一定以上のフレーム数押されていたら長押しされていたと判定
            {
                rep = (u16)(rep | (u16)(0x0001 << i));
                repcount[i] = 25 - 10;
            }
            else
            {
                repcount[i]++;
            }
        }
        else
        {
            repcount[i] = 0;
        }
    }
    return rep;
}

//=========================================================
//
// メインメニュー
//
//=========================================================

// 描画
static void DrawParentalControlMenuScene( void )
{
    u16   utf16Button[2];
    int   x,y,i;
    u16   buf[2] = {0,0};

    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"PARENTAL CONTROL" );
    PrintfSJIS( 25*8, 0, TXT_COLOR_BLUE, "%d／3", sCurrentPage+1 );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"RETURN" );
    if( sCurrentPage > 0 )
    {
        utf16Button[0] = 0xE004;
        utf16Button[1] = 0;
        PutStringUTF16( PREVPAGE_BUTTON_TOP_X,     PREVPAGE_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16*)utf16Button );
        PutStringUTF16( PREVPAGE_BUTTON_TOP_X+2*8, PREVPAGE_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16*)L"Prev Page" );
    }
    if( sCurrentPage < (MS_PARENTAL_NUMOF_PAGES-1) )
    {
        utf16Button[0] = 0xE005;
        utf16Button[1] = 0;
        PutStringUTF16( SUCCPAGE_BUTTON_TOP_X,     SUCCPAGE_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16*)L"Next Page" );
        PutStringUTF16( SUCCPAGE_BUTTON_TOP_X+9*8, SUCCPAGE_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16*)utf16Button );
    }
    // メニュー項目
    DrawMenu( sCursorMenu, &s_settingParam );
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得して表示
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    switch( sCurrentPage )      // ページ別に表示項目が変わる
    {
        case 0:
            // ペアレンタルコントロール制限のON/OFF
            PutStringUTF16( 23*8, s_settingPos[0].y, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsSetParentalControl() ? L"ON" : L"OFF" );
            // レーティング年齢
            PrintfSJIS( 23*8, s_settingPos[1].y, TXT_UCOLOR_G0, "%d", LCFG_TSD_GetPCTLRatingAge() );
            // レーティング団体
            PutStringUTF16( 6*8, 2*8+s_settingPos[2].y, TXT_UCOLOR_G0, 
                            sppRatingOgnCharTable[ LCFG_TSD_GetPCTLRatingOgn() ] );
        break;
        
        case 1:
            // 暗証番号
            for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
            {
                const char *pPassword = LCFG_TSD_GetPCTLPasswordPtr();
                if( ('0' <= pPassword[i]) && (pPassword[i] <= '9') )
                {
                    PrintfSJIS( 23*8+i*8, s_settingPos[3].y, TXT_UCOLOR_G0, "%c", pPassword[i] );
                }
                else
                {
                    PrintfSJIS( 23*8+i*8, s_settingPos[3].y, TXT_UCOLOR_G0, "*" );
                }
            }
            //PrintfSJIS( 23*8, s_settingPos[3].y, TXT_UCOLOR_G0, "%s", LCFG_TSD_GetPCTLPasswordPtr() );
            // 秘密の質問ID
            PrintfSJIS( 23*8, s_settingPos[4].y, TXT_UCOLOR_G0, "%d", LCFG_TSD_GetPCTLSecretQuestionID() );
            // 秘密の質問への回答
            for( y=0; y < (MS_PARENTAL_ANS_NUMOF_ROWS+1); y++ )             // 1行ごとに表示
            {
                const u16 *pAnswer = LCFG_TSD_GetPCTLSecretAnswerPtr();
                for( x=0; x < MS_PARENTAL_ANS_COL; x++ )                    // 1文字ずつ表示(半角/全角混ざっても等間隔で表示したいため)
                {
                    buf[0] = pAnswer[MS_PARENTAL_ANS_COL*y + x];
                    buf[1] = 0;
                    PutStringUTF16( 4*8 + 12*x, s_settingPos[5].y + 2*8*y + 2*8, TXT_UCOLOR_G0, buf );
                }
            }
        break;
        
        case 2:
#ifdef BROADON_UI
            PutStringUTF16( 17*8, 6*8, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsRestrictWiiPoint() ? L"Restricted" : L"Not Restricted" );
            PutStringUTF16( 17*8, 8*8, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsRestrictBrowserBoot() ? L"Restricted" : L"Not Restricted" );
            PutStringUTF16( 17*8, 10*8, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsRestrictPictoChatBoot() ? L"Restricted" : L"Not Restricted" );
            PutStringUTF16( 17*8, 12*8, TXT_UCOLOR_G0, 
                            LCFG_TSD_IsRestrictNintendoSpot() ? L"Restricted" : L"Not Restricted" );
#endif // BROADON_UI
        break;
    }
}

// 表示項目をページごとにセット
static void SetDrawnPageElement( void )
{
    int  index;
    int  page;
    int  i;
    
    // 先頭の項目のindexを調べる
    index = 0;
    for( page=0; page < sCurrentPage; page++ )
    {
        index = index + sNumOfPageElements[page];
    }

    //DEBUGPRINT( "current page is %d\n", sCurrentPage );
    //DEBUGPRINT( "index is %d\n", index );
    
    // NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
    s_settingParam.num = sNumOfPageElements[ sCurrentPage ];
    s_settingParam.pos = &(s_settingPos[ index ]);
    for( i=0; i < sNumOfPageElements[ sCurrentPage ]; i++,index++ )
    {
        s_pStrSetting[ i ] = s_pStrSettingElemTbl[ index ][ LCFG_TSD_GetLanguage() ];
    }
}

// 初期化
void SetParentalControlInit( void )
{
	// ページの初期化
    if( sbInitPage )            // 特別にFALSEが指定されたときだけページをそのままにする
    {
        sCurrentPage = 0;
    }
    sbInitPage = TRUE;
    SetDrawnPageElement();

    // BGデータのロード処理
    GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
    GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));

    DrawParentalControlMenuScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// メニューから呼ばれるメイン
int SetParentalControlMain( void )
{
    SelectSomethingFunc func[3]={SelectCancelFunc, SelectPrevPageFunc, SelectSuccPageFunc};
    BOOL        tp_select;
    BOOL        tpCommit = FALSE;
    u16         padrep;
    static u16  commit;

    ReadTP();

    padrep = DetectPadRepeat();     // 長押し検出

    // ページが変わってカーソルがはみ出していたら修正
    if( sCursorMenu >= (u16)sNumOfPageElements[ sCurrentPage ] )
    {
        sCursorMenu = (u16)(sNumOfPageElements[ sCurrentPage ] - 1);
    }

    // メニューからの項目選択
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) ){                               // カーソルの移動
        if( ++sCursorMenu >= (u16)sNumOfPageElements[ sCurrentPage ] ) {
            sCursorMenu = 0;
        }
    }
    if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) ){
        if( --sCursorMenu & 0x80 ) {
            sCursorMenu = (u16)(sNumOfPageElements[ sCurrentPage ] - 1);
        }
    }
    tp_select = SelectMenuByTP( &sCursorMenu, &s_settingParam );

    // 特殊ボタンタッチ
    tpCommit = SelectSomethingByTP( &commit, func, 3 );

    // メニューへの分岐
    if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {         // メニュー項目への分岐
        if( (s_settingParam.pos[sCursorMenu]).enable ) {
            switch( sCurrentPage )
            {
                case 0:
                    switch( sCursorMenu )
                    {
                        case 0:
                            SetRatingLockInit();
                            g_pNowProcess = SetRatingLockMain;
                        break;
                        case 1:
                            SetRatingAgeInit();
                            g_pNowProcess = SetRatingAgeMain;
                        break;
                        case 2:
                            SetRatingOgnInit();
                            g_pNowProcess = SetRatingOgnMain;
                        break;
                    }
                break;

                case 1:
                    switch( sCursorMenu )
                    {
                        case 0:
                            SetPasswordInit();
                            g_pNowProcess = SetPasswordMain;
                        break;
                        case 1:
                            SetSecretQuestionIDInit();
                            g_pNowProcess = SetSecretQuestionIDMain;
                        break;
                        case 2:
                            SetAnswerInit();
                            g_pNowProcess = SetAnswerMain;
                        break;
                        case 3:
                        break;
                    }
                break;

                case 2:
                    switch( sCursorMenu )
                    {
#ifdef BROADON_UI
                        case 0:
                            sBroadOnMenu = MS_BROADON_WIIPOINT;   // すべてON/OFF設定なので設定関数を共通化してフラグで設定項目切り替え
                            SetBroadOnSettingInit();
                            g_pNowProcess = SetBroadOnSettingMain;
                        break;
                        case 1:
                            sBroadOnMenu = MS_BROADON_BROWSER;
                            SetBroadOnSettingInit();
                            g_pNowProcess = SetBroadOnSettingMain;
                        break;
                        case 2:
                            sBroadOnMenu = MS_BROADON_PICTOCHAT;
                            SetBroadOnSettingInit();
                            g_pNowProcess = SetBroadOnSettingMain;
                        break;
                        case 3:
                            sBroadOnMenu = MS_BROADON_NINTENDOSPOT;
                            SetBroadOnSettingInit();
                            g_pNowProcess = SetBroadOnSettingMain;
                        break;
#else   // BROADON_UI
                        case 0:
                        break;
#endif  // BROADON_UI
                    }
                break;

            } // switch( sCurrentPage )
        } // if( (s_settingParam.pos[sCursorMenu]).enable )
    } // if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) )
    else if( (pad.trg & PAD_BUTTON_B) || (tpCommit && (commit == KEY_CANCEL)) )
    {
        MachineSettingInit();
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_L) || (tpCommit && (commit == KEY_PREVPAGE)) )
    {
        if( sCurrentPage > 0 )
        {
            sCurrentPage--;
        }
    }
    else if( (pad.trg & PAD_BUTTON_R) || (tpCommit && (commit == KEY_SUCCPAGE)) )
    {
        if( sCurrentPage < (MS_PARENTAL_NUMOF_PAGES -1) )
        {
            sCurrentPage++;
        }
    }

    // 再描画
    SetDrawnPageElement();
    DrawParentalControlMenuScene();
    return 0;
}

//=========================================================
//
// ソフトウェアキーボード
//
//=========================================================

// キーの表示
static void DrawCharKeys( void )
{
	int l;
	u16 code;

	for( l=0; l<CHAR_LIST_CHAR_NUM; l++ )
	{
		int color=TXT_COLOR_BLACK;
		code = char_tbl[s_char_mode][l];
		if (s_key_csr == l) color = TXT_COLOR_GREEN;
		if(code != EOM_)
		{
			if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
			{
				int x = code - CODE_BUTTON_TOP_;
                if( !sbEnableOKButton && (s_key_csr == l) && (code == OK_BUTTON_) )
                {
                    color = TXT_COLOR_RED;              // OKさせないときは色を変える
                }
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, str_button[x] );
			}
			else
			{
				u16 s[2];
				s[0] = code;
				s[1] = 0;
				PutStringUTF16( CLIST_LT_X + CLIST_MARGIN*(l%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((l%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT) ,
				CLIST_LT_Y + CLIST_MARGIN*(l/KEY_PER_LINE) , color, s );
			}
		}
	}
}

// 一文字削除
static void DeleteACharacter( eUseSoftKey noc )
{
    u16 *buf;
    u8 *length;
    if(noc == USE_SOFTKEY_COMMON)
    {
    }
    else if(noc == USE_SOFTKEY_ANSWER)
    {
        buf = spBufAnswer;
        length = &sCurrentLenAnswer;
    }
    else
    {
        //unknown
        return;
    }

    if(*length > 0) buf[--(*length)] = CHAR_USCORE;
}


// 選択中文字キー・特殊キーで決定した時の挙動
static void PushKeys( u16 code, eUseSoftKey noc )
{
    u16 *buf;
    u8 *length;
    u16 min_length;
    u16 max_length;
    u16 bufsize;
    if(noc == USE_SOFTKEY_COMMON)
    {
        // unused
        return;
    }
    else if(noc == USE_SOFTKEY_ANSWER)
    {
        buf = spBufAnswer;
        length = &sCurrentLenAnswer;
        // 最大長は言語によって異なる
        min_length = sMinLenAnswer;
        max_length = sMaxLenAnswer;
        bufsize    = sBufSizeAnswer;
    }
    else
    {
        //unknown
        return;
    }

    if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
    {
    	// 特殊キー
    	switch(code)
    	{
    		case VAR_BUTTON1_:
    		case VAR_BUTTON2_:
    			SetSoftKeyboardButton(next_char_mode[code - VAR_BUTTON1_]);
    			break;
    		case DEL_BUTTON_:
    			DeleteACharacter(noc);
    			break;
    		case SPACE_BUTTON_:
    			if(*length < max_length) buf[(*length)++] = L'　';
    			break;
    		case OK_BUTTON_:
                if( *length >= min_length )              // 最小文字数に満たないときはOKできない
                {
                    //DEBUGPRINT( "OK: length = %d, min_length = %d, max_length = %d\n", *length, min_length, max_length );

                    // 上限とバッファサイズが異なるときに中間のデータを0クリアしておいたほうが安全
                    SVC_CpuClear(0, buf + *length, (bufsize - *length) * 2, 16 );     // 文字数の上限までではなくバッファの最後まで0クリア

                    // デバッグ用
                    //for( i=0; i < bufsize; i++ )
                    //{
                    //    DEBUGPRINT( "%04x", buf[i] );
                    //    if( ((i+1)%16) == 0 ) DEBUGPRINT( ",\n" );
                    //    else if( (i+1) == bufsize ) DEBUGPRINT( "\n" );
                    //    else DEBUGPRINT( "," );
                    //}

                    if(noc == USE_SOFTKEY_COMMON)
                    {
                    }
                    else if(noc == USE_SOFTKEY_ANSWER)
                    {
                        LCFG_TSD_SetPCTLSecretAnswer( buf );
                    }
                    // ::::::::::::::::::::::::::::::::::::::::::::::
                    // TWL設定データファイルへの書き込み
                    // ::::::::::::::::::::::::::::::::::::::::::::::
                    if( !MY_WriteTWLSettings() )
                    {
                        OS_TPrintf( "TWL settings write failed.\n" );
                    }
                    sbInitPage = FALSE;
                    SetParentalControlInit();
                    g_pNowProcess = SetParentalControlMain;
                }
                break;
                case CANCEL_BUTTON_:
                    sbInitPage = FALSE;                         // ページをそのままにしておく
                    SetParentalControlInit();
                    g_pNowProcess = SetParentalControlMain;
                break;
			default:// unknown code
				break;
		}
	}
	else
	{
		// 普通キー
		if(*length < max_length) buf[(*length)++] = code;
	}
}

// PadDetectOnKeyのSelectSomethingByTPで使うSelectSomethingFuncの実装
static BOOL SelectSoftwareKeyFunc( u16 *csr, TPData *tgt )
{
	// まずは候補となる座標（カーソル単位）を取得
	int csrx;
	int csry;
	int csrxy;
	int a;
	int b;
	NNSG2dTextRect rect;
	u16 code;
	BOOL ret;
	
	csrx = tgt->x - CLIST_LT_X;
	csrx = csrx - (CLIST_SEGMENT_INTERVAL*(csrx/(CLIST_MARGIN*CLIST_KEY_PER_SEGMENT+CLIST_SEGMENT_INTERVAL)));
	csrx = csrx / CLIST_MARGIN;
	csry = (tgt->y - CLIST_LT_Y) / CLIST_MARGIN;
	if(csrx < 0 ) return FALSE;

	if ( csrx >= KEY_PER_LINE ) csrx = KEY_PER_LINE - 1;
	csrxy = csrx + csry * KEY_PER_LINE;

	if ( csrxy < 0 || csrxy >= CHAR_LIST_CHAR_NUM) return FALSE;// 明らかにはみ出した

	// 候補座標のキーコード取得
	code = char_tbl[s_char_mode][csrxy];
	if(code == EOM_) return FALSE;
	
	// 候補座標の領域取得
	if( (code >= CODE_BUTTON_TOP_) && (code < CODE_BUTTON_BOTTOM_) )
	{
		int x = code - CODE_BUTTON_TOP_;
		rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, str_button[x] );
	}
	else
	{
		u16 s[2];
		s[0] = code;
		s[1] = 0;
		// rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, s );
		// 文字幅じゃかなり判定が厳しい……ギリギリまでとってみる
		rect.width = CLIST_MARGIN;
		rect.height = CLIST_MARGIN;
	}
	a = CLIST_LT_X + CLIST_MARGIN*(csrxy%KEY_PER_LINE) + CLIST_SEGMENT_INTERVAL*((csrxy%KEY_PER_LINE)/CLIST_KEY_PER_SEGMENT);
	b = CLIST_LT_Y + CLIST_MARGIN*(csrxy/KEY_PER_LINE);
	
	// 候補座標の領域にタッチ座標が含まれているかチェック
	ret = WithinRangeTP( a, b, a+rect.width, b+rect.height, tgt );
	
	if(ret)
	{
		*csr = (u16)csrxy;
	}
	return ret;
}

// ソフトウェアキー上でのキーパッド及びタッチパッド処理
// 先にReadTPしておくこと。
static void PadDetectOnKey( eUseSoftKey noc )
{
	SelectSomethingFunc func[1];
	BOOL tp_select = FALSE;
    u16  padrep;

    // 長押し検出
    padrep = DetectPadRepeat();

	//--------------------------------------
	//  キー入力処理
	//--------------------------------------
	if( (pad.trg & PAD_KEY_RIGHT) || (padrep & PAD_KEY_RIGHT) ){									// カーソルの移動
		do
		{
			if(s_key_csr%KEY_PER_LINE != KEY_PER_LINE-1) s_key_csr++;
			else s_key_csr -= KEY_PER_LINE-1;
			if( s_key_csr == CHAR_LIST_CHAR_NUM ) s_key_csr -= s_key_csr%KEY_PER_LINE;
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	if( (pad.trg & PAD_KEY_LEFT) || (padrep & PAD_KEY_LEFT) ){
		do
		{
			if(s_key_csr%KEY_PER_LINE != 0) s_key_csr--;
			else s_key_csr += KEY_PER_LINE-1;
			if( s_key_csr & 0x8000 ) s_key_csr = KEY_PER_LINE-1;
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) ){									// カーソルの移動
		do
		{
			s_key_csr += KEY_PER_LINE;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= KEY_PER_LINE*(s_key_csr/KEY_PER_LINE);
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) ){
		do
		{
			if( s_key_csr < KEY_PER_LINE ) s_key_csr += (CHAR_LIST_CHAR_NUM/KEY_PER_LINE)*KEY_PER_LINE;
			else s_key_csr -= KEY_PER_LINE;
			if( s_key_csr >= CHAR_LIST_CHAR_NUM ) s_key_csr -= KEY_PER_LINE;
		}
		while(char_tbl[s_char_mode][s_key_csr]==EOM_);
	}
	
	func[0] = (SelectSomethingFunc)SelectSoftwareKeyFunc;
	tp_select = SelectSomethingByTP(&s_key_csr, func, 1 );
	
	if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {				// キーが押された
		PushKeys( char_tbl[s_char_mode][s_key_csr], noc );
	}else if( pad.trg & PAD_BUTTON_B ) {
		DeleteACharacter(noc);
	}
}

/*
// 押している間数字が一定スピードで変化するような処理
static void Birthday_AutoNumScrollByTP( void )
{
	static u16 first_csr = 0xffff;
	u16 temp_csr;
	static int same_count = 0;
	
	if( tpd.disp.touch )
	{
		BOOL t = SelectBirthdayFunc( &temp_csr, &tpd.disp );
		if( t )
		{
			if(same_count == 0) // count start
			{
				first_csr = temp_csr;
				same_count = 1;
			}else if(first_csr == temp_csr)
			{
				if( same_count == 1 || (same_count > 29 && same_count%10==0))
				{
					switch(temp_csr)
					{
						case 0:
							s_birth_csr = TRUE;
							s_temp_birthday.month++;
							break;
						case 1:
							s_birth_csr = FALSE;
							s_temp_birthday.day++;
							break;
						case 2:
							s_birth_csr = TRUE;
							s_temp_birthday.month--;
							break;
						case 3:
							s_birth_csr = FALSE;
							s_temp_birthday.day--;
							break;
						default:
							break;
					}
				}
				same_count++;
			}
		}
	}else // touch==0
	{
		same_count = 0;
		first_csr = 0xffff;
	}
}
*/

//=========================================================
//
// レーティング団体の設定 (リストからの選択)
//
//=========================================================

// 描画処理
static void DrawSetRatingOgnScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"RATING ORGANIZATION" );

    PutStringUTF16(  4*8, 4*8, TXT_UCOLOR_G0, (const u16 *)L"Region: " );
    PutStringUTF16( 12*8, 4*8, TXT_UCOLOR_G0, sppRegionCharTable[ sRegion ] );
    PutStringUTF16(  4*8, 6*8, TXT_UCOLOR_G0, (const u16 *)L"Organization List: " );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"CANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"OK" );

    DrawMenu( sCursorRatingOgn, &sRatingOgnMenuParam );
}

// 初期化
static void SetRatingOgnInit( void )
{
    int               i;
    LCFGTWLRatingOgn  ogn;

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    ogn     = LCFG_TSD_GetPCTLRatingOgn();
    sRegion = LCFG_THW_GetRegion();

    // リージョンにあわせて選択させる団体リストをつくる(国別のほうがいいかも)
    sRatingOgnFirstIndex = (u16)sIndexFirstOgnTable[ sRegion ];
    DEBUGPRINT( "ogn index = %d\n", sRatingOgnFirstIndex );
    sRatingOgnMenuParam.num = spNumOfRatingOgnsTable[ sRegion ];                            // リストのメンバ数
    sRatingOgnMenuParam.pos = &(spRatingOgnPosTable[ sRatingOgnFirstIndex ]);               // 表示位置
    for( i=0; i < spNumOfRatingOgnsTable[ sRegion ]; i++ )
    {
        sppRatingOgnCharList[i] = sppRatingOgnCharTable[ (int)sRatingOgnFirstIndex + i ];   // 表示文字列
    }

    // カーソル位置を決定
    if( (ogn < sRatingOgnFirstIndex) || ((sRatingOgnFirstIndex + spNumOfRatingOgnsTable[ sRegion ] - 1) < ogn) )
    {
        sCursorRatingOgn = 0;       // はみ出ているときは補正
    }
    else
    {
        sCursorRatingOgn = (u16)(ogn - sRatingOgnFirstIndex);
    }

    DrawSetRatingOgnScene();

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetRatingOgnMain( void )
{
    SelectSomethingFunc func[2]={SelectCancelFunc, SelectOKFunc};
    u16  commit;
    BOOL tpSelect = FALSE;
    BOOL tpCommit = FALSE;

    ReadTP();
    
    // 選択
    if( pad.trg & PAD_KEY_DOWN ){                               // カーソルの移動
        if( ++sCursorRatingOgn >= (u16)spNumOfRatingOgnsTable[ sRegion ] ) {
            sCursorRatingOgn = 0;
        }
    }
    if( pad.trg & PAD_KEY_UP ){
        if( --sCursorRatingOgn & 0x8000 ) {
            sCursorRatingOgn = (u16)(spNumOfRatingOgnsTable[ sRegion ] - 1);
        }
    }
    tpSelect = SelectMenuByTP( &sCursorRatingOgn, &sRatingOgnMenuParam );

    // 決定ボタンチェック
    tpCommit = SelectSomethingByTP( &commit, func, 2 );

    // 決定
    if( (pad.trg & PAD_BUTTON_A) || (tpCommit && (commit == KEY_OK)) )
    {
        LCFG_TSD_SetPCTLRatingOgn( (LCFGTWLRatingOgn)(sRatingOgnFirstIndex + sCursorRatingOgn) );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL設定データファイルへの書き込み
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_B) || (tpCommit && (commit == KEY_CANCEL)) )
    {
        SetParentalControlInit();                   // キャンセルのときセットしない
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    DrawSetRatingOgnScene();
    return 0;
}

//=========================================================
//
// レーティング制限のON/OFF (ON/OFFスイッチ切り替え)
//
//=========================================================

// 描画処理
static void DrawSetRatingLockScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"RATING LOCK" );

    if( sbRatingLock )
    {
        PutStringUTF16( ON_BUTTON_TOP_X,  ON_BUTTON_TOP_Y,  TXT_COLOR_GREEN, (const u16*)L"ON" );
        PutStringUTF16( OFF_BUTTON_TOP_X, OFF_BUTTON_TOP_Y, TXT_UCOLOR_G0,   (const u16*)L"OFF" );
    }
    else
    {
        PutStringUTF16( ON_BUTTON_TOP_X,  ON_BUTTON_TOP_Y,  TXT_UCOLOR_G0,   (const u16*)L"ON" );
        PutStringUTF16( OFF_BUTTON_TOP_X, OFF_BUTTON_TOP_Y, TXT_COLOR_GREEN, (const u16*)L"OFF" );
    }
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"CANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"OK" );
}

// 初期化
static void SetRatingLockInit( void )
{
    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sbRatingLock = LCFG_TSD_IsSetParentalControl();

    DrawSetRatingLockScene();

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetRatingLockMain( void )
{
    SelectSomethingFunc func[4]={SelectCancelFunc, SelectOKFunc, SelectONFunc, SelectOFFFunc };
    u16  commit;
    BOOL tp_touch = FALSE;

    ReadTP();
    
    // キーによる選択
    if( (pad.trg & PAD_KEY_LEFT) || (pad.trg & PAD_KEY_RIGHT) )
    {
        sbRatingLock = !sbRatingLock;
    }

    // タッチによる選択
    tp_touch = SelectSomethingByTP( &commit, func, 4 );
    if( tp_touch && (commit == KEY_ON) )
    {
        sbRatingLock = TRUE;
    }
    else if( tp_touch && (commit == KEY_OFF) )
    {
        sbRatingLock = FALSE;
    }

    // 決定
    if( (pad.trg & PAD_BUTTON_A) || (tp_touch && (commit == KEY_OK)) )
    {
        LCFG_TSD_SetFlagParentalControl( sbRatingLock );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL設定データファイルへの書き込み
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_B) || (tp_touch && (commit == KEY_CANCEL)) )
    {
        SetParentalControlInit();                   // キャンセルのときセットしない
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    DrawSetRatingLockScene();
    return 0;
}

//=========================================================
//
// レーティング年齢の選択 (数値選択)
//
//=========================================================

// 描画処理
static void DrawSetRatingAgeScene( void )
{
    u16 iconUp[2]   = {0xE01B, 0};
    u16 iconDown[2] = {0xE01C, 0};
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"RATING TARGET AGE" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"CANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"OK" );
    PutStringUTF16( UP_BUTTON_TOP_X,     UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconUp );
    PutStringUTF16( UP_BUTTON_TOP_X+2*8, UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"UP" );
    PutStringUTF16( DOWN_BUTTON_TOP_X,     DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconDown );
    PutStringUTF16( DOWN_BUTTON_TOP_X+2*8, DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"DOWN" );
    PutStringUTF16( 8*8, 10*8, TXT_UCOLOR_G0, (const u16 *)L"Age" );
    PrintfSJIS( 12*8, 10*8, TXT_COLOR_GREEN, "%d", sRatingAge );
}

// 初期化
static void SetRatingAgeInit( void )
{
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sRatingAge = LCFG_TSD_GetPCTLRatingAge();

    DrawSetRatingAgeScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetRatingAgeMain( void )
{
    SelectSomethingFunc func[2]={SelectCancelFunc, SelectOKFunc};
    BOOL tpCommit = FALSE;
    BOOL tpUD = FALSE;
    u16  csrCommit;
    u16  csrUD;
    u16  padrep;

    ReadTP();

    // TPチェック
    tpCommit = SelectSomethingByTP( &csrCommit, func, 2 );
    tpUD = DetectTouchUD( &csrUD );

    padrep = DetectPadRepeat();    // キーの長押し検出

    // 変更
    if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) || (tpUD && (csrUD == KEY_UP)) )
    {
        if( (--sRatingAge) > LCFG_TWL_PCTL_RATING_AGE_MAX )
        {
            sRatingAge = 0;
        }
    }
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) || (tpUD && (csrUD == KEY_DOWN)) )
    {
        if( (++sRatingAge) & 0x80 )
        {
            sRatingAge = LCFG_TWL_PCTL_RATING_AGE_MAX;
        }
    }

    // 決定
    if( pad.trg & PAD_BUTTON_A || (tpCommit && (csrCommit == KEY_OK)) )
    {
        LCFG_TSD_SetPCTLRatingAge( sRatingAge );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL設定データファイルへの書き込み
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( ( pad.trg & PAD_BUTTON_B ) || (tpCommit && (csrCommit == KEY_CANCEL)) )
    {
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    // 再描画
    DrawSetRatingAgeScene();
    return 0;
}

//=========================================================
//
// 秘密の質問IDの選択 (数値選択)
//
//=========================================================

// 描画処理
static void DrawSetSecretQuestionIDScene( void )
{
    u16 iconUp[2]   = {0xE01B, 0};
    u16 iconDown[2] = {0xE01C, 0};
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"SECRET QUESTION ID" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"CANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"OK" );
    PutStringUTF16( UP_BUTTON_TOP_X,     UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconUp );
    PutStringUTF16( UP_BUTTON_TOP_X+2*8, UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"UP" );
    PutStringUTF16( DOWN_BUTTON_TOP_X,     DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconDown );
    PutStringUTF16( DOWN_BUTTON_TOP_X+2*8, DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"DOWN" );
    PutStringUTF16( 8*8, 10*8, TXT_UCOLOR_G0, (const u16 *)L"No." );
    PrintfSJIS( 12*8, 10*8, TXT_COLOR_GREEN, "%d", sSecretQuestionID );
}

// 初期化
static void SetSecretQuestionIDInit( void )
{
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sSecretQuestionID = LCFG_TSD_GetPCTLSecretQuestionID();

    DrawSetSecretQuestionIDScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetSecretQuestionIDMain( void )
{
    SelectSomethingFunc func[2]={SelectCancelFunc, SelectOKFunc};
    BOOL tpCommit = FALSE;
    BOOL tpUD     = FALSE;
    u16  csrCommit;
    u16  csrUD;
    u16  rep;

    ReadTP();

    // TPチェック
    tpCommit = SelectSomethingByTP( &csrCommit, func, 2 );
    tpUD     = DetectTouchUD( &csrUD );

    rep = DetectPadRepeat();

    // 変更
    if( (pad.trg & PAD_KEY_UP) || (rep & PAD_KEY_UP) || (tpUD && (csrUD == KEY_UP)) )
    {
        if( (--sSecretQuestionID) > LCFG_TWL_PCTL_SECRET_QUESTION_ID_MAX )
        {
            sSecretQuestionID = 0;
        }
    }
    if( (pad.trg & PAD_KEY_DOWN) || (rep & PAD_KEY_DOWN) || (tpUD && (csrUD == KEY_DOWN)) )
    {
        if( (++sSecretQuestionID) & 0x80 )
        {
            sSecretQuestionID = LCFG_TWL_PCTL_SECRET_QUESTION_ID_MAX;
        }
    }

    // 決定
    if( pad.trg & PAD_BUTTON_A || (tpCommit && (csrCommit == KEY_OK)) )
    {
        LCFG_TSD_SetPCTLSecretQuestionID( sSecretQuestionID );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL設定データファイルへの書き込み
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( ( pad.trg & PAD_BUTTON_B ) || (tpCommit && (csrCommit == KEY_CANCEL)) )
    {
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    // 再描画
    DrawSetSecretQuestionIDScene();
    return 0;
}

//=========================================================
//
// 秘密の質問への回答 (ソフトウェアキーボード入力)
//
//=========================================================

// 描画処理
static void DrawSetAnswerScene( void )
{
    //u16 tempbuf[MS_PARENTAL_ANS_COL*2+2];      // 改行分 + NULL文字分
    int row;
    int color;
    int i;
    u16 buf[2] = {0,0};     // 1文字ずつ表示する

    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"ANSWER FOR A SECRET QUESTION" );

    // 文字数が最小文字数より短いときOKさせない
    if( sCurrentLenAnswer < sMinLenAnswer )
    {
        color            = TXT_UCOLOR_RED;  // 表示色も変える
        sbEnableOKButton = FALSE;
    }
    else
    {
        color            = TXT_UCOLOR_G0;
        sbEnableOKButton = TRUE;
    }
    // 何行目から表示するか決定
    if( sCurrentLenAnswer <= (MS_PARENTAL_ANS_COL*2) )
    {
        row = 0;
    }
    else
    {
        row = 2;
    }
    for( i=0; i < MS_PARENTAL_ANS_COL; i++ )                    // 1文字ずつ表示(等間隔で表示したいため)
    {
        buf[0] = spBufAnswer[MS_PARENTAL_ANS_COL*row + i];
        buf[1] = 0;
        PutStringUTF16( 4*8 + 12*i, 15, color, buf );           // 1行目
        buf[0] = spBufAnswer[MS_PARENTAL_ANS_COL*(row+1) + i];
        PutStringUTF16( 4*8 + 12*i, 31, color, buf );           // 2行目
    }

    DrawCharKeys();
}

// 初期化
static void SetAnswerInit( void )
{
    //int i=0;

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // コメント用テンポラリバッファの初期化
    LCFG_TSD_GetPCTLSecretAnswer( spBufAnswer );
    sCurrentLenAnswer = LCFG_TSD_GetPCTLSecretAnswerLength();

    // 日本版だけ文字制限が変わる
    sRegion = LCFG_THW_GetRegion();                             // 日本版だけ入力の仕様が変わる
    if( sRegion == OS_TWL_REGION_JAPAN )
    {
        SetSoftKeyboardButton(0);       // 日本語が設定されているときだけデフォルトのキーボードを変える
        sMinLenAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MIN >> 1;
        sMaxLenAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX >> 1;
    }
    else
    {
        SetSoftKeyboardButton(2);
        sMinLenAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MIN;     // 6
        sMaxLenAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX;     // 64
    }
    s_key_csr = KEY_START;
    sBufSizeAnswer = LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1;    // 64 + 1 = 65

    // 文字列の例外処理
    if( sCurrentLenAnswer > sMaxLenAnswer )        // 上限が32文字なのにそれ以上のとき/上限が64文字でも65文字目が0でないとき
    {
        sCurrentLenAnswer = sMaxLenAnswer;         // 文字列長を切り詰めて上限以降を取り扱えないようにする
        SVC_CpuClear( 0, &spBufAnswer[ sMaxLenAnswer ], (sBufSizeAnswer - sMaxLenAnswer) * 2, 16 );     // 上限以降をすべて0クリア
    }

    //DEBUGPRINT( "-- for debug\n" );
    //for( i=0; i < sBufSizeAnswer; i++ )
    //{
    //    DEBUGPRINT( "%04x", spBufAnswer[i] );
    //    if( ((i+1)%16) == 0 ) DEBUGPRINT( ",\n" );
    //    else if( (i+1) == sBufSizeAnswer ) DEBUGPRINT( "\n" );
    //    else DEBUGPRINT( "," );
    //}
    //DEBUGPRINT( "currentlen = %d, maxlen = %d, clear size = %d\n", sCurrentLenAnswer, sMaxLenAnswer, (sBufSizeAnswer - sMaxLenAnswer) );

    // 表示文字列の生成
    if( sCurrentLenAnswer < LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX )
    {
        SVC_CpuClear( CHAR_USCORE, &spBufAnswer[ sCurrentLenAnswer ],                           // "_"をつめる
                      (LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX - sCurrentLenAnswer) * 2, 16);
    }
    spBufAnswer[ LCFG_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX ] = 0;  // 終端処理

    DrawSetAnswerScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetAnswerMain( void )
{
    ReadTP();
    PadDetectOnKey(USE_SOFTKEY_ANSWER);

    DrawSetAnswerScene();

    return 0;
}


// UTF16の文字列長算出
static u8 MY_StrLen( const u16 *pStr )
{
	u8 len = 0;
	while( *pStr++ ) {
		++len;
		if( len == 255 ) {
			break;
		}
	}
	return len;
}

//=========================================================
//
// パスワード (複数数値選択)
//
//=========================================================

// 描画処理
static void DrawSetPasswordScene( void )
{
    u16  iconUp[2]   = {0xE01B, 0};
    u16  iconDown[2] = {0xE01C, 0};
    int  posUpX, posUpY, posDownX, posDownY;
    int  i;

    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );

    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"PASSWORD" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"CANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, (sbValidPassword)?TXT_UCOLOR_G0:TXT_UCOLOR_RED, (const u16 *)L"OK" );   // 不正なときOKさせない

    for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
    {
        posUpX   = MULTI_UP_BUTTON_OFFSET_X + (MULTI_UP_BUTTON_WIDTH_X*i) + (MULTI_UP_BUTTON_INTERVAL_X*i);
        posUpY   = MULTI_UP_BUTTON_OFFSET_Y;
        posDownX = MULTI_DOWN_BUTTON_OFFSET_X + (MULTI_DOWN_BUTTON_WIDTH_X*i) + (MULTI_DOWN_BUTTON_INTERVAL_X*i);
        posDownY = MULTI_DOWN_BUTTON_OFFSET_Y;
        PutStringUTF16( posUpX,   posUpY,   TXT_UCOLOR_G0, (const u16 *)iconUp );
        PutStringUTF16( posDownX, posDownY, TXT_UCOLOR_G0, (const u16 *)iconDown );
        if( (spBufPassword[i] < 0) || (9 < spBufPassword[i]) )
        {
            PrintfSJIS( posUpX, posUpY+4*8,(sCursorPassword == i)?TXT_COLOR_GREEN:TXT_COLOR_BLACK, "*" );   // 例外処理
        }
        else
        {
            PrintfSJIS( posUpX, posUpY+4*8,(sCursorPassword == i)?TXT_COLOR_GREEN:TXT_COLOR_BLACK, "%d", spBufPassword[i] );
        }
    }
}

// 初期化
static void SetPasswordInit( void )
{
    int i;

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    LCFG_TSD_GetPCTLPassword( spBufPassword );
    sbValidPassword = TRUE;
    for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
    {
        if( ('0' <= spBufPassword[i]) && (spBufPassword[i] <= '9' ) )
        {
            spBufPassword[i] = (u8)(spBufPassword[i] - '0');  // 計算しやすいように文字コードから数値に直しておく
        }
        else
        {
            spBufPassword[i] = 0xFF;                          // 不正
            sbValidPassword  = FALSE;
        }
    }
    sCursorPassword = 0;                            // カーソルは設定のたびに先頭になる

    DrawSetPasswordScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetPasswordMain( void )
{
    SelectSomethingFunc func[2]={SelectCancelFunc, SelectOKFunc};
    BOOL tpCommit = FALSE;
    BOOL tpUD     = FALSE;
    u16  csrCommit;
    u16  csrUD;
    int  i;
    u16  padrep;

    ReadTP();

    padrep = DetectPadRepeat();

    // 左右キーで桁を移動
    if( pad.trg & PAD_KEY_RIGHT )
    {
        if( ++sCursorPassword > (LCFG_TWL_PCTL_PASSWORD_LENGTH - 1) )        // 現在選択されている桁の値を変更
        {
            sCursorPassword = 0;
        }
    }
    if( pad.trg & PAD_KEY_LEFT )
    {
        if( --sCursorPassword & 0x80 )
        {
            sCursorPassword = (LCFG_TWL_PCTL_PASSWORD_LENGTH - 1);
        }
    }

    // 十字キーで各桁の値を変更
    if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) )
    {
        if( --(spBufPassword[sCursorPassword]) > 9 )        // 現在選択されている桁の値を変更
        {
            spBufPassword[sCursorPassword] = 0;
        }
    }
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) )
    {
        if( ++(spBufPassword[sCursorPassword]) & 0x80 )
        {
            (spBufPassword[sCursorPassword]) = 9;
        }
    }

    // TPチェック
    tpCommit = SelectSomethingByTP( &csrCommit, func, 2 );
    tpUD     = DetectTouchMultiUD( &csrUD );

    // タッチパネルで各桁の値を変更
    if( tpUD && ((csrUD & MASK_MULTI_KEY) == MULTI_KEY_UP) )
    {
        sCursorPassword = (u16)(csrUD & ~MASK_MULTI_KEY);        // 桁を特定
        if( --(spBufPassword[sCursorPassword]) > 9 )
        {
            spBufPassword[sCursorPassword] = 0;
        }
    }
    if( tpUD && ((csrUD & MASK_MULTI_KEY) == MULTI_KEY_DOWN) )
    {
        sCursorPassword = (u16)(csrUD & ~MASK_MULTI_KEY);
        if( ++(spBufPassword[sCursorPassword]) & 0x80 )
        {
            (spBufPassword[sCursorPassword]) = 9;
        }
    }

    // すべての値が正当かどうかチェック
    sbValidPassword = TRUE;
    for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
    {
        if( !((0 <= spBufPassword[i])&&(spBufPassword[i] <= 9)) )
        {
            sbValidPassword = FALSE;
            break;
        }
    }

    // 決定
    if( ((pad.trg & PAD_BUTTON_A) || (tpCommit && (csrCommit == KEY_OK))) && sbValidPassword )     // 正当じゃないときはコミット不可
    {
        for( i=0; i < LCFG_TWL_PCTL_PASSWORD_LENGTH; i++ )
        {
            spBufPassword[i] = (u8)(spBufPassword[i] + '0');          // 文字コードに直しておく
        }
        LCFG_TSD_SetPCTLPassword( spBufPassword );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL設定データファイルへの書き込み
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( ( pad.trg & PAD_BUTTON_B ) || (tpCommit && (csrCommit == KEY_CANCEL)) )
    {
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    // 再描画
    DrawSetPasswordScene();
    return 0;
}

//=========================================================
//
// BroadOn用設定 (複数のON/OFFスイッチ切り替え)
//
//=========================================================

// 描画処理
static void DrawSetBroadOnSettingScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    switch( sBroadOnMenu )          // 複数の設定画面をフラグで切り替える
    {
        case MS_BROADON_PICTOCHAT:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"PICTOCHAT BOOT" );
        break;
        case MS_BROADON_DS_DOWNLOAD:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"DS DOWNLOAD BOOT" );
        break;
        case MS_BROADON_BROWSER:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"BROWSER BOOT" );
        break;
        case MS_BROADON_WIIPOINT:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"WII POINT USING" );
        break;
        case MS_BROADON_PHOTO_EXCHANGE:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"PHOTO EXCHANGE USING" );
        break;
        case MS_BROADON_UGC:
            PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"UGC USING" );
        break;
    }

    if( sbBroadOnSetting )
    {
        PutStringUTF16( NOTRESTRICT_BUTTON_TOP_X, NOTRESTRICT_BUTTON_TOP_Y, TXT_UCOLOR_G0,   (const u16*)L"Not Restrict" );
        PutStringUTF16( RESTRICT_BUTTON_TOP_X,   RESTRICT_BUTTON_TOP_Y,     TXT_COLOR_GREEN, (const u16*)L"Restrict" );
    }
    else
    {
        PutStringUTF16( NOTRESTRICT_BUTTON_TOP_X, NOTRESTRICT_BUTTON_TOP_Y, TXT_COLOR_GREEN, (const u16*)L"Not Restrict" );
        PutStringUTF16( RESTRICT_BUTTON_TOP_X,   RESTRICT_BUTTON_TOP_Y,     TXT_UCOLOR_G0,   (const u16*)L"Restrict" );
    }
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"CANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"OK" );
}

// 初期化
static void SetBroadOnSettingInit( void )
{
    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    switch( sBroadOnMenu )
    {
        case MS_BROADON_PICTOCHAT:
            sbBroadOnSetting = LCFG_TSD_IsRestrictPictoChatBoot();
        break;
        case MS_BROADON_DS_DOWNLOAD:
            sbBroadOnSetting = LCFG_TSD_IsRestrictDSDownloadBoot();
        break;
        case MS_BROADON_BROWSER:
            sbBroadOnSetting = LCFG_TSD_IsRestrictBrowserBoot();
        break;
        case MS_BROADON_WIIPOINT:
            sbBroadOnSetting = LCFG_TSD_IsRestrictWiiPoint();
        break;
        case MS_BROADON_PHOTO_EXCHANGE:
            sbBroadOnSetting = LCFG_TSD_IsRestrictPhotoExchange();
        break;
        case MS_BROADON_UGC:
            sbBroadOnSetting = LCFG_TSD_IsRestrictUGC();
        break;
    }

    DrawSetBroadOnSettingScene();

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetBroadOnSettingMain( void )
{
    SelectSomethingFunc func[4]={SelectCancelFunc, SelectOKFunc, SelectRESTRICTFunc, SelectNOTRESTRICTFunc };
    u16  commit;
    BOOL tp_touch = FALSE;

    ReadTP();
    
    // キーによる選択
    if( (pad.trg & PAD_KEY_DOWN) || (pad.trg & PAD_KEY_UP) )
    {
        sbBroadOnSetting = !sbBroadOnSetting;
    }

    // タッチによる選択
    tp_touch = SelectSomethingByTP( &commit, func, 4 );
    if( tp_touch && (commit == KEY_RESTRICT) )
    {
        sbBroadOnSetting = TRUE;
    }
    else if( tp_touch && (commit == KEY_NOTRESTRICT) )
    {
        sbBroadOnSetting = FALSE;
    }

    // 決定
    if( (pad.trg & PAD_BUTTON_A) || (tp_touch && (commit == KEY_OK)) )
    {
        switch( sBroadOnMenu )
        {
            case MS_BROADON_PICTOCHAT:
                LCFG_TSD_SetRestrictPictoChatBoot( sbBroadOnSetting );
            break;
            case MS_BROADON_DS_DOWNLOAD:
                LCFG_TSD_SetRestrictDSDownloadBoot( sbBroadOnSetting );
            break;
            case MS_BROADON_BROWSER:
                LCFG_TSD_SetRestrictBrowserBoot( sbBroadOnSetting );
            break;
            case MS_BROADON_WIIPOINT:
                LCFG_TSD_SetRestrictWiiPoint( sbBroadOnSetting );
            break;
            case MS_BROADON_PHOTO_EXCHANGE:
                LCFG_TSD_SetRestrictPhotoExchange( sbBroadOnSetting );
            break;
            case MS_BROADON_UGC:
                LCFG_TSD_SetRestrictUGC( sbBroadOnSetting );
            break;
        }
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL設定データファイルへの書き込み
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        sbInitPage = FALSE;
        SetParentalControlInit();
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_B) || (tp_touch && (commit == KEY_CANCEL)) )
    {
        sbInitPage = FALSE;
        SetParentalControlInit();                   // キャンセルのときセットしない
        g_pNowProcess = SetParentalControlMain;
        return 0;
    }
    
    DrawSetBroadOnSettingScene();
    return 0;
}


//======================================================
// ソフトウェアキーボード用キャラテーブル
//======================================================

static const u16 char_tbl[CHAR_LIST_MODE_NUM][CHAR_LIST_CHAR_NUM] = {
	{	// ひらがな
		L'あ',	L'い',	L'う',	L'え',	L'お',
		L'か',	L'き',	L'く',	L'け',	L'こ',	DEL_BUTTON_,
		L'さ',	L'し',	L'す',	L'せ',	L'そ',
		L'た',	L'ち',	L'つ',	L'て',	L'と',	SPACE_BUTTON_,
		L'な',	L'に',	L'ぬ',	L'ね',	L'の',
		L'は',	L'ひ',	L'ふ',	L'へ',	L'ほ',	EOM_,
		
		L'ま',	L'み',	L'む',	L'め',	L'も',
		L'や',	EOM_,	L'ゆ',	EOM_,	L'よ',	VAR_BUTTON1_,
		L'ら',	L'り',	L'る',	L'れ',	L'ろ',
		L'わ',	EOM_,	EOM_,	L'を',	L'ん',	VAR_BUTTON2_,
		L'ぁ',	L'ぃ',	L'ぅ',	L'ぇ',	L'ぉ',
		L'ゃ',	EOM_,	L'ゅ',	EOM_,	L'ょ',	EOM_,
		
		L'が',	L'ぎ',	L'ぐ',	L'げ',	L'ご',
		L'ざ',	L'じ',	L'ず',	L'ぜ',	L'ぞ',	EOM_,
		L'だ',	L'ぢ',	L'づ',	L'で',	L'ど',
		L'ば',	L'び',	L'ぶ',	L'べ',	L'ぼ',	OK_BUTTON_,
		L'ぱ',	L'ぴ',	L'ぷ',	L'ぺ',	L'ぽ',
		L'っ',	L'、',	L'。',	L'！',	L'？',	EOM_,
		
		L'「',	L'」',	L'～',	EOM_,	L'ー',
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// カタカナ
		L'ア',	L'イ',	L'ウ',	L'エ',	L'オ',
		L'カ',	L'キ',	L'ク',	L'ケ',	L'コ',	DEL_BUTTON_,
		L'サ',	L'シ',	L'ス',	L'セ',	L'ソ',
		L'タ',	L'チ',	L'ツ',	L'テ',	L'ト',	SPACE_BUTTON_,
		L'ナ',	L'ニ',	L'ヌ',	L'ネ',	L'ノ',
		L'ハ',	L'ヒ',	L'フ',	L'ヘ',	L'ホ',	EOM_,
		
		L'マ',	L'ミ',	L'ム',	L'メ',	L'モ',
		L'ヤ',	EOM_,	L'ユ',	EOM_,	L'ヨ',	VAR_BUTTON1_,
		L'ラ',	L'リ',	L'ル',	L'レ',	L'ロ',
		L'ワ',	EOM_,	L'ヲ',	EOM_,	L'ン',	VAR_BUTTON2_,
		L'ァ',	L'ィ',	L'ゥ',	L'ェ',	L'ォ',
		L'ャ',	EOM_,	L'ュ',	EOM_,	L'ョ',	EOM_,
		
		L'ガ',	L'ギ',	L'グ',	L'ゲ',	L'ゴ',
		L'ザ',	L'ジ',	L'ズ',	L'ゼ',	L'ゾ',	EOM_,
		L'ダ',	L'ヂ',	L'ヅ',	L'デ',	L'ド',
		L'バ',	L'ビ',	L'ブ',	L'ベ',	L'ボ',	OK_BUTTON_,
		L'パ',	L'ピ',	L'プ',	L'ペ',	L'ポ',
		L'ッ',	L'、',	L'。',	L'！',	L'ー',	EOM_,
		
		L'「',	L'」',	L'～',	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
	
	{	// 英数
        // 'A'から順にUTF-16文字コード(リトルエンディアン)で代入していく
		0x0041,	0x0042,	0x0043,	0x0044,	0x0045,
		0x0046,	0x0047,	0x0048,	0x0049,	0x004a,	DEL_BUTTON_,
		0x004b,	0x004c,	0x004d,	0x004e,	0x004f,
		0x0050,	0x0051,	0x0052,	0x0053,	0x0054,	SPACE_BUTTON_,
		0x0055,	0x0056,	0x0057,	0x0058,	0x0059,
		0x005a,	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		// 'a'から順
		0x0061,	0x0062,	0x0063,	0x0064,	0x0065,
		0x0066,	0x0067,	0x0068,	0x0069,	0x006a,	VAR_BUTTON1_,
		0x006b,	0x006c,	0x006d,	0x006e,	0x006f,
		0x0070,	0x0071,	0x0072,	0x0073,	0x0074,	VAR_BUTTON2_,
		0x0075,	0x0076,	0x0077,	0x0078,	0x0079,
		0x007a,	EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		// '0'から順
		0x0030,	0x0031,	0x0032,	0x0033,	0x0034,
		0x0035,	0x0036,	0x0037,	0x0038,	0x0039,	EOM_,
        // 特殊文字
        // '!'           '&'             '/'
		0x0021,	EOM_,	0x0026,	EOM_,	0x002f,
        // ','           '.'             '-'
		0x002c,	EOM_,	0x002e,	EOM_,	0x002d,	OK_BUTTON_,
        // '''           '"'
		0x0027,	EOM_,	0x2033,	EOM_,	EOM_,
        // '@'           '('             ')'
		0x0040,	EOM_,	0x0028,	EOM_,	0x0029,	EOM_,
		
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,	CANCEL_BUTTON_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
		EOM_,	EOM_,	EOM_,	EOM_,	EOM_,
	},
};


