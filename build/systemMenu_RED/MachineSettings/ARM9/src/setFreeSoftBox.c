/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     setFreeSoftBox.c

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
// 以下のフリーソフトBOXの設定
// ++ フリーソフトBOX数
// ++ (連動して使用ソフトBOX数が設定される)
//
// <例外処理>
// ++ 本来はフリーソフトBOX数+使用ソフトBOX数=max値となるが初期状態では両方とも0である可能性がある
//    -> フリーソフトBOX数を本UIを用いて設定されるまで値はセットされない
//       -- キャンセルされたときには値はセットされないので上式は満たされないままとなる
//       -- 設定が正しいかどうかを知らせるため上式が満たされないとき赤色の文字で値を表示する
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
#define ON_BUTTON_TOP_X                     ( 8 * 8 )
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
#define MS_FREESOFTBOX_NUMOF_ELEMENTS          1

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

static void SetFreeSoftBoxCountInit( void );
static int  SetFreeSoftBoxCountMain( void );

// static variable------------------------------
// 一時的にしか使わない物をstaticにしているので
// 少しでもダイエットしたい時はWork扱いにしてAlloc→Freeしましょう

// メニューで使用
static u16    sCursorMenu = 0;

// 特定のモードでのみ使用
static u8     sFreeSoftBoxCount = 0;        // フリーソフトBOX数
static u8     sInstalledSoftBoxCount = 0;   // 使用ソフトBOX数

// const data-----------------------------------

// ++ メインメニュー

static const u16 *s_pStrSetting[ MS_FREESOFTBOX_NUMOF_ELEMENTS ];          // メインメニュー用文字テーブルへのポインタリスト

static const u16 *const s_pStrSettingElemTbl[ MS_FREESOFTBOX_NUMOF_ELEMENTS ][ LCFG_TWL_LANG_CODE_MAX ] = {

    {
        (const u16 *)L"フリーソフトBOX数",
        (const u16 *)L"FreeSoft Box Count",
        (const u16 *)L"FreeSoft Box Count(F)",
        (const u16 *)L"FreeSoft Box Count(G)",
        (const u16 *)L"FreeSoft Box Count(I)",
        (const u16 *)L"FreeSoft Box Count(S)",
        (const u16 *)L"FreeSoft Box Count(C)",
        (const u16 *)L"FreeSoft Box Count(K)",
    },
};

// ++ 補足表示
static const u16 *const s_pStrInstalledSoftBox[ LCFG_TWL_LANG_CODE_MAX ] =
{
    (const u16 *)L"使用ソフトBOX数",
    (const u16 *)L"InstalledSoft Box Count",
    (const u16 *)L"InstalledSoft Box Count(F)",
    (const u16 *)L"InstalledSoft Box Count(G)",
    (const u16 *)L"InstalledSoft Box Count(I)",
    (const u16 *)L"InstalledSoft Box Count(S)",
    (const u16 *)L"InstalledSoft Box Count(C)",
    (const u16 *)L"InstalledSoft Box Count(K)",
};

// 表示位置
static MenuPos s_settingPos[] = {

    { TRUE,  2 * 8,   6 * 8 },
};

// 表示パラメータ
static const MenuParam s_settingParam = 
{
    MS_FREESOFTBOX_NUMOF_ELEMENTS,
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
static void DrawFreeSoftBoxMenuScene( void )
{
    u8    installCount, freeCount;
    BOOL  bRegular = FALSE;

    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"FREESOFT BOX" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"RETURN" );
    // メニュー項目
    DrawMenu( sCursorMenu, &s_settingParam );
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得して表示
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // フリーソフトBOX数
    freeCount    = LCFG_TSD_GetFreeSoftBoxCount();
    installCount = LCFG_TSD_GetInstalledSoftBoxCount();
    bRegular     = (installCount + freeCount) == LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX;      // 満たされるべき式
    PrintfSJIS( 27*8, s_settingPos[0].y, (bRegular)?TXT_UCOLOR_G0:TXT_COLOR_RED, "%2d", freeCount );     // 値が不正のとき赤色で表示
    PutStringUTF16(  1*8, s_settingPos[0].y+4*8, TXT_UCOLOR_G0, (const u16 *)L"(" );    // 補足情報
    PutStringUTF16( 29*8, s_settingPos[0].y+4*8, TXT_UCOLOR_G0, (const u16 *)L")" );
    PutStringUTF16( s_settingPos[0].x, s_settingPos[0].y+4*8, TXT_UCOLOR_G0, s_pStrInstalledSoftBox[ LCFG_TSD_GetLanguage() ] );
    PrintfSJIS( 27*8, s_settingPos[0].y+4*8, (bRegular)?TXT_UCOLOR_G0:TXT_COLOR_RED, "%2d", installCount );
}

