/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     cleanupMachine.c

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
#include <sysmenu/namut.h>
#include "misc.h"
#include "MachineSetting.h"

//
// <処理>
// NANDの初期化処理(擬似フォーマット)
//
// <例外処理>
//

// テスト表示
#if 1
#define DEBUGPRINT              OS_TPrintf
#else
#define DEBUGPRINT(...)         ((void)0)
#endif

// define data----------------------------------

// 戻るボタン領域
#define RETURN_BUTTON_TOP_X                 ( 2 * 8 )
#define RETURN_BUTTON_TOP_Y                 ( 21 * 8 )
#define RETURN_BUTTON_BOTTOM_X              ( RETURN_BUTTON_TOP_X + (8 * 8) )
#define RETURN_BUTTON_BOTTOM_Y              ( RETURN_BUTTON_TOP_Y + (2 * 8) )

// OKボタン領域
#define OK_BUTTON_TOP_X                     ( 26 * 8 )
#define OK_BUTTON_TOP_Y                     ( 21 * 8 )
#define OK_BUTTON_BOTTOM_X                  ( OK_BUTTON_TOP_X + (4 * 8) )
#define OK_BUTTON_BOTTOM_Y                  ( OK_BUTTON_TOP_Y + (2 * 8) )

// YES/NOボタン領域
#define YES_BUTTON_TOP_X                    ( 8 * 8 )
#define YES_BUTTON_TOP_Y                    ( 8 * 8 )
#define YES_BUTTON_BOTTOM_X                 ( YES_BUTTON_TOP_X + (3 * 8) )
#define YES_BUTTON_BOTTOM_Y                 ( YES_BUTTON_TOP_Y + (2 * 8) )
#define NOT_BUTTON_TOP_X                    ( 18 * 8 )
#define NOT_BUTTON_TOP_Y                    (  8 * 8 )
#define NOT_BUTTON_BOTTOM_X                 ( NOT_BUTTON_TOP_X + (2 * 8) )
#define NOT_BUTTON_BOTTOM_Y                 ( NOT_BUTTON_TOP_Y + (2 * 8) )

#define KEY_OK          0xffff
#define KEY_RETURN      0xfffe
#define KEY_YES         0xfffd
#define KEY_NOT         0xfffc

// extern data----------------------------------

extern u32 bg_char_data[8 * 6];
extern u16 bg_scr_data[32 * 32];
extern u16 bg_birth_scr_data[32 * 32];

// function's prototype-------------------------


// static variable------------------------------
// 一時的にしか使わない物をstaticにしているので
// 少しでもダイエットしたい時はWork扱いにしてAlloc→Freeしましょう

typedef enum
{
    MS_FORMATMACHINE_SELECT,          // ユーザ選択中
    MS_FORMATMACHINE_PROCESSING,      // 処理中
    MS_FORMATMACHINE_ERROR,           // エラー発生
    MS_FORMATMACHINE_SUCCESS          // 成功
} eState;
static BOOL    sbInit   = FALSE;                     // 初期化するかどうか
static eState  sState   = MS_FORMATMACHINE_SELECT;   // 現在の実行状態

// const data-----------------------------------

// ++ メインメニュー

//=========================================================
//
// ボタンのタッチ処理
//
//=========================================================

