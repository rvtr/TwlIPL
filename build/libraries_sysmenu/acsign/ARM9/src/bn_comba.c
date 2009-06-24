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

#include "bn_lcl.h"

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))

#ifdef NO_SPLIT
#define SPLIT_BN_MUL_COMBA8
#define SPLIT_BN_MUL_COMBA4
#define SPLIT_BN_SQR_COMBA8
#define SPLIT_BN_SQR_COMBA4
#endif /* NO_SPLIT */

#ifndef OPT_BN_ASM
/* #if 1 */

#ifdef BN_LLONG
#define mul_add_c(a,b,c0,c1,c2) \
    t=(BN_ULLONG)a*b; \
    t1=(BN_ULONG)Lw(t); \
    t2=(BN_ULONG)Hw(t); \
    c0=(c0+t1)&BN_MASK2; if ((c0) < t1) t2++; \
    c1=(c1+t2)&BN_MASK2; if ((c1) < t2) c2++;

#define mul_add_c2(a,b,c0,c1,c2) \
    t=(BN_ULLONG)a*b; \
    tt=(t+t)&BN_MASK; \
    if (tt < t) c2++; \
    t1=(BN_ULONG)Lw(tt); \
    t2=(BN_ULONG)Hw(tt); \
    c0=(c0+t1)&BN_MASK2;  \
    if ((c0 < t1) && (((++t2)&BN_MASK2) == 0)) c2++; \
    c1=(c1+t2)&BN_MASK2; if ((c1) < t2) c2++;

#define sqr_add_c(a,i,c0,c1,c2) \
    t=(BN_ULLONG)a[i]*a[i]; \
    t1=(BN_ULONG)Lw(t); \
    t2=(BN_ULONG)Hw(t); \
    c0=(c0+t1)&BN_MASK2; if ((c0) < t1) t2++; \
    c1=(c1+t2)&BN_MASK2; if ((c1) < t2) c2++;

#define sqr_add_c2(a,i,j,c0,c1,c2) \
    mul_add_c2((a)[i],(a)[j],c0,c1,c2)
#else
#define mul_add_c(a,b,c0,c1,c2) \
    t1=LBITS(a); t2=HBITS(a); \
    bl=LBITS(b); bh=HBITS(b); \
    mul64(t1,t2,bl,bh); \
    c0=(c0+t1)&BN_MASK2; if ((c0) < t1) t2++; \
    c1=(c1+t2)&BN_MASK2; if ((c1) < t2) c2++;

#define mul_add_c2(a,b,c0,c1,c2) \
    t1=LBITS(a); t2=HBITS(a); \
    bl=LBITS(b); bh=HBITS(b); \
    mul64(t1,t2,bl,bh); \
    if (t2 & BN_TBIT) c2++; \
    t2=(t2+t2)&BN_MASK2; \
    if (t1 & BN_TBIT) t2++; \
    t1=(t1+t1)&BN_MASK2; \
    c0=(c0+t1)&BN_MASK2;  \
    if ((c0 < t1) && (((++t2)&BN_MASK2) == 0)) c2++; \
    c1=(c1+t2)&BN_MASK2; if ((c1) < t2) c2++;

#define sqr_add_c(a,i,c0,c1,c2) \
    sqr64(t1,t2,(a)[i]); \
    c0=(c0+t1)&BN_MASK2; if ((c0) < t1) t2++; \
    c1=(c1+t2)&BN_MASK2; if ((c1) < t2) c2++;

#define sqr_add_c2(a,i,j,c0,c1,c2) \
    mul_add_c2((a)[i],(a)[j],c0,c1,c2)
#endif

#if defined(BN_MUL_COMBA)

