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

/*
 * Copyright (C) 1998-2002 RSA Security Inc. All rights reserved.
 *
 * This work contains proprietary information of RSA Security.
 * Distribution is limited to authorized licensees of RSA
 * Security. Any unauthorized reproduction, distribution or
 * modification of this work is strictly prohibited.
 *
 */

////!!!!#include "r_com.h"

#ifndef NO_SHA1
#undef  SHA_0
#define SHA_1
#include "sha.h"
#include "sha_locl.h"
#ifdef CPU_X86
#include "r_cpuid.h"
#endif /* NO_SHA1 */

const char *SHA1_version="SHA1 part of RCOM 2.3.0 11-Jun-2002";

/* Implemented from SHA-1 document - The Secure Hash Algorithm
 */

#define INIT_DATA_h0 (SHA_LONG)0x67452301L
#define INIT_DATA_h1 (SHA_LONG)0xefcdab89L
#define INIT_DATA_h2 (SHA_LONG)0x98badcfeL
#define INIT_DATA_h3 (SHA_LONG)0x10325476L
#define INIT_DATA_h4 (SHA_LONG)0xc3d2e1f0L

#define K_00_19 (SHA_LONG)0x5a827999L
#define K_20_39 (SHA_LONG)0x6ed9eba1L
#define K_40_59 (SHA_LONG)0x8f1bbcdcL
#define K_60_79 (SHA_LONG)0xca62c1d6L

#ifndef CCONV
#define CCONV 
#endif

#ifndef PRE_CCONV
#define PRE_CCONV
#endif

/* Endian flags are only used for the assembler code */ 
#ifndef OPT_SHA1_ASM 
#undef L_ENDIAN 
#undef B_ENDIAN 
#endif 


#ifdef OPT_SHA1_ASM

#ifdef CPU_X86
void CCONV sha1_block_586(SHA_CTX *c,const unsigned char *p, int num);
void CCONV sha1_block_686(SHA_CTX *c,const unsigned char *p, int num);
void CCONV sha1_block_786(SHA_CTX *c,const unsigned char *p, int num);
unsigned long r_cpuid(unsigned long *,char **name);

#elif OPT_SHA1_ARM
PRE_CCONV void CCONV sha1_arm4_fast(SHA_CTX *c, const unsigned char *p,
    int num);
PRE_CCONV void CCONV sha1_arm4_small(SHA_CTX *c, const unsigned char *p,
    int num);
#else
void CCONV sha1_block_asm(SHA_CTX *c, const unsigned char *p, int num);
#endif /* CPU_X86 */

#else /* OPT_SHA1_ASM */

void sha1_block(SHA_CTX *c, SHA_LONG *p, int num);

#endif /* OPT_SHA1_ASM */

#undef M_c2nl
#undef M_p_c2nl
#undef M_c2nl_p
#undef M_p_c2nl_p
#undef M_nl2c

#if defined(L_ENDIAN) && !defined(OPT_SHA1_ASM)
#  define   M_c2nl      c2l
#  define   M_p_c2nl    p_c2l
#  define   M_c2nl_p    c2l_p
#  define   M_p_c2nl_p  p_c2l_p
#  define   M_nl2c      l2c
#else
#  define   M_c2nl      c2nl
#  define   M_p_c2nl    p_c2nl
#  define   M_c2nl_p    c2nl_p
#  define   M_p_c2nl_p  p_c2nl_p
#  define   M_nl2c      nl2c
#endif /* defined(L_ENDIAN) && !defined(OPT_SHA1_ASM) */

int SHA1_Setup(c,sha_block)
SHA_CTX *c;
void (PRE_CCONV CCONV *sha_block)(SHA_CTX *c, const unsigned char *W, int num);
    {
    c->sha_block=sha_block;
    return(0);
    }

