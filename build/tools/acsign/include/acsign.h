#ifndef _ACSIGN_H_
#define _ACSIGN_H_

#include "sha.h"

#ifdef __cplusplus
extern "C" {
#endif

#define     HASHContext     SHA_CTX

#define     HASHReset( _context )             \
do{                                           \
            (_context)->sha_block = NULL;     \
            (void)SHA1_Init( _context );      \
} while(0)
#define     HASHUpdate( _context, _ptr, _len )      (void)SHA1_Update( _context, _ptr, _len )
#define     HASHGetDigest( _context, _ptr )         (void)SHA1_Final( _ptr, _context )

//
BOOL ACSign_Encrypto(void *sign, const void *key, const void *data, int length);
BOOL ACSign_Decrypto(void *buf, const void *key, const void *sign, int length);

//
int     ACSign_DigestUnit(
                    void*         buffer,     //  出力領域
                    const void*   buf,        //  データへのポインタ
                    unsigned int  len         //  データの長さ
                    );

//
int    ACSign_CompareUnit(
                    const void* decedHash,    //  ACSign_Decryptoの出力
                    const void* digest        //  ACSign_DigestUnitの出力
                    );

//
int     ACSign_GetKey(
                    void*           dest_ptr,   // 出力データへのポインタ
                    unsigned int    dest_len,   // 出力データの長さ
                    const void*     src_ptr,    // 入力データへのポインタ
                    unsigned int    src_len     // 入力データの長さ
                    );

#ifdef __cplusplus
}
#endif

#endif //_ACSIGN_H_
