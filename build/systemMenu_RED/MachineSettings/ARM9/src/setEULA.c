/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     setEULA.c

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
// 以下のEULA設定
// ++ agree/not agreeフラグ
// ++ agree eula version
// <例外処理>
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

// ON/OFFボタン領域
#define ON_BUTTON_TOP_X                     ( 6 * 8 )
#define ON_BUTTON_TOP_Y                     ( 8 * 8 )
#define ON_BUTTON_BOTTOM_X                  ( ON_BUTTON_TOP_X + (2 * 8) )
#define ON_BUTTON_BOTTOM_Y                  ( ON_BUTTON_TOP_Y + (2 * 8) )
#define OFF_BUTTON_TOP_X                    ( 18 * 8 )
#define OFF_BUTTON_TOP_Y                    (  8 * 8 )
#define OFF_BUTTON_BOTTOM_X                 ( OFF_BUTTON_TOP_X + (3 * 8) )
#define OFF_BUTTON_BOTTOM_Y                 ( OFF_BUTTON_TOP_Y + (2 * 8) )

// UP/DOWNボタン領域
#define UP_BUTTON_TOP_X                     ( 21 * 8 )
#define UP_BUTTON_TOP_Y                     (  7 * 8 )
#define UP_BUTTON_BOTTOM_X                  ( UP_BUTTON_TOP_X + (2 * 8) )
#define UP_BUTTON_BOTTOM_Y                  ( UP_BUTTON_TOP_Y + (2 * 8) )
#define DOWN_BUTTON_TOP_X                   ( 21 * 8 )
#define DOWN_BUTTON_TOP_Y                   ( 13 * 8 )
#define DOWN_BUTTON_BOTTOM_X                ( DOWN_BUTTON_TOP_X + (2 * 8) )
#define DOWN_BUTTON_BOTTOM_Y                ( DOWN_BUTTON_TOP_Y + (2 * 8) )


// 項目の総数
#define MS_EULA_NUMOF_ELEMENTS          	2

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
#define MULTI_KEY_UP    0xffe0
#define MULTI_KEY_DOWN  0xffd0
#define MASK_MULTI_KEY  0xfff0

// extern data----------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_birth_scr_data[32 * 32];

// function's prototype-------------------------

static void SetAgreeEULAInit( void );
static int  SetAgreeEULAMain( void );
static void SetAgreedVersionInit( void );
static int  SetAgreedVersionMain( void );

// static variable------------------------------
// 一時的にしか使わない物をstaticにしているので
// 少しでもダイエットしたい時はWork扱いにしてAlloc→Freeしましょう

// メニューで使用
static u16    sCursorMenu = 0;

// 特定のモードでのみ使用
static BOOL   sbAgreeEULA = FALSE;
static u8     sAgreedVersion;

// const data-----------------------------------

// ++ メインメニュー

static const u16 *s_pStrSetting[ MS_EULA_NUMOF_ELEMENTS ];          // メインメニュー用文字テーブルへのポインタリスト

static const u16 *const s_pStrSettingElemTbl[ MS_EULA_NUMOF_ELEMENTS ][ LCFG_TWL_LANG_CODE_MAX ] = {

    {
        (const u16 *)L"EULAへの同意",
        (const u16 *)L"Agree to EULA",
        (const u16 *)L"Agree to EULA(F)",
        (const u16 *)L"Agree to EULA(G)",
        (const u16 *)L"Agree to EULA(I)",
        (const u16 *)L"Agree to EULA(S)",
        (const u16 *)L"Agree to EULA(C)",
        (const u16 *)L"Agree to EULA(K)",
    },
    {
        (const u16 *)L"同意したバージョン",
        (const u16 *)L"Agreed version",
        (const u16 *)L"Agreed version(F)",
        (const u16 *)L"Agreed version(G)",
        (const u16 *)L"Agreed version(I)",
        (const u16 *)L"Agreed version(S)",
        (const u16 *)L"Agreed version(C)",
        (const u16 *)L"Agreed version(K)",
    },
};

// 表示位置
static MenuPos s_settingPos[] = {

    { TRUE,  2 * 8,   6 * 8 },
    { TRUE,  2 * 8,  10 * 8 },
};

