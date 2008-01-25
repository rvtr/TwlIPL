/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     keypad.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "keypad.h"

/*---------------------------------------------------------------------------*
    ’è”’è‹`
 *---------------------------------------------------------------------------*/

#define KEY_REPEAT_TRIGGER_START  20
#define KEY_REPEAT_TRIGGER_TERM    5

/*---------------------------------------------------------------------------*
    “à•”•Ï”’è‹`
 *---------------------------------------------------------------------------*/

static u16     Cont;
static u16     Trg;
static u16     Release;
static u16     RepeatTrg;
static u8      key = 60;

static int repeat_counter;

/*---------------------------------------------------------------------------*
    “à•”ŠÖ”’è‹`
 *---------------------------------------------------------------------------*/
void
kamiPadRead(void)
{
    u16     ReadData;

    ReadData = PAD_Read();
    Trg      = (u16)(ReadData & (ReadData ^ Cont));
	Release  = (u16)(Cont & (ReadData ^ Cont));
    Cont = ReadData;

	RepeatTrg = Trg;
	if (++repeat_counter > (KEY_REPEAT_TRIGGER_START + KEY_REPEAT_TRIGGER_TERM))
	{
		repeat_counter = KEY_REPEAT_TRIGGER_START;
	}
	if (repeat_counter == KEY_REPEAT_TRIGGER_START)
	{
		RepeatTrg = ReadData;
	}
	if (!ReadData)
	{
		repeat_counter = 0;
	}
}

BOOL 
kamiPadIsTrigger(u16 key)
{
	return (Trg	& key)? TRUE : FALSE;
}

BOOL
kamiPadIsRepeatTrigger(u16 key)
{
	return (RepeatTrg & key)? TRUE : FALSE;
}

BOOL 
kamiPadIsPress(u16 key)
{
	return (Cont & key)? TRUE : FALSE;
}

