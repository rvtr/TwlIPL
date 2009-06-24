/*---------------------------------------------------------------------------*
  Project:  NitroSDK - SPI - demos - pm-1
  File:     font.h

  Copyright 2003-2005 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: font.h,v $
  Revision 1.2  2005/02/28 05:26:12  yosizaki
  do-indent.

  Revision 1.1  2004/08/07 01:59:51  yada
  modified much

  $NoKeywords: $
 *---------------------------------------------------------------------------*/

#ifndef	FONT_H_
#define	FONT_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include	<nitro/types.h>

typedef	enum
{
    FONT_BLOCK = 0,
    FONT_RED = 1,
    FONT_GREEN = 2,
    FONT_BLUE = 3,
    FONT_YELLOW = 4,
    FONT_PURPLE = 5,
    FONT_CYAAN = 6,
    FONT_WHITE = 15
}
MYFontColor;

extern const u32 d_CharData[8 * 256];
extern const u32 d_PaletteData[8 * 16];

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* FONT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
