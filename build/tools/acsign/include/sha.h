/* $Id$ */
/*
 * Copyright (C) 1998-2002 RSA Security Inc. All rights reserved.
 *
 * This work contains proprietary information of RSA Security.
 * Distribution is limited to authorized licensees of RSA
 * Security. Any unauthorized reproduction, distribution or
 * modification of this work is strictly prohibited.
 *
 */

#ifndef HEADER_COMMON_SHA_H
#define HEADER_COMMON_SHA_H

#ifdef  __cplusplus
extern "C" {
#endif

#if !defined(CCONV)
#define CCONV
#endif

#ifndef PRE_CCONV
#define PRE_CCONV
#endif

#define SHA_CBLOCK	64
#define SHA_LBLOCK	16
#define SHA_BLOCK	16
#define SHA_LAST_BLOCK  56
#define SHA_LENGTH_BLOCK 8
#define SHA_DIGEST_LENGTH 20

/* the default is to use unsigned long */
#if !defined(OPT_SHA_INT) && !defined(OPT_SHA_LONG)
#define OPT_SHA_LONG
#endif

#ifdef OPT_SHA_LONG
#define SHA_LONG unsigned long
#endif

#ifdef OPT_SHA_INT
#define SHA_LONG unsigned int
#endif

typedef struct sha_ctx_st SHA_CTX;

struct sha_ctx_st
	{
	SHA_LONG h0,h1,h2,h3,h4;
	SHA_LONG Nl,Nh;
	SHA_LONG data[SHA_LBLOCK];
	int num;
	void (PRE_CCONV CCONV *sha_block)(SHA_CTX *c, const unsigned char *W, 
	                                  int num);
	};

#ifndef NOPROTO
int SHA1_Setup(SHA_CTX *c, void (PRE_CCONV CCONV *sha_block)(SHA_CTX *c, 
                           const unsigned char *W, int num));
void SHA_Init(SHA_CTX *c);
void SHA_Update(SHA_CTX *c, const unsigned char *data, unsigned long len);
void SHA_Final(unsigned char *md, SHA_CTX *c);
unsigned char *SHA(const unsigned char *d, unsigned long n,unsigned char *md);
void SHA_Transform(SHA_CTX *c, const unsigned char *data);
void SHA1_Init(SHA_CTX *c);
void SHA1_Update(SHA_CTX *c, const unsigned char *data, unsigned long len);
void SHA1_Final(unsigned char *md, SHA_CTX *c);
unsigned char *SHA1(const unsigned char *d, unsigned long n,unsigned char *md);
void SHA1_Transform(SHA_CTX *c, const unsigned char *data);
#else
void SHA_Init();
void SHA_Update();
void SHA_Final();
unsigned char *SHA();
void SHA_Transform();
void SHA1_Init();
void SHA1_Update();
void SHA1_Final();
unsigned char *SHA1();
void SHA1_Transform();
#endif

#ifdef  __cplusplus
}
#endif

#endif /* HEADER_COMMON_SHA_H */
