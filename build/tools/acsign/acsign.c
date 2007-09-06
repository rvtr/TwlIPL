#include <string.h>
#include "format_sign.h"
#include "acsign.h"

//  SHA1
#include    "sha1dgst.c"

// RSA

//  BN
#include    "bn.h"

#include    "bn_lib.c"
#include    "bn_asm.c"
#include    "bn_comba.c"
#include    "bn_lsh.c"
#include    "bn_rsh.c"
#include    "bn_word.c"
#include    "bn_add.c"
#include    "bn_mul.c"
#include    "bn_div.c"
#include    "bn_exp.c"
#include    "bn_sqr.c"
#include    "bn_fm_w.c"
#include    "bn_gcd.c"
#include    "bn_ex_str.c"
#include    "bn_ms_w.c"
//#include    "bn_me.c"
#include    "bn_rec.c"
#include    "bn_mont.c"
#include    "bn_recp.c"
#include    "bn_wdiv.c"
#include    "bn_r_exp.c"
#include    "bn_m_exp.c"



#define BER_NULL                 5
#define BER_OBJECT               6
#define BER_SEQUENCE            16
#define BER_OCTET_STRING         4
#define BER_CONSTRUCTED     0x20


/**
 * Perform the EMSA-PKCS1_v1_5 padding
 *
 * @param random   random context for seed data generation
 * @param out      destination buffer
 * @param out_len  length written to out
 * @param in       data to be padded
 * @param in_len   length of in
 * @param flags    mask of operation directives
 *
 * @pre  out_len must contain length of data allocated to out
 * @pre  out must be the allocated the total length to be padded
 *
 * @note Method from PKCS#1 v2.1 standard 9.2.1 EMSA-PKCS1-v1_5
 * @li In: Hash is hash funtion, hLen denotes length of octets of hash output
 * @li In: M is message to be encoded
 * @li In: emLen is intended length of octets
 * @li Out: EM encoded message of length emLen
 *
 * @li Apply the hash function to message M to produce a hash value H
 *     If hash output is message to long the output message too long and
 *     stop
 * @li Encode the algorithm ID for the hash in function into an ASN1 value
 * @li if emLen < 10 + BER encoded message T output message length too short
 *     and stop
 * @li generate octet string PS consisting of emLen - Length(T) - 2 octets
 *     with value FF
 * @li Concatenate PS, DER encoding T and other padding such that EM is:
 *     01 || PS || 00 || T
 *
 * @note it is the out layers responsibility to DER encode the
 * data prior to padding as described in the first 2 steps in method
 */
//
//  rsa_padding_add_pkcs1_type_1関数相当
//
static int add_padding(unsigned char *out,
        int out_len, const unsigned char *in, int in_len)
{
    unsigned char *p;
    int j,i;

    if ((in_len+11) > out_len)
        return(1);

    /* First we copy data bytes to the output buffer, this
     * way the input and output buffers can be the same
     * and things will still work */
    p=out+out_len-in_len;
    for (i=in_len-1; i>=0; i--)
        p[i]=in[i];
    p=out;
    *(p++)=0;
    *(p++)=1; /* Private Key BT (Block Type) */

    /* pad with 0xff data */
    j=out_len-3-in_len;
    Memset(p,0xff,j);
    p[j]='\0';
    return(0);
}

static void debug_dump(const void* buf, int len, const char* str, int line_elms)
{
    const u8* bufp = (u8*)buf;
    int i,ii;

    if (str)
    {
        debug_printf("%s :\n", str);
    }
    for (i=0; i<=len/line_elms; i++)
    {
        if (i*line_elms >= len)
        {
            break;
        }
        for (ii=0; ii<line_elms; ii++)
        {
            if (i*line_elms+ii >= len)
            {
                break;
            }
            debug_printf("%02x ", bufp[i*line_elms+ii]);
        }
        debug_printf("\n");
    }
}

//
// RSA
//
BOOL ACSign_Encrypto(void *sign, const void *key, const void *data, int length)
{
    BN_CTX          *ctx;
    BIGNUM          src, dst, exp, mod;
    u8*             key_exp = &((u8*)key)[ACS_RSA_PRVEXP_OFFSET];
    u8*             key_mod = &((u8*)key)[ACS_RSA_PRVMOD_OFFSET];
    u8              buf[ACS_ENCRYPTED_HASH_LEN];
    u32             len = length;
    BOOL            result = TRUE;

    if (NULL == sign || NULL == key || NULL == data || 0 > length) {
        return FALSE;
    }

    if ( add_padding( buf, ACS_ENCRYPTED_HASH_LEN, data, length ) ) {
        debug_printf2("encode_padding was failed.\n");
        result = FALSE;
        goto end;
    }

    {
        debug_dump(buf, ACS_ENCRYPTED_HASH_LEN, "padded hash", 16);
        debug_dump(key_mod, ACS_RSA_PRVMOD_LEN, "key mod", 16);
        debug_dump(key_exp, ACS_RSA_PRVEXP_LEN, "key exp", 16);
    }

    ctx = BN_CTX_new();

    BN_init(&src);
    BN_init(&dst);
    BN_init(&exp);
    BN_init(&mod);

    BN_bin2bn((u8*)buf, ACS_ENCRYPTED_HASH_LEN, &src);
    BN_bin2bn(key_exp, ACS_RSA_PRVEXP_LEN, &exp);
    BN_bin2bn(key_mod, ACS_RSA_PRVMOD_LEN, &mod);

    BN_mod_exp( &dst, &src, &exp, &mod, ctx );

    len = BN_bn2bin( &dst, sign );

    BN_free(&src);
    BN_free(&dst);
    BN_free(&exp);
    BN_free(&mod);

    if (ctx) {
        BN_CTX_free(ctx);
    }

    if ( len != ACS_DECRYPTED_HASH_LEN ) {
        result = FALSE;
        goto end;
    }
end:
    return result;
}

