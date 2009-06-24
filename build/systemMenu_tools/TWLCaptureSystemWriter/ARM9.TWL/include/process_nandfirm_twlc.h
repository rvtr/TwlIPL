/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_nandfirm_twlc.h

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
#ifndef PROCESS_NANDFIRM_TWLC
#define PROCESS_NANDFIRM_TWLC

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <twl.h>

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/

void* NandfirmProcessTWLC0(void);
void* NandfirmProcessTWLC1(void);
void* NandfirmProcessTWLC2(void);

/*===========================================================================*/


#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif // PROCESS_NANDFIRM_TWLC