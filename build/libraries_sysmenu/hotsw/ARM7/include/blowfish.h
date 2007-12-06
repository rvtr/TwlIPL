/*---------------------------------------------------------------------------*
  Project:  TwlFirm - GCD - include
  File:     blowfish.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#ifndef FIRM_GCD_BLOWFISH_H
#define FIRM_GCD_BLOWFISH_H


#include <twl/types.h>
#include <hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/

// èâä˙âªÉeÅ[ÉuÉã
extern const BLOWFISH_CTX  GCDi_BlowfishInitTableDS;

// Function Prototype ------------------------------------------------------------------------
// Blowfish èâä˙âª
void InitBlowfish(BLOWFISH_CTX *ctx, const unsigned char *key, int keyLen);

// Blowfish KeyÇ∆TableÇÃèâä˙âª
void InitBlowfishKeyAndTableDS(BLOWFISH_CTX *ctx, u32 *keyBufp, s32 keyLen);

// Blowfish ïúçÜâª
void EncryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr);

// Blowfish à√çÜâª
void DecryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr);

// Key Table ÇÃê∂ê¨
void GCDm_MakeBlowfishTableDS(BLOWFISH_CTX *tableBufp, ROM_Header_Short *rhs, u32 *keyBufp, s32 keyLen);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // FIRM_GCD_BLOWFISH_H
