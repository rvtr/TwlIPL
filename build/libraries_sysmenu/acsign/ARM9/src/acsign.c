/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <sysmenu/acsign.h>
#include "acmemory.h"

//  SHA1
#include    "sha.h"
#include    "sha1dgst.c"

#define     _DLENGTH_       (160/8)
#define     _BLENGTH_       (512/8)

#define     HASHContext     SHA_CTX

#define     HASHReset( _context )                   (void)SHA1_Init( _context )
#define     HASHSetSource( _context, _ptr, _len )   (void)SHA1_Update( _context, _ptr, _len )
#define     HASHGetDigest( _context, _ptr )         (void)SHA1_Final( _ptr, _context )

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


#if     !defined( RSA_ENC_DEC )

#define BER_NULL                 5
#define BER_OBJECT               6
#define BER_SEQUENCE            16
#define BER_OCTET_STRING         4
#define BER_CONSTRUCTED     0x20


//
//  rsa_padding_check_pkcs1_type_1関数相当
//
static 
int decode_padding( unsigned char* pdst_adrs, unsigned int* pdst_size, 
                    unsigned char* psrc_adrs, unsigned int  nsrc_size,
                    int length )
{
    unsigned char* p;
    unsigned char* q;
    unsigned char* pe;
    int i;

    if (nsrc_size >= length )
        return 0;

    /* The leading 0 was removed by the bignum->bin conversion */
    if (nsrc_size < 10)
        return 0;

    p  = psrc_adrs;
    pe = &(psrc_adrs[nsrc_size]);
    /* if (p[0] != 0) goto err; */
    if ( p[0] != 1 )
        return 0;
    p++;

    for ( i = 0; i < 8; i++ )
    {
        if ( *(p++) != 0xFF )
            return 0;
    }
    for ( ; p != pe; p++ )
    {
        if ( *p != 0xFF )
            break;
    }
    if ( p == pe )
        return 0;

    if ( *(p++) != 0 )
        return 0;

    *pdst_size = (unsigned int)((int)pe - (int)p);
    q = pdst_adrs;
    while ( p != pe )
        *(q++)= *(p++);

    return 1;
}

//
//
//
static
int pk_skip( unsigned char **datap, unsigned int *dlenp,
             unsigned char type, unsigned int *lenp)
{
    unsigned char *data = *datap;
    unsigned int dlen = *dlenp;
    unsigned int len = 0;
    unsigned char l;

    if (*(data++) != type)
        return 0;

    if (dlen < 1)
        return 0;
    dlen--;

    if ( (*data & 0x80) != 0 )
    {
        l = (unsigned char)( *data & 0x7F );
        if (dlen < (unsigned int)l + 1)
            return 0;
        dlen -= l;
        if (lenp != NULL)
        {
            data++;
            do
            {
                 len <<= 7;
                 len += (*data & 0x7f);
                 l--;
            }
            while (l > 0);
        }
        else
        {
            data += l;
        }
    }
    else
    {
        len = *data;
        data++;
        if (dlen < 1)
            return 0;
        dlen--;
    }

    *datap = data;
    *dlenp = dlen;
    if (lenp != NULL)
    {
        *lenp = len;
    }

    return 1;
}

