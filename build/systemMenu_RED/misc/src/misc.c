/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     misc.c

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

// define data-----------------------------------------------------------------
#define NTR_IPL_FONT_DATA			"data/NTR_IPL_font_m.NFTR"

#define STRING_LENGTH_MAX			256

#define GRAY(x) GX_RGB(x, x, x)

enum
{
    TXT_CPALETTE_MAIN,
    TXT_CPALETTE_USERCOLOR,
    TXT_CPALETTE_USERCOLORSAMPLE,
    TXT_CPALETTE_4BPP,
    TXT_NUM_CPALEETE
};

// デモ共通のカラーパレット
GXRgb TXTColorPalette[TXT_NUM_CPALEETE * 16] =
{
    GX_RGB(31, 31, 31), GX_RGB(31, 31, 31), GX_RGB( 0,  0,  0), GX_RGB(31,  0,  0),// 基本文字色
    GX_RGB( 0, 31,  0), GX_RGB( 0,  0, 31), GX_RGB( 0, 31, 31), GX_RGB(31,  0, 31),
    GX_RGB(31, 31,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),
    GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),

    GX_RGB( 0,  0,  0), GX_RGB(12, 16, 19), GX_RGB(23,  9,  0), GX_RGB(31,  0,  3),// ユーザーカラー領域
    GX_RGB(31, 17, 31), GX_RGB(31, 18,  0), GX_RGB(30, 28,  0), GX_RGB(21, 31,  0),
    GX_RGB( 0, 20,  7), GX_RGB( 9, 27, 17), GX_RGB( 6, 23, 30), GX_RGB( 0, 11, 30),
    GX_RGB( 0,  0, 18), GX_RGB(17,  0, 26), GX_RGB(26,  0, 29), GX_RGB(31,  0, 18),

    GX_RGB(12, 16, 19), GX_RGB(23,  9,  0), GX_RGB(31,  0,  3), GX_RGB(31, 17, 31),// ユーザーカラーのサンプル領域
    GX_RGB(31, 18,  0), GX_RGB(30, 28,  0), GX_RGB(21, 31,  0), GX_RGB( 0, 31,  0),
    GX_RGB( 0, 20,  7), GX_RGB( 9, 27, 17), GX_RGB( 6, 23, 30), GX_RGB( 0, 11, 30),
    GX_RGB( 0,  0, 18), GX_RGB(17,  0, 26), GX_RGB(26,  0, 29), GX_RGB(31,  0, 18),

    GRAY(31),           GRAY(29),           GRAY(27),           GRAY(25),
    GRAY(23),           GRAY(21),           GRAY(19),           GRAY(17),
    GRAY(15),           GRAY(14),           GRAY(12),           GRAY(10),
    GRAY( 8),           GRAY( 6),           GRAY( 3),           GRAY( 0),
};

// グラデパレットを作るためのマクロ
#define MAKECOLOR(r,g,b)	GX_RGB( 0,  0,  0), GX_RGB(r, g, b), MC(r,g,b,1), MC(r,g,b,2),\
							MC(r,g,b,3), MC(r,g,b,4), MC(r,g,b,5), MC(r,g,b,6),\
							MC(r,g,b,7), MC(r,g,b,8), MC(r,g,b,9), MC(r,g,b,10),\
							MC(r,g,b,11), MC(r,g,b,12), MC(r,g,b,13), GX_RGB(31, 31, 31)

#define MC(r,g,b,i)		GX_RGB(r+((31-r)*i)/14, g+((31-g)*i)/14, b+((31-b)*i)/14)

