/********************************************************************

タイトル：
　テキストＶＲＡＭエミュレーション

********************************************************************/
#include "fontdata.h"
#include "text.h"

/************************************************************************

☆グローバル変数宣言

 ************************************************************************/
/* テキストＶＲＡＭ本体 */
TEXT_VRAM_TYPE wText;

/************************************************************************

☆プロトタイプ宣言

 ************************************************************************/
static void UpdateTextVram( void );
static void Init( void );



/************************************************************************

☆システム関数群

 ************************************************************************/

/************************************************************************

　初期化

概要：
  キャラ・パレットの転送
  ＢＧ０コントロール

  ***********************************************************************/
static void Init( void ){

    GX_SetBankForBG(GX_VRAM_BG_128_A);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    G2_SetBG0Priority(0);
    G2_BlendNone();                                 // no 2D alpha blending or brightness change
    G2_BG0Mosaic(FALSE);
    GX_SetVisibleWnd(GX_WNDMASK_NONE);              // ウインドウ無し
    G2_SetBG0Offset(0, 0);                          //オフセット０
    GX_SetBGScrOffset(GX_BGSCROFFSET_0x00000);

    //表示リソースの再ロード
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,        // 256pix x 256pix text
                     GX_BG_COLORMODE_16,                // use 256 colors mode
                     GX_BG_SCRBASE_0x0000,              // screen base offset + 0x0000 is the address for BG #0 screen
                     GX_BG_CHARBASE_0x04000,            // character base offset + 0x04000 is the address for BG #0 characters
                     GX_BG_EXTPLTT_01                   // use BGExtPltt slot #0 if BGExtPltt is enabled
                     );
    GX_LoadBG0Char(CharData_Sample, 0, sizeof(CharData_Sample));
    GX_LoadBGPltt(PlttData_Sample, 0, sizeof(PlttData_Sample));

}

/************************************************************************

 テキストＶＲＡＭサービス初期化

注意事項：
  プログラム開始時に一度だけコールしてください。

  ***********************************************************************/
extern void wInitTextVram( void ){

    static BOOL Initialized = FALSE;

    if( !Initialized ){

        /* 初期設定をおこなう */
        Init();

        // Vブランク割り込み設定
        OS_InitIrqTable();
        OS_SetIrqFunction(OS_IE_V_BLANK, UpdateTextVram);
        (void)OS_EnableIrqMask(OS_IE_V_BLANK);
        (void)OS_EnableIrq();
        (void)GX_VBlankIntr(TRUE);
        (void)OS_EnableInterrupts();

        Initialized = TRUE;
    }
}

/************************************************************************

  テキストＶＲＡＭの更新ルーチン

注意事項：
　割込みのコールバックルーチンです。

  画面を自動的に更新するために定期的に呼び出してください。
  （上記のInitTextVramを使用する場合は不要）

  ***********************************************************************/
static void UpdateTextVram( void ){

    MI_CpuCopy16( wText.Map, G2_GetBG0ScrPtr(), sizeof(wText.Map) );
    DC_StoreRange( G2_GetBG0ScrPtr(), sizeof(wText.Map) );

}

