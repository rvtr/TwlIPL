/*---------------------------------------------------------------------------*
  Project:  TwlFirm - GCD - include
  File:     blowfish.h

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
#ifndef FIRM_GCD_BLOWFISH_H
#define FIRM_GCD_BLOWFISH_H

#include <twl/types.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u32 P[16 + 2];
    u32 S[4][256];
} BLOWFISH_CTX;


void InitBlowfish(BLOWFISH_CTX *ctx, const unsigned char *key, int keyLen);
void EncryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr);
void DecryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // FIRM_GCD_BLOWFISH_H
