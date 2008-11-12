/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - demos - consoleType-1
  File:     screen.h

  Copyright 2003-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: main.c,v $
  Revision 1.23  2006/01/18 02:11:29  kitase_hirotake
  do-indent

  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef	SCREEN_H_
#define	SCREEN_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/
#include	<nitro/types.h>

extern u16 gScreen[32 * 32];

#define CONSOLE_WHITE 15
void    ClearScreen(void);
void    PrintString(s16 x, s16 y, u8 palette, char *text, ...);
void    ColorString(s16 x, s16 y, s16 length, u8 palette);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* SCREEN_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
