/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     main.h

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

#ifndef	__MAIN_H__
#define	__MAIN_H__

#include <twl.h>
#include <sysmenu.h>
#include "misc.h"

#define NNS_G2D_UNICODE
#include <nnsys.h>
#include <nnsys/g2d/g2d_Font.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/g2d_TextCanvas.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

// DrawText での左上寄せ
#define TXT_DRAWTEXT_FLAG_DEFAULT   (NNS_G2D_VERTICALORIGIN_TOP | NNS_G2D_HORIZONTALORIGIN_LEFT | NNS_G2D_HORIZONTALALIGN_LEFT)

// TXTColorPalette の色名 16色パレットへのロードを想定
enum
{
    // パレット0 TXT_CPALETTE_MAIN
    TXT_COLOR_NULL=0,
    TXT_COLOR_WHITE,
    TXT_COLOR_BLACK,
    TXT_COLOR_RED,
    TXT_COLOR_GREEN,
    TXT_COLOR_BLUE,
    TXT_COLOR_CYAN,
    TXT_COLOR_MAGENTA,
    TXT_COLOR_YELLOW,

    // パレット1 TXT_CPALETTE_USERCOLOR
    TXT_UCOLOR_NULL=0,
    TXT_UCOLOR_GRAY,
    TXT_UCOLOR_BROWN,
    TXT_UCOLOR_RED,
    TXT_UCOLOR_PINK,
    TXT_UCOLOR_ORANGE,
    TXT_UCOLOR_YELLOW,
    TXT_UCOLOR_LIMEGREEN,
    TXT_UCOLOR_DARKGREEN,
    TXT_UCOLOR_SEAGREEN,
    TXT_UCOLOR_TURQUOISE,
    TXT_UCOLOR_BLUE,
    TXT_UCOLOR_DARKBLUE,
    TXT_UCOLOR_PURPLE,
    TXT_UCOLOR_VIOLET,
    TXT_UCOLOR_MAGENTA,

    // パレット TXT_CPALETTE_4BPP
    TXT_COLOR_4BPP_NULL=0,
    TXT_COLOR_4BPP_BG=1,
    TXT_COLOR_4BPP_TEXT=1
};


// 時計表示場所
#define RTC_DATE_TOP_X				( 18 * 8 )
#define RTC_DATE_TOP_Y				(  2 * 8 )
#define RTC_TIME_TOP_X				( 25 * 8 )
#define RTC_TIME_TOP_Y				(  4 * 8 )

	// IPL2のブートタイプ指定
typedef enum IPL2BootType {
	BOOT_TYPE_UNSOLVED = 0,
	BOOT_TYPE_NITRO,
	BOOT_TYPE_PICT_CHAT,
	BOOT_TYPE_WIRELESS_BOOT,
	BOOT_TYPE_BMENU
}IPL2BootType;


// キーデータワーク
typedef struct {
	u16 trg;									// トリガ入力
	u16 cont;									// ベタ  入力
}KeyWork;


// タッチパネルワーク
typedef struct {
	int	   detached;							// 今回のデータ入力でタッチが離れたことを示す。
	TPData disp;								// 今回の入力値（LCD座標）
	TPData raw;									// 今回の入力値（TP 座標）
	TPData last;								// 前回の入力値（LCD座標）
}TpWork;


// メニュー構成パラメータ構造体
typedef struct MenuComponent {
	int			num;
	int			pos_x;
	int			pos_y;
	int			next_x_num;
	int			next_y_num;
	int			name_length;
	int			normal_color;
	int			select_color;
	const u8	**str_elem;
}MenuComponent;


typedef struct MenuPos {
	BOOL		enable;
	int			x;
	int			y;
}MenuPos;

// メニュー構成パラメータ構造体
typedef struct MenuParam {
	int			num;
	int			normal_color;
	int			select_color;
	int			disable_color;
	MenuPos		*pos;
	const u16	**str_elem;
}MenuParam;


// global variables--------------------------------------------------
extern GXOamAttr oamBakS[ 128 ];			// OAM バックアップ
extern u16		 bgBakM[ 32*24 ];			// BG  バックアップ
extern u16		 bgBakS[ 32*24 ];			// BG  バックアップ
extern TpWork	 tpd;						// タッチパネルデータ
extern KeyWork	 pad;						// キーパッド入力データ
extern const u8 *const g_strWeek[ 7 ];

extern NNSFndAllocator         g_allocator;
extern NNSG2dFont              gFont;          // フォント
extern NNSG2dCharCanvas        gCanvas;        // CharCanvas
extern NNSG2dTextCanvas        gTextCanvas;    // TextCanvas

// function-------------------------------------------------------------
void LauncherInit( void );
IPL2BootType LauncherMain( BOOL boot_decision );


void InitBG( void );
void PutStringUTF16( int x, int y, int color, const u16 *strUTF16 );
void PrintfSJIS( int x, int y, int color, const char *fmt, ... );

void ReadKeyPad( void );
void ReadTpData( void );
void ReadTpDataLogoDirectBootCancel( void );
BOOL WaitDetachTP( void );
void StartDetachTP( void );
void SetBannerIconOBJ( BannerFileV1 *bannerp );

void DrawMenu( u16 nowCsr, const MenuComponent *menu );
BOOL SelectMenuByTp( u16 *nowCsr, const MenuComponent *menu );
BOOL SelectMenuByTP( u16 *nowCsr, const MenuParam *pMenu );
BOOL InRangeTp( int top_x, int top_y, int bottom_x, int bottom_y, TPData *tgt );

void InitGetAndDrawRtcData( int drawDatePos_x, int drawDatePos_y, int drawTimePos_x, int drawTimePos_y );
void GetAndDrawRtcData( void );

#ifdef __cplusplus
}
#endif

#endif  // __MAIN_H__
