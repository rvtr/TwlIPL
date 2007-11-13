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
#include "Card.h"

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************************/

// 初期化テーブル
extern const BLOWFISH_CTX  GCDi_BlowfishInitTableDS;

// Function Prototype ------------------------------------------------------------------------
// Blowfish 初期化
void InitBlowfish(BLOWFISH_CTX *ctx, const unsigned char *key, int keyLen);

// Blowfish 復号化
void EncryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr);

// Blowfish 暗号化
void DecryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr);

// Key Table の生成
void GCDm_MakeBlowfishTableDS(BLOWFISH_CTX *tableBufp, ROM_Header_Short *rhs, u32 *keyBufp, s32 keyLen);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // FIRM_GCD_BLOWFISH_H