BOOL ACSign_Decrypto(void *buf, const void *key, const void *sign, int length)
{
    BN_CTX          *ctx;
    BIGNUM          src, dst, exp, mod;
    u32             key_exp = ACS_RSA_EXP;
    u8*             key_mod = &((u8*)key)[ACS_RSA_PRVMOD_OFFSET];
    u8*             bufp = (u8*)buf;
    u32             len = length;
    BOOL            result = TRUE;

    if (NULL == buf || NULL == key || NULL == sign || 0 > length) {
        return FALSE;
    }

    ctx = BN_CTX_new();

    BN_init(&src);
    BN_init(&dst);
    BN_init(&exp);
    BN_init(&mod);

    BN_bin2bn((u8*)sign, ACS_ENCRYPTED_HASH_LEN, &src);
    BN_bin2bn((u8*)&key_exp, ACS_RSA_PUBEXP_LEN, &exp);
    BN_bin2bn(key_mod, ACS_RSA_PRVMOD_LEN, &mod);

    BN_mod_exp( &dst, &src, &exp, &mod, ctx );

    len = BN_bn2bin( &dst, bufp );

    BN_free(&src);
    BN_free(&dst);
    BN_free(&exp);
    BN_free(&mod);

    if (ctx) {
        BN_CTX_free(ctx);
    }

    if ( len != ACS_DECRYPTED_HASH_LEN ) {
        result = FALSE;
        goto end;
    }
end:
    return result;
}

//
int     ACSign_DigestUnit(
                    void*         buffer,     //  出力領域
                    const void*   buf,        //  データへのポインタ
                    unsigned int  len         //  データの長さ
                    )
{
    HASHContext     context;
    unsigned char   *bufferp = buffer;

    HASHReset( &context );
    HASHUpdate( &context, buf, len );
    HASHGetDigest( &context, bufferp );

    return TRUE;
}

//
int    ACSign_CompareUnit(
                    const void* decedHash,    //  ACSign_Decryptoの出力
                    const void* digest        //  ACSign_DigestUnitの出力
                    )
{
    const unsigned char* dgt = digest;
    const unsigned char* dgtCmp = decedHash;
    int i;
    int test = TRUE;

    if ( !decedHash )  return FALSE;
    if ( !digest )     return FALSE;

    for ( i = 0; i < ACS_HASH_LEN; i++ )
    {
        if ( *dgt++ != *dgtCmp++ )
        {
            test = FALSE;
            break;
        }
    }
    return test;
}

/*
    任意バイト数の入力データから
    任意バイト数の出力データを得ます。
    偏っているかもしれない入力値を一様な値にマップするだけです。
    入出力メモリがかぶっていても問題ない
*/
int ACSign_GetKey(
                    void*           dest_ptr,   // 出力データへのポインタ
                    unsigned int    dest_len,   // 出力データの長さ
                    const void*     src_ptr,    // 入力データへのポインタ
                    unsigned int    src_len     // 入力データの長さ
                    )
{
    HASHContext ctx;
    unsigned char *ptr;
    unsigned char state[ACS_HASH_LEN];
    unsigned char output[ACS_HASH_LEN];
    int i;

    if (dest_ptr == NULL)
        return 1;
    if (src_ptr == NULL && src_len > 0)
        return 0;

    HASHReset(&ctx);

    HASHUpdate(&ctx, src_ptr, src_len);
    HASHGetDigest(&ctx, state);

    ptr = dest_ptr;
    while (dest_len > 0)
    {
        unsigned int len = dest_len < ACS_HASH_LEN ? dest_len : ACS_HASH_LEN;

        // plus one
        for (i = 0; i < ACS_HASH_LEN; i++)
        {
            if (state[ACS_HASH_LEN-1-i]++)
                break;
        }

        HASHUpdate(&ctx, state, ACS_HASH_LEN);
        HASHGetDigest(&ctx, output);
        memcpy(ptr, output, len);
        ptr += len;
        dest_len -= len;
    }
    memset(state, 0, ACS_HASH_LEN);
    memset(output, 0, ACS_HASH_LEN);
    return 1;
}