#ifdef SPLIT_BN_MUL_COMBA8
#undef bn_mul_comba8
void bn_mul_comba8(r,a,b)
BN_ULONG *r,*a,*b;
    {
#ifdef BN_LLONG
    BN_ULLONG t;
#else
    BN_ULONG bl,bh;
#endif
    BN_ULONG t1,t2;
    BN_ULONG c1,c2,c3;

    c1=0;
    c2=0;
    c3=0;
    mul_add_c(a[0],b[0],c1,c2,c3);
    r[0]=c1;
    c1=0;
    mul_add_c(a[0],b[1],c2,c3,c1);
    mul_add_c(a[1],b[0],c2,c3,c1);
    r[1]=c2;
    c2=0;
    mul_add_c(a[2],b[0],c3,c1,c2);
    mul_add_c(a[1],b[1],c3,c1,c2);
    mul_add_c(a[0],b[2],c3,c1,c2);
    r[2]=c3;
    c3=0;
    mul_add_c(a[0],b[3],c1,c2,c3);
    mul_add_c(a[1],b[2],c1,c2,c3);
    mul_add_c(a[2],b[1],c1,c2,c3);
    mul_add_c(a[3],b[0],c1,c2,c3);
    r[3]=c1;
    c1=0;
    mul_add_c(a[4],b[0],c2,c3,c1);
    mul_add_c(a[3],b[1],c2,c3,c1);
    mul_add_c(a[2],b[2],c2,c3,c1);
    mul_add_c(a[1],b[3],c2,c3,c1);
    mul_add_c(a[0],b[4],c2,c3,c1);
    r[4]=c2;
    c2=0;
    mul_add_c(a[0],b[5],c3,c1,c2);
    mul_add_c(a[1],b[4],c3,c1,c2);
    mul_add_c(a[2],b[3],c3,c1,c2);
    mul_add_c(a[3],b[2],c3,c1,c2);
    mul_add_c(a[4],b[1],c3,c1,c2);
    mul_add_c(a[5],b[0],c3,c1,c2);
    r[5]=c3;
    c3=0;
    mul_add_c(a[6],b[0],c1,c2,c3);
    mul_add_c(a[5],b[1],c1,c2,c3);
    mul_add_c(a[4],b[2],c1,c2,c3);
    mul_add_c(a[3],b[3],c1,c2,c3);
    mul_add_c(a[2],b[4],c1,c2,c3);
    mul_add_c(a[1],b[5],c1,c2,c3);
    mul_add_c(a[0],b[6],c1,c2,c3);
    r[6]=c1;
    c1=0;
    mul_add_c(a[0],b[7],c2,c3,c1);
    mul_add_c(a[1],b[6],c2,c3,c1);
    mul_add_c(a[2],b[5],c2,c3,c1);
    mul_add_c(a[3],b[4],c2,c3,c1);
    mul_add_c(a[4],b[3],c2,c3,c1);
    mul_add_c(a[5],b[2],c2,c3,c1);
    mul_add_c(a[6],b[1],c2,c3,c1);
    mul_add_c(a[7],b[0],c2,c3,c1);
    r[7]=c2;
    c2=0;
    mul_add_c(a[7],b[1],c3,c1,c2);
    mul_add_c(a[6],b[2],c3,c1,c2);
    mul_add_c(a[5],b[3],c3,c1,c2);
    mul_add_c(a[4],b[4],c3,c1,c2);
    mul_add_c(a[3],b[5],c3,c1,c2);
    mul_add_c(a[2],b[6],c3,c1,c2);
    mul_add_c(a[1],b[7],c3,c1,c2);
    r[8]=c3;
    c3=0;
    mul_add_c(a[2],b[7],c1,c2,c3);
    mul_add_c(a[3],b[6],c1,c2,c3);
    mul_add_c(a[4],b[5],c1,c2,c3);
    mul_add_c(a[5],b[4],c1,c2,c3);
    mul_add_c(a[6],b[3],c1,c2,c3);
    mul_add_c(a[7],b[2],c1,c2,c3);
    r[9]=c1;
    c1=0;
    mul_add_c(a[7],b[3],c2,c3,c1);
    mul_add_c(a[6],b[4],c2,c3,c1);
    mul_add_c(a[5],b[5],c2,c3,c1);
    mul_add_c(a[4],b[6],c2,c3,c1);
    mul_add_c(a[3],b[7],c2,c3,c1);
    r[10]=c2;
    c2=0;
    mul_add_c(a[4],b[7],c3,c1,c2);
    mul_add_c(a[5],b[6],c3,c1,c2);
    mul_add_c(a[6],b[5],c3,c1,c2);
    mul_add_c(a[7],b[4],c3,c1,c2);
    r[11]=c3;
    c3=0;
    mul_add_c(a[7],b[5],c1,c2,c3);
    mul_add_c(a[6],b[6],c1,c2,c3);
    mul_add_c(a[5],b[7],c1,c2,c3);
    r[12]=c1;
    c1=0;
    mul_add_c(a[6],b[7],c2,c3,c1);
    mul_add_c(a[7],b[6],c2,c3,c1);
    r[13]=c2;
    c2=0;
    mul_add_c(a[7],b[7],c3,c1,c2);
    r[14]=c3;
    r[15]=c1;
    }
