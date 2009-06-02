 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - WCM - demos - wcm-list-2
  File:     screen.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#include <nitro.h>
#include "screen.h"
#include "font.h"

/*---------------------------------------------------------------------------*
    定数 定義
 *---------------------------------------------------------------------------*/
#define TEXT_SCREEN_SIZE    2048

/*---------------------------------------------------------------------------*
    内部変数 定義
 *---------------------------------------------------------------------------*/

// 仮想スクリーン[ 上下画面 ][ BG 枚数 ][ キャラクタ数 ]
static u16  gScreen[2 ][ 1 ][ TEXT_SCREEN_SIZE / sizeof(u16) ] ATTRIBUTE_ALIGN(32);

/*---------------------------------------------------------------------------*
  Name:         InitScreen

  Description:  文字表示システムのために、表示設定を初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void InitScreen(void)
{
    // 各 V-RAM 初期化
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    // OAM 初期化
    MI_CpuFillFast((void*)HW_OAM, 0xc0, HW_OAM_SIZE);
    MI_CpuFillFast((void*)HW_DB_OAM, 0xc0, HW_DB_OAM_SIZE);

    // パレット初期化
    MI_CpuClearFast((void*)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuClearFast((void*)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    // 上画面設定
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    
    GX_SetBankForBG(GX_VRAM_BG_128_A);
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000,
                     GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);
    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    ((u16*)HW_PLTT)[0] = 0x0000;    // black
    MI_CpuFillFast(gScreen[0][0], 0, TEXT_SCREEN_SIZE);
    DC_StoreRange(gScreen[0][0], TEXT_SCREEN_SIZE);
    GX_LoadBG0Scr(gScreen[0][0], 0, TEXT_SCREEN_SIZE);

    // 下画面設定
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_32_H);
    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000,
                      GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0);
    GXS_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GXS_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));
    ((u16*)HW_DB_PLTT)[0] = 0x0000; // black
    MI_CpuFillFast(gScreen[1][0], 0, TEXT_SCREEN_SIZE);
    DC_StoreRange(gScreen[1][0], TEXT_SCREEN_SIZE);
    GXS_LoadBG0Scr(gScreen[1][0], 0, TEXT_SCREEN_SIZE);
}

/*---------------------------------------------------------------------------*
  Name:         ClearScreen

  Description:  画面のテキスト表示をクリアする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void ClearScreen(void)
{
    MI_CpuClearFast(gScreen[0][0], TEXT_SCREEN_SIZE);
    MI_CpuClearFast(gScreen[1][0], TEXT_SCREEN_SIZE);
}
void ClearMainScreen(void)
{
    MI_CpuClearFast(gScreen[0][0], TEXT_SCREEN_SIZE);

}
void ClearSubScreen(void)
{
    MI_CpuClearFast(gScreen[1][0], TEXT_SCREEN_SIZE);
}
/*---------------------------------------------------------------------------*
  Name:         PutMainScreen

  Description:  メイン画面にテキスト出力する。

  Arguments:    text    -   出力する文字列。
                ...     -   仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PutMainScreen(s32 x, s32 y, u8 palette, char* text, ...)
{
    va_list vlist;
    char    temp[33];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    for (i = 0; i < 32; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }

        gScreen[0][0][((y * 32) + x + i) % (32 * 32)] = (u16) (palette << 12 | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         PrintSubScreen

  Description:  サブ画面にテキスト出力する。

  Arguments:    text    -   出力する文字列。
                ...     -   仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void PutSubScreen(s32 x, s32 y, u8 palette, char* text, ...)
{
    va_list vlist;
    char    temp[33];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    for (i = 0; i < 32; i++)
    {
        if (temp[i] == 0x00)
        {
            break;
        }

        gScreen[1][0][((y * 32) + x + i) % (32 * 32)] = (u16) (palette << 12 | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         UpdateScreen

  Description:  仮想スクリーンを V-RAM に反映する。
                V ブランク期間中での呼び出しを想定。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void UpdateScreen(void)
{
    // 仮想スクリーンを V-RAM に反映
    DC_StoreRange(gScreen[0][0], TEXT_SCREEN_SIZE);
    GX_LoadBG0Scr(gScreen[0][0], 0, TEXT_SCREEN_SIZE);
    DC_StoreRange(gScreen[1][0], TEXT_SCREEN_SIZE);
    GXS_LoadBG0Scr(gScreen[1][0], 0, TEXT_SCREEN_SIZE);
}

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
