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

#ifndef HEADER_COMMON_BN_LCL_H
#define HEADER_COMMON_BN_LCL_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
//#include "r_error.h"
//#include "err.h"
#include <string.h>

#ifndef STANDALONE
#include "r_com.h"
#else
#define NO_STDLIB_MAPPING
#define Malloc(a)   malloc(a)
#define Free(a)     free(a)
#define Memset(a,b,c)   memset(a,b,c)
#define Memcpy(a,b,c)   memcpy(a,b,c)
#endif

/* TEMP FIX */
#undef BNerr
#define BNerr(a,b)
#include "bn.h"

#define BN_EXP_TABLE_SIZE           16

/* see the bn_limit_ fields which are the start of a runtime
 * tunable set of values to match these fixed compile-time constants
 */
/* Pentium pro 16,16,16,32,64 */
/* Alpha       16,16,16,16.64 */
/* StrongARM  */ 
#define BN_MULL_SIZE_NORMAL         (16) /* 32 */
#define BN_MUL_RECURSIVE_SIZE_NORMAL        (16) /* 32 */ /* less than */
#define BN_SQR_RECURSIVE_SIZE_NORMAL        (16) /* 32 */
#define BN_MUL_LOW_RECURSIVE_SIZE_NORMAL    (32) /* 32 */
#define BN_MONT_CTX_SET_SIZE_WORD       (64) /* 32 */

/*************************************************************
 * Using the long long type
 */
#define Lw(t)    (((BN_ULONG)(t))&BN_MASK2)
#define Hw(t)    (((BN_ULONG)((t)>>BN_BITS2))&BN_MASK2)

/* These are used for internal error checking and are not normally used */
#ifdef BN_DEBUG
#define bn_check_top(a) \
    { if (  ((a)->top < 0) || \
        ((a)->top > (a)->max) || \
        (((a)->top == 0) && (a)->neg) || \
        (((a)->top == 1) && (a->d[0] == 0))) \
        { char *nullp=NULL; *nullp='z'; } }
#define bn_check_num(a) if ((a) < 0) { char *nullp=NULL; *nullp='z'; }
#define bn_assert(a) if (!(a)) { char *nullp=NULL; *nullp='z'; }
#define bn_set_used_check(l,size,num)   bn_su_check(l,size,num)
#ifdef BN_DEBUG2
#define bn_do_used_check(fp,str,l,size,num) bn_du_check(fp,str,l,size,num)
#else
#define bn_do_used_check(fp,str,l,size,num)
#endif
#else
#define bn_assert(a)
#define bn_check_top(a)
#define bn_check_num(a)
#define bn_set_used_check(l,size,num)
#define bn_do_used_check(fp,str,l,size,num)
#endif


/***********************************************
* IA64 32 bit build SWIZZLE code 
* This essentially casts our points which are
* 32bits under abi32 to 64bit unsigned long 
* longs and then grabs the top two bits of the 
* pointer and places it in the 2nd and 3rd bits
* of the unsigned long long.
**********************************************/
#ifdef HPUX_IA64_32
#define SWIZLLE(VALUE) ((((unsigned long long)(VALUE) & 0xc0000000LL) <<31)\
|((unsigned long long)(VALUE)))
#else
#define SWIZZLE(VALUE) (VALUE)
#endif

#define bn_neg_words(a,n) \
    { \
    int iii; \
 \
    for (iii=0; iii<n; iii++) \
        { \
        a[iii]= (-a[iii])&BN_MASK2; \
        } \
    for (;;) \
        if ((++a[iii])&BN_MASK2) break; \
    }

/* This macro is to add extra stuff for development checking */
#ifdef BN_DEBUG
#define bn_set_max(r) ((r)->max=(r)->top,BN_set_flags((r),BN_FLG_STATIC_DATA))
#else
#define bn_set_max(r)
#endif

/* These macros are used to 'take' a section of a bignum for read only use */
#define bn_set_low(r,a,n) \
    { \
    (r)->top=((a)->top > (n))?(n):(a)->top; \
    (r)->d=(a)->d; \
    (r)->neg=(a)->neg; \
    (r)->flags|=BN_FLG_STATIC_DATA; \
    bn_set_max(r); \
    }

