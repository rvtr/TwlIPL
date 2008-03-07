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
#include <hotswTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define USE_LOCAL_KEYTABLE

/*************************************************************************/

#ifdef USE_LOCAL_KEYTABLE
// 初期化テーブル
extern const BLOWFISH_CTX  GCDi_BlowfishInitTableDS;
#else
extern BLOWFISH_CTX  GCDi_BlowfishInitTableBufDS;
#endif

// Function Prototype ------------------------------------------------------------------------
// Blowfish 初期化
void InitBlowfish(BLOWFISH_CTX *ctx, const unsigned char *key, int keyLen);

// Blowfish KeyとTableの初期化
void InitBlowfishKeyAndTableDS(BLOWFISH_CTX *ctx, u32 *keyBufp, s32 keyLen);

// Blowfish 復号化
void EncryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr);

// Blowfish 暗号化
void DecryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr);

// Key Table の生成
void GCDm_MakeBlowfishTableDS(CardBootData *cbd, s32 keyLen);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif  // HOTSW_GCD_BLOWFISH_H