// 表示パラメータ
static const MenuParam s_settingParam = 
{
    MS_EULA_NUMOF_ELEMENTS,
    TXT_COLOR_BLACK,
    TXT_COLOR_GREEN,
    TXT_COLOR_RED,
    &s_settingPos[0],
    (const u16 **)&s_pStrSetting,
};


//=========================================================
//
// ボタンのタッチ処理
//
//=========================================================

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
static void DrawEULAMenuScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"EULA" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"RETURN" );
    // メニュー項目
    DrawMenu( sCursorMenu, &s_settingParam );
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得して表示
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    PutStringUTF16( 21*8, s_settingPos[0].y, TXT_UCOLOR_G0, 
                    LCFG_TSD_IsAgreeEULA() ? (const u16*)L"Agree":(const u16*)L"Not agree" );
    PrintfSJIS( 21*8, s_settingPos[1].y, TXT_UCOLOR_G0, "%d",
                    LCFG_TSD_GetAgreedEULAVersion() );
}

// 初期化
void SetEULAInit( void )
{
    int  i;
	
    // NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
    for( i=0; i < MS_EULA_NUMOF_ELEMENTS; i++ )
    {
        s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ LCFG_TSD_GetLanguage() ];
    }

    // BGデータのロード処理
    GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
    GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));

    DrawEULAMenuScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// メニューから呼ばれるメイン
int SetEULAMain( void )
{
    SelectSomethingFunc func[1]={SelectCancelFunc};
    BOOL        tp_select;
    BOOL        tpCommit = FALSE;
    u16         padrep;
    static u16  commit;

    ReadTP();

    padrep = DetectPadRepeat();     // 長押し検出

    // メニューからの項目選択
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) ){                               // カーソルの移動
        if( ++sCursorMenu >= MS_EULA_NUMOF_ELEMENTS ) {
            sCursorMenu = 0;
        }
    }
    if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) ){
        if( --sCursorMenu & 0x80 ) {
            sCursorMenu = (u16)(MS_EULA_NUMOF_ELEMENTS - 1);
        }
    }
    tp_select = SelectMenuByTP( &sCursorMenu, &s_settingParam );

    // 特殊ボタンタッチ
    tpCommit = SelectSomethingByTP( &commit, func, 1 );

    // メニューへの分岐
    if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) ) {         // メニュー項目への分岐
        if( (s_settingParam.pos[sCursorMenu]).enable ) {
            switch( sCursorMenu )
            {
                case 0:
                    SetAgreeEULAInit();
                    g_pNowProcess = SetAgreeEULAMain;
                break;
                case 1:
                    SetAgreedVersionInit();
                    g_pNowProcess = SetAgreedVersionMain;
                break;
            }
        } // if( (s_settingParam.pos[sCursorMenu]).enable )
    } // if( ( pad.trg & PAD_BUTTON_A ) || ( tp_select ) )
    else if( (pad.trg & PAD_BUTTON_B) || (tpCommit && (commit == KEY_CANCEL)) )
    {
        MachineSettingInit();
        return 0;
    }

    // 再描画
    DrawEULAMenuScene();
    return 0;
}

//=========================================================
//
// EULA同意／非同意のセット
//
//=========================================================

// 描画処理
static void DrawSetAgreeEULAScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"Agree EULA" );

    if( !sbAgreeEULA )
    {
        PutStringUTF16( ON_BUTTON_TOP_X,  ON_BUTTON_TOP_Y,  TXT_COLOR_GREEN, (const u16*)L"Not agree" );
        PutStringUTF16( OFF_BUTTON_TOP_X, OFF_BUTTON_TOP_Y, TXT_UCOLOR_G0,   (const u16*)L"Agree" );
    }
    else
    {
        PutStringUTF16( ON_BUTTON_TOP_X,  ON_BUTTON_TOP_Y,  TXT_UCOLOR_G0,   (const u16*)L"Not agree" );
        PutStringUTF16( OFF_BUTTON_TOP_X, OFF_BUTTON_TOP_Y, TXT_COLOR_GREEN, (const u16*)L"Agree" );
    }
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"CANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"OK" );
}