#define bn_set_high(r,a,n) \
    { \
    if ((a)->top > (n)) \
        { \
        (r)->top=(a)->top-n; \
        (r)->d= &((a)->d[n]); \
        } \
    else \
        (r)->top=0; \
    (r)->neg=(a)->neg; \
    (r)->flags|=BN_FLG_STATIC_DATA; \
    bn_set_max(r); \
    }

/* #define bn_expand(n,b) ((((b)/BN_BITS2) <= (n)->max)?(n):bn_expand2((n),(b))) */

#ifdef BN_LLONG
#define mul_add(r,a,w,c) { \
    BN_ULLONG t; \
    t=(BN_ULLONG)w * (a) + (r) + (c); \
    (r)= Lw(t); \
    (c)= Hw(t); \
    }

#define mul(r,a,w,c) { \
    BN_ULLONG t; \
    t=(BN_ULLONG)w * (a) + (c); \
    (r)= Lw(t); \
    (c)= Hw(t); \
    }

#else
/*************************************************************
 * No long long type
 */

#define LBITS(a)    ((a)&BN_MASK2l)
#define HBITS(a)    (((a)>>BN_BITS4)&BN_MASK2lh)
#define L2HBITS(a)  ((BN_ULONG)((a)&BN_MASK2lh)<<BN_BITS4)

#define mul64(l,h,bl,bh) \
    { \
    BN_ULONG m,m1,lt,ht; \
 \
    lt=l; \
    ht=h; \
    m =(bh)*(lt); \
    lt=(bl)*(lt); \
    m1=(bl)*(ht); \
    ht =(bh)*(ht); \
    m=(m+m1)&BN_MASK2; if (m < m1) ht+=L2HBITS(1L); \
    ht+=HBITS(m); \
    m1=L2HBITS(m); \
    lt=(lt+m1)&BN_MASK2; if (lt < m1) ht++; \
    (l)=lt; \
    (h)=ht; \
    }

#define sqr64(lo,ho,in) \
    { \
    BN_ULONG l,h,m; \
 \
    h=(in); \
    l=LBITS(h); \
    h=HBITS(h); \
    m =(l)*(h); \
    l*=l; \
    h*=h; \
    h+=(m&BN_MASK2h1)>>(BN_BITS4-1); \
    m =((m&BN_MASK2l)<<(BN_BITS4+1))&BN_MASK2; \
    l=(l+m)&BN_MASK2; if (l < m) h++; \
    (lo)=l; \
    (ho)=h; \
    }

#if 0
#define mul_add(r,a,bl,bh,b_hl,c) { \
    BN_ULONG l,h; \
    BN_ULONG lt,mt; \
 \
    h= (a); \
    l=LBITS(h); \
    h=HBITS(h); \
    mt=bl*l; \
    lt=mt+c+(r); \
    c=bh*h; \
    mt+=c+(l-h)*b_hl; \
    mt+=(lt>>BN_BITS4); \
    (r)=(lt&BN_MASK2l)|((mt&BN_MASK2l)<<BN_BITS4); \
    c+=(mt>>BN_BITS4); \
    }
#else /* Normal version */
#define mul_add(r,a,bl,bh,b_hl,c) { \
    BN_ULONG l,h; \
 \
    h= (a); \
    l=LBITS(h); \
    h=HBITS(h); \
    mul64(l,h,(bl),(bh)); \
 \
    /* non-multiply part */ \
    l=(l+(c))&BN_MASK2; if (l < (c)) h++; \
    (c)=(r); \
    l=(l+(c))&BN_MASK2; if (l < (c)) h++; \
    (c)=h&BN_MASK2; \
    (r)=l; \
    }
#endif

#define mul(r,a,bl,bh,c) { \
    BN_ULONG l,h; \
 \
    h= (a); \
    l=LBITS(h); \
    h=HBITS(h); \
    mul64(l,h,(bl),(bh)); \
 \
    /* non-multiply part */ \
    l+=(c); if ((l&BN_MASK2) < (c)) h++; \
    (c)=h&BN_MASK2; \
    (r)=l&BN_MASK2; \
    }

