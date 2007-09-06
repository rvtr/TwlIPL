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

#ifndef HEADER_COMMON_SHA_LOCL_H
#define HEADER_COMMON_SHA_LOCL_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef STANDALONE
//#include "r_com.h"
#else
#if 1   //RSA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#define Memcpy memcpy
//#define Memset memset
#define Strcpy strcpy
#else   //NITRO
#include "acmemory.h"
#define Malloc(a)       ACMemory_Alloc(a)
#define Free(a)         ACMemory_Free(a)
#define Memset(a,b,c)   ACMemory_Memset(a,b,c)
#define Memcpy(a,b,c)   ACMemory_Memcpy(a,b,c)
#endif
#endif

#ifdef undef
/* one or the other must be defined */
#ifndef SHA_1 /* FIPE 180-1 */
#define SHA_0 /* FIPS 180   */
#endif
#endif

#define ULONG   unsigned long
#define UCHAR   unsigned char
#define UINT    unsigned int

#ifdef NOCONST
#define const
#endif

#undef c2nl
#define c2nl(c,l)   (l =(((unsigned long)(*((c)++)))<<24), \
             l|=(((unsigned long)(*((c)++)))<<16), \
             l|=(((unsigned long)(*((c)++)))<< 8), \
             l|=(((unsigned long)(*((c)++)))    ))

#undef p_c2nl
#define p_c2nl(c,l,n)   { \
            switch (n) { \
            case 0: l =((unsigned long)(*((c)++)))<<24; \
            case 1: l|=((unsigned long)(*((c)++)))<<16; \
            case 2: l|=((unsigned long)(*((c)++)))<< 8; \
            case 3: l|=((unsigned long)(*((c)++))); \
                } \
            }

#undef c2nl_p
/* NOTE the pointer is not incremented at the end of this */
#define c2nl_p(c,l,n)   { \
            l=0; \
            (c)+=n; \
            switch (n) { \
            case 3: l =((unsigned long)(*(--(c))))<< 8; \
            case 2: l|=((unsigned long)(*(--(c))))<<16; \
            case 1: l|=((unsigned long)(*(--(c))))<<24; \
                } \
            }

#undef p_c2nl_p
#define p_c2nl_p(c,l,sc,len) { \
            switch (sc) \
                { \
            case 0: l =((unsigned long)(*((c)++)))<<24; \
                if (--len == 0) break; \
            case 1: l|=((unsigned long)(*((c)++)))<<16; \
                if (--len == 0) break; \
            case 2: l|=((unsigned long)(*((c)++)))<< 8; \
                } \
            }

#undef nl2c
#define nl2c(l,c)   (*((c)++)=(unsigned char)(((l)>>24)&0xff), \
             *((c)++)=(unsigned char)(((l)>>16)&0xff), \
             *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
             *((c)++)=(unsigned char)(((l)    )&0xff))

#undef c2l
#define c2l(c,l)    (l =(((unsigned long)(*((c)++)))    ), \
             l|=(((unsigned long)(*((c)++)))<< 8), \
             l|=(((unsigned long)(*((c)++)))<<16), \
             l|=(((unsigned long)(*((c)++)))<<24))

#undef p_c2l
#define p_c2l(c,l,n)    { \
            switch (n) { \
            case 0: l =((unsigned long)(*((c)++))); \
            case 1: l|=((unsigned long)(*((c)++)))<< 8; \
            case 2: l|=((unsigned long)(*((c)++)))<<16; \
            case 3: l|=((unsigned long)(*((c)++)))<<24; \
                } \
            }

#undef c2l_p
/* NOTE the pointer is not incremented at the end of this */
#define c2l_p(c,l,n)    { \
            l=0; \
            (c)+=n; \
            switch (n) { \
            case 3: l =((unsigned long)(*(--(c))))<<16; \
            case 2: l|=((unsigned long)(*(--(c))))<< 8; \
            case 1: l|=((unsigned long)(*(--(c)))); \
                } \
            }

#undef p_c2l_p
#define p_c2l_p(c,l,sc,len) { \
            switch (sc) \
                { \
            case 0: l =((unsigned long)(*((c)++))); \
                if (--len == 0) break; \
            case 1: l|=((unsigned long)(*((c)++)))<< 8; \
                if (--len == 0) break; \
            case 2: l|=((unsigned long)(*((c)++)))<<16; \
                } \
            }

#undef l2c
#define l2c(l,c)    (*((c)++)=(unsigned char)(((l)    )&0xff), \
             *((c)++)=(unsigned char)(((l)>> 8)&0xff), \
             *((c)++)=(unsigned char)(((l)>>16)&0xff), \
             *((c)++)=(unsigned char)(((l)>>24)&0xff))

#ifdef OPT_SHA_B_ENDIAN
#define B_ENDIAN
#endif
#ifdef OPT_SHA_L_ENDIAN
#define L_ENDIAN
#endif

#undef ROTATE
#if defined(WIN32)
#define ROTATE(a,n)     _lrotl(a,n)
#else
#define ROTATE(a,n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#endif

/* byte order reversal */
#if defined(WIN32)
/* 5 instructions with rotate instruction, else 9 */
#define Endian_Reverse32(a) \
    { \
    unsigned long l=(a); \
    (a)=((ROTATE(l,8)&0x00FF00FF)|(ROTATE(l,24)&0xFF00FF00)); \
    }