//
//  PK_decode_sighash関数相当
//
static 
int decode_sighash( unsigned char* data, unsigned int dlen,
                    unsigned char** ppdgst_adrs, unsigned int* pdgst_size,
                    unsigned char** pphash_adrs, unsigned int* phash_size )
{
    #if 0
        /* 01 */ OB_cmpu(BER_SEQUENCE, 0, 0),
        /* 02 */ OB_down(0, 1),
        /* 03 */ OB_cmpu(BER_SEQUENCE, 0, 0),
        /* 04 */ OB_down(0, 1),
        /* 05 */ OB_cmpu(BER_OBJECT, 0, 0),
        /* 06 */ OB_call(OB_F_SET, PK_TYPE_SIG, PK_SIG_DIGEST_OID),

        /* go up a level - we got all we need out of the
         * sequence that contains the parameters
         * - we could OB_next(0,1) and check that we have BER_NULL
         * as then next item but there is no real need to do that
         */

        /* 07 */ OB_up(0, 1),
        /* 08 */ OB_next(0, 1),
        /* 09 */ OB_cmpu(BER_OCTET_STRING, 0, 0),
        /* 10 */ OB_call(OB_F_SET, PK_TYPE_SIG, PK_SIG_HASH),

        /* 11 */ OB_exit(0),
        /* 12 */ OB_FINISH,
    #endif

    unsigned int len;

    if ( !pk_skip(&data, &dlen, (BER_SEQUENCE|BER_CONSTRUCTED), NULL) )
        return 0;

    if ( !pk_skip(&data, &dlen, (BER_SEQUENCE|BER_CONSTRUCTED), NULL) )
        return 0;

    if ( !pk_skip(&data, &dlen, (BER_OBJECT), &len) )
        return 0;

    /*
    if (digest != NULL)
    {
        digest->adrs = data;
        digest->size = len;
    }
    */
    if ( ppdgst_adrs )  *ppdgst_adrs = data;
    if ( pdgst_size  )  *pdgst_size  = len;

    data += len;
    if (dlen < len)
        return 0;
    dlen -= len;

    if ( !pk_skip(&data, &dlen, BER_NULL, &len) )
        return 0;
    data += len;
    if (dlen < len)
        return 0;
    dlen -= len;

    if ( !pk_skip(&data, &dlen, (BER_OCTET_STRING), &len) )
        return 0;

    /*
    if (hash != NULL)
    {
        hash->adrs = data;
        hash->size = len;
    }
    */
    if ( pphash_adrs )  *pphash_adrs = data;
    if ( phash_size  )  *phash_size  = len;

    return 1;
}

#endif




//
#define     SGN_LEN     128
#define     MOD_LEN     128
#define     EXP_LEN     3

int     ACSign_Decrypto(
                    void*   buffer,     //  出力領域
                    void*   sgn_ptr,    //  データへのポインタ
                    void*   key_ptr     //  キーへのポインタ
                    )
{
    BN_CTX*         ctx;
    BIGNUM          src, dst, exp, mod;
    void*           exp_ptr;
    void*           mod_ptr;
    int             nTmp, nWrk;
    unsigned char*  pAdrs = 0;
    unsigned int    nSize = 0;
    unsigned long   nDummyExp = 0x00010001;     // 65537固定
    unsigned long   aBufferA[ 256 / sizeof (unsigned long) ];
    unsigned long   aBufferB[ 256 / sizeof (unsigned long) ];


    if ( !buffer  ) return 0;
    if ( !sgn_ptr ) return 0;
    if ( !key_ptr ) return 0;

    //
    ACMemory_Clear( );

    //
    exp_ptr = &nDummyExp;
    mod_ptr = key_ptr;

    (void)ACMemory_Memset( aBufferA, 0, sizeof aBufferA );
    (void)ACMemory_Memset( aBufferB, 0, sizeof aBufferB );

    ctx=BN_CTX_new();

    BN_init( &src );
    BN_init( &dst );
    BN_init( &exp );
    BN_init( &mod );

    (void)BN_bin2bn( sgn_ptr, SGN_LEN, &src );
    (void)BN_bin2bn( exp_ptr, EXP_LEN, &exp );
    (void)BN_bin2bn( mod_ptr, MOD_LEN, &mod );

    nTmp = BN_mod_exp( &dst, &src, &exp, &mod, ctx );

    nTmp = BN_bn2bin( &dst, (unsigned char*)aBufferA );

    BN_free( &src );
    BN_free( &dst );
    BN_free( &exp );
    BN_free( &mod );

    if (ctx != NULL)
        BN_CTX_free(ctx);

  #if   defined( RSA_ENC_DEC )

    pAdrs = (unsigned char*)aBufferA + 4;   //ダミー部４バイトは読み飛ばし
    nSize = nTmp;

    (void)ACMemory_Memcpy( buffer, pAdrs, _DLENGTH_ * 4 );

  #else

    if ( !decode_padding( (unsigned char*)aBufferB, (unsigned int*)&nWrk, (unsigned char*)aBufferA, (unsigned int )nTmp, SGN_LEN ) )
        return 0;
    if ( !decode_sighash( (unsigned char*)aBufferB, (unsigned int)nWrk, NULL, NULL, (unsigned char**)&pAdrs, (unsigned int*)&nSize ) )
        return 0;
    if ( nSize != _DLENGTH_ )
        return 0;

    (void)ACMemory_Memcpy( buffer, pAdrs, _DLENGTH_ * 1 );

  #endif

    return 1;
}