#endif

#ifndef BN_MUL_COMBA
#define bn_mul_comba4(r,a,b)    bn_mul_normal(r,a,4,b,4)
#define bn_mul_comba5(r,a,b)    bn_mul_normal(r,a,5,b,5)
#define bn_mul_comba6(r,a,b)    bn_mul_normal(r,a,6,b,6)
#define bn_mul_comba8(r,a,b)    bn_mul_normal(r,a,8,b,8)
#define bn_mul_comba11(r,a,b)   bn_mul_normal(r,a,11,b,11)
#define bn_mul_comba12(r,a,b)   bn_mul_normal(r,a,12,b,12)
#define bn_mul_comba16(r,a,b)   bn_mul_normal(r,a,16,b,16)
#endif
#if 1
#ifndef BN_SQR_COMBA
#define bn_sqr_comba4(r,a)  bn_mul_normal(r,a,4,a,4)
#define bn_sqr_comba5(r,a)  bn_mul_normal(r,a,5,a,5)
#define bn_sqr_comba6(r,a)  bn_mul_normal(r,a,6,a,6)
#define bn_sqr_comba8(r,a)  bn_mul_normal(r,a,8,a,8)
#define bn_sqr_comba11(r,a) bn_mul_normal(r,a,11,a,11)
#define bn_sqr_comba12(r,a) bn_mul_normal(r,a,12,a,12)
#define bn_sqr_comba16(r,a) bn_mul_normal(r,a,16,a,16)
#endif
#else
#ifndef BN_SQR_COMBA
#define bn_sqr_comba4(r,a)  bn_mul_comba4(r,a,a)
#define bn_sqr_comba5(r,a)  bn_mul_comba5(r,a,a)
#define bn_sqr_comba6(r,a)  bn_mul_comba6(r,a,a)
#define bn_sqr_comba8(r,a)  bn_mul_comba8(r,a,a)
#define bn_sqr_comba11(r,a) bn_mul_comba11(r,a,a)
#define bn_sqr_comba12(r,a) bn_mul_comba12(r,a,a)
#define bn_sqr_comba16(r,a) bn_mul_comba16(r,a,a)
#endif
#endif
#ifndef BN_REDUCE_COMBA
#define r0_bn_mont_comba4(r,a,n,num,n0) bn_from_montgomery_words(r,a,n,num,n0)
#define r0_bn_mont_comba5(r,a,n,num,n0) bn_from_montgomery_words(r,a,n,num,n0)
#define r0_bn_mont_comba6(r,a,n,num,n0) bn_from_montgomery_words(r,a,n,num,n0)
#define r0_bn_mont_comba8(r,a,n,num,n0) bn_from_montgomery_words(r,a,n,num,n0)
#define r0_bn_mont_comba11(r,a,n,num,n0) bn_from_montgomery_words(r,a,n,num,n0)
#define r0_bn_mont_comba12(r,a,n,num,n0) bn_from_montgomery_words(r,a,n,num,n0)
#define r0_bn_mont_comba16(r,a,n,num,n0) bn_from_montgomery_words(r,a,n,num,n0)
#endif

#ifndef NOPROTO

BIGNUM *bn_expand2(BIGNUM *b, int bits);

#ifdef X86_ASM
void bn_add_words(BN_ULONG *r,BN_ULONG *a,int num);
#endif

#else

BIGNUM *bn_expand2();
#ifdef X86_ASM
BN_ULONG bn_add_words();
#endif

#endif

#ifdef  __cplusplus
}
#endif

#endif /* HEADER_COMMON_BN_LCL_H */

void bn_mul_low_recursive(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b,int n2,BN_ULONG *t);
void bn_mul_high(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b,BN_ULONG *l,int n2, BN_ULONG *t);
int BN_gen_exp_string(unsigned char *str, BIGNUM *p, int bits);
void bn_from_montgomery_rec_full(BN_ULONG *rp, BN_ULONG *ap,
    BN_ULONG *np, BN_ULONG *nip, BN_ULONG *tmp,BN_REC *rec);

int bn_mont_ctx_new_word(const BN_ME_METH *meth, BN_ME_CTX **retp);

