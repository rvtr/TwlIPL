/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     misc.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-15#$
  $Rev: 188 $
  $Author: yoshida_teruhisa $
 *---------------------------------------------------------------------------*/

#ifndef	__MISC_SIMPLE_H__
#define	__MISC_SIMPLE_H__

#include <twl.h>
#include <DEMO.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

#define MYPAD_IS_TRIG		DEMO_IS_TRIG
#define MYPAD_IS_PRESS		DEMO_IS_PRESS

// TXTColorPalette の色名 256色パレットへのロードを想定
enum
{
    TXT_COLOR_BLACK=0,
    TXT_COLOR_RED,
    TXT_COLOR_GREEN,
    TXT_COLOR_BLUE,
    TXT_COLOR_YELLOW,
    TXT_COLOR_PURPLE,
    TXT_COLOR_LIGHTBLUE,
    TXT_COLOR_DARKRED,
    TXT_COLOR_DARKGREEN,
    TXT_COLOR_DARKBLUE,
    TXT_COLOR_DARKYELLOW,
    TXT_COLOR_DARKPURPLE,
    TXT_COLOR_DARKLIGHTBLUE,
    TXT_COLOR_GRAY,
    TXT_COLOR_DARKGRAY,
    TXT_COLOR_WHITE
};

typedef enum
{
	MAIN_SCREEN = 0,
	SUB_SCREEN,
	SCREEN_MAX
}
MyScreen;

// メニュー要素座標
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
	const char	**str_elem;
}MenuParam;

// global variables--------------------------------------------------

// function-------------------------------------------------------------
void myInit( void );
void myPreMain( void );
void myProMain( void );

void myDp_Cls16(MyScreen scr);
void myDp_Cls8(MyScreen scr);
void myDp_Putchar(s32 x, s32 y, u8 color, MyScreen scr, u8 c);
void myDp_Write(s32 x, s32 y, u8 color, MyScreen scr, char* str);
void myDp_Printf(s32 x, s32 y, u8 color, MyScreen scr, const char *s, ...);
void myDp_DrawMenu( u16 csr, MyScreen scr, const MenuParam *param );

#ifdef __cplusplus
}
#endif

#endif  // __MISC_H__