#else
/* 6 instructions with rotate instruction, else 8 */
#define Endian_Reverse32(a) \
    { \
    unsigned long l=(a); \
    l=(((l&0xFF00FF00)>>8L)|((l&0x00FF00FF)<<8L)); \
    (a)=ROTATE(l,16L); \
    }
#endif

/* F() below can be
 * simplified to the code in F_00_19.  
 * #define F(x,y,z) (((x) & (y))  |  ((~(x)) & (z)))
 * another tweak to be made
 * in F_40_59, (x&a)|(y&a) -> (x|y)&a
 */
#define F_00_19(b,c,d)  ((((c) ^ (d)) & (b)) ^ (d)) 
#define F_20_39(b,c,d)  ((b) ^ (c) ^ (d))
#define F_40_59(b,c,d)  (((b) & (c)) | (((b)|(c)) & (d))) 
#define F_60_79(b,c,d)  F_20_39(b,c,d)

#ifdef SHA_0
#undef Xupdate
#define Xupdate(a,i,ia,ib,ic,id) X[(i)&0x0f]=(a)=\
    (ia[(i)&0x0f]^ib[((i)+2)&0x0f]^ic[((i)+8)&0x0f]^id[((i)+13)&0x0f]);
#endif
#ifdef SHA_1
#undef Xupdate
#define Xupdate(a,i,ia,ib,ic,id) (a)=\
    (ia[(i)&0x0f]^ib[((i)+2)&0x0f]^ic[((i)+8)&0x0f]^id[((i)+13)&0x0f]);\
    X[(i)&0x0f]=(a)=ROTATE((a),1);
#endif

#define BODY_00_15(i,a,b,c,d,e,f,xa) \
    (f)=xa[i]+(e)+K_00_19+ROTATE((a),5)+F_00_19((b),(c),(d)); \
    (b)=ROTATE((b),30);

#define BODY_60_79(i,a,b,c,d,e,f,xa) \
    Xupdate(f,i,xa,xa,xa,xa); \
    (f)=X[(i)&0x0f]+(e)+K_60_79+ROTATE((a),5)+F_60_79((b),(c),(d)); \
    (b)=ROTATE((b),30);

/*
 * The CodeWarrior compiler has a problem correctly expanding things like
 *
 *  (f)+=(e)+K_00_19+ROTATE((a),5)+F_00_19((b),(c),(d));
 *
 * so we separate this to 3 explicit adds, just for PalmOS.
 */
#ifndef UNDER_PALMOS

#define BODY_16_19(i,a,b,c,d,e,f,xa,xb,xc,xd) \
    Xupdate(f,i,xa,xb,xc,xd); \
    (f)+=(e)+K_00_19+ROTATE((a),5)+F_00_19((b),(c),(d)); \
    (b)=ROTATE((b),30);

#define BODY_20_31(i,a,b,c,d,e,f,xa,xb,xc,xd) \
    Xupdate(f,i,xa,xb,xc,xd); \
    (f)+=(e)+K_20_39+ROTATE((a),5)+F_20_39((b),(c),(d)); \
    (b)=ROTATE((b),30);

#define BODY_32_39(i,a,b,c,d,e,f,xa) \
    Xupdate(f,i,xa,xa,xa,xa); \
    (f)+=(e)+K_20_39+ROTATE((a),5)+F_20_39((b),(c),(d)); \
    (b)=ROTATE((b),30);

#define BODY_40_59(i,a,b,c,d,e,f,xa) \
    Xupdate(f,i,xa,xa,xa,xa); \
    (f)+=(e)+K_40_59+ROTATE((a),5)+F_40_59((b),(c),(d)); \
    (b)=ROTATE((b),30);

#else   /* UNDER_PALMOS */

#define BODY_16_19(i,a,b,c,d,e,f,xa,xb,xc,xd) \
    Xupdate(f,i,xa,xb,xc,xd); \
    (f)+=(e); \
    (f)+=K_00_19; \
    (f)+=ROTATE((a),5)+F_00_19((b),(c),(d)); \
    (b)=ROTATE((b),30);

#define BODY_20_31(i,a,b,c,d,e,f,xa,xb,xc,xd) \
    Xupdate(f,i,xa,xb,xc,xd); \
    (f)+=(e); \
    (f)+=K_20_39; \
    (f)+=ROTATE((a),5)+F_20_39((b),(c),(d)); \
    (b)=ROTATE((b),30);

#define BODY_32_39(i,a,b,c,d,e,f,xa) \
    Xupdate(f,i,xa,xa,xa,xa); \
    (f)+=(e); \
    (f)+=K_20_39; \
    (f)+=ROTATE((a),5)+F_20_39((b),(c),(d)); \
    (b)=ROTATE((b),30);

#define BODY_40_59(i,a,b,c,d,e,f,xa) \
    Xupdate(f,i,xa,xa,xa,xa); \
    (f)+=(e); \
    (f)+=K_40_59; \
    (f)+=ROTATE((a),5)+F_40_59((b),(c),(d)); \
    (b)=ROTATE((b),30);

#endif  /* UNDER_PALMOS */

#ifdef  __cplusplus
}
#endif

#endif  /* HEADER_COMMON_SHA_LOCL_H */
