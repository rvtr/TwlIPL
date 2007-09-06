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
                    void*         buffer,     //  �o�͗̈�
                    const void*   buf,        //  �f�[�^�ւ̃|�C���^
                    unsigned int  len         //  �f�[�^�̒���
                    );

//
int    ACSign_CompareUnit(
                    const void* decedHash,    //  ACSign_Decrypto�̏o��
                    const void* digest        //  ACSign_DigestUnit�̏o��
                    );

//
int     ACSign_GetKey(
                    void*           dest_ptr,   // �o�̓f�[�^�ւ̃|�C���^
                    unsigned int    dest_len,   // �o�̓f�[�^�̒���
                    const void*     src_ptr,    // ���̓f�[�^�ւ̃|�C���^
                    unsigned int    src_len     // ���̓f�[�^�̒���
                    );

#ifdef __cplusplus
}
#endif

#endif //_ACSIGN_H_