// 戻るボタン専用SelectSomethingFuncの実装
static BOOL SelectReturnFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y,
                         RETURN_BUTTON_BOTTOM_X, RETURN_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_RETURN;
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
static BOOL SelectYESFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( YES_BUTTON_TOP_X, YES_BUTTON_TOP_Y,
                         YES_BUTTON_BOTTOM_X, YES_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_YES;
    return ret;
}
static BOOL SelectNOTFunc( u16 *csr, TPData *tgt )
{
    BOOL ret;
    ret = WithinRangeTP( NOT_BUTTON_TOP_X, NOT_BUTTON_TOP_Y,
                         NOT_BUTTON_BOTTOM_X, NOT_BUTTON_BOTTOM_Y, tgt );
    if(ret) *csr = KEY_NOT;
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
static void DrawCleanupMachineScene( void )
{
    NNS_G2dCharCanvasClear( &gCanvas, TXT_COLOR_NULL );
    PutStringUTF16( 0, 0, TXT_COLOR_BLUE, (const u16 *)L"MACHINE CLEAN UP" );

    PutStringUTF16( 4*8, 4*8, TXT_COLOR_BLUE, (const u16 *)L"Start Cleaning?" );
    if( sbInit )
    {
        PutStringUTF16( YES_BUTTON_TOP_X-1*8, YES_BUTTON_TOP_Y, TXT_COLOR_GREEN, (const u16*)L">YES<" );
        PutStringUTF16( NOT_BUTTON_TOP_X-1*8, NOT_BUTTON_TOP_Y, TXT_UCOLOR_G0,   (const u16*)L" NO" );
    }
    else
    {
        PutStringUTF16( YES_BUTTON_TOP_X-1*8, YES_BUTTON_TOP_Y, TXT_UCOLOR_G0,   (const u16*)L" YES" );
        PutStringUTF16( NOT_BUTTON_TOP_X-1*8, NOT_BUTTON_TOP_Y, TXT_COLOR_GREEN, (const u16*)L">NO<" );
    }

    // 実行状況によって表示が異なる
    switch( sState )
    {
        case MS_FORMATMACHINE_SELECT:
            PutStringUTF16( OK_BUTTON_TOP_X, OK_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"OK" );
        break;
        
        case MS_FORMATMACHINE_PROCESSING:
            PutStringUTF16( 4*8,  12*8,  TXT_COLOR_GREEN, (const u16*)L"In Processing..." );
        break;

        case MS_FORMATMACHINE_ERROR:
            PutStringUTF16( 4*8,  12*8,  TXT_UCOLOR_G0, (const u16*)L"In Processing..." );
            PutStringUTF16( 4*8,  14*8,  TXT_COLOR_RED, (const u16*)L"ERROR!" );
            PutStringUTF16( RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"RETURN" );
        break;
        
        case MS_FORMATMACHINE_SUCCESS:
            PutStringUTF16( 4*8,  12*8,  TXT_UCOLOR_G0,   (const u16*)L"In Processing..." );
            PutStringUTF16( 4*8,  14*8,  TXT_COLOR_GREEN, (const u16*)L"Completed!" );
            PutStringUTF16( RETURN_BUTTON_TOP_X, RETURN_BUTTON_TOP_Y, TXT_UCOLOR_G0, (const u16 *)L"RETURN" );
        break;
        
        default:
            DEBUGPRINT( "invalid state\n" );
        break;
    }
}

// 初期化
void CleanupMachineInit( void )
{
    // BGデータのロード処理
    GX_LoadBG1Char(bg_char_data, 0, sizeof(bg_char_data));
    GX_LoadBG1Scr(bg_scr_data, 0, sizeof(bg_scr_data));

    DrawCleanupMachineScene();

    SVC_CpuClear( 0x0000, &tpd, sizeof(TpWork), 16 );

    GX_SetVisiblePlane ( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );

    sbInit = FALSE;
    sState = MS_FORMATMACHINE_SELECT;       // 初期状態
}

// メニューから呼ばれるメイン
int CleanupMachineMain( void )
{
    SelectSomethingFunc funcSelect[3]={SelectOKFunc, SelectYESFunc, SelectNOTFunc};
    SelectSomethingFunc funcFinish[1]={SelectReturnFunc};
    BOOL        tpCommit = FALSE;
    static u16  commit;

    ReadTP();

    // タッチできるボタンは実行状況によって異なる
    switch( sState )
    {
        case MS_FORMATMACHINE_SELECT:
            if( (pad.trg & PAD_KEY_LEFT) || (pad.trg & PAD_KEY_RIGHT) )
            {
                sbInit = !sbInit;
            }
            tpCommit = SelectSomethingByTP( &commit, funcSelect, 3 );   // 選択とOKのみ
            if( tpCommit && (commit==KEY_YES) )
            {
                sbInit = TRUE;
            }
            else if( tpCommit && (commit == KEY_NOT) )
            {
                sbInit = FALSE;
            }
            
            if( (pad.trg & PAD_BUTTON_A) || (tpCommit && (commit == KEY_OK)) )
            {
                if( sbInit )
                {
                    sState = MS_FORMATMACHINE_PROCESSING;     // 遷移
                }
                else
                {
                    MachineSettingInit();
                    return 0;
                }
            }
        break;
        
        case MS_FORMATMACHINE_PROCESSING:
            // 選択を受け付けない
            if( !NAMUT_FormatCore(FALSE,TRUE) )                   // 初期化実行
            {
                sState = MS_FORMATMACHINE_ERROR;
            }
            else
            {
                sState = MS_FORMATMACHINE_SUCCESS;
            }
        break;
        
        case MS_FORMATMACHINE_ERROR:
        case MS_FORMATMACHINE_SUCCESS:
            tpCommit = SelectSomethingByTP( &commit, funcFinish, 1 );   // Returnのみ
            if( (pad.trg & PAD_BUTTON_B) || (tpCommit && (commit == KEY_RETURN)) )
            {
                MachineSettingInit();
                return 0;
            }
        break;
    }

    // 再描画
    DrawCleanupMachineScene();
    return 0;
}

