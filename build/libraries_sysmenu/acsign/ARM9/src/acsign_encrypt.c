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
#include "ber.h"

//  SHA1
#include    "sha.h"

#define     _DLENGTH_       (160/8)
#define     _BLENGTH_       (512/8)

#define     HASHContext     SHA_CTX

#define     HASHReset( _context )                   (void)SHA1_Init( _context )
#define     HASHSetSource( _context, _ptr, _len )   (void)SHA1_Update( _context, _ptr, _len )
#define     HASHGetDigest( _context, _ptr )         (void)SHA1_Final( _ptr, _context )

//  BN
#include    "bn.h"
#include    "bn_lcl.h"

#define BER_NULL                 5
#define BER_OBJECT               6
#define BER_SEQUENCE            16
#define BER_OCTET_STRING         4
#define BER_CONSTRUCTED     0x20


// RSAキー構成要素のパラメータ
typedef struct KeyParam {
	u8 	*pData;
	int	length;
}KeyParam;

static BOOL GetRSAPrivateKeyParam( const u8 *pKeyDER, KeyParam *pMod, KeyParam *pPrvExp );


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
    Memset(p,0xff,(u32)j);
    p[j]='\0';
    return(0);
}


//
// RSA
//
#define SIGN_DATA_ENCODE_BER_LEN	0x0f
#define ACS_ENCRYPTED_SIGN_LEN	128
BOOL ACSign_Encrypto(void *sign, const void *key, const void *data, int length, BOOL isEncodeBER )
{
    BN_CTX          *ctx;
    BIGNUM          src, dst, exp, mod;
    u8              buf[ACS_ENCRYPTED_SIGN_LEN];
	u8				dataBER[ACS_ENCRYPTED_SIGN_LEN];
    BOOL            result = TRUE;
	KeyParam		key_mod;
	KeyParam		key_prvExp;
	const void		*pData;
    int             len;
	
    if (NULL == sign || NULL == key || NULL == data || 0 > length || ( ACS_ENCRYPTED_SIGN_LEN - SIGN_DATA_ENCODE_BER_LEN ) < length ) {
        return FALSE;
    }

	if( !GetRSAPrivateKeyParam( key, &key_mod, &key_prvExp ) ) {
		OS_TPrintf( "RSA PrivKey Param get failed.\n" );
		return FALSE;
	}
	
	if( isEncodeBER ) {
		const u8 *pSignDataEncodeBER = (const u8 *)"\x30\x21\x30\x09\x06\x05\x2b\x0e\x03\x02\x1a\x05\x00\x04\x14";
		u8 *p = dataBER;
		int i;
		for( i = 0; i < SIGN_DATA_ENCODE_BER_LEN; i++ ) {
			*p++ = pSignDataEncodeBER[ i ];
		}
		for( i = 0; i < length; i++ ) {
			*p++ = ((const u8 *)data)[ i ];
		}
		pData = dataBER;
		length += SIGN_DATA_ENCODE_BER_LEN;
	}else {
		pData = data;
	}
	
    if ( add_padding( buf, ACS_ENCRYPTED_SIGN_LEN, pData, length ) ) {
        OS_TPrintf("encode_padding was failed.\n");
        result = FALSE;
        goto end;
    }


    ctx = BN_CTX_new();

    BN_init(&src);
    BN_init(&dst);
    BN_init(&exp);
    BN_init(&mod);

    BN_bin2bn((u8*)buf, ACS_ENCRYPTED_SIGN_LEN, &src);
    BN_bin2bn(key_prvExp.pData, key_prvExp.length, &exp);
    BN_bin2bn(key_mod.pData,    key_mod.length,    &mod);

    BN_mod_exp( &dst, &src, &exp, &mod, ctx );

	{	// BIGNUMは最上位が"0"の時、長さが減らされるので、あらかじめ調整しておく
		u8 *pSign = sign;
		int padLen = ACS_ENCRYPTED_SIGN_LEN - BN_num_bytes( &dst );
		while( ( padLen > 0 ) && ( padLen < ACS_ENCRYPTED_SIGN_LEN ) ) {
			*pSign++ = 0;
			padLen--;
		}
		
	    len = BN_bn2bin( &dst, pSign );
	}

    BN_free(&src);
    BN_free(&dst);
    BN_free(&exp);
    BN_free(&mod);

    if (ctx) {
        BN_CTX_free(ctx);
    }

    if ( len > ACS_ENCRYPTED_SIGN_LEN ) {	// サイズチェック。
		OS_TPrintf( "len = %d\n", len );
        result = FALSE;
        goto end;
    }
end:
    return result;
}


// DERフォーマットのRSA PublicKeyからパラメータを取得
static BOOL GetRSAPrivateKeyParam( const u8 *pKeyDER, KeyParam *pMod, KeyParam *pPrvExp )
{
	BOOL retval = FALSE;
	BER_ITEMS_SK sk;
	BER_ITEMS *bi;
	BER_ITEM item;
	
	// 先頭アイテムから、ファイル全体のサイズを算出
	if( BER_read_item( &item, (unsigned char *)pKeyDER, 16 ) ) {
		return FALSE;
	}
	
	BER_ITEMS_SK_init( &sk, NULL, 0, 0 );
	if( BER_parse( &sk, (unsigned char *)pKeyDER, item.data.len + item.hlen, NULL ) == BER_OK ) {
		bi = BER_ITEMS_SK_items( &sk, 2 );	// mod
		pMod->pData  = bi->item.data.bytes;
		pMod->length = (int)bi->item.data.len;
		
		bi = BER_ITEMS_SK_items( &sk, 4 );	// private exp
		pPrvExp->pData  = bi->item.data.bytes;
		pPrvExp->length = (int)bi->item.data.len;
		
		retval = TRUE;
	}
	BER_ITEMS_SK_free( &sk );
	
	return retval;
}