// ユーザーカラーパレット切り替え用
GXRgb TempPalette[16 * 16] =
{
	MAKECOLOR(12, 16, 19),
	MAKECOLOR(23,  9,  0),
	MAKECOLOR(31,  0,  3),
	MAKECOLOR(31, 17, 31),
	MAKECOLOR(31, 18,  0),
	MAKECOLOR(30, 28,  0),
	MAKECOLOR(21, 31,  0),
	MAKECOLOR( 0, 31,  0),
	MAKECOLOR( 0, 20,  7),
	MAKECOLOR( 9, 27, 17),
	MAKECOLOR( 6, 23, 30),
	MAKECOLOR( 0, 11, 30),
	MAKECOLOR( 0,  0, 18),
	MAKECOLOR(17,  0, 26),
	MAKECOLOR(26,  0, 29),
	MAKECOLOR(31,  0, 18),
};

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------
#define CANVAS_WIDTH        32      // 文字描画域の幅    (キャラクタ単位)
#define CANVAS_HEIGHT       24      // 文字描画域の高さ  (キャラクタ単位)
#define CANVAS_LEFT         0       // 文字描画域の位置X (キャラクタ単位)
#define CANVAS_TOP          0       // 文字描画域の位置Y (キャラクタ単位)

#define TEXT_HSPACE         1       // 文字列描画時の文字間 (ピクセル単位)
#define TEXT_VSPACE         1       // 文字列描画時の行間   (ピクセル単位)

#define CHARACTER_OFFSET    0       // 使用するキャラクタ列の開始番号

// RTCデータ表示位置ワーク
typedef struct RtcDrawPos{
	int date_x;
	int date_y;
	int time_x;
	int time_y;
}RtcDrawPos;

// function's prototype-------------------------------------------------------
static BOOL WaitDetachTP( void );
static void StartDetachTP( void );
static void InitScreen( void );
static void InitCanvas( void );

// global variable-------------------------------------------------------------
KeyWork					pad;			// キーパッド入力データ
TpWork					tpd;			// タッチパネル入力データ
NNSFndAllocator			g_allocator;	// メモリアロケータ
NNSG2dFont              gFont;          // フォント
NNSG2dCharCanvas        gCanvas;        // CharCanvas
NNSG2dTextCanvas        gTextCanvas;    // TextCanvas
NNSG2dCharCanvas        gCanvasSub;     // CharCanvas
NNSG2dTextCanvas        gTextCanvasSub; // TextCanvas

// static variable-------------------------------------------------------------
static int        s_detach_count;
static RtcDrawPos s_rtcPos;
static RTCDate    s_rtcDate;
static RTCTime    s_rtcTime;
static u16        s_vcount;

static char s_strBuffer[ STRING_LENGTH_MAX * 2 ] ATTRIBUTE_ALIGN(2);
static u16  s_strBufferUTF16[ STRING_LENGTH_MAX ];

// const data------------------------------------------------------------------

// 曜日データ表示用文字コード
const u8 *const g_strWeek[] ATTRIBUTE_ALIGN(2) = {
	(const u8 *)"SUN",
	(const u8 *)"MON",
	(const u8 *)"TUE",
	(const u8 *)"WED",
	(const u8 *)"THU",
	(const u8 *)"FRI",
	(const u8 *)"SAT",
};


// ============================================================================
// メモリ管理
// ============================================================================

// アロケータの初期化
void InitAllocator( void )
{
	NNSFndAllocator	*pAllocator = &g_allocator;
    u32   arenaLow      = MATH_ROUNDUP  ( (u32)OS_GetMainArenaLo(), 16 );
    u32   arenaHigh     = MATH_ROUNDDOWN( (u32)OS_GetMainArenaHi(), 16 );
    u32   heapSize      = arenaHigh - arenaLow;
    void* heapMemory    = OS_AllocFromMainArenaLo( heapSize, 16 );
    NNSFndHeapHandle    heapHandle;
    SDK_NULL_ASSERT( pAllocator );
    
    heapHandle = NNS_FndCreateExpHeap( heapMemory, heapSize );
    SDK_ASSERT( heapHandle != NNS_FND_HEAP_INVALID_HANDLE );
	
    NNS_FndInitAllocatorForExpHeap( pAllocator, heapHandle, 32 );
}