// 初期化
void SetFreeSoftBoxInit( void )
{
    int  i;

    // NITRO設定データのlanguageに応じたメインメニュー構成言語の切り替え
    for( i=0; i < MS_FREESOFTBOX_NUMOF_ELEMENTS; i++ )
    {
        s_pStrSetting[ i ] = s_pStrSettingElemTbl[ i ][ LCFG_TSD_GetLanguage() ];
    }

    // BGデータのロード処理
    GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
    GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));

    DrawFreeSoftBoxMenuScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// メニューから呼ばれるメイン
int SetFreeSoftBoxMain( void )
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
        if( ++sCursorMenu >= MS_FREESOFTBOX_NUMOF_ELEMENTS ) {
            sCursorMenu = 0;
        }
    }
    if( (pad.trg & PAD_KEY_UP) || (padrep & PAD_KEY_UP) ){
        if( --sCursorMenu & 0x80 ) {
            sCursorMenu = (u16)(MS_FREESOFTBOX_NUMOF_ELEMENTS - 1);
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
                    SetFreeSoftBoxCountInit();
                    g_pNowProcess = SetFreeSoftBoxCountMain;
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
    DrawFreeSoftBoxMenuScene();
    return 0;
}

//=========================================================
//
// フリーソフトBOX数の選択 (数値選択)
//
//=========================================================

// 描画処理
static void DrawSetFreeSoftBoxCountScene( void )
{
    u16 iconUp[2]   = {0xE01B, 0};
    u16 iconDown[2] = {0xE01C, 0};
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"FREESOFT BOX COUNT" );
    PutStringUTF16( CANCEL_BUTTON_TOP_X, CANCEL_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"CANCEL" );
    PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"OK" );
    PutStringUTF16( UP_BUTTON_TOP_X,     UP_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconUp );
    PutStringUTF16( DOWN_BUTTON_TOP_X,     DOWN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)iconDown );
    PutStringUTF16( 6*8, 10*8, TXT_UCOLOR_G0, (const u16 *)L"FreeSoft Box" );
    PrintfSJIS( 21*8, 10*8, TXT_COLOR_GREEN, "%2d", sFreeSoftBoxCount );
    PutStringUTF16(  2*8, 18*8, TXT_UCOLOR_G0, (const u16 *)L"(" ); 
    PutStringUTF16( 24*8, 18*8, TXT_UCOLOR_G0, (const u16 *)L")" );
    PutStringUTF16(  4*8, 18*8, TXT_UCOLOR_G0, (const u16 *)L"InstalledSoft Box" );
    PrintfSJIS( 21*8, 18*8, TXT_UCOLOR_G0, "%2d", sInstalledSoftBoxCount );
}

// 初期化
static void SetFreeSoftBoxCountInit( void )
{
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    // あらかじめTWL設定データファイルから読み込み済みの設定を取得
    // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
    sFreeSoftBoxCount      = LCFG_TSD_GetFreeSoftBoxCount();
    sInstalledSoftBoxCount = (u8)(LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - sFreeSoftBoxCount);

    DrawSetFreeSoftBoxCountScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );
}

// 表示プロセスとして呼び出されるメイン
static int SetFreeSoftBoxCountMain( void )
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
        if( (++sFreeSoftBoxCount) > LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX )
        {
            sFreeSoftBoxCount = 0;
        }
    }
    if( (pad.trg & PAD_KEY_DOWN) || (padrep & PAD_KEY_DOWN) || (tpUD && (csrUD == KEY_DOWN)) )
    {
        if( (--sFreeSoftBoxCount) & 0x80 )
        {
            sFreeSoftBoxCount = LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX;
        }
    }

    // フリーソフト数が変わると使用ソフト数も変わる
    sInstalledSoftBoxCount = (u8)(LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX - sFreeSoftBoxCount);

    // 決定
    if( pad.trg & PAD_BUTTON_A || (tpCommit && (csrCommit == KEY_OK)) )
    {
        LCFG_TSD_SetFreeSoftBoxCount( sFreeSoftBoxCount );
        LCFG_TSD_SetInstalledSoftBoxCount( sInstalledSoftBoxCount );
        // ::::::::::::::::::::::::::::::::::::::::::::::
        // TWL設定データファイルへの書き込み
        // ::::::::::::::::::::::::::::::::::::::::::::::
        if( !MY_WriteTWLSettings() )
        {
            OS_TPrintf( "TWL settings write failed.\n" );
        }
        SetFreeSoftBoxInit();
        g_pNowProcess = SetFreeSoftBoxMain;
        return 0;
    }
    else if( ( pad.trg & PAD_BUTTON_B ) || (tpCommit && (csrCommit == KEY_CANCEL)) )
    {
        SetFreeSoftBoxInit();
        g_pNowProcess = SetFreeSoftBoxMain;
        return 0;
    }
    
    // 再描画
    DrawSetFreeSoftBoxCountScene();
    return 0;
}