// 
#define     ROMH_SIZE       0x0160
int     ACSign_Digest(
                    void*   buffer,     //  出力領域
                    void*   romh_ptr,   //  データへのポインタ
                    void*   mbin_ptr,   //  データへのポインタ
                    int     mbin_len,   //  データの長さ
                    void*   sbin_ptr,   //  データへのポインタ
                    int     sbin_len,   //  データの長さ
                    u32     serial_num  //  シリアルナンバー
                    )
{
  #if   defined( RSA_ENC_DEC )
    HASHContext     context;


    if ( !buffer )                  return 0;
    if ( !romh_ptr )                return 0;
    if ( !mbin_ptr || !mbin_len )   return 0;
    if ( !sbin_ptr || !sbin_len )   return 0;


    HASHReset( &context );
    HASHSetSource( &context, romh_ptr, ROMH_SIZE );
    HASHGetDigest( &context, (unsigned char*)buffer + _DLENGTH_ * 0 );

    HASHReset( &context );
    HASHSetSource( &context, mbin_ptr, mbin_len );
    HASHGetDigest( &context, (unsigned char*)buffer + _DLENGTH_ * 1 );

    HASHReset( &context );
    HASHSetSource( &context, sbin_ptr, sbin_len );
    HASHGetDigest( &context, (unsigned char*)buffer + _DLENGTH_ * 2 );

    HASHReset( &context );
    HASHSetSource( &context, buffer, _DLENGTH_ * 3 );
    HASHGetDigest( &context, (unsigned char*)buffer + _DLENGTH_ * 3 );

  #else

    HASHContext     context;
    unsigned char   aBuffer[ (_DLENGTH_ * 3 + sizeof (unsigned long)) ];

    if ( !buffer )                  return 0;
    if ( !romh_ptr )                return 0;
    if ( !mbin_ptr || !mbin_len )   return 0;
    if ( !sbin_ptr || !sbin_len )   return 0;

    HASHReset( &context );
    HASHSetSource( &context, romh_ptr, ROMH_SIZE );
    HASHGetDigest( &context, (unsigned char*)aBuffer + _DLENGTH_ * 0 );

    HASHReset( &context );
    HASHSetSource( &context, mbin_ptr, mbin_len );
    HASHGetDigest( &context, (unsigned char*)aBuffer + _DLENGTH_ * 1 );

    HASHReset( &context );
    HASHSetSource( &context, sbin_ptr, sbin_len );
    HASHGetDigest( &context, (unsigned char*)aBuffer + _DLENGTH_ * 2 );

    //  シリアル番号分  今は０固定
    aBuffer[ (_DLENGTH_ * 3) + 0 ] = (unsigned char)((serial_num >>  0) & 0xFF);
    aBuffer[ (_DLENGTH_ * 3) + 1 ] = (unsigned char)((serial_num >>  8) & 0xFF);
    aBuffer[ (_DLENGTH_ * 3) + 2 ] = (unsigned char)((serial_num >> 16) & 0xFF);
    aBuffer[ (_DLENGTH_ * 3) + 3 ] = (unsigned char)((serial_num >> 24) & 0xFF);

    HASHReset( &context );
    HASHSetSource( &context, (unsigned char*)aBuffer, _DLENGTH_ * 3 + sizeof (unsigned long) );
    HASHGetDigest( &context, (unsigned char*)buffer );
  #endif
    return 1;
}


//
int     ACSign_Compare(
                    void* decrypto,      //  ACSign_Decryptoの出力
                    void* digest        //  ACSign_Digestの出力
                    )
{
    unsigned char* ptrA = (unsigned char*)decrypto;
    unsigned char* ptrB = (unsigned char*)digest;
    int loop;
    int test = 1;

    if ( !decrypto )  return 0;
    if ( !digest )  return 0;

  #if       defined( RSA_ENC_DEC )
    for ( loop = 0; loop < _DLENGTH_ * 4; loop++ )
  #else
    for ( loop = 0; loop < _DLENGTH_ * 1; loop++ )
  #endif
    {
        if ( *ptrA++ != *ptrB++ )
        {
            test = 0;
            break;
        }
    }
    return test;
}


