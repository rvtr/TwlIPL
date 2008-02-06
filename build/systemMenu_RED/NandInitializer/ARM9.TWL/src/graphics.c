/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     praphics.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include <twl.h>
//#include <twl/mic.h>
#include <nitro/snd.h>
#include "kami_font.h"
#include "graphics.h"

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         InitGraphics

  Description:  表示の初期化処理

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
InitGraphics(void)
{
    GX_DispOff();
    GXS_DispOff();

    //---- VRAM クリア
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);

    //---- OAMとパレットクリア
    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    //---- ＯＢＪバンクの設定
    GX_SetBankForOBJ(GX_VRAM_OBJ_128_A);
	GX_SetBankForSubOBJ(GX_VRAM_SUB_OBJ_128_D);

	//---- ＢＧバンクの設定
	GX_SetBankForBG(GX_VRAM_BG_128_B);
	GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);

	//---- 表示モード設定
    GX_SetGraphicsMode (GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D);
    GX_SetVisiblePlane (GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_OBJ);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_OBJ);

	//---- 3D初期化
    G3X_Init();
    G3X_InitMtxStack();
    G2_SetBG0Priority(0);
    G3X_AlphaTest(FALSE, 0);
    G3X_AntiAlias(TRUE);
    G3X_EdgeMarking(FALSE);
    G3X_SetFog(FALSE, (GXFogBlend)0, (GXFogSlope)0, 0);
    G3X_SetClearColor(0, 0, 0x7fff, 63, FALSE);
    G3_ViewPort(0, 0, 255, 191);
    G3_MtxMode(GX_MTXMODE_POSITION_VECTOR);

    //---- 32KバイトのOBJで2Dマップモードで使用
    GX_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);

	// スクリーンオフセット及びキャラクターオフセットの設定
	GX_SetBGScrOffset(GX_BGSCROFFSET_0x00000);
	GX_SetBGCharOffset(GX_BGCHAROFFSET_0x10000);

	G2_SetBG1Control(
	    GX_BG_SCRSIZE_TEXT_256x256,
	    GX_BG_COLORMODE_16,
	    GX_BG_SCRBASE_0x0000 ,
	    GX_BG_CHARBASE_0x00000,
	    GX_BG_EXTPLTT_01
	);

	G2S_SetBG0Control(
	    GX_BG_SCRSIZE_TEXT_256x256,
	    GX_BG_COLORMODE_16,
	    GX_BG_SCRBASE_0x0000 ,
	    GX_BG_CHARBASE_0x04000,
	    GX_BG_EXTPLTT_01
	);

	G2S_SetBG1Control(
	    GX_BG_SCRSIZE_TEXT_256x256,
	    GX_BG_COLORMODE_16,
	    GX_BG_SCRBASE_0x2000 ,
	    GX_BG_CHARBASE_0x04000,
	    GX_BG_EXTPLTT_01
	);

	G2_BG1Mosaic(FALSE);
	G2S_BG1Mosaic(FALSE);

    //---- データロード
	GX_LoadOBJ    ( sampleCharData, 0, sizeof(sampleCharData));
	GX_LoadBG1Char( sampleCharData, 0, sizeof(sampleCharData));
	GX_LoadBGPltt ( PlttDataMain,   0, sizeof(PlttDataMain));
	GX_LoadBG1Scr ( BgScDataMain,   0, sizeof(BgScDataMain));
	GX_LoadOBJPltt( PlttDataObj,    0, sizeof(PlttDataObj));

	GXS_LoadOBJ    ( sampleCharData, 0, sizeof(sampleCharData));
	GXS_LoadBG0Char( sampleCharData, 0, sizeof(sampleCharData));
	GXS_LoadBGPltt ( PlttDataSub,    0, sizeof(PlttDataSub));
	GXS_LoadBG0Scr ( BgScDataSub,    0, sizeof(BgScDataSub));
	GXS_LoadOBJPltt( PlttDataObj,    0, sizeof(PlttDataObj));

    GX_DispOn();
    GXS_DispOn();
}

/*---------------------------------------------------------------------------*
  Name:         DrawLine

  Description:  三角ポリゴンで線を描画

  Arguments:    sx    - 描画する線の開始点のｘ座標
                sy    - 描画する線の開始点のｙ座標
                ex    - 描画する線の終点のｘ座標
                ey    - 描画する線の終点のｙ座標
				color - 描画する線の色

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
DrawLine(s16 sx, s16 sy, s16 ex, s16 ey, GXRgb color)
{
    fx16    fsx = (fx16)(((sx - 128) * 0x1000) / 128);
    fx16    fsy = (fx16)(((96 - sy) * 0x1000) / 96);
    fx16    fex = (fx16)(((ex - 128) * 0x1000) / 128);
    fx16    fey = (fx16)(((96 - ey) * 0x1000) / 96);

    G3_Begin(GX_BEGIN_TRIANGLES);
    {
        G3_Color( color );
        G3_Vtx(fsx, fsy, 0);
        G3_Color( color );
        G3_Vtx(fex, fey, 0);
        G3_Color( color );
        G3_Vtx(fsx, fsy, 1);
    }
    G3_End();
}
