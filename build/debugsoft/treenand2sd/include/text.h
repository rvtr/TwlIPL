/************************************************************************

  テキストＶＲＡＭエミュレーション

  ************************************************************************/
#include "basicdef.h"

/************************************************************************
  二重定義や宣言/参照を同じヘッダで使用するためのプリプロセッサ
  ************************************************************************/

/* 多重インクルードの回避 */
#ifndef TEXT_H_INCLUDED
#define TEXT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
☆パブリック関数の宣言
 ************************************************************************/
extern void wInitTextVram( void );
extern void wSuspendTextVram( void );
extern void wResumeTextVram( void );

extern void swSuspendTextVram( void );
extern void swResumeTextVram( void );

extern void wRemoveTextVram( void );

/************************************************************************
☆テキストＶＲＡＭで使用する定数値
 ************************************************************************/

#define CLEAR_CHARACTER ' '          /* 画面クリア時に使用するキャラクタ */

/************************************************************************
☆テキストＶＲＡＭで使用する構造体
 ************************************************************************/


/*
 * テキストＶＲＡＭ本体です。
 * Map       :表示文字をアスキーコードで入力します
 *
 * 現在、他の属性はなし。
 */


typedef struct {                                   /* テキストＶＲＡＭ操作用*/
    u16        Map[VIRTUAL_SCREEN_HEIGHT][VIRTUAL_SCREEN_WIDTH];
} TEXT_VRAM_TYPE;

/************************************************************************
☆テキストＶＲＡＭの参照宣言
 ************************************************************************/

/* テキストＶＲＡＭ本体 */
extern TEXT_VRAM_TYPE wText;
extern TEXT_VRAM_TYPE swText;

#ifdef __cplusplus
}
#endif



/* 多重インクルードの回避 */
#endif
