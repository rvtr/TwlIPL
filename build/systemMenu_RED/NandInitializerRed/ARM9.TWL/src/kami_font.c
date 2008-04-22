/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_font.c

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

#include <twl.h>
#include "kami_font.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

#define SCREEN_DATA_COLOR_PLTT_SHIFT   12
#define NUM_OF_PRINT_TARGET             2

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static u8 sXPos;
static u8 sYPos;

static u32 sBackColorCharData[24*8];

static u16 sFontScreenDataMain[32 * 24] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static u16 sFontScreenDataSub[24*32] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

static u16 sBackColorScreenData[32 * 24] = {
	0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,0xf0de,
	0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,0xf0df,
	0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,0xf0e0,
	0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,0xf0e1,
	0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,0xf0e2,
	0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,0xf0e3,
	0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,0xf0e4,
	0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,0xf0e5,
	0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,0xf0e6,
	0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,0xf0e7,
	0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,0xf0e8,
	0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,0xf0e9,
	0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,0xf0ea,
	0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,0xf0eb,
	0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,0xf0ec,
	0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,0xf0ed,
	0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,0xf0ee,
	0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,0xf0ef,
	0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,0xf0f0,
	0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,0xf0f1,
	0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,0xf0f2,
	0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,0xf0f3,
	0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,0xf0f4,
	0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,0xf0f5,
};

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static void kamiFontReturnConsole( void );

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         

  Description:  

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
kamiFontInit(void)
{
	// 背景用キャラクタデータ
	MI_CpuCopy32( &sampleCharData[8 * 0xde], sBackColorCharData, sizeof(sBackColorCharData) );

	// 背景用スクリーンデータセット
	DC_FlushRange  ( sBackColorScreenData,    sizeof(sBackColorScreenData) );
	GXS_LoadBG1Scr ( sBackColorScreenData, 0, sizeof(sBackColorScreenData) );
}

