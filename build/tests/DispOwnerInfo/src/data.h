/********************************************************************/
/*      data.h                                                      */
/*          DebugForIPL2                                            */
/*                                                                  */
/*              Copyright (C) 2003-2004 NINTENDO Co.,Ltd.           */
/********************************************************************/
/*
	データ定義　ヘッダ
*/

#ifndef	__DATA_H__
#define	__DATA_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <nitro.h>
#include <fnt.h>

// define data---------------------------------------------

	// パレットカラー
#define WHITE			(  1 << 12 )
#define RED				(  2 << 12 )
#define	GREEN			(  3 << 12 )
#define	BLUE			(  4 << 12 )
#define	YELLOW			(  5 << 12 )
#define	CYAN			(  6 << 12 )
#define	PURPLE			(  7 << 12 )
#define	LIGHTGREEN		(  8 << 12 )
#define	HIGHLIGHT_Y		(  9 << 12 )
#define	HIGHLIGHT_C		( 10 << 12 )
#define	HIGHLIGHT_W		( 11 << 12 )
#define	HIGHLIGHT_B		( 12 << 12 )
#define	HIGHLIGHT_R		( 13 << 12 )


// fntライブラリのカラー指定
#define FNT_BLACK				0
#define FNT_RED					1
#define FNT_LIGHT_GREEN			2
#define FNT_YELLOW				3
#define FNT_BLUE				4
#define FNT_PURPLE				5
#define FNT_VERMILION			6
#define FNT_WHITE				7
#define FNT_SYUIRO				8
#define FNT_GREEN				9
#define FNT_USER_COLOR			10


#define CANVAS_WIDTH 			256			// 文字表示キャンパス横ドット数
#define CANVAS_HEIGHT			192			// 　　　〃　　　　　縦ドット数
#define LINE_DOT_NUM			10


	// キーデータワーク
typedef struct {
	u16 trg;									// トリガ入力
	u16 cont;									// ベタ  入力
}KeyWork;


	// タッチパネルワーク
typedef struct {
	int		detached;							// 今回のデータ入力でタッチが離れたことを示す。
	BOOL	initial;							// 初期化直後は、TPがデタッチされるまで、データ取得しないようにする。
	TPData	disp;								// 今回の入力値（LCD座標）
	TPData	raw;								// 今回の入力値（TP 座標）
	TPData	last;								// 前回の入力値（LCD座標）
}TpWork;


// global variables----------------------------------------
extern int				(*nowProcess)( void );
extern GXOamAttr		oamBakM[ 128 ];				// OAM バックアップ
extern GXOamAttr		oamBakS[ 128 ];				// OAM バックアップ
extern u16				bgBakM[ 32*24 ];			// BG  バックアップ
extern u16				bgBakS[ 32*24 ];			// BG  バックアップ
extern TpWork			tpd;						// タッチパネルデータ
extern KeyWork			pad;						// キーパッド入力データ

extern tFntEntry		font_m;
extern tFntDrawContext	context_m;
extern u16				canvas_m[ CANVAS_WIDTH * CANVAS_HEIGHT / 4 ] ATTRIBUTE_ALIGN(32);
extern u16				screen_m[ (CANVAS_WIDTH>>3) * (CANVAS_HEIGHT>>3) ] ATTRIBUTE_ALIGN(32);

extern tFntEntry		font_s;
extern tFntDrawContext	context_s;
extern u16				canvas_s[ CANVAS_WIDTH * CANVAS_HEIGHT / 4 ] ATTRIBUTE_ALIGN(32);
extern u16				screen_s[ (CANVAS_WIDTH>>3) * (CANVAS_HEIGHT>>3) ] ATTRIBUTE_ALIGN(32);

extern u16				s_Palette[ 0x10 ];

// global const data---------------------------------------
extern const u16 myPlttData[13][16];


// function------------------------------------------------


#ifdef __cplusplus
}
#endif

#endif  // __DATA_H__
