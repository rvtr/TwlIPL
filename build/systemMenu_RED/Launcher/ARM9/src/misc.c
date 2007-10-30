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
    TXT_CPALETTE_4BPP,
    TXT_NUM_CPALEETE
};

// デモ共通のカラーパレット
GXRgb TXTColorPalette[TXT_NUM_CPALEETE * 16] =
{
    GX_RGB(31, 31, 31), GX_RGB(31, 31, 31), GX_RGB( 0,  0,  0), GX_RGB(31,  0,  0),
    GX_RGB( 0, 31,  0), GX_RGB( 0,  0, 31), GX_RGB( 0, 31, 31), GX_RGB(31,  0, 31),
    GX_RGB(31, 31,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),
    GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0), GX_RGB( 0,  0,  0),

    GX_RGB( 0,  0,  0), GX_RGB(12, 16, 19), GX_RGB(23,  9,  0), GX_RGB(31,  0,  3),
    GX_RGB(31, 17, 31), GX_RGB(31, 18,  0), GX_RGB(30, 28,  0), GX_RGB(21, 31,  0),
    GX_RGB( 0, 20,  7), GX_RGB( 9, 27, 17), GX_RGB( 6, 23, 30), GX_RGB( 0, 11, 30),
    GX_RGB( 0,  0, 18), GX_RGB(17,  0, 26), GX_RGB(26,  0, 29), GX_RGB(31,  0, 18),

    GRAY(31),           GRAY(29),           GRAY(27),           GRAY(25),
    GRAY(23),           GRAY(21),           GRAY(19),           GRAY(17),
    GRAY(15),           GRAY(14),           GRAY(12),           GRAY(10),
    GRAY( 8),           GRAY( 6),           GRAY( 3),           GRAY( 0),
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
static void InitScreen( void );
static void InitCanvas( void );
static void GetAndDrawRtcDataCore( BOOL forceGetFlag );

// global variable-------------------------------------------------------------
KeyWork		pad;													// キーパッド入力データ
TpWork		tpd;													// タッチパネル入力データ

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
// function's description
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
        GX_BG_COLORMODE_16,             // カラーモード     16色
        GX_BG_SCRBASE_0xf800,           // スクリーンベース
        GX_BG_CHARBASE_0x00000,         // キャラクタベース
        GX_BG_EXTPLTT_01                // 拡張パレットスロット
    );
    GX_SetVisiblePlane( GX_PLANEMASK_BG0 );
	
	// サブ画面 BG 0 を設定
	G2S_SetBG0Control(
		GX_BG_SCRSIZE_TEXT_256x256,				// 256pix x 256pix text
		GX_BG_COLORMODE_16,						// use 256 colors mode
		GX_BG_SCRBASE_0xf800,					// screen base offset + 0x0000 is the address for BG #0 screen
		GX_BG_CHARBASE_0x00000,					// character base offset + 0x04000 is the address for BG #0 characters
		GX_BG_EXTPLTT_01 						// use BGExtPltt slot #0 if BGExtPltt is enabled
	);
    GXS_SetVisiblePlane( GX_PLANEMASK_BG0 );					// display only BG #0
}