// メモリ割り当て
void *Alloc( u32 size )
{
	return NNS_FndAllocFromAllocator( &g_allocator, size );
}


// メモリ解放
void Free( void *pBuffer )
{
	NNS_FndFreeToAllocator( &g_allocator, pBuffer );
}


// ============================================================================
// 画面設定
// ============================================================================

// BG初期化
void InitBG(void)
{
	// 画面OFF
	GX_DispOff();
	GXS_DispOff();
	
	// VRAMの割り当てを全て解除
	GX_DisableBankForBG();
	GX_DisableBankForOBJ();
	GX_DisableBankForSubBG();
	GX_DisableBankForSubOBJ();
	
	// メイン2Dエンジンの出力を下画面に
	GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
	
	// メインLCD
	{
		// VRAM割り当て
		GX_SetBankForBG ( GX_VRAM_BG_128_A );						
		GX_SetBankForOBJ( GX_VRAM_OBJ_128_B );
		
		MI_CpuClearFast( (void *)HW_BG_VRAM,   0x20000 );			// BG -VRAM クリア
		MI_CpuClearFast( (void *)HW_OBJ_VRAM,  0x20000 );			// OBJ-VRAM クリア
		
		// カラーパレットを設定
	    GX_LoadBGPltt( TXTColorPalette, 0, sizeof(TXTColorPalette) );
		
		// BGモード設定
	    GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );
		
	    GX_SetBGScrOffset ( GX_BGSCROFFSET_0x10000 );
	    GX_SetBGCharOffset( GX_BGCHAROFFSET_0x00000 );
	}
	// サブLCD
	{
		// VRAM割り当て
	    GX_SetBankForSubBG ( GX_VRAM_SUB_BG_128_C );               	// VRAM-C for BGs
	    GX_SetBankForSubOBJ( GX_VRAM_SUB_OBJ_128_D );              	// VRAM-D for BGs
		
		MI_CpuClearFast( (void *)HW_DB_BG_VRAM,  0x20000 );			// BG -VRAM クリア
		MI_CpuClearFast( (void *)HW_DB_OBJ_VRAM, 0x20000 );			// OBJ -VRAM クリア
		
		// カラーパレットを設定
	    GXS_LoadBGPltt( TXTColorPalette, 0, sizeof(TXTColorPalette) );
		
		// BGモード設定
	    GXS_SetGraphicsMode( GX_BGMODE_0 );                       	// BGMODE is 0
	}
	InitScreen();
    InitCanvas();
}


// スクリーン初期化
static void InitScreen( void )
{
	// メイン画面 BG 0 を設定
    G2_SetBG0Control(
        GX_BG_SCRSIZE_TEXT_256x256,     // スクリーンサイズ 256x256
        GX_BG_COLORMODE_256,             // カラーモード     256色
        GX_BG_SCRBASE_0xf800,           // スクリーンベース
        GX_BG_CHARBASE_0x00000,         // キャラクタベース
        GX_BG_EXTPLTT_01                // 拡張パレットスロット
    );
	G2_SetBG1Control(
		GX_BG_SCRSIZE_TEXT_256x256,				// 256pix x 256pix text
		GX_BG_COLORMODE_16,					// use 16 colors mode
		GX_BG_SCRBASE_0xe800,					// screen base offset + 0x0000 is the address for BG #1 screen
		GX_BG_CHARBASE_0x10000,					// character base offset + 0x04000 is the address for BG #0 characters
		GX_BG_EXTPLTT_01 						// use BGExtPltt slot #0 if BGExtPltt is enabled
	);
	G2_SetBG2ControlText(
		GX_BG_SCRSIZE_TEXT_256x256,				// 256pix x 256pix text
		GX_BG_COLORMODE_16,					// use 16 colors mode
		GX_BG_SCRBASE_0xd800,					// screen base offset + 0x0000 is the address for BG #1 screen
		GX_BG_CHARBASE_0x14000					// character base offset + 0x04000 is the address for BG #0 characters
	);
    GX_SetVisiblePlane( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1);
    G2_SetBG2Priority(0);
    G2_SetBG0Priority(1);
    G2_SetBG1Priority(2);
    	
	// サブ画面 BG 0 を設定
	G2S_SetBG0Control(
		GX_BG_SCRSIZE_TEXT_256x256,				// 256pix x 256pix text
		GX_BG_COLORMODE_16,						// use 16 colors mode
		GX_BG_SCRBASE_0xf800,					// screen base offset + 0x0000 is the address for BG #0 screen
		GX_BG_CHARBASE_0x00000,					// character base offset + 0x04000 is the address for BG #0 characters
		GX_BG_EXTPLTT_01 						// use BGExtPltt slot #0 if BGExtPltt is enabled
	);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );					// display only BG #0
}


