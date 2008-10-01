/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     keypad.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef KAMI_NNS_FONT_H_
#define KAMI_NNS_FONT_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <twl.h>
#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/load/g2d_NFT_load.h> 
#include <nnsys/g2d/g2di_Char.h>

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

// 文字列描画関数が扱う文字をUnicodeにする
#define NNS_G2D_UNICODE

#define CANVAS_WIDTH        32      // 文字描画域の幅    (キャラクタ単位)
#define CANVAS_HEIGHT       24      // 文字描画域の高さ  (キャラクタ単位)
#define CANVAS_LEFT         0       // 文字描画域の位置X (キャラクタ単位)
#define CANVAS_TOP          0       // 文字描画域の位置Y (キャラクタ単位)

#define TEXT_HSPACE         1       // 文字列描画時の文字間 (ピクセル単位)
#define TEXT_VSPACE         1       // 文字列描画時の行間   (ピクセル単位)

#define CHARACTER_OFFSET    1       // 使用するキャラクタ列の開始番号

// DrawText での左上寄せ
#define TXT_DRAWTEXT_FLAG_DEFAULT   (NNS_G2D_VERTICALORIGIN_TOP | NNS_G2D_HORIZONTALORIGIN_LEFT | NNS_G2D_HORIZONTALALIGN_LEFT)

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

// TXTColorPalette の色名 16色パレットへのロードを想定
enum
{
    // パレット0 TXT_CPALETTE_MAIN
    TXT_COLOR_WHITE=0,											// 0
	TXT_COLOR_BLACK,											// 1
    TXT_COLOR_GREEN,											// 2
    TXT_COLOR_RED,												// 3
    TXT_COLOR_WHITE_BASE_START,									// 4
    TXT_COLOR_WHITE_BASE,										// 5
	TXT_COLOR_BLACK_BASE_START = TXT_COLOR_WHITE_BASE_START+4,	// 8
	TXT_COLOR_BLACK_BASE = TXT_COLOR_WHITE_BASE+4,				// 9
	TXT_COLOR_FREE_BASE_START = TXT_COLOR_BLACK_BASE_START+4,	// 12
	TXT_COLOR_FREE_BASE = TXT_COLOR_BLACK_BASE+4				// 13
};

/*---------------------------------------------------------------------------*
    外部変数宣言
 *---------------------------------------------------------------------------*/

extern NNSG2dFont          gFont;          // フォント
extern NNSG2dCharCanvas    gCanvas;        // CharCanvas
extern NNSG2dTextCanvas    gTextCanvas;    // TextCanvas
extern NNSG2dCharCanvas    gCanvas2;       // CharCanvas
extern NNSG2dTextCanvas    gTextCanvas2;   // TextCanvas

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

void InitFont(void);
void UpdateFreePltt(u16 color);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* KAMI_NNS_FONT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