void SHA1_Init(c)
SHA_CTX *c;
    {
    c->h0=INIT_DATA_h0;
    c->h1=INIT_DATA_h1;
    c->h2=INIT_DATA_h2;
    c->h3=INIT_DATA_h3;
    c->h4=INIT_DATA_h4;
    c->Nl=0;
    c->Nh=0;
    c->num=0;

#ifdef OPT_SHA1_ASM
#ifdef CPU_X86
    if (c->sha_block == NULL)
            {
            unsigned long cpu,attrib;
        
            /* We should make the methods loadable */
            cpu=r_cpuid(&attrib,NULL);
            if (attrib & R_CPU_X86_HAS_PENTIUM_IV)
            c->sha_block=sha1_block_786;
        else if (attrib & R_CPU_X86_HAS_PENTIUM_PRO)
            c->sha_block=sha1_block_686;
        else
            c->sha_block=sha1_block_586;
        }
#else /* CPU_X86 */

#ifndef OPT_SHA1_ARM
    c->sha_block=sha1_block_asm;
#else   /* OPT_SHA1_ARM */
    if (c->sha_block == NULL)
        {
#ifdef SMALL_CODE_SIZE
        c->sha_block = sha1_arm4_small;
#else /* SMALL_CODE_SIZE */
        c->sha_block = sha1_arm4_fast;
#endif /* SMALL_CODE_SIZE */
        }
#endif  /* OPT_SHA1_ARM */
#endif /* CPU_X86 */

#else /* OPT_SHA1_ASM */
    c->sha_block=(void (PRE_CCONV CCONV *)(SHA_CTX *, const unsigned char *, int))sha1_block;
#endif /* OPT_SHA1_ASM */
    }

#ifdef OPT_SHA1_ASM

void SHA1_Update(c, data, len)
SHA_CTX *c;
const register unsigned char *data;
unsigned long len;
    {
    int i;
    unsigned int    alignment;
    unsigned long l;
    unsigned char *cp=(unsigned char *)c->data;

    if (len == 0) return;

    l=(c->Nl+(len<<3))&0xffffffffL;
    if (l < c->Nl) /* overflow */
        c->Nh++;
    c->Nh+=(len>>29);
    c->Nl=l;

    if (c->num != 0)
        {
        if (c->num+len >= SHA_CBLOCK)
            {
            i=SHA_CBLOCK-c->num;
            Memcpy(&(cp[c->num]),data,i);
            len-=i;
            data+=i;

            c->sha_block(c,cp,64);
            c->num=0;
            /* drop through and do the rest */
            }
        else
            {
            Memcpy(&(cp[c->num]),data,len);
            c->num+=(int)len;
            return;
            }
        }
    /* we now can process the input data in blocks of SHA_CBLOCK
     * chars and save the leftovers to c->data. */
    if (len >= SHA_CBLOCK)
        {
        i=(int)(len& ~63);
        len-=i;

        /*
         * Check to see if the input data lies on a word boundary.
         * Do this as the ASM relies on input data being word aligned.
         */
        alignment = (((unsigned int)data) & (sizeof(unsigned int)-1))
                & 0x03;
        if (alignment == 0)
            {
            c->sha_block(c,data,i);
            data+=i;
            }
        else
            {
            do  {
                Memcpy(cp, data, SHA_CBLOCK);
                data += SHA_CBLOCK;
                c->sha_block(c, cp, SHA_CBLOCK);
                i -= SHA_CBLOCK;
                } while (i > 0);
            }
        }
    c->num=len;
    if (len)
        {
        Memcpy(cp,data,(int)len);
        }
    }

void SHA1_Transform(c,b)
SHA_CTX *c;
const unsigned char *b;
    {
    c->sha_block(c,b,64);
    }

void SHA1_Final(md, c)
unsigned char *md;
SHA_CTX *c;
    {
    register int i,j;
    register SHA_LONG l;
    register SHA_LONG *p;
    const static unsigned char end[4]={0x80,0x00,0x00,0x00};
    unsigned char *cp= (unsigned char *)end;
    unsigned char *pc;

    /* c->num should definitly have room for at least one more byte. */
    p=c->data;
    j=c->num;
    i=j>>2;

#ifdef PURIFY
        /* PURIFY */
        /* we reference uninitialised data but don't keep the result
         * which purify complains about ... and we don't want to have
         * to come back here to find a non-existant problem later
         */

    /* purify often complains about the following line as an
     * Uninitialized Memory Read.  While this can be true, the
     * following p_c2l macro will reset l when that case is true.
     * This is because j&0x03 contains the number of 'valid' bytes
     * already in p[i].  If and only if j&0x03 == 0, the UMR will
     * occur but this is also the only time p_c2l will do
     * l= *(cp++) instead of l|= *(cp++)
         */
        if ((j&0x03) == 0) p[i]=0;
#endif

    pc=(unsigned char *)c->data;
    pc[j]=0x80;
    for (j++; j & 0x03; j++)
        pc[j]=0;
    i++;
    /* i is the next 'undefined word' */
    if (c->num >= SHA_LAST_BLOCK)
        {
        for (; i<SHA_LBLOCK; i++)
            p[i]=0;
        c->sha_block(c,(unsigned char *)p,64);
        i=0;
        }
    for (; i<(SHA_LBLOCK-2); i++)
        p[i]=0;

    l=c->Nl;
    pc[63]=(unsigned char)((l    )&0xff);
    pc[62]=(unsigned char)((l>> 8)&0xff);
    pc[61]=(unsigned char)((l>>16)&0xff);
    pc[60]=(unsigned char)((l>>24)&0xff);
    l=c->Nh;
    pc[59]=(unsigned char)((l    )&0xff);
    pc[58]=(unsigned char)((l>> 8)&0xff);
    pc[57]=(unsigned char)((l>>16)&0xff);
    pc[56]=(unsigned char)((l>>24)&0xff);

    c->sha_block(c,(unsigned char *)p,64);
    cp=md;
    l=c->h0; nl2c(l,cp);
    l=c->h1; nl2c(l,cp);
    l=c->h2; nl2c(l,cp);
    l=c->h3; nl2c(l,cp);
    l=c->h4; nl2c(l,cp);

    /* clear stuff, sha1_block_asm may be leaving some stuff on the stack
     * but I'm not worried :-) */
    c->num=0;
/*  Memset((char *)&c,0,sizeof(c));*/
    }

