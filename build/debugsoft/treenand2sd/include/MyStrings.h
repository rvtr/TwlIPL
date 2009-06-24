/*************************************************************************

簡単な文字列表示する関数群

  ************************************************************************/
#include <stdio.h>

#include "basicdef.h"
#include "text.h"

/************************************************************************
  二重定義や宣言/参照を同じヘッダで使用するためのプリプロセッサ
  ************************************************************************/

/* 多重インクルードの回避 */
#ifndef MYSTRINGS_H_INCLUDED
#define MYSTRINGS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************

☆使用する構造体

 ************************************************************************/


/* 画面出力するために使用するコンソール */
typedef struct {
    COORDINATE_TYPE Cursor;        /* 現在のカーソル位置 */
    int             Palette;          /* 現在使用するパレット  */

    int             ScrollStart;   /* スクロール開始・終了位置 */
    int             ScrollEnd;

    TEXT_VRAM_TYPE  *Text;         /* 書き込み先 */

} CONSOLE_TYPE;


/************************************************************************

☆参照宣言
 ************************************************************************/

extern CONSOLE_TYPE StdConsole;
extern CONSOLE_TYPE *Console;



/************************************************************************
☆パブリックと同等に使用するマクロ
 ************************************************************************/
#define wPuts( str__ )            wcPuts( &StdConsole, str__ )
#define _Puts( str__ )            _cPuts( &StdConsole, str__ )
#define wPutchar( character__ )   wcPutchar( &StdConsole, character__ )
#define wPutcharNC( character__ ) wcPutcharNC( &StdConsole, character__ )
#define wGotoxy( x__, y__ )       wcGotoxy( &StdConsole, x__, y__ )
#define wSetPalette( Palette__ )  wcSetPalette( &StdConsole, Palette__ );


/************************************************************************
☆パブリック関数の宣言
 ************************************************************************/
extern int     wcPuts      ( CONSOLE_TYPE *Console, const char *str );
extern void    _cPuts      ( CONSOLE_TYPE *Console, const char *str );
extern void    wcGotoxy    ( CONSOLE_TYPE *Console, int x, int y );
extern void    wcSetPalette( CONSOLE_TYPE *Console, int Palette );
extern int     wcPutchar   ( CONSOLE_TYPE *Console, int Character );
extern int     wcPutcharNC ( CONSOLE_TYPE *Console, int Character );
extern void    wcPrintf    ( CONSOLE_TYPE *Console, const char *fmt , ... );
extern void    wPrintf     ( const char *fmt , ... );
extern void    swPrintf    ( const char *fmt , ... );
extern int     wCountLine  ( const char *Str );

#ifdef __cplusplus
}
#endif

/* 多重インクルードの回避 */
#endif

