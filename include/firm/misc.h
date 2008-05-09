/*---------------------------------------------------------------------------*
  Project:  TwlFirm - include - 
  File:     misc.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef FIRM_MISC_H_
#define FIRM_MISC_H_

#include <nitro/misc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define offsetof(t, memb) ((size_t)(&(((t *)0)->memb)))

#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_MISC_H_ */
#endif
