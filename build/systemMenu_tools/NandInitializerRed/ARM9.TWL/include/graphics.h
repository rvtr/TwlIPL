/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     graphics.h

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

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/

void InitGraphics(void);
void DrawLine(s16 sx, s16 sy, s16 ex, s16 ey, GXRgb color);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* GRAPHICS_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
