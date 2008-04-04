/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_font.h

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

#ifndef KAMI_FONT_H_
#define KAMI_FONT_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <twl.h>

extern u8 prog_state;
enum
{ STATE_NONE, STATE_SELECT, STATE_KS_PARENTINIT, STATE_KS_PARENT, STATE_KS_CHILDINIT,
    STATE_KS_CHILDSCAN, STATE_KS_CHILD
};
extern OSHeapHandle heapHandle;        // Heapハンドル;

extern const u16 BgScDataMain[32 * 24];
extern const u16 BgScDataSub[32 * 24];
extern const u32 sampleCharData[8 * 0x100];
extern const u16 PlttDataObj[16][16];
extern const u16 PlttDataMain[16][16];
extern const u16 PlttDataSub[16][16];

void kamiFontInit(void);
void kamiFontClear(void);
void kamiFontPut(u16 x, u16 y, u16 color, u16 no);
void kamiFontPrintf(s16 x, s16 y, u8 color, char *text, ...);
void kamiFontPrintfMain(s16 x, s16 y, u8 color, char *text, ...);
void kamiFontFill(s16 x, s16 y, u8 color, s16 value, s32 length);
void kamiFontFillChar(int lineNo, u8 color1, u8 color2);
void kamiFontLoadScreenData(void);
void kamiFontPrintfConsole(u8 color, const char *text, ...);
void kamiFontPrintfConsoleEx(u8 color, const char *text, ...);

// 上画面コンソール文字列用パレット
#define CONSOLE_ORANGE	0
#define CONSOLE_RED		1
#define CONSOLE_GREEN	2

// 下画面フォント用パレット
#define FONT_COLOR_BLACK 	0
#define FONT_COLOR_RED   	1
#define FONT_COLOR_GREEN 	2
#define FONT_COLOR_BLUE  	3
#define FONT_COLOR_YELLOW	4
#define FONT_COLOR_CYAN 	5
#define FONT_COLOR_PURPLE	6

// 下画面背景用パレット
#define BG_COLOR_TRANS   0
#define BG_COLOR_WHITE   1
#define BG_COLOR_BLACK   2
#define BG_COLOR_GRAY    3
#define BG_COLOR_PURPLE  4
#define BG_COLOR_PINK    5
#define BG_COLOR_BLUE    6
#define BG_COLOR_GREEN   7
#define BG_COLOR_VIOLET  8
#define BG_COLOR_RED     9
#define BG_COLOR_YELLOW 10

#define BG_COLOR_NONE   0xff

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* KAMI_FONT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