#endif

#ifdef SPLIT_BN_MUL_COMBA4
#undef bn_mul_comba4
void bn_mul_comba4(r,a,b)
BN_ULONG *r,*a,*b;
    {
#ifdef BN_LLONG
    BN_ULLONG t;
#else
    BN_ULONG bl,bh;
#endif
    BN_ULONG t1,t2;
    BN_ULONG c1,c2,c3;

    c1=0;
    c2=0;
    c3=0;
    mul_add_c(a[0],b[0],c1,c2,c3);
    r[0]=c1;
    c1=0;
    mul_add_c(a[0],b[1],c2,c3,c1);
    mul_add_c(a[1],b[0],c2,c3,c1);
    r[1]=c2;
    c2=0;
    mul_add_c(a[2],b[0],c3,c1,c2);
    mul_add_c(a[1],b[1],c3,c1,c2);
    mul_add_c(a[0],b[2],c3,c1,c2);
    r[2]=c3;
    c3=0;
    mul_add_c(a[0],b[3],c1,c2,c3);
    mul_add_c(a[1],b[2],c1,c2,c3);
    mul_add_c(a[2],b[1],c1,c2,c3);
    mul_add_c(a[3],b[0],c1,c2,c3);
    r[3]=c1;
    c1=0;
    mul_add_c(a[3],b[1],c2,c3,c1);
    mul_add_c(a[2],b[2],c2,c3,c1);
    mul_add_c(a[1],b[3],c2,c3,c1);
    r[4]=c2;
    c2=0;
    mul_add_c(a[2],b[3],c3,c1,c2);
    mul_add_c(a[3],b[2],c3,c1,c2);
    r[5]=c3;
    c3=0;
    mul_add_c(a[3],b[3],c1,c2,c3);
    r[6]=c1;
    r[7]=c2;
    }
#endif

#else

#ifdef SPLIT_BN_MUL_COMBA4
#undef bn_mul_comba4
void bn_mul_comba4(r,a,b)
BN_ULONG *r,*a,*b;
    {
    r[4]=bn_mul_words(    &(r[0]),a,4,b[0]);
    r[5]=bn_mul_add_words(&(r[1]),a,4,b[1]);
    r[6]=bn_mul_add_words(&(r[2]),a,4,b[2]);
    r[7]=bn_mul_add_words(&(r[3]),a,4,b[3]);
    }
#endif

#ifdef SPLIT_BN_MUL_COMBA8
#undef bn_mul_comba8
void bn_mul_comba8(r,a,b)
BN_ULONG *r,*a,*b;
    {
    r[ 8]=bn_mul_words(    &(r[0]),a,8,b[0]);
    r[ 9]=bn_mul_add_words(&(r[1]),a,8,b[1]);
    r[10]=bn_mul_add_words(&(r[2]),a,8,b[2]);
    r[11]=bn_mul_add_words(&(r[3]),a,8,b[3]);
    r[12]=bn_mul_add_words(&(r[4]),a,8,b[4]);
    r[13]=bn_mul_add_words(&(r[5]),a,8,b[5]);
    r[14]=bn_mul_add_words(&(r[6]),a,8,b[6]);
    r[15]=bn_mul_add_words(&(r[7]),a,8,b[7]);
    }
#endif

#endif /* BN_MUL_COMBA */

#ifdef BN_SQR_COMBA

