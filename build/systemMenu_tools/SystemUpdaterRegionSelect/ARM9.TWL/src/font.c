/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     main.c

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

#include <twl.h>
#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/load/g2d_NFT_load.h> 
#include <nnsys/g2d/g2di_Char.h>
#include "font.h"
#include "kami_global.h"

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

/*---------------------------------------------------------------------------*
    グローバル変数定義
 *---------------------------------------------------------------------------*/

NNSG2dFont          gFont;          // フォント
NNSG2dCharCanvas    gCanvas;        // CharCanvas
NNSG2dCharCanvas    gCanvas2;       // CharCanvas
NNSG2dTextCanvas    gTextCanvas;    // TextCanvas
NNSG2dTextCanvas    gTextCanvas2;   // TextCanvas

/*---------------------------------------------------------------------------*
    内部定数定義
 *---------------------------------------------------------------------------*/
const int max_colors = 32;

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

// バックドロップ色(白) + 黒 + 緑 + 赤 + 最大フォント階調数 (32階調まで)
static u16 colorPalette[1 + 1 + 1 + 1 + max_colors*5] =
{
    GX_RGB(31, 31, 31), GX_RGB(0, 0, 0), GX_RGB(0, 31, 0), GX_RGB(31, 0, 0)
};

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

static u32 LoadFile(void** ppFile, const char* fpath);

/*---------------------------------------------------------------------------*
  Name:         LoadFile

  Description:  ファイルをメモリにロードします。
                ファイルデータが不要になった場合は
                CMN_UnloadFile( *ppFile, pAlloc ) でファイルデータを
                解放します。

  Arguments:    ppFile: ファイルをロードしたメモリアドレスを受け取る
                        バッファへのポインタ。
                fpath:  ロードするファイルのパス
                pAlloc: アロケータへのポインタ

  Returns:      ロードしたファイルのファイルサイズを返します。
                0 の場合はファイルロードに失敗した事を表します。
                この場合 *ppFile の値は無効です。
 *---------------------------------------------------------------------------*/
static u32 LoadFile(void** ppFile, const char* fpath)
{
    BOOL bSuccess;
    FSFile f;
    u32 length;
    u32 read;

    SDK_NULL_ASSERT( ppFile );
    SDK_NULL_ASSERT( fpath );

    FS_InitFile(&f);

    bSuccess = FS_OpenFile(&f, fpath);
    if( ! bSuccess )
    {
        OS_Warning("file (%s) not found", fpath);
        return 0;
    }

    length = FS_GetLength(&f);
    *ppFile = OS_AllocFromMain(length);
    if( *ppFile == NULL )
    {
        OS_Warning("cant allocate memory for file: %s", fpath);
        return 0;
    }

    read = (u32)FS_ReadFile(&f, *ppFile, (s32)length);
    if( read != length )
    {
        OS_Warning("fail to load file: %s", fpath);
        OS_FreeToMain(*ppFile);
        return 0;
    }

    bSuccess = FS_CloseFile(&f);
    if( ! bSuccess )
    {
        OS_Warning("fail to close file: %s", fpath);
    }

    return length;
}

/*---------------------------------------------------------------------------*
  Name:         InitCanvas

  Description:  文字列描画の初期化をします。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
static void InitCanvas(void)
{
    GXCharFmt16* pCharBase = (GXCharFmt16*)G2_GetBG1CharPtr();
    GXCharFmt16* pCharBase2 = (GXCharFmt16*)G2_GetBG2CharPtr();
    int cOffset = CHARACTER_OFFSET;

    // フォントを読み込みます
    {
//      他のデモと同じように TXT_LoadFont を使ってもフォントを読み込めます。
//      このデモでは NNS_G2dFontInitUTF16 を使う場合を示しています。
//        TXT_LoadFont( &gFont, "/data/fontu16.NFTR" );

        void* pFontFile;
        u32 size;

        size = LoadFile( &pFontFile, "/local/tbf_ww_s.NFTR" );
        NNS_G2D_ASSERT( size > 0 );

        NNS_G2dFontInitUTF16(&gFont, pFontFile);
        NNS_G2dPrintFont(&gFont);
    }

    // CharCanvas の初期化
    NNS_G2dCharCanvasInitForBG(
        &gCanvas,
        pCharBase + cOffset,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16
    );
    NNS_G2dCharCanvasInitForBG(
        &gCanvas2,
        pCharBase2 + cOffset,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16
    );

    // TextCanvasの初期化
    NNS_G2dTextCanvasInit(
        &gTextCanvas,
        &gCanvas,
        &gFont,
        TEXT_HSPACE,
        TEXT_VSPACE
    );
    NNS_G2dTextCanvasInit(
        &gTextCanvas2,
        &gCanvas2,
        &gFont,
        TEXT_HSPACE,
        TEXT_VSPACE
    );

    // スクリーンを設定
    NNS_G2dMapScrToCharText(
        G2_GetBG1ScrPtr(),
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        CANVAS_LEFT,
        CANVAS_TOP,
        NNS_G2D_TEXT_BG_WIDTH_256,
        CHARACTER_OFFSET,
        0
    );
    NNS_G2dMapScrToCharText(
        G2_GetBG2ScrPtr(),
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        CANVAS_LEFT,
        CANVAS_TOP,
        NNS_G2D_TEXT_BG_WIDTH_256,
        CHARACTER_OFFSET,
        0
    );

	// パレットをロード
	{
	    // ロードしたフォントに応じてカラーパレットを作成
	    {
	        const int nColors = MATH_IMin((1 << NNS_G2dFontGetBpp(&gFont)), max_colors);
	        int i;

			// 白地用パレット 31,20,10, 0
	        for( i = 0; i < nColors; ++i )
	        {
	            int level = ((nColors - 1 - i) * (max_colors - 1) / (nColors - 1));

	            colorPalette[i+TXT_COLOR_WHITE_BASE_START] = GX_RGB(level, level, level);
	        }

			// 黒地用パレット 0, 10, 20, 31
	        for( i = 0; i < nColors; ++i )
	        {
	            int level = ((i) * (max_colors - 1) / (nColors - 1));

	            colorPalette[i+TXT_COLOR_BLACK_BASE_START] = GX_RGB(level, level, level);
	        }

			// 水色地用パレット
	        for( i = 0; i < nColors; ++i )
	        {
	            int level = ((nColors - 1 - i) * (max_colors - 1) / (nColors - 1));

	            colorPalette[i+TXT_COLOR_FREE_BASE_START] = GX_RGB((int)(level*0.7), (int)(level*0.9), level);
	        }
	    }

	    // カラーパレットをロード
	    GX_LoadBGPltt(colorPalette, 0, sizeof(colorPalette));
	}
}

/*---------------------------------------------------------------------------*
  Name:         UpdateFreePltt

  Description:  

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void UpdateFreePltt(u16 color)
{
	const int nColors = MATH_IMin((1 << NNS_G2dFontGetBpp(&gFont)), max_colors);
	s32 i;

	// フリーパレット
    for( i = 0; i < nColors; ++i )
    {
	    int level = ((nColors - 1 - i) * (max_colors - 1) / (nColors - 1));

        colorPalette[i+TXT_COLOR_FREE_BASE_START] = 
			GX_RGB( (int)(level*((color & GX_RGBA_R_MASK)>>GX_RGBA_R_SHIFT)/32.0), 
					(int)(level*((color & GX_RGBA_G_MASK)>>GX_RGBA_G_SHIFT)/32.0), 
					(int)(level*((color & GX_RGBA_B_MASK)>>GX_RGBA_B_SHIFT)/32.0));
    }

    // カラーパレットをロード
    GX_LoadBGPltt(colorPalette, 0, sizeof(colorPalette));
}

/*---------------------------------------------------------------------------*
  Name:         InitFont

  Description:  

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void InitFont(void)
{
    InitCanvas();

	OS_WaitVBlankIntr();
    NNS_G2dCharCanvasClear(&gCanvas, TXT_COLOR_WHITE);
	OS_WaitVBlankIntr();
    NNS_G2dCharCanvasClear(&gCanvas2, TXT_COLOR_WHITE);
	OS_WaitVBlankIntr();
    NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0,   0, 256,  30);

    NNS_G2dTextCanvasDrawText(&gTextCanvas2, 55, 6,
        TXT_COLOR_BLACK_BASE, TXT_DRAWTEXT_FLAG_DEFAULT,
        (const char *)SYSTEM_UPDATER_NAME
    );
}
