/*---------------------------------------------------------------------------*
  Project:  TwlIPL - HOTSW - include
  File:     blowfish.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#ifndef HOTSW_GCD_BLOWFISH_H
#define HOTSW_GCD_BLOWFISH_H


#include <twl/types.h>
#include <firm/gcd/blowfish.h>
#include <hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/
extern BLOWFISH_CTX  HotSwBlowfishInitTableBufDS;
extern BLOWFISH_CTX	 HotSwBlowfishInitTableTWL_dev;

// Function Prototype ------------------------------------------------------------------------

// Blowfish KeyÇ∆TableÇÃèâä˙âª
void InitBlowfishKeyAndTableDS(BLOWFISH_CTX *ctx, u32 *keyBufp, s32 keyLen);

// Key Table ÇÃê∂ê¨
void MakeBlowfishTableDS(CardBootData *cbd, s32 keyLen);

// Key Table2 ÇÃê∂ê¨
void MakeBlowfishTableTWL(CardBootData *cbd, s32 keyLen, u16 bondingOp);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // HOTSW_GCD_BLOWFISH_H
