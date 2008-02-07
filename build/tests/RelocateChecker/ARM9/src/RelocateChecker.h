/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     RelocateChecker.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef	__RELOCATE_CHECKER_H__
#define	__RELOCATE_CHECKER_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <twl.h>

// define data----------------------------------------------------------

void RelocateCheckerInit( void );
void RelocateCheckerMain( void );

#ifdef __cplusplus
}
#endif

#endif  // __RELOCATE_CHECKER_H__
