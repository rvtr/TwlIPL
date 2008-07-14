// from main.c


void myDEMOInit( void )
{
	DEMOInitCommon();
	DEMOInitVRAM();
	DEMOInitDisplayBitmap();
	DEMOHookConsole();
	DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
	DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
	DEMOStartDisplay();
}

/*---------------------------------------------------------------------------*
  Name:		 InitScreen

  Description:  BG 面を設定します。

  Arguments:	なし。

  Returns:	  なし。
 *---------------------------------------------------------------------------*/
static void InitScreen(void)
{
	GXRgb myPalette[16] =
	{
		GX_RGB( 0,  0,  0), GX_RGB(31, 31, 31), GX_RGB( 31,  31,  0), GX_RGB(31,  0,  0),
		GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),
		GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),
		GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),
	};
	
	// BG 1 を設定
	G2S_SetBG1Control(
		GX_BG_SCRSIZE_TEXT_256x256,	 // スクリーンサイズ 256x256
		GX_BG_COLORMODE_16,			 // カラーモード	 16色
		GX_BG_SCRBASE_0x0000,		   // スクリーンベース
		GX_BG_CHARBASE_0x00000,		 // キャラクタベース
		GX_BG_EXTPLTT_01				// 拡張パレットスロット
	);

	// BG1 を可視に
	GXS_SetVisiblePlane(GX_GetVisiblePlane() & ~GX_PLANEMASK_BG1);

	// カラーパレットを設定
	GX_LoadBGPltt( myPalette, 0, sizeof(myPalette) );
}

/*---------------------------------------------------------------------------*
  Name:		 InitCanvas

  Description:  文字列描画の初期化をします。

  Arguments:	なし。

  Returns:	  なし。
 *---------------------------------------------------------------------------*/
static void InitCanvas(void)
{

	// BGキャラクタベースへのポインタ
	GXCharFmt16* const pCharBase = (GXCharFmt16*)G2_GetBG1CharPtr();
	int cOffset = CHARACTER_OFFSET;
	
	TXT_LoadFont(&gFont, FONT_RESOURCE);

	// CharCanvas の初期化と BG スクリーンの設定
	// CharCanvas 0
	NNS_G2dCharCanvasInitForBG(
		&gCanvas[0],				// CharCanvasへのポインタ
		pCharBase + cOffset,		// 使用するキャラクタ列先頭へのポインタ
		CANVAS0_WIDTH,				// CharCanvas幅
		CANVAS0_HEIGHT,				// CharCanvas高さ
		NNS_G2D_CHARA_COLORMODE_16	// カラーモード
	);
	NNS_G2dMapScrToCharText(
		G2S_GetBG1ScrPtr(),			// スクリーンベースへのポインタ
		CANVAS0_WIDTH,				// CharCanvas幅
		CANVAS0_HEIGHT,				// CharCanvas高さ
		CANVAS0_LEFT,				// CharCanvas表示位置
		CANVAS0_TOP,				// CharCanvas表示位置
		NNS_G2D_TEXT_BG_WIDTH_256,	// スクリーン幅
		cOffset,					// 使用するキャラクタ列先頭のキャラクタ番号
		0			// カラーパレット番号
	);

	// CharCanvas 0 が使う分のキャラクタ数をオフセットに加算
	cOffset += CANVAS0_WIDTH * CANVAS0_HEIGHT;

	// CharCanvas 1
	NNS_G2dCharCanvasInitForBG(
		&gCanvas[1],
		pCharBase + cOffset,
		CANVAS1_WIDTH,
		CANVAS1_HEIGHT,
		NNS_G2D_CHARA_COLORMODE_16
	);
	NNS_G2dMapScrToCharText(
		G2S_GetBG1ScrPtr(),
		CANVAS1_WIDTH,
		CANVAS1_HEIGHT,
		CANVAS1_LEFT,
		CANVAS1_TOP,
		NNS_G2D_TEXT_BG_WIDTH_256,
		cOffset,
		0
	);

	// CharCanvas 1 が使う分のキャラクタ数をオフセットに加算
	cOffset += CANVAS1_WIDTH * CANVAS1_HEIGHT;

  
	// TextCanvas の初期化
	// CharCanvas 毎に TextCanvas を用意することもできますし、
	// 複数の TextCanvas で1つの CharCanvas を共有する事もできます。
	// 複数の CharCanvas を1つの TextCanvas で使う場合は
	// TextCanvas の CharCanvas を置き換えながら使う事になります。

	// 1) 複数の TextCanvas で1つの CharCanvas を共有
	// CharCanvas 1 を TextCanvas 0, 1 で共有
	{
		NNS_G2dTextCanvasInit(
			&gTextCanvas[0],	// TextCanvasへのポインタ
			&gCanvas[0],		// 描画先のCharCanvasへのポインタ
			(NNSG2dFont*)s_pFontBuffer,		// 描画に用いるフォントへのポインタ
			TEXT_HSPACE,		// 文字間
			TEXT_VSPACE		// 行間
		);
	}

	// 2) CharCanvas 毎に TextCanvas を用意
	// CharCanvas 2 を TextCanvas 2 単独で使用
	{
		NNS_G2dTextCanvasInit(
			&gTextCanvas[1],
			&gCanvas[1],
			(NNSG2dFont*)s_pFontBuffer,
			TEXT_HSPACE,
			TEXT_VSPACE
		);
	}
}

