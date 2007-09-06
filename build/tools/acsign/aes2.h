#ifndef __AES_2_H__
#define __AES_2_H__

#include "aes.h"

#define AES_BLOCK_SIZE  16
#define AES_NONCE_SIZE  12
#define AES_QUANTITY    (AES_BLOCK_SIZE - AES_NONCE_SIZE - 1)

#ifdef __cplusplus
extern "C" {
#endif

void AES_SetKey(AES_KEY *key, const unsigned char seed[AES_BLOCK_SIZE], const unsigned char id[AES_BLOCK_SIZE]);
void AES_Ctr(AES_KEY *key, unsigned char *outdata, const unsigned char *indata, int len, unsigned char iv[AES_BLOCK_SIZE]);

#ifdef __cplusplus
}
#endif

#endif  // __AES_2_H__