#ifdef SPLIT_BN_SQR_COMBA8
#undef bn_sqr_comba8
void bn_sqr_comba8(r,a)
BN_ULONG *r,*a;
    {
#ifdef BN_LLONG
    BN_ULLONG t,tt;
#else
    BN_ULONG bl,bh;
#endif
    BN_ULONG t1,t2;
    BN_ULONG c1,c2,c3;

    c1=0;
    c2=0;
    c3=0;
    sqr_add_c(a,0,c1,c2,c3);
    r[0]=c1;
    c1=0;
    sqr_add_c2(a,1,0,c2,c3,c1);
    r[1]=c2;
    c2=0;
    sqr_add_c(a,1,c3,c1,c2);
    sqr_add_c2(a,2,0,c3,c1,c2);
    r[2]=c3;
    c3=0;
    sqr_add_c2(a,3,0,c1,c2,c3);
    sqr_add_c2(a,2,1,c1,c2,c3);
    r[3]=c1;
    c1=0;
    sqr_add_c(a,2,c2,c3,c1);
    sqr_add_c2(a,3,1,c2,c3,c1);
    sqr_add_c2(a,4,0,c2,c3,c1);
    r[4]=c2;
    c2=0;
    sqr_add_c2(a,5,0,c3,c1,c2);
    sqr_add_c2(a,4,1,c3,c1,c2);
    sqr_add_c2(a,3,2,c3,c1,c2);
    r[5]=c3;
    c3=0;
    sqr_add_c(a,3,c1,c2,c3);
    sqr_add_c2(a,4,2,c1,c2,c3);
    sqr_add_c2(a,5,1,c1,c2,c3);
    sqr_add_c2(a,6,0,c1,c2,c3);
    r[6]=c1;
    c1=0;
    sqr_add_c2(a,7,0,c2,c3,c1);
    sqr_add_c2(a,6,1,c2,c3,c1);
    sqr_add_c2(a,5,2,c2,c3,c1);
    sqr_add_c2(a,4,3,c2,c3,c1);
    r[7]=c2;
    c2=0;
    sqr_add_c(a,4,c3,c1,c2);
    sqr_add_c2(a,5,3,c3,c1,c2);
    sqr_add_c2(a,6,2,c3,c1,c2);
    sqr_add_c2(a,7,1,c3,c1,c2);
    r[8]=c3;
    c3=0;
    sqr_add_c2(a,7,2,c1,c2,c3);
    sqr_add_c2(a,6,3,c1,c2,c3);
    sqr_add_c2(a,5,4,c1,c2,c3);
    r[9]=c1;
    c1=0;
    sqr_add_c(a,5,c2,c3,c1);
    sqr_add_c2(a,6,4,c2,c3,c1);
    sqr_add_c2(a,7,3,c2,c3,c1);
    r[10]=c2;
    c2=0;
    sqr_add_c2(a,7,4,c3,c1,c2);
    sqr_add_c2(a,6,5,c3,c1,c2);
    r[11]=c3;
    c3=0;
    sqr_add_c(a,6,c1,c2,c3);
    sqr_add_c2(a,7,5,c1,c2,c3);
    r[12]=c1;
    c1=0;
    sqr_add_c2(a,7,6,c2,c3,c1);
    r[13]=c2;
    c2=0;
    sqr_add_c(a,7,c3,c1,c2);
    r[14]=c3;
    r[15]=c1;
    }
#endif

#ifdef SPLIT_BN_SQR_COMBA4
#undef bn_sqr_comba4
void bn_sqr_comba4(r,a)
BN_ULONG *r,*a;
    {
#ifdef BN_LLONG
    BN_ULLONG t,tt;
#else
    BN_ULONG bl,bh;
#endif
    BN_ULONG t1,t2;
    BN_ULONG c1,c2,c3;

    c1=0;
    c2=0;
    c3=0;
    sqr_add_c(a,0,c1,c2,c3);
    r[0]=c1;
    c1=0;
    sqr_add_c2(a,1,0,c2,c3,c1);
    r[1]=c2;
    c2=0;
    sqr_add_c(a,1,c3,c1,c2);
    sqr_add_c2(a,2,0,c3,c1,c2);
    r[2]=c3;
    c3=0;
    sqr_add_c2(a,3,0,c1,c2,c3);
    sqr_add_c2(a,2,1,c1,c2,c3);
    r[3]=c1;
    c1=0;
    sqr_add_c(a,2,c2,c3,c1);
    sqr_add_c2(a,3,1,c2,c3,c1);
    r[4]=c2;
    c2=0;
    sqr_add_c2(a,3,2,c3,c1,c2);
    r[5]=c3;
    c3=0;
    sqr_add_c(a,3,c1,c2,c3);
    r[6]=c1;
    r[7]=c2;
    }
#endif
#else

#ifdef SPLIT_BN_SQR_COMBA4
/* hmm... is it faster just to do a multiply? */
#undef bn_sqr_comba4
void bn_sqr_comba4(r,a)
BN_ULONG *r,*a;
    {
    BN_ULONG t[8];
    bn_sqr_normal(r,a,4,t);
    }
#endif

#ifdef SPLIT_BN_SQR_COMBA8
#undef bn_sqr_comba8
void bn_sqr_comba8(r,a)
BN_ULONG *r,*a;
    {
    BN_ULONG t[16];
    bn_sqr_normal(r,a,8,t);
    }
#endif

#endif

#endif /* OPT_BN_ASM */

#endif