// 文字列描画の初期化
static void InitCanvas( void )
{
	CMN_InitFileSystem( &g_allocator );
	
    // フォントを読み込みます
	{
		void* pFontFile;
		u32 size = CMN_LoadFile( &pFontFile, NTR_IPL_FONT_DATA, &g_allocator);
		NNS_G2D_ASSERT( size > 0 );
		NNS_G2dFontInitUTF16(&gFont, pFontFile);
//		NNS_G2dPrintFont(&gFont);
	}
	
	{
		// CharCanvas の初期化
		NNS_G2dCharCanvasInitForBG(
			&gCanvas,
			(GXCharFmt256*)G2_GetBG0CharPtr() + CHARACTER_OFFSET,
			CANVAS_WIDTH,
			CANVAS_HEIGHT,
			NNS_G2D_CHARA_COLORMODE_256
		);
		
		// TextCanvasの初期化
		NNS_G2dTextCanvasInit(
			&gTextCanvas,
			&gCanvas,
			&gFont,
			TEXT_HSPACE,
			TEXT_VSPACE
		);
		
		// スクリーンを設定
		NNS_G2dMapScrToCharText(
			G2_GetBG0ScrPtr(),
			CANVAS_WIDTH,
			CANVAS_HEIGHT,
			CANVAS_LEFT,
			CANVAS_TOP,
			NNS_G2D_TEXT_BG_WIDTH_256,
			CHARACTER_OFFSET,
			TXT_CPALETTE_MAIN
		);
	}
	{
		// CharCanvas の初期化
		NNS_G2dCharCanvasInitForBG(
			&gCanvasSub,
			(GXCharFmt16*)G2S_GetBG0CharPtr() + CHARACTER_OFFSET,
			CANVAS_WIDTH,
			CANVAS_HEIGHT,
			NNS_G2D_CHARA_COLORMODE_16
		);
		
		// TextCanvasの初期化
		NNS_G2dTextCanvasInit(
			&gTextCanvasSub,
			&gCanvasSub,
			&gFont,
			TEXT_HSPACE,
			TEXT_VSPACE
		);
		
		// スクリーンを設定
		NNS_G2dMapScrToCharText(
			G2S_GetBG0ScrPtr(),
			CANVAS_WIDTH,
			CANVAS_HEIGHT,
			CANVAS_LEFT,
			CANVAS_TOP,
			NNS_G2D_TEXT_BG_WIDTH_256,
			CHARACTER_OFFSET,
			TXT_CPALETTE_MAIN
		);
	}
}


// UTF16での直接文字表示
void PutStringUTF16( int x, int y, int color, const u16 *strUTF16 )
{
	NNS_G2dTextCanvasDrawText( &gTextCanvas, x, y, color, TXT_DRAWTEXT_FLAG_DEFAULT,
							   strUTF16 );
}

void PutStringUTF16Sub( int x, int y, int color, const u16 *strUTF16 )
{
	NNS_G2dTextCanvasDrawText( &gTextCanvasSub, x, y, color, TXT_DRAWTEXT_FLAG_DEFAULT,
							   strUTF16 );
}


