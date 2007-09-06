/* $Id$ */
/*
 * Copyright (C) 1998-2003 RSA Security Inc. All rights reserved.
 *
 * This work contains proprietary information of RSA Security.
 * Distribution is limited to authorized licensees of RSA
 * Security. Any unauthorized reproduction, distribution or
 * modification of this work is strictly prohibited.
 */

#ifndef HEADER_COMMON_AES_H
#define HEADER_COMMON_AES_H

#ifdef __cplusplus
extern "C" {
#endif

#define AES_ecb_encrypt(key, out, in) \
        AES_ecb_encrypt_com((key), (out), (in), AES_encrypt)
#define AES_ecb_encrypt_m(key, out, in) \
        AES_ecb_encrypt_com((key), (out), (in), AES_encrypt_m)
#define AES_ecb_encrypt_s(key, out, in) \
        AES_ecb_encrypt_com((key), (out), (in), AES_encrypt_s)

#define AES_ecb_decrypt(key, out, in) \
        AES_ecb_decrypt_com((key), (out), (in), AES_decrypt)
#define AES_ecb_decrypt_m(key, out, in) \
        AES_ecb_decrypt_com((key), (out), (in), AES_decrypt_m)
#define AES_ecb_decrypt_s(key, out, in) \
        AES_ecb_decrypt_com((key), (out), (in), AES_decrypt_s)

#define AES_cbc_encrypt(ctx, out, in, len, iv) \
        AES_cbc_encrypt_com((ctx), (out), (in), (len), (iv), AES_encrypt)
#define AES_cbc_encrypt_m(ctx, out, in, len, iv) \
        AES_cbc_encrypt_com((ctx), (out), (in), (len), (iv), AES_encrypt_m)
#define AES_cbc_encrypt_s(ctx, out, in, len, iv) \
        AES_cbc_encrypt_com((ctx), (out), (in), (len), (iv), AES_encrypt_s)

#define AES_cbc_decrypt(ctx, out, in, len, iv) \
        AES_cbc_decrypt_com((ctx), (out), (in), (len), (iv), AES_decrypt)
#define AES_cbc_decrypt_m(ctx, out, in, len, iv) \
        AES_cbc_decrypt_com((ctx), (out), (in), (len), (iv), AES_decrypt_m)
#define AES_cbc_decrypt_s(ctx, out, in, len, iv) \
        AES_cbc_decrypt_com((ctx), (out), (in), (len), (iv), AES_decrypt_s)

#define AES_cfb128_encrypt(ks, out, in, len, iv, num) \
        AES_cfb128_encrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_encrypt)
#define AES_cfb128_encrypt_m(ks, out, in, len, iv, num) \
        AES_cfb128_encrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_encrypt_m)
#define AES_cfb128_encrypt_s(ks, out, in, len, iv, num) \
        AES_cfb128_encrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_encrypt_s)

#define AES_cfb128_decrypt(ks, out, in, len, iv, num) \
        AES_cfb128_decrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_decrypt)
#define AES_cfb128_decrypt_m(ks, out, in, len, iv, num) \
        AES_cfb128_decrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_decrypt_m)
#define AES_cfb128_decrypt_s(ks, out, in, len, iv, num) \
        AES_cfb128_decrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_decrypt_s)

/** Note. OFB encryption is also used for decryption */
#define AES_ofb128_encrypt(ks, out, in, len, iv, num) \
        AES_ofb128_encrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_encrypt)
#define AES_ofb128_encrypt_m(ks, out, in, len, iv, num) \
        AES_ofb128_encrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_encrypt_m)
#define AES_ofb128_encrypt_s(ks, out, in, len, iv, num) \
        AES_ofb128_encrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_encrypt_s)

#define AES_ofb128_decrypt(ks, out, in, len, iv, num) \
        AES_ofb128_encrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_encrypt)
#define AES_ofb128_decrypt_m(ks, out, in, len, iv, num) \
        AES_ofb128_encrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_encrypt_m)
#define AES_ofb128_decrypt_s(ks, out, in, len, iv, num) \
        AES_ofb128_encrypt_com((ks), (out), (in), (len), (iv), (num), \
                AES_encrypt_s)


#define AES_MAXROUNDS           14

#define AES_ENCRYPT 1
#define AES_DECRYPT 0

typedef unsigned int AES_INT4;

typedef struct aes_key_st {
    unsigned int rounds;
    unsigned int key_size;
    AES_INT4 ks[(AES_MAXROUNDS+1)*8];
    } AES_KEY;

typedef void (* AES_ENC_DEC_FN)(AES_KEY *, AES_INT4 *);

void AES_encrypt(AES_KEY *ctx,AES_INT4 *data);
void AES_encrypt_m(AES_KEY *ctx,AES_INT4 *data);
void AES_encrypt_s(AES_KEY *ctx,AES_INT4 *data);
void AES_decrypt(AES_KEY *ctx,AES_INT4 *data);
void AES_decrypt_m(AES_KEY *ctx,AES_INT4 *data);
void AES_decrypt_s(AES_KEY *ctx,AES_INT4 *data);

void AES_ecb_encrypt_com(AES_KEY *key, unsigned char *out,
        const unsigned char *in, AES_ENC_DEC_FN enc_fn);
void AES_ecb_decrypt_com(AES_KEY *key, unsigned char *out,
        const unsigned char *in, AES_ENC_DEC_FN dec_fn);
void AES_cbc_encrypt_com(AES_KEY *ctx, unsigned char *out,
        const unsigned char *in, long length, unsigned char *iv,
        AES_ENC_DEC_FN enc_fn);
void AES_cbc_decrypt_com(AES_KEY *ctx, unsigned char *out,
        const  unsigned char *in, long length, unsigned char *iv,
        AES_ENC_DEC_FN dec_fn);
void AES_ofb128_encrypt_com(AES_KEY *ks,const unsigned char *in,
        unsigned char *out, long length, unsigned char *ivec, int *num,
        AES_ENC_DEC_FN enc_fn);
void AES_cfb128_encrypt_com(AES_KEY *ks, const  unsigned char *in,
        unsigned char *out, long length, unsigned char *ivec, int *num,
        AES_ENC_DEC_FN enc_fn);
void AES_cfb128_decrypt_com(AES_KEY *ks, const unsigned char *in,
        unsigned char *out, long length, unsigned char *ivec, int *num,
        AES_ENC_DEC_FN dec_fn);

int AES_set_key(AES_KEY *ctx, const unsigned char *key, int len);
void AES_convert_key(AES_KEY *ctx);
AES_INT4 AES_rotate(AES_INT4 u);
unsigned char AES_xtime(AES_INT4 x);

#ifdef __cplusplus
}
#endif

#endif /* HEADER_COMMON_AES_H */