#else /* !OPT_SHA1_ASM */

void SHA1_Update(c, data, len)
SHA_CTX *c;
const register unsigned char *data;
unsigned long len;
    {
    register SHA_LONG *p;
    int ew,ec,sw,sc;
    SHA_LONG l;

    if (len == 0) return;

    l=(c->Nl+(len<<3))&0xffffffffL;
    if (l < c->Nl) /* overflow */
        c->Nh++;
    c->Nh+=(len>>29);
    c->Nl=l;

    if (c->num != 0)
        {
        p=c->data;
        sw=c->num>>2;
        sc=c->num&0x03;

        if ((c->num+len) >= SHA_CBLOCK)
            {
            l= p[sw];
            M_p_c2nl(data,l,sc);
            p[sw++]=l;
            for (; sw<SHA_LBLOCK; sw++)
                {
                M_c2nl(data,l);
                p[sw]=l;
                }
            len-=(SHA_CBLOCK-c->num);

            c->sha_block(c,(unsigned char *)p,64);
            c->num=0;
            /* drop through and do the rest */
            }
        else
            {
            c->num+=(int)len;
            if ((sc+len) < 4) /* ugly, add char's to a word */
                {
                l= p[sw];
                M_p_c2nl_p(data,l,sc,len);
                p[sw]=l;
                }
            else
                {
                ew=(c->num>>2);
                ec=(c->num&0x03);
                l= p[sw];
                M_p_c2nl(data,l,sc);
                p[sw++]=l;
                for (; sw < ew; sw++)
                    { M_c2nl(data,l); p[sw]=l; }
                if (ec)
                    {
                    M_c2nl_p(data,l,ec);
                    p[sw]=l;
                    }
                }
            return;
            }
        }
    /* We can only do the following code for assember, the reason
     * being that the sha1_block 'C' version changes the values
     * in the 'data' array.  The assember code avoids this and
     * copies it to a local array.  I should be able to do this for
     * the C version as well....
     */
#if defined(B_ENDIAN) || defined(OPT_SHA1_ASM)
    if ((((unsigned long)data)%sizeof(SHA_LONG)) == 0)
        {
        sw=len/SHA_CBLOCK;
        if (sw)
            {
            sw*=SHA_CBLOCK;
            c->sha_block(c,(SHA_LONG *)data,sw);
            data+=sw;
            len-=sw;
            }
        }
#endif
    /* we now can process the input data in blocks of SHA_CBLOCK
     * chars and save the leftovers to c->data. */
    p=c->data;
    while (len >= SHA_CBLOCK)
        {
#if defined(B_ENDIAN) || defined(L_ENDIAN)
        if (p != (SHA_LONG *)data)
            Memcpy(p,data,SHA_CBLOCK);
        data+=SHA_CBLOCK;
#  ifdef L_ENDIAN
#    ifndef OPT_SHA1_ASM /* Will not happen */
        for (sw=(SHA_LBLOCK/4); sw; sw--)
            {
            Endian_Reverse32(p[0]);
            Endian_Reverse32(p[1]);
            Endian_Reverse32(p[2]);
            Endian_Reverse32(p[3]);
            p+=4;
            }
        p=c->data;
#    endif
#  endif
#else
        for (sw=(SHA_BLOCK/4); sw; sw--)
            {
            M_c2nl(data,l); *(p++)=l;
            M_c2nl(data,l); *(p++)=l;
            M_c2nl(data,l); *(p++)=l;
            M_c2nl(data,l); *(p++)=l;
            }
        p=c->data;
#endif
        c->sha_block(c,(unsigned char *)p,64);
        len-=SHA_CBLOCK;
        }
    ec=(int)len;
    c->num=ec;
    ew=(ec>>2);
    ec&=0x03;

    for (sw=0; sw < ew; sw++)
        { M_c2nl(data,l); p[sw]=l; }
    M_c2nl_p(data,l,ec);
    p[sw]=l;
    }