// 書式付きで作成した文字列を表示した場合の幅を取得する
int GetPrintfWidth( const NNSG2dTextCanvas *pCanvas, const char *fmt, ... )
{
	int srcLen;
	int dstLen = sizeof(s_strBufferUTF16);
	va_list vlist;
    va_start(vlist, fmt);
	srcLen = STD_TVSNPrintf( s_strBuffer, sizeof(s_strBuffer), fmt, vlist);
    va_end(vlist);
	s_strBuffer[ srcLen ] = 0;
	
	(void)STD_ConvertStringSjisToUnicode( s_strBufferUTF16, &dstLen, s_strBuffer, &srcLen, NULL );
	s_strBufferUTF16[ dstLen ] = 0;
	
	return NNS_G2dTextCanvasGetTextWidth( pCanvas, s_strBufferUTF16 );
}


// SJISでPrintf形式で文字表示（内部でUTF16に変換)
void PrintfSJIS( int x, int y, int color, const char *fmt, ... )
{
	int srcLen;
	int dstLen = sizeof(s_strBufferUTF16);
	va_list vlist;
    va_start(vlist, fmt);
	srcLen = STD_TVSNPrintf( s_strBuffer, sizeof(s_strBuffer), fmt, vlist);
    va_end(vlist);
	s_strBuffer[ srcLen ] = 0;
	
	(void)STD_ConvertStringSjisToUnicode( s_strBufferUTF16, &dstLen, s_strBuffer, &srcLen, NULL );
	s_strBufferUTF16[ dstLen ] = 0;
	
	NNS_G2dTextCanvasDrawText(&gTextCanvas, x, y, color, TXT_DRAWTEXT_FLAG_DEFAULT,
							  s_strBufferUTF16 );
}

void PrintfSJISSub( int x, int y, int color, const char *fmt, ... )
{
	int srcLen;
	int dstLen = sizeof(s_strBufferUTF16);
	va_list vlist;
    va_start(vlist, fmt);
	srcLen = STD_TVSNPrintf( s_strBuffer, sizeof(s_strBuffer), fmt, vlist);
    va_end(vlist);
	s_strBuffer[ srcLen ] = 0;
	
	(void)STD_ConvertStringSjisToUnicode( s_strBufferUTF16, &dstLen, s_strBuffer, &srcLen, NULL );
	s_strBufferUTF16[ dstLen ] = 0;
	
	NNS_G2dTextCanvasDrawText(&gTextCanvasSub, x, y, color, TXT_DRAWTEXT_FLAG_DEFAULT,
							  s_strBufferUTF16 );
}


// キー入力読み出し--------------------------------
void ReadKeyPad(void)
{
	u16 readData = PAD_Read();
	pad.trg	 = (u16)(readData & (readData ^ pad.cont));				// トリガ 入力
	pad.cont = readData;											//   ベタ 入力
}


