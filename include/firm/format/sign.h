/*---------------------------------------------------------------------------*
  Project:  TwlFirm - format - sign
  File:     sign.h

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
#ifndef FORMAT_SIGN_H_
#define FORMAT_SIGN_H_


#ifdef __cplusplus
extern "C" {
#endif

// signed hash index
typedef enum
{
    FIRM_SIGNED_HASH_IDX_HEADER     = 0,
    FIRM_SIGNED_HASH_IDX_ARM9       = 1,
    FIRM_SIGNED_HASH_IDX_ARM7       = 2,
    FIRM_SIGNED_HASH_IDX_HASH_TABLE = 3,
    FIRM_SIGNED_HASH_IDX_FINAL      = 4,

    FIRM_SIGNED_HASH_NUM            = 5
}
FIRMSignedHashIndex;


#define ACS_BASE_BLOCK_SIZE  4096    // should be the multiple of 16
#define ACS_META_BLOCK_SIZE  5120    // should be the multiple of 16 and the multiple of 20
                                    // would be the multiple of 512, then the mutilple of 2560

#define ACS_PUBKEY_LEN       128
#define ACS_HASH_LEN         20
#define ACS_AES_LEN          16

#define ACS_ENCRYPTED_HASH_LEN   ACS_PUBKEY_LEN
#define ACS_DECRYPTED_HASH_LEN   ACS_HASH_LEN

#define ACS_RSA_EXP          0x00010001
#define ACS_RSA_EXP_LEN      3

// DER format of RSA keys

#define ACS_RSA_PRVMOD_OFFSET    0x0B
#define ACS_RSA_PRVEXP_OFFSET    0x93
#define ACS_RSA_PRVMOD_LEN       128
#define ACS_RSA_PRVEXP_LEN       128

#define ACS_RSA_PUBMOD_OFFSET    0x1D
#define ACS_RSA_PUBEXP_OFFSET    0x93
#define ACS_RSA_PUBMOD_LEN       128
//#define ACS_RSA_PUBEXP_LEN       ACS_RSA_EXP_LEN

#define FIRM_HEADER_2ND_HASH_AREA_LEN   (sizeof(FIRMSignedContext) - ACS_HASH_LEN)


// 署名コンテキスト
typedef struct
{
    unsigned char   aes_key[ACS_AES_LEN];
    unsigned char   hash[FIRM_SIGNED_HASH_NUM][ACS_HASH_LEN];
}
FIRMSignedContext;

// 署名
typedef union
{
    struct
    {
        unsigned char  prePad[(ACS_ENCRYPTED_HASH_LEN - sizeof(FIRMSignedContext))-1];
        FIRMSignedContext  c;
        unsigned char  postPad[1];
    }
    e;
    unsigned int    raw[ACS_ENCRYPTED_HASH_LEN/4];
}
FIRMPaddedSign;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //FORMAT_SIGN_H_