void SHA1_Transform(c,b)
SHA_CTX *c;
const unsigned char *b;
    {
    SHA_LONG p[16];
#ifndef B_ENDIAN
    SHA_LONG *q;
    int i;
#endif

#if defined(B_ENDIAN) || defined(L_ENDIAN)
    Memcpy(p,b,64);
#ifdef L_ENDIAN
    q=p;
    for (i=(SHA_LBLOCK/4); i; i--)
        {
        Endian_Reverse32(q[0]);
        Endian_Reverse32(q[1]);
        Endian_Reverse32(q[2]);
        Endian_Reverse32(q[3]);
        q+=4;
        }
#endif
#else
    q=p;
    for (i=(SHA_LBLOCK/4); i; i--)
        {
        SHA_LONG l;
        c2nl(b,l); *(q++)=l;
        c2nl(b,l); *(q++)=l;
        c2nl(b,l); *(q++)=l;
        c2nl(b,l); *(q++)=l; 
        } 
#endif
    c->sha_block(c,(unsigned char *)p,64);
    }


void sha1_block(c, W, num)
SHA_CTX *c;
SHA_LONG *W;
int num;
    {
#ifndef SMALL_CODE_SIZE
    register SHA_LONG A,B,C,D,E,T;
    SHA_LONG X[16];

    A=c->h0;
    B=c->h1;
    C=c->h2;
    D=c->h3;
    E=c->h4;

    for (;;)
        {
    BODY_00_15( 0,A,B,C,D,E,T,W);
    BODY_00_15( 1,T,A,B,C,D,E,W);
    BODY_00_15( 2,E,T,A,B,C,D,W);
    BODY_00_15( 3,D,E,T,A,B,C,W);
    BODY_00_15( 4,C,D,E,T,A,B,W);
    BODY_00_15( 5,B,C,D,E,T,A,W);
    BODY_00_15( 6,A,B,C,D,E,T,W);
    BODY_00_15( 7,T,A,B,C,D,E,W);
    BODY_00_15( 8,E,T,A,B,C,D,W);
    BODY_00_15( 9,D,E,T,A,B,C,W);
    BODY_00_15(10,C,D,E,T,A,B,W);
    BODY_00_15(11,B,C,D,E,T,A,W);
    BODY_00_15(12,A,B,C,D,E,T,W);
    BODY_00_15(13,T,A,B,C,D,E,W);
    BODY_00_15(14,E,T,A,B,C,D,W);
    BODY_00_15(15,D,E,T,A,B,C,W);
    BODY_16_19(16,C,D,E,T,A,B,W,W,W,W);
    BODY_16_19(17,B,C,D,E,T,A,W,W,W,W);
    BODY_16_19(18,A,B,C,D,E,T,W,W,W,W);
    BODY_16_19(19,T,A,B,C,D,E,W,W,W,X);

    BODY_20_31(20,E,T,A,B,C,D,W,W,W,X);
    BODY_20_31(21,D,E,T,A,B,C,W,W,W,X);
    BODY_20_31(22,C,D,E,T,A,B,W,W,W,X);
    BODY_20_31(23,B,C,D,E,T,A,W,W,W,X);
    BODY_20_31(24,A,B,C,D,E,T,W,W,X,X);
    BODY_20_31(25,T,A,B,C,D,E,W,W,X,X);
    BODY_20_31(26,E,T,A,B,C,D,W,W,X,X);
    BODY_20_31(27,D,E,T,A,B,C,W,W,X,X);
    BODY_20_31(28,C,D,E,T,A,B,W,W,X,X);
    BODY_20_31(29,B,C,D,E,T,A,W,W,X,X);
    BODY_20_31(30,A,B,C,D,E,T,W,X,X,X);
    BODY_20_31(31,T,A,B,C,D,E,W,X,X,X);
    BODY_32_39(32,E,T,A,B,C,D,X);
    BODY_32_39(33,D,E,T,A,B,C,X);
    BODY_32_39(34,C,D,E,T,A,B,X);
    BODY_32_39(35,B,C,D,E,T,A,X);
    BODY_32_39(36,A,B,C,D,E,T,X);
    BODY_32_39(37,T,A,B,C,D,E,X);
    BODY_32_39(38,E,T,A,B,C,D,X);
    BODY_32_39(39,D,E,T,A,B,C,X);

    BODY_40_59(40,C,D,E,T,A,B,X);
    BODY_40_59(41,B,C,D,E,T,A,X);
    BODY_40_59(42,A,B,C,D,E,T,X);
    BODY_40_59(43,T,A,B,C,D,E,X);
    BODY_40_59(44,E,T,A,B,C,D,X);
    BODY_40_59(45,D,E,T,A,B,C,X);
    BODY_40_59(46,C,D,E,T,A,B,X);
    BODY_40_59(47,B,C,D,E,T,A,X);
    BODY_40_59(48,A,B,C,D,E,T,X);
    BODY_40_59(49,T,A,B,C,D,E,X);
    BODY_40_59(50,E,T,A,B,C,D,X);
    BODY_40_59(51,D,E,T,A,B,C,X);
    BODY_40_59(52,C,D,E,T,A,B,X);
    BODY_40_59(53,B,C,D,E,T,A,X);
    BODY_40_59(54,A,B,C,D,E,T,X);
    BODY_40_59(55,T,A,B,C,D,E,X);
    BODY_40_59(56,E,T,A,B,C,D,X);
    BODY_40_59(57,D,E,T,A,B,C,X);
    BODY_40_59(58,C,D,E,T,A,B,X);
    BODY_40_59(59,B,C,D,E,T,A,X);

    BODY_60_79(60,A,B,C,D,E,T,X);
    BODY_60_79(61,T,A,B,C,D,E,X);
    BODY_60_79(62,E,T,A,B,C,D,X);
    BODY_60_79(63,D,E,T,A,B,C,X);
    BODY_60_79(64,C,D,E,T,A,B,X);
    BODY_60_79(65,B,C,D,E,T,A,X);
    BODY_60_79(66,A,B,C,D,E,T,X);
    BODY_60_79(67,T,A,B,C,D,E,X);
    BODY_60_79(68,E,T,A,B,C,D,X);
    BODY_60_79(69,D,E,T,A,B,C,X);
    BODY_60_79(70,C,D,E,T,A,B,X);
    BODY_60_79(71,B,C,D,E,T,A,X);
    BODY_60_79(72,A,B,C,D,E,T,X);
    BODY_60_79(73,T,A,B,C,D,E,X);
    BODY_60_79(74,E,T,A,B,C,D,X);
    BODY_60_79(75,D,E,T,A,B,C,X);
    BODY_60_79(76,C,D,E,T,A,B,X);
    BODY_60_79(77,B,C,D,E,T,A,X);
    BODY_60_79(78,A,B,C,D,E,T,X);
    BODY_60_79(79,T,A,B,C,D,E,X);

    c->h0=(c->h0+E)&0xffffffffL;
    c->h1=(c->h1+T)&0xffffffffL;
    c->h2=(c->h2+A)&0xffffffffL;
    c->h3=(c->h3+B)&0xffffffffL;
    c->h4=(c->h4+C)&0xffffffffL;

    num-=64;
    if (num <= 0) break;

    A=c->h0;
    B=c->h1;
    C=c->h2;
    D=c->h3;
    E=c->h4;

    W+=16;
        }
#else /* SMALL_CODE_SIZE */
    SHA_LONG A,B,C,D,E,T;
    SHA_LONG X[16];
    SHA_LONG *a1,*a2,*a3;

    A=c->h0;
    B=c->h1;
    C=c->h2;
    D=c->h3;
    E=c->h4;

    for (;;)
        {
        int i;

    for (i=0; i<16; i++)
        {
        BODY_00_15(i,A,B,C,D,E,T,W);
        E=D; D=C; C=B; B=A; A=T;
        }

    a1=W;
    for (i=16; i<20; i++)
        {
        if (i == 19) a1=X;
        BODY_16_19(i,A,B,C,D,E,T,W,W,W,a1);
        E=D; D=C; C=B; B=A; A=T;
        }

    a1=a2=a3=W;
    for (i=20; i<40; i++)
        {
        if (i == 24) a3=X;
        if (i == 30) a2=X;
        if (i == 32) a1=X;
        BODY_20_31(i,A,B,C,D,E,T,a1,a2,a3,X);
        E=D; D=C; C=B; B=A; A=T;
        }

    for (i=40; i<60; i++)
        {
        BODY_40_59(i,A,B,C,D,E,T,X);
        E=D; D=C; C=B; B=A; A=T;
        }

    for (i=60; i<80; i++)
        {
        BODY_60_79(i,A,B,C,D,E,T,X);
        E=D; D=C; C=B; B=A; A=T;
        }

    c->h0=(c->h0+A)&0xffffffffL;
    c->h1=(c->h1+B)&0xffffffffL;
    c->h2=(c->h2+C)&0xffffffffL;
    c->h3=(c->h3+D)&0xffffffffL;
    c->h4=(c->h4+E)&0xffffffffL;

    num-=64;
    if (num <= 0) break;

    A=c->h0;
    B=c->h1;
    C=c->h2;
    D=c->h3;
    E=c->h4;

    W+=16;
        }
#endif /* SMALL_CODE_SIZE */
    }