// タッチパネルデータの取得-----------------------
void ReadTP(void)
{
	TP_GetCalibratedPoint( &tpd.last, &tpd.raw );					// 前回のTPデータを退避
	
	if( TP_RequestRawSampling(&tpd.raw) ) {							// タッチパネルのサンプリング
		SVC_CpuClear(0x0000, &tpd.raw, sizeof(tpd.raw), 16);		// SPI-busyでデータ取得に失敗した時は”データなし”でリターン。
		return;
	}
	TP_GetCalibratedPoint( &tpd.disp, &tpd.raw );					// TP座標からLCD座標に変換。
	
	if( !WaitDetachTP() ) {											// TPデタッチ待ちを行う。
		SVC_CpuClear(0x0000, &tpd.disp, sizeof(tpd.disp), 16);		// SPI-busyでデータ取得に失敗した時は”データなし”でリターン。
		return;
	}
#if 1
	if(tpd.disp.touch) {											// 現在のTPデータを表示
		switch ( tpd.disp.validity ) {
			case TP_VALIDITY_VALID:
				OS_Printf("(  %3d,  %3d ) -> (  %3d,  %3d )\n", tpd.raw.x, tpd.raw.y, tpd.disp.x, tpd.disp.y);
				break;
			case TP_VALIDITY_INVALID_X:
				OS_Printf("( *%3d,  %3d ) -> ( *%3d,  %3d )\n", tpd.raw.x, tpd.raw.y, tpd.disp.x, tpd.disp.y);
				break;
			case TP_VALIDITY_INVALID_Y:
				OS_Printf("(  %3d, *%3d ) -> (  %3d, *%3d )\n", tpd.raw.x, tpd.raw.y, tpd.disp.x, tpd.disp.y);
				break;
			case TP_VALIDITY_INVALID_XY:
				OS_Printf("( *%3d, *%3d ) -> ( *%3d, *%3d )\n", tpd.raw.x, tpd.raw.y, tpd.disp.x, tpd.disp.y);
				break;
		}
	}
#endif
}


// TPデタッチを待つ
static BOOL WaitDetachTP( void )
{
	// s_detach_countが始動していたら、カウント判定。
	if(s_detach_count > 0) {
		if(tpd.disp.touch == 0) {									// TPが押されていなければ、カウント進行し規定値で再入力を受け付ける。
			s_detach_count--;
		}else {
			s_detach_count = TP_CSR_DETACH_COUNT;
		}
		return FALSE;
	}
	return TRUE;
}


// TPデタッチ待ちの開始
static void StartDetachTP( void )
{
	s_detach_count = TP_CSR_DETACH_COUNT;
}


//======================================================
// メニュー制御
//======================================================

// メニュー描画
void DrawMenu( u16 nowCsr, const MenuParam *pMenu )
{
	int i;
	int color;
	
	for( i = 0; i < pMenu->num; i++ ) {
		if(i == nowCsr)	{
			if( !pMenu->pos[ i ].enable ) {
				color = pMenu->disable_color;
			}else {
				color = pMenu->select_color;
			}
		}else {
			color = pMenu->normal_color;
		}
		PutStringUTF16( pMenu->pos[ i ].x, pMenu->pos[ i ].y, color, (pMenu->str_elem)[ i ] );
	}
}

// タッチパネルによる選択を行う関数・汎用版。
// 主にチャタリングを吸収する程度の機能を持つ。
// SelectSomethingFunc型の関数ポインタfuncには、次に示す条件を満たす関数を作り、リストにしてセットして使う。
// １．要素上でタッチしていればTRUEを返し、そうでなければFALSEを返す。
// ２．第一引数に与えたポインタの指す変数にカーソル位置を入れて返す。
BOOL SelectSomethingByTP( u16 *nowCsr, SelectSomethingFunc func[], int funcnum )
{
	int i;
	static	u16 detach_count	= 0;
	static 	u16 csr_old			= 0xffff;
	static  u16 same_csr_count	= 0;
	
	// detach_countが始動していたら、カウント判定。
	if( detach_count > 0 ) {
		if( tpd.disp.touch == 0 ) {									// TPが押されていなければ、カウント進行し、TP_CSR_DETACH_COUNTカウントでメニュー選択
			if( ++detach_count == TP_CSR_DETACH_COUNT ) {
				detach_count = 0;
				same_csr_count = 0;
				*nowCsr = csr_old;
				return TRUE;
			}else {
				return FALSE;
			}
		}
	}
	detach_count=0;													// detachカウント値のクリア

	for( i = 0; i < funcnum; i++ ) {
		if( tpd.disp.touch ) {										// タッチパネルがタッチされているなら、
			u16 csr;
			if( func[i]( &csr, &tpd.disp ) ) {									// funcは要素上にタッチされていればTRUEを返し、カーソル位置も返してくれる関数
				OS_TPrintf( "InRange\n" );
				if( tpd.disp.validity == TP_VALIDITY_VALID ) {		// カーソルをその要素に移動
					if( csr_old == csr ) {
						if( same_csr_count < TP_CSR_TOUCH_COUNT ) {
							same_csr_count++;
						}
						if( same_csr_count == TP_CSR_TOUCH_COUNT ) {
							*nowCsr = csr;
						}
						return FALSE;
					}else {
						csr_old = csr;
					}
					break;
				}
			}else {
				OS_TPrintf( "OutRange\n" );
			}
		}else {	// touch==0
			if( same_csr_count == TP_CSR_TOUCH_COUNT ) {
				detach_count = 1;
				break;
			}
		}
	}
	same_csr_count = 0;
	return FALSE;
}

