/**************************************************************************

  基本定義ヘッダファイル

  多くのソースファイルで使用する基本的な情報を記述しています。


 *************************************************************************/


/* 多重インクルードの回避 */
#ifndef __BASICDEF_H__
#define __BASICDEF_H__

/**************************************************************************
 標準でインクルードするヘッダファイル
 *************************************************************************/
#include <nitro.h>
#include <stdio.h>  // NULLを使用するため

#ifdef __cplusplus
extern "C" {
#endif

/**************************************************************************
  #define系プリプロセッサ命令 記述領域
 *************************************************************************/

//Ｖ周期
#define VERTICAL_CYCLE          16743     //単位μS


//画面関係の定義
#define SCREEN_WIDTH    32             /* コンソールで使用するテキストＲＡＭの列数 */
#define SCREEN_HEIGHT   24             /* コンソールで使用するテキストＲＡＭの行数 */

#define VIRTUAL_SCREEN_WIDTH  32     /* 仮想画面のサイズ */
#define VIRTUAL_SCREEN_HEIGHT 32

#define CHARACTER_SIDE_LENGTH   8       /* キャラクタの一辺の長さ */

#define SCREEN_WIDTH_DOT    256
#define SCREEN_HEIGHT_DOT   192

/**************************************************************************
    汎用マクロ定義
 *************************************************************************/

//配列の要素数を求める  'Code Complete 上　第二版　P379 から引用
#define ARRAY_LENGTH(x) (sizeof(x)/sizeof(x[0]))

/**************************************************************************
    型定義
 *************************************************************************/


#if 0
/* ブール型の設定 */
#undef FALSE
#undef TRUE
typedef enum { FALSE=0, TRUE=1 } BOOL;
#endif

/* 整数型座標構造体 */
typedef struct {    /* 座標を指定する構造体   */
    int x;         /*  Ｘ座標                */
    int y;        /*   Ｙ座標               */
} COORDINATE_TYPE;

/* 各色の成分 */
typedef enum {
    RAW_COLOR_BLACK     = 0x0000,
    RAW_COLOR_RED       = 0x001f,
    RAW_COLOR_GREEN     = 0x03e0,
    RAW_COLOR_YELLOW    = 0x03ff,
    RAW_COLOR_BLUE      = 0x7c00,
    RAW_COLOR_MAGENTA   = 0x7c1f,
    RAW_COLOR_CYAN      = 0x7fe0,
    RAW_COLOR_WHITE     = 0x7fff,

    RAW_COLOR_GRAY1     = 0x1004,
    RAW_COLOR_GRAY2     = 0x2108,
    RAW_COLOR_GRAY3     = 0x318c,
    RAW_COLOR_GRAY4     = 0x4210,
    RAW_COLOR_GRAY5     = 0x5294,
    RAW_COLOR_GRAY6     = 0x6318

} RAW_COLOR;

/* パレット設定 */
typedef enum {
    COLOR_TRANSLUCENT   = 0,
    COLOR_RED           = 1,
    COLOR_GREEN         = 2,
    COLOR_YELLOW        = 3,
    COLOR_BLUE          = 4,
    COLOR_MAGENTA       = 5,
    COLOR_CYAN          = 6,
    COLOR_WHITE         = 7,
    COLOR_BLACK         = 8,
    COLOR_GRAY1         = 9,
    COLOR_GRAY2         =10,
    COLOR_GRAY3         =11,
    COLOR_GRAY4         =12,
    COLOR_GRAY5         =13,
    COLOR_GRAY6         =14


} COLOR_TYPE;

#define COLOR_GRAY COLOR_GRAY5

#ifdef __cplusplus
}
#endif

/* 多重インクルードの回避 */
#endif