void SHA1_Final(md, c)
unsigned char *md;
SHA_CTX *c;
    {
    register int i,j;
    register SHA_LONG l;
    register SHA_LONG *p;
    const static unsigned char end[4]={0x80,0x00,0x00,0x00};
    unsigned char *cp= (unsigned char *)end;

    /* c->num should definitly have room for at least one more byte. */
    p=c->data;
    j=c->num;
    i=j>>2;

#ifdef PURIFY
        /* PURIFY */
        /* we reference uninitialised data but don't keep the result
         * which purify complains about ... and we don't want to have
         * to come back here to find a non-existant problem later
         */

    /* purify often complains about the following line as an
     * Uninitialized Memory Read.  While this can be true, the
     * following p_c2l macro will reset l when that case is true.
     * This is because j&0x03 contains the number of 'valid' bytes
     * already in p[i].  If and only if j&0x03 == 0, the UMR will
     * occur but this is also the only time p_c2l will do
     * l= *(cp++) instead of l|= *(cp++)
         */
        if ((j&0x03) == 0) p[i]=0;
#endif

    l=p[i];
    M_p_c2nl(cp,l,j&0x03);
    p[i]=l;
    i++;
    /* i is the next 'undefined word' */
    if (c->num >= SHA_LAST_BLOCK)
        {
        for (; i<SHA_LBLOCK; i++)
            p[i]=0;
        c->sha_block(c,(unsigned char *)p,64);
        i=0;
        }
    for (; i<(SHA_LBLOCK-2); i++)
        p[i]=0;
    p[SHA_LBLOCK-2]=c->Nh;
    p[SHA_LBLOCK-1]=c->Nl;
#if defined(L_ENDIAN)
    Endian_Reverse32(p[SHA_LBLOCK-2]);
    Endian_Reverse32(p[SHA_LBLOCK-1]);
#endif
    c->sha_block(c,(unsigned char *)p,64);
    cp=md;
    l=c->h0; nl2c(l,cp);
    l=c->h1; nl2c(l,cp);
    l=c->h2; nl2c(l,cp);
    l=c->h3; nl2c(l,cp);
    l=c->h4; nl2c(l,cp);

    /* clear stuff, sha1_block may be leaving some stuff on the stack
     * but I'm not worried :-) */
    c->num=0;
/*  Memset((char *)&c,0,sizeof(c));*/
    }
#endif

#if 0
int pr(ctx)
SHA_CTX *ctx;
    {
    int i,j;
    unsigned char *p=(unsigned char *)(ctx->data);

    fprintf(stderr,"num = %08X%08X\n",ctx->Nh,ctx->Nl);
    fprintf(stderr,"      %08X %08X %08X %08X %08X\n",
        ctx->h0,ctx->h1,ctx->h2,ctx->h3,ctx->h4);
    fprintf(stderr,"bufnum = %d\n",ctx->num);
    fprintf(stderr,"      ");
    for (j=0; j<64; j+=16)
        {
        for (i=0; i<16; i++)
            {
/*
            if ((i+j) >= ctx->num)
                fprintf(stderr,"--");
            else
*/
                fprintf(stderr,"%02X",p[i+j]);
            }
        if ((j+16) >=64)
            fprintf(stderr,"\n");
        else
            fprintf(stderr,"\n      ");
        }
    }

#endif

#endif /* NO_SHA1 */