// 文字列描画の初期化
static void InitCanvas( void )
{
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
			(GXCharFmt16*)G2_GetBG0CharPtr() + CHARACTER_OFFSET,
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
void ReadTpData(void)
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
BOOL WaitDetachTP( void )
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
void StartDetachTP( void )
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


// タッチパネルによるメニュー選択
BOOL SelectMenuByTP( u16 *nowCsr, const MenuParam *pMenu )
{
	u16		i;
	TPData *target;
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
	
	// 通常は、TPデータがメニュー上にあるかどうかを判定。
	if( tpd.disp.touch )	target = &tpd.disp;
	else					target = &tpd.last;
	
	for( i = 0; i < pMenu->num; i++ ) {
		if( tpd.disp.touch ) {										// タッチパネルがメニューの要素上でタッチされているなら、
			NNSG2dTextRect rect = NNS_G2dTextCanvasGetTextRect( &gTextCanvas, (pMenu->str_elem)[ i ] );
			u16 top_x = (u16)( pMenu->pos[ i ].x );					// メニュー要素のLCD座標を算出
			u16 top_y = (u16)( pMenu->pos[ i ].y - 4 );
			u16 bottom_x = (u16)( top_x + rect.width );
			u16 bottom_y = (u16)( top_y + rect.height + 4 );		// ※Y座標は±4のマージン
			
			OS_TPrintf( "MENU[ %d ] : top_x = %02d  top_y = %02d  bot_x = %02d  bot_y = %02d : ",
						i, top_x, top_y, bottom_x, bottom_y );
			
			if( InRangeTp( top_x, top_y, bottom_x, bottom_y, target ) ) {
				OS_TPrintf( "InRange\n" );
				if( tpd.disp.validity == TP_VALIDITY_VALID ) {		// カーソルをその要素に移動
					if( csr_old == i ) {
						if( same_csr_count < TP_CSR_TOUCH_COUNT ) {
							same_csr_count++;
						}else {
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
BOOL InRangeTp( int top_x, int top_y, int bottom_x, int bottom_y, TPData *tgt )
{
	if( ( tgt->x >= top_x    ) &&
		( tgt->x <= bottom_x ) &&
		( tgt->y >= top_y    ) &&
		( tgt->y <= bottom_y ) ) {
		return TRUE;
	}else {
		return FALSE;
	}
}


//===============================================
// RTCアクセスルーチン
//===============================================

// RTCデータ取得＆表示の初期化
void InitGetAndDrawRtcData( int drawDatePos_x, int drawDatePos_y, int drawTimePos_x, int drawTimePos_y)
{
	s_vcount = 0;
	s_rtcPos.date_x = drawDatePos_x;
	s_rtcPos.date_y = drawDatePos_y;
	s_rtcPos.time_x = drawTimePos_x;
	s_rtcPos.time_y = drawTimePos_y;
	
	(void)RTC_GetDateTime( &s_rtcDate, &s_rtcTime);
	GetAndDrawRtcDataCore( TRUE );
}


void GetAndDrawRtcData( void )
{
	GetAndDrawRtcDataCore( FALSE );
}


// RTC情報の取得＆表示
static void GetAndDrawRtcDataCore( BOOL forceGetFlag )
{
	u32 year;
	RTCDate date_old;
	RTCTime time_old;
	
	// RTC情報の取得
	if( forceGetFlag || ( s_vcount++ == 60 ) ) {
		s_vcount = 0;
		MI_CpuCopy16( &s_rtcDate, &date_old, sizeof(RTCDate) );
		MI_CpuCopy16( &s_rtcTime, &time_old, sizeof(RTCTime) );
		
		(void)RTC_GetDateTime( &s_rtcDate, &s_rtcTime );
		
		// 前RTC情報の消去
		{
			year = s_rtcDate.year + 2000;
			PrintfSJISSub( s_rtcPos.date_x,  s_rtcPos.date_y, TXT_COLOR_WHITE, "%d/%02d/%02d[%3s]",
						year,
						date_old.month,
						date_old.day,
						g_strWeek[ date_old.week ]
						);
			PrintfSJISSub( s_rtcPos.time_x,  s_rtcPos.time_y, TXT_COLOR_WHITE, "%02d:%02d:%02d",
						time_old.hour,
						time_old.minute,
						time_old.second
						);
		}
		// RTC情報の表示
		{
			year = s_rtcDate.year + 2000;
			PrintfSJISSub( s_rtcPos.date_x,  s_rtcPos.date_y, TXT_COLOR_BLACK, "%d/%02d/%02d[%3s]",
						year,
						s_rtcDate.month,
						s_rtcDate.day,
						g_strWeek[ s_rtcDate.week ]
						);
			PrintfSJISSub( s_rtcPos.time_x,  s_rtcPos.time_y, TXT_COLOR_BLACK, "%02d:%02d:%02d",
						s_rtcTime.hour,
						s_rtcTime.minute,
						s_rtcTime.second
						);
		}
	}
}


// バナーアイコンOBJのロード
void SetBannerIconOBJ( GXOamAttr *pDstOAM, BannerFileV1 *bannerp )
{
	GXS_LoadOBJPltt( bannerp->pltt, 15, BNR_PLTT_SIZE );
	MI_CpuCopyFast(  bannerp->image, (void *)(HW_DB_OBJ_VRAM + 0x20), BNR_IMAGE_SIZE );
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
