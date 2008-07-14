/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     control.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#ifndef	__DISPLAY_SYSTEM_CONTROL_H__
#define	__DISPLAY_SYSTEM_CONTROL_H__

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif


BOOL control( int *menu, int *line );
int getMaxPage( int menu );
int getMaxLine( int menu , int page );

#ifdef __cplusplus
}
#endif

#endif