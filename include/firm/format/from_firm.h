/*---------------------------------------------------------------------------*
  Project:  TwlFirm - format - from_firm
  File:     from_firm.h

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
#ifndef FIRM_FORMAT_FROM_FIRM_H_
#define FIRM_FORMAT_FROM_FIRM_H_

#include <firm/gcd/blowfish.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9
#define RSA_PUBKEY_NUM_FROM_FIRM    8
#define AESKEY_NUM_FROM_FIRM        8
#else // SDK_ARM7
#define RSA_PUBKEY_NUM_FROM_FIRM    4
#define AESKEY_NUM_FROM_FIRM        4
#endif // SDK_ARM7

/*
    reservedは、ファームヘッダの署名の中に埋められた値(現状0x00で埋められている)
    未使用の場合(現状)、0x00で埋められていることを確認すべき
*/

typedef struct
{
    u8            rsa_pubkey[RSA_PUBKEY_NUM_FROM_FIRM][ACS_PUBKEY_LEN];  // 1KB
    u8            aes_key[AESKEY_NUM_FROM_FIRM][ACS_AES_LEN];  // 128B
    u8            reserved[ACS_HASH_LEN];  // 20B

    BLOWFISH_CTX  ds_blowfish;      // 4KB + α
    BLOWFISH_CTX  twl_blowfish;     // 4KB + α
}
OSFromFirm9Buf;

typedef struct
{
    u8            rsa_pubkey[RSA_PUBKEY_NUM_FROM_FIRM][ACS_PUBKEY_LEN];  // 512B
    u8            aes_key[AESKEY_NUM_FROM_FIRM][ACS_AES_LEN];  // 64B
    u8            reserved[ACS_HASH_LEN];  // 20B

    BLOWFISH_CTX  twl_blowfish[2];  // (4KB + α) * 2
}
OSFromFirm7Buf;

#ifdef SDK_ARM9
typedef OSFromFirm9Buf  OSFromFirmBuf;
#else // SDK_ARM7
typedef OSFromFirm7Buf  OSFromFirmBuf;
#endif // SDK_ARM7


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // FIRM_FORMAT_FROM_FIRM_H_
