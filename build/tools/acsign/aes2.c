#include <stdlib.h>
#include <string.h>

#include "aes2.h"

#include "aes_e.c"
#include "aes_e_ecb.c"
#include "aes_skey.c"

//
// swap with memory allocation
//
static unsigned char* AES_Swap(const unsigned char *src, int len, int unit)
{
    int i;
    int j;
    unsigned char *dest = malloc(len);
    for (i = 0; i < len; i+=unit)
        for (j = 0; j < unit; j++)
            dest[i + j] = src[i + unit - j - 1];
    return dest;
}

//
// set keys
//
void AES_SetKey(AES_KEY *key, const unsigned char seed[AES_BLOCK_SIZE], const unsigned char id[AES_BLOCK_SIZE])
{
    static const unsigned char f[AES_BLOCK_SIZE] = {0xff, 0xfe, 0xfb, 0x4e, 0x29, 0x59, 0x02, 0x58, 0x2a, 0x68, 0x0f, 0x5f, 0x1a, 0x4f, 0x3e, 0x79};
    static const unsigned char s = 0x2a;

    unsigned char key1[AES_BLOCK_SIZE];
    unsigned char key2[AES_BLOCK_SIZE];
    int i;
    int o = 0;

    for (i = 0; i < AES_BLOCK_SIZE; i++) {
        key1[i] = seed[AES_BLOCK_SIZE-i-1] ^ id[AES_BLOCK_SIZE-i-1];
    }
    for (i = AES_BLOCK_SIZE - 1; i >= 0; i--) {
        int t = key1[i] + f[i] + o;
        o = (t > 0xFF ? 1 : 0);
        key1[i] = t & 0xFF;
    }
    for (i = 0; i < AES_BLOCK_SIZE; i++) {
        int j1 = (i + s / 8) % AES_BLOCK_SIZE;
        int j2 = (j1 + 1) % AES_BLOCK_SIZE;
        int k = s % 8;
        key2[i] = ((key1[j1] << k) & 0xFF) | ((key1[j2] >> (8 - k)) & 0xFF);
    }
    AES_set_key(key, key2, 16);
    memset(key2, 0, 16);
    memset(key1, 0, 16);
}

//
// ctr mode
//
#define GETU32(pt) (((unsigned long)(pt)[0] << 24) ^ ((unsigned long)(pt)[1] << 16) ^ ((unsigned long)(pt)[2] <<  8) ^ ((unsigned long)(pt)[3]))
#define PUTU32(ct, st) { (ct)[0] = (unsigned char)((st) >> 24); (ct)[1] = (unsigned char)((st) >> 16); (ct)[2] = (unsigned char)((st) >>  8); (ct)[3] = (unsigned char)(st); }

/* increment counter as u128 */
static void AES_ctr128_increment(unsigned char *counter) {
    unsigned long c;

    c = GETU32(counter + 12);
    c++;    c &= 0xFFFFFFFF;
    PUTU32(counter + 12, c);
    if (c)  return; // return unless overflow

    c = GETU32(counter +  8);
    c++;    c &= 0xFFFFFFFF;
    PUTU32(counter +  8, c);
    if (c)  return; // return unless overflow

    c = GETU32(counter +  4);
    c++;    c &= 0xFFFFFFFF;
    PUTU32(counter +  4, c);
    if (c)  return; // return unless overflow

    c = GETU32(counter +  0);
    c++;    c &= 0xFFFFFFFF;
    PUTU32(counter +  0, c);
}
static void AES_ctr128(const unsigned char *in, unsigned char *out, const unsigned long length, AES_KEY *key,
    unsigned char ivec[AES_BLOCK_SIZE], unsigned char ebuf[AES_BLOCK_SIZE], unsigned int *num) {

    unsigned int n;
    unsigned long l=length;

    n = *num;

    while (l--) {   // loop each byte
        if (n == 0) {
            AES_ecb_encrypt(key, ebuf, ivec);   // encrypt counter
            AES_ctr128_increment(ivec);         // increment counter
        }
        *(out++) = *(in++) ^ ebuf[n];
        n = (n+1) % AES_BLOCK_SIZE;
    }

    *num=n;
}

void AES_Ctr(AES_KEY *key, unsigned char *outdata, const unsigned char *indata, int len, unsigned char iv[AES_BLOCK_SIZE])
{
    unsigned char ebuf[AES_BLOCK_SIZE];
    unsigned char *tmp;
    unsigned int nums;

    if (len <= 0 || len > 0xFFFF00 || (len % AES_BLOCK_SIZE) != 0 || indata == NULL || outdata == NULL)
    {
        return;
    }

    // CTR
    memset(ebuf, 0, AES_BLOCK_SIZE);
    nums = 0;

    tmp = AES_Swap(iv, AES_BLOCK_SIZE, AES_BLOCK_SIZE);
    memcpy(iv, tmp, AES_BLOCK_SIZE);
    free(tmp);

    tmp = AES_Swap(indata, len, AES_BLOCK_SIZE);
    AES_ctr128(tmp, outdata, len, key, iv, ebuf, &nums);
    free(tmp);

    tmp = AES_Swap(outdata, len, AES_BLOCK_SIZE);
    memcpy(outdata, tmp, len);
    free(tmp);
    return;
}