// 初期化
static void SetAgreeEULAInit( void )
{
    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sbAgreeEULA = LCFG_TSD_IsAgreeEULA();

    DrawSetAgreeEULAScene();

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetAgreeEULAMain( void )
{
    SelectSomethingFunc func[4]={SelectCancelFunc, SelectOKFunc, SelectONFunc, SelectOFFFunc };
    u16  commit;
    BOOL tp_touch = FALSE;

    ReadTP();
    
    // キーによる選択
    if( (pad.trg & PAD_KEY_LEFT) || (pad.trg & PAD_KEY_RIGHT) )
    {
        sbAgreeEULA = !sbAgreeEULA;
    }

    // タッチによる選択
    tp_touch = SelectSomethingByTP( &commit, func, 4 );
    if( tp_touch && (commit == KEY_ON) )
    {
        sbAgreeEULA = TRUE;
    }
    else if( tp_touch && (commit == KEY_OFF) )
    {
        sbAgreeEULA = FALSE;
    }

    // 決定
    if( (pad.trg & PAD_BUTTON_A) || (tp_touch && (commit == KEY_OK)) )
    {
        LCFG_TSD_SetFlagAgreeEULA( sbAgreeEULA );
		if( !sbAgreeEULA ) {
	        LCFG_TSD_SetAgreedEULAVersion( 0 );
		}
		// ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL設定データファイルへの書き込み
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
		OS_TPrintf( "AgreeEULA : %s\n", OS_IsAgreeEULA() ? "Agree" : "Not agree" );
        SetEULAInit();
        g_pNowProcess = SetEULAMain;
        return 0;
    }
    else if( (pad.trg & PAD_BUTTON_B) || (tp_touch && (commit == KEY_CANCEL)) )
    {
        SetEULAInit();                   // キャンセルのときセットしない
        g_pNowProcess = SetEULAMain;
        return 0;
    }
    
    DrawSetAgreeEULAScene();
    return 0;
}


//=========================================================
//
// 同意EULAバージョンのセット
//
//=========================================================

// 描画処理
static void DrawSetAgreedVersionScene( void )
{
    u16 iconUp[2]   = {0xE01B, 0};
    u16 iconDown[2] = {0xE01C, 0};
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"AGREED EULA VERSION" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"CANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"OK" );
    PutStringUTF16( UP_BUTTON_TOP_X,     UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconUp );
    PutStringUTF16( DOWN_BUTTON_TOP_X,     DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconDown );
    PutStringUTF16( 4*8, 10*8, TXT_UCOLOR_G0, (const u16 *)L"Agreed EULA ver." );
    PrintfSJIS( 20*8, 10*8, TXT_COLOR_GREEN, "%3d", sAgreedVersion );
}

// 初期化
static void SetAgreedVersionInit( void )
{
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sAgreedVersion = LCFG_TSD_GetAgreedEULAVersion();

    DrawSetAgreedVersionScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetAgreedVersionMain( void )
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
        --sAgreedVersion;
    }
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) || (tpUD && (csrUD == KEY_DOWN)) )
    {
        ++sAgreedVersion;
    }
    if(pad.trg & PAD_BUTTON_START)
    {
        sAgreedVersion = 0;
    }

    // 決定
    if( pad.trg & PAD_BUTTON_A || (tpCommit && (csrCommit == KEY_OK)) )
    {
        LCFG_TSD_SetAgreedEULAVersion( sAgreedVersion );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL設定データファイルへの書き込み
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
		OS_TPrintf( "Agreed EULA version : %d\n", OS_GetAgreedEULAVersion );
        SetEULAInit();
        g_pNowProcess = SetEULAMain;
        return 0;
    }
    else if( ( pad.trg & PAD_BUTTON_B ) || (tpCommit && (csrCommit == KEY_CANCEL)) )
    {
        SetEULAInit();
        g_pNowProcess = SetEULAMain;
        return 0;
    }
    
    // 再描画
    DrawSetAgreedVersionScene();
    return 0;
}

