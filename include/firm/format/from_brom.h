/*---------------------------------------------------------------------------*
  Project:  TwlFirm - format - from_brom
  File:     from_brom.h

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
#ifndef FIRM_FORMAT_FROM_BROM_H_
#define FIRM_FORMAT_FROM_BROM_H_

#include <firm/gcd/blowfish.h>
#include <firm/format/sign.h>
#include <firm/format/norfirm.h>
#include <firm/format/nandfirm.h>
#include <firm/format/gcdfirm.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef SDK_ARM9
#define RSA_PUBKEY_NUM_FROM_BROM    8
#define AESKEY_NUM_FROM_BROM        8
#else // SDK_ARM7
#define RSA_PUBKEY_NUM_FROM_BROM    4
#define AESKEY_NUM_FROM_BROM        4
#endif // SDK_ARM7


// sizeof(SDPortContext)
typedef struct SDportContextData
{
    u32 data[17];
}
SDPortContextData;

/*
    hash_table_hashは、ファームヘッダの署名の中に埋められた値(現状0xffで埋められている)
    ファームブート後に追加データをメインメモリにロードする必要に駆られた場合、
    そのハッシュ値をmakenandfirm等で埋め込むようにすることで保護できる。
    (Static部分に埋めても大差ないと思われるが・・・)
    未使用の場合(現状)、0xffで埋められていることを確認すべき
*/
typedef struct
{
    union
    {
        NANDHeader nand;
        NORHeader  nor;
        GCDHeader  gcd;
        u8         max[0x400];
    }
    header;  // 1KB

    u8            rsa_pubkey[RSA_PUBKEY_NUM_FROM_BROM][ACS_PUBKEY_LEN];  // 1KB
    u8            aes_key[AESKEY_NUM_FROM_BROM][ACS_AES_LEN];  // 128B
    u8            hash_table_hash[ACS_HASH_LEN];  // 20B

    BLOWFISH_CTX  ds_blowfish;      // 4KB + α
    BLOWFISH_CTX  twl_blowfish;     // 4KB + α
}
OSFromBrom9Buf;

typedef struct
{
    union
    {
        NANDHeader nand;
        NORHeader  nor;
        GCDHeader  gcd;
        u8         max[0x400];
    }
    header;  // 1KB

    u8            rsa_pubkey[RSA_PUBKEY_NUM_FROM_BROM][ACS_PUBKEY_LEN];  // 512B
    u8            aes_key[AESKEY_NUM_FROM_BROM][ACS_AES_LEN];  // 64B
    u8            hash_table_hash[ACS_HASH_LEN];  // 20B

    BLOWFISH_CTX  twl_blowfish[2];  // (4KB + α) * 2

    SDPortContextData  SDNandContext;
}
OSFromBrom7Buf;

#ifdef SDK_ARM9
typedef OSFromBrom9Buf  OSFromBromBuf;
#else // SDK_ARM7
typedef OSFromBrom7Buf  OSFromBromBuf;
#endif // SDK_ARM7


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // FIRM_FORMAT_FROM_BROM_H_