// タッチパネルによるメニュー選択
BOOL SelectMenuByTP( u16 *nowCsr, const MenuParam *pMenu )
{
	u16		i;
	static	u16 detach_count	= 0;
	static 	u16 csr_old			= 0xff;
	static  u16 same_csr_count	= 0;
	
	// detach_countが始動していたら、カウント判定。
	if( detach_count > 0 ) {
		if( tpd.disp.touch == 0 ) {									// TPが押されていなければ、カウント進行し、１０カウントでメニュー選択
			if( ++detach_count == TP_CSR_DETACH_COUNT ) {
				detach_count = 0;
				return TRUE;
			}else {
				return FALSE;
			}
		}
	}
	detach_count=0;													// detachカウント値のクリア
	
	for( i = 0; i < pMenu->num; i++ ) {
		if( tpd.disp.touch ) {										// タッチパネルがメニューの要素上でタッチされているなら、
			NNSG2dTextRect rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, (pMenu->str_elem)[ i ] );
			u16 top_x = (u16)( pMenu->pos[ i ].x );					// メニュー要素のLCD座標を算出
			u16 top_y = (u16)( pMenu->pos[ i ].y );
			u16 bottom_x = (u16)( top_x + rect.width );
			u16 bottom_y = (u16)( top_y + rect.height );
			
			OS_TPrintf( "MENU[ %d ] : top_x = %02d  top_y = %02d  bot_x = %02d  bot_y = %02d : ",
						i, top_x, top_y, bottom_x, bottom_y );
			
			if( WithinRangeTP( top_x, top_y, bottom_x, bottom_y, &tpd.disp ) ) {
				OS_TPrintf( "InRange\n" );
				if( tpd.disp.validity == TP_VALIDITY_VALID ) {		// カーソルをその要素に移動
					if( csr_old == i ) {
						if( same_csr_count < TP_CSR_TOUCH_COUNT ) {
							same_csr_count++;
						}
						if( same_csr_count == TP_CSR_TOUCH_COUNT ) {
							*nowCsr = i;
						}
						return FALSE;
					}else {
						csr_old = i;
					}
					break;
				}
			}else {
				OS_TPrintf( "OutRange\n" );
			}
		}else {	// touch==0
			if( same_csr_count == TP_CSR_TOUCH_COUNT ) {
				detach_count = 1;
				break;
			}
		}
	}
	same_csr_count = 0;
	return FALSE;
}


// 現在のタッチパネル座標が指定領域内にあるかどうかを返す。
BOOL WithinRangeTP( int top_x, int top_y, int bottom_x, int bottom_y, TPData *tgt )
{
	if( ( tgt->x >= top_x    ) &&
		( tgt->x <= bottom_x ) &&
		( tgt->y >= top_y    ) &&
		( tgt->y <= bottom_y ) ) {
		OS_TPrintf( "\nRANGE : tx=%3d ty=%3d bx=%3d by=%3d : x=%3d y=%3d\n",
					top_x, top_y, bottom_x, bottom_y, tgt->x, tgt->y );
		return TRUE;
	}else {
		return FALSE;
	}
}