/*---------------------------------------------------------------------------*
  Name:         

  Description:  

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void
kamiFontLoadScreenData(void)
{
	DC_FlushRange  ( sFontScreenDataMain,     sizeof(sFontScreenDataMain) );
	GX_LoadBG0Scr  ( sFontScreenDataMain,  0, sizeof(sFontScreenDataMain) );

	DC_FlushRange  ( sFontScreenDataSub,         sizeof(sFontScreenDataSub) );
	GXS_LoadBG0Scr ( sFontScreenDataSub,      0, sizeof(sFontScreenDataSub) );

	// 背景キャラクタデータ書き換え
	DC_FlushRange( sBackColorCharData, sizeof(sBackColorCharData) );
	GXS_LoadBG0Char( sBackColorCharData, 0xde*32, sizeof(sBackColorCharData) );
}

/*---------------------------------------------------------------------------*
  Name:         kamiFontClear

  Description:  仮想スクリーンをクリアする

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
kamiFontClear(void)
{
	MI_CpuClear8( sFontScreenDataSub, sizeof(sFontScreenDataSub) );
}

/*---------------------------------------------------------------------------*
  Name:         kamiFontClearMain

  Description:  仮想スクリーンをクリアする

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
kamiFontClearMain(void)
{
	MI_CpuClear8( sFontScreenDataMain, sizeof(sFontScreenDataMain) );
}

/*---------------------------------------------------------------------------*
  Name:         kamiFontPrintf

  Description:  仮想スクリーンに文字列を配置する。文字列は32文字まで。

  Arguments:    x       - 文字列の先頭を配置する x 座標( × 8 ドット )。
                y       - 文字列の先頭を配置する y 座標( × 8 ドット )。
                color   - 文字の色をパレット番号で指定。
                text    - 配置する文字列。終端文字はNULL。
                ...     - 仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
kamiFontPrintf(s16 x, s16 y, u8 color, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    *(u16 *)(&temp[32]) = 0x0000;
    for (i = 0;temp[i] != 0x00; i++)
    {
        sFontScreenDataSub[((y * 32) + x + i) % (24 * 32)] = 
			(u16)((color << SCREEN_DATA_COLOR_PLTT_SHIFT) | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         kamiFontPrintf

  Description:  仮想スクリーンに文字列を配置する。文字列は32文字まで。

  Arguments:    x       - 文字列の先頭を配置する x 座標( × 8 ドット )。
                y       - 文字列の先頭を配置する y 座標( × 8 ドット )。
                color   - 文字の色をパレット番号で指定。
                text    - 配置する文字列。終端文字はNULL。
                ...     - 仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
kamiFontPrintfMain(s16 x, s16 y, u8 color, char *text, ...)
{
    va_list vlist;
    char    temp[32 + 2];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 33, text, vlist);
    va_end(vlist);

    *(u16 *)(&temp[32]) = 0x0000;
    for (i = 0;temp[i] != 0x00; i++)
    {
        sFontScreenDataMain[((y * 32) + x + i) % (24 * 32)] = 
			(u16)((color << SCREEN_DATA_COLOR_PLTT_SHIFT) | temp[i]);
    }
}

/*---------------------------------------------------------------------------*
  Name:         kamiFontFillChar

  Description:  仮想スクリーンに

  Arguments:    x       - 文字列の先頭を配置する x 座標( × 8 ドット )。
                y       - 文字列の先頭を配置する y 座標( × 8 ドット )。
                color   - 文字の色をパレット番号で指定。
                value   

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
kamiFontFillChar(int lineNo, u8 color1, u8 color2)
{
    s32 i;
	u32 line;
	int charNo = 0xde + lineNo;

	if (color1 < 0x10)
	{
		line = (u32)(0x11111111 * color1);

	    for (i = 0;i<4; i++)
	    {
			sBackColorCharData[8 * lineNo + i] = line;
	    }
	}

	if (color2 < 0x10)
	{
		line = (u32)(0x11111111 * color2);

	    for (i = 4;i<8; i++)
	    {
			sBackColorCharData[8 * lineNo + i] = line;
	    }
	}
}

/*---------------------------------------------------------------------------*
  Name:         kamiFontPrintfConsole

  Description:  仮想コンソールに文字列を配置する。文字列は256文字まで。

  Arguments:    color   - 文字の色をパレット番号で指定。
                text    - 配置する文字列。終端文字はNULL。
                ...     - 仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
kamiFontPrintfConsole(u8 color, const char *text, ...)
{
    va_list vlist;
    char    temp[256 + 2];
    s32     i;

    va_start(vlist, text);
    (void)vsnprintf(temp, 256, text, vlist);
    va_end(vlist);

	// 終端追加
    *(u16 *)(&temp[256]) = 0x0000;

	for(i=0; temp[i] != 0x00; i++)
    {
		if (temp[i] == 0x0A)
		{
			// 改行コード
			kamiFontReturnConsole();
		}
		else
		{
			// 一文字ずつ書き込み
	        sFontScreenDataMain[((sYPos * 32) + sXPos) % (24 * 32)] = 
				(u16)((color << SCREEN_DATA_COLOR_PLTT_SHIFT) | temp[i]);

			// X座標が右端に到達した場合は改行処理
			if (++sXPos >= 32)
			{
				kamiFontReturnConsole();
			}
		}
    }
}

/*---------------------------------------------------------------------------*
  Name:         kamiFontPrintfConsoleEx

  Description:  仮想コンソールに文字列を配置する。文字列は256文字まで。
				OS_Printfもついでに実行する。

  Arguments:    color   - 文字の色をパレット番号で指定。
                text    - 配置する文字列。終端文字はNULL。
                ...     - 仮想引数。

  Returns:      None.
 *---------------------------------------------------------------------------*/
void 
kamiFontPrintfConsoleEx(u8 color, const char *text, ...)
{
    va_list vlist;
    char    temp[256 + 2];

    va_start(vlist, text);
    (void)vsnprintf(temp, 256, text, vlist);
    va_end(vlist);

	kamiFontPrintfConsole(color, temp);
	OS_TPrintf(temp);
}

/*---------------------------------------------------------------------------*
  Name:         kamiFontReturnConsole

  Description:  仮想コンソールにおける改行処理を行う

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void 
kamiFontReturnConsole( void )
{
	sXPos = 0;
	if (sYPos < 23) 
	{
		// 次の行へ
		sYPos++; 
	}
	else
	{
		// 既に最終行に到達している場合シフトを行う
		MI_CpuCopy32( &sFontScreenDataMain[32], sFontScreenDataMain, sizeof(u16)*32*23 );
		MI_CpuClear32( &sFontScreenDataMain[32*23], sizeof(u16)*32);
	}
}

