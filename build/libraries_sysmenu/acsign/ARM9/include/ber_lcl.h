/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makerom.TWL
  File:     ber_lcl.h

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
/* $Id$ */
/*
 * Copyright (C) 1998-2002 RSA Security Inc. All rights reserved.
 *
 * This work contains proprietary information of RSA Security.
 * Distribution is limited to authorized licensees of RSA
 * Security. Any unauthorized reproduction, distribution or
 * modification of this work is strictly prohibited.
 *
 */

#ifndef HEADER_COMMON_BER_LCL_H
#define HEADER_COMMON_BER_LCL_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef STANDALONE
#include "r_com.h"
#else
#include <stdio.h>
#include <stdlib.h>
#define NO_STDLIB_MAPPING
#include "acmemory.h"
#define Malloc(a)       ACMemory_Alloc(a)
#define Free(a)         ACMemory_Free(a)
#define Memset(a,b,c)   ACMemory_Memset(a,b,c)
#define Memcpy(a,b,c)   ACMemory_Memcpy(a,b,c)
#define Realloc(a,b,c)	ACMemory_Realloc(a,b,c)
#define Memcmp(a,b,c)	memcmp(a,b,c)
#define Bsearch(a,b,c,d,e) bsearch(a,b,c,d,e)
#endif

#include "ber.h" 

#ifdef UNDER_CE
#include "wcestdlb.h"	/* include for bsearch */
#endif
 
#ifdef  __cplusplus
}
#endif

#endif /* !HEADER_COMMON_BER_H */