// バナーアイコンOBJのロード
void SetBannerIconOBJ( GXOamAttr *pDstOAM, BannerFileV1 *bannerp )
{
	GXS_LoadOBJPltt( bannerp->pltt, 15, BANNER_PLTT_SIZE );
	MI_CpuCopyFast(  bannerp->image, (void *)(HW_DB_OBJ_VRAM + 0x20), BANNER_IMAGE_SIZE );
	G2_SetOBJAttr(  pDstOAM,										// OAM pointer
					32,												// X position
					32,												// Y position
					0,												// Priority
					GX_OAM_MODE_NORMAL,								// Bitmap mode
					FALSE,											// mosaic off
					GX_OAM_EFFECT_NONE,								// affine off
					GX_OAM_SHAPE_32x32,								// 16x16 size
					GX_OAM_COLOR_16,								// 16 color
					1,												// charactor
					15,												// palette
					0);												// affine
}


//===============================================
// RTCアクセスルーチン
//===============================================

// RTCデータの取得
BOOL GetRTCData( RTCDrawProperty *pRTCDraw, BOOL forceGetFlag )
{
	if( forceGetFlag || ( pRTCDraw->vcount++ == 59 ) ) {
		pRTCDraw->vcount = 0;
		MI_CpuCopy16( &pRTCDraw->date, &pRTCDraw->date_old, sizeof(RTCDate) );
		MI_CpuCopy16( &pRTCDraw->time, &pRTCDraw->time_old, sizeof(RTCTime) );
		(void)RTC_GetDateTime( &pRTCDraw->date, &pRTCDraw->time );
		return TRUE;
	}
	return FALSE;
}


// RTCデータの表示
void DrawRTCData( RTCDrawProperty *pRTCDraw )
{
	void (*pPrintFunc)( int x, int y, int color, const char *fmt, ... ) =
		( pRTCDraw->isTopLCD ) ? PrintfSJISSub : PrintfSJIS;
	
	// 前RTC情報の消去
	{
		u32 year = pRTCDraw->date_old.year + 2000;
		pPrintFunc( pRTCDraw->date_x, pRTCDraw->date_y, TXT_COLOR_WHITE, "%04d/%02d/%02d[%3s]",
					year,
					pRTCDraw->date_old.month,
					pRTCDraw->date_old.day,
					g_strWeek[ pRTCDraw->date_old.week ]
					);
		pPrintFunc( pRTCDraw->time_x, pRTCDraw->time_y, TXT_COLOR_WHITE, "%02d:%02d:%02d",
					pRTCDraw->time_old.hour,
					pRTCDraw->time_old.minute,
					pRTCDraw->time_old.second
					);
	}
	// RTC情報の表示
	{
		u32 year = pRTCDraw->date.year + 2000;
		pPrintFunc( pRTCDraw->date_x, pRTCDraw->date_y, TXT_COLOR_BLACK, "%d/%02d/%02d[%3s]",
					year,
					pRTCDraw->date.month,
					pRTCDraw->date.day,
					g_strWeek[ pRTCDraw->date.week ]
					);
		pPrintFunc( pRTCDraw->time_x, pRTCDraw->time_y, TXT_COLOR_BLACK, "%02d:%02d:%02d",
					pRTCDraw->time.hour,
					pRTCDraw->time.minute,
					pRTCDraw->time.second
					);
	}
}

// RTCデータの取得&表示
void GetAndDrawRTCData( RTCDrawProperty *pRTCDraw, BOOL forceGetFlag )
{
	if( GetRTCData( pRTCDraw, forceGetFlag ) ) {
		DrawRTCData( pRTCDraw );
	}
}

// ユーザーカラー領域の切り替え
void ChangeUserColor( u8 col )
{
	int l;
	if(col > 15) return;
	for(l=0;l<16;l++)
	{
		TXTColorPalette[16+l] = TempPalette[16 * col + l];
	}
	GX_LoadBGPltt( TXTColorPalette, 0, sizeof(TXTColorPalette) );
}
