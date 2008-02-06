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
/**
 * @file bn_lib.c
 * @brief BN library functions
 * @todo add dox for undocumented functions
 */
#include "bn_lcl.h"
//#include "err.h"
#include "r_stdiag.h"

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))

#ifdef NO_SPLIT
#define SPLIT_BN_VALUE_ONE
#define SPLIT_BN_NUM_BITS_WORD
#define SPLIT_BN_NUM_BITS
#define SPLIT_BN_CLEAR_FREE
#define SPLIT_BN_FREE
#define SPLIT_BN_INIT
#define SPLIT_BN_NEW
#define SPLIT_BN_CTX_NEW
#define SPLIT_CTX_INIT
#define SPLIT_BN_CTX_FREE
#define SPLIT_BN_EXPAND2
#define SPLIT_BN_DUP
#define SPLIT_BN_COPY
#define SPLIT_BN_CLEAR
#define SPLIT_BN_GET_WORD
#define SPLIT_BN_SET_WORD
#define SPLIT_BN_BIN2BN
#define SPLIT_BN_BN2BIN
#define SPLIT_BN_UCMP
#define SPLIT_BN_CMP
#define SPLIT_BN_SET_BIT
#define SPLIT_BN_CLEAR_BIT
#define SPLIT_BN_IS_BIT_SET
#define SPLIT_BN_MASK_BITS
#define SPLIT_BN_CMP_WORDS
#endif /* NO_SPLIT */

#ifdef SPLIT_BN_VALUE_ONE
const char *BN_version="Big Number part of RCOM 2.3.0 11-Jun-2002";

BIGNUM *BN_value_one()
    {
    const static BN_ULONG data_one=1L;
    const static BIGNUM const_one={(BN_ULONG *)&data_one,1,1,0};

    return((BIGNUM *)&const_one);
    }
#endif

/**
 * Returns the maximum number of bits required to represent the
 * number <tt>l</tt>
 * @param l [In] a word (size of word depends on machine)
 * @returns number of bits in parameter <tt>l</tt>
 */
#ifdef SPLIT_BN_NUM_BITS_WORD
int BN_num_bits_word(l)
BN_ILONG l;
    {
    int i;

    /*
     * This table represents the number of bits required to
     * represent a "number", where "number" is indexed from
     * 0 into the table.  So:
     * number 0 = # bits req'd to represent 0 = bits[0] = 0
     * number 1 = # bits req'd to represent 1 = bits[1] = 1
     * number 2 = # bits req'd to represent 2 = bits[2] = 2
     * number 3 = # bits req'd to represent 3 = bits[3] = 2
     * number 4 = # bits req'd to represent 4 = bits[4] = 3
     * etc.
     *
     * The small code size table exists to save some space,
     * but requires (at most) an extra shift, an extra AND,
     * and an extra add.
     */
#ifdef SMALL_CODE_SIZE
    const static char bits[16]={
        0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4
        };
#else
    const static char bits[256]={
        0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
        };
#endif

#if defined(SIXTY_FOUR_BIT_LONG)
    if (l & 0xffffffff00000000L)
        {
        if (l & 0xffff000000000000L)
            {
            if (l & 0xff00000000000000L)
                i=56;
            else    i=48;
            }
        else
            {
            if (l & 0x0000ff0000000000L)
                i=40;
            else    i=32;
            }
        }
    else
#else
#ifdef SIXTY_FOUR_BIT
    if (l & 0xffffffff00000000LL)
        {
        if (l & 0xffff000000000000LL)
            {
            if (l & 0xff00000000000000LL)
                i=56;
            else    i=48;
            }
        else
            {
            if (l & 0x0000ff0000000000LL)
                i=40;
            else    i=32;
            }
        }
    else
#endif
#endif
        {
#if defined(THIRTY_BIT) || defined(THIRTY_ONE_BIT) || defined(THIRTY_TWO_BIT) || defined(SIXTY_FOUR_BIT) || defined(SIXTY_FOUR_BIT_LONG)
        if (l & 0xffff0000L)
            {
            if (l & 0xff000000L)
                i=24;
            else    i=16;
            }
        else
#endif
            {
            if (l & 0xff00L)
                i=8;
            else    
                i=0;
            }
        }
#ifndef SMALL_CODE_SIZE
    return(i+bits[l>>i]);
#else
    l>>=i;
    if (l & 0xf0)
        return(bits[l>>4]+i+4);
    else
        return(bits[l]+i);
#endif
    }
#endif

#ifdef SPLIT_BN_NUM_BITS
int BN_num_bits(a)
BIGNUM *a;
    {
    BN_ULONG l;
    int i;

    bn_check_top(a);

    if (a->top == 0) return(0);
    l=a->d[a->top-1];
    i=(a->top-1)*BN_BITS2;
#ifdef BN_DEBUG
    if (l == 0)
        {
#if !defined(NO_FP_API) && !defined(WIN16)
        fprintf(stderr,"BAD TOP VALUE\n");
#endif
        abort(); /* BN_DEBUG */
                /* TODO: need to understand what sort of error can
         *       be reported for this error
         */
        return(0);
        }
#endif
    return(i+BN_num_bits_word(l));
    }
#endif

/**
 * Clears and free a BIGNUM
 *
 * @param a [In]  BIGNUM to clear then free
 */
#ifdef SPLIT_BN_CLEAR_FREE
void BN_clear_free(BIGNUM *a)
{
    if (a == NULL)
    {
        return;
    }

    BN_clear(a);
    BN_free(a);
}
#endif

#ifdef SPLIT_BN_FREE
/**
 * Frees a BIGNUM
 *
 * @param a [In]  BIGNUM to free
 */
void BN_free(a)
BIGNUM *a;
    {
    if (a == NULL) return;
    if ((a->d != NULL) && !(BN_get_flags(a,BN_FLG_STATIC_DATA)))
        Free(a->d);
    a->flags|=BN_FLG_FREE; /* REMOVE? */
    if (a->flags & BN_FLG_MALLOCED)
        Free(a);
    }
#endif

#ifdef SPLIT_BN_INIT

#ifdef SMALL_CODE_SIZE
/**
 * Zero expands a bignum
 *
 * @param a [In]  BIGNUM to expand
 * @param n [In]  number of bits to expand to
 *
 * @pre <i>a</i> is a valid BIGNUM.
 */
void bn_zexpand(a,n)
BIGNUM *a;
int n;
    {
    if ((a)->top < n)
        {
        int i;
        bn_wexpand((a),n);
        if (a->d!=NULL)
            {
            for (i=(a)->top; i<n; i++)
                (a)->d[i]=0;
            }
        }
    }

/**
 * Fixes the top value in the BIGNUM to be the count of BN_ULONGs with data.
 *
 * @param a [In]  BIGNUM to free
 *
 * @pre <i>a</i> is a valid BIGNUM.
 */
void bn_fix_top(a)
BIGNUM *a;
    {
    BN_ULONG *ftl;
    if ((a)->top > 0)
        {
        for (ftl= &((a)->d[(a)->top-1]); (a)->top > 0; (a)->top--)
        if (*(ftl--)) break;
        }
    }
#endif

/**
 * Initializes a BIGNUM
 *
 * @param a [In]  BIGNUM reference
 *
 * @pre <i>a</i> is a valid BIGNUM.
 */
void BN_init(a)
BIGNUM *a;
    {
    (void)Memset(a,0,sizeof(BIGNUM));
    }
#endif

#ifdef SPLIT_BN_NEW
/**
 * Allocates a BIGNUM
 *
 * @return pointer to allocated BIGNUM
 */
BIGNUM *BN_new()
    {
    BIGNUM *ret;

    if ((ret=(BIGNUM *)Malloc(sizeof(BIGNUM))) == NULL)
        {
#ifndef NO_ERR
        BNerr(BN_F_BN_NEW,ERR_R_MALLOC_FAILURE);
#endif
        return(NULL);
        }
    ret->flags=BN_FLG_MALLOCED;
    ret->top=0;
    ret->neg=0;
    ret->max=0;
    ret->d=NULL;
    return(ret);
    }
#endif

#ifdef SPLIT_BN_CTX_NEW
/**
 * Allocates a BIGNUM context
 *
 * @return pointer to allocated BIGNUM context
 */
BN_CTX *BN_CTX_new()
    {
    BN_CTX *ret;

    ret=(BN_CTX *)Malloc(sizeof(BN_CTX));
    if (ret == NULL)
        {
#ifndef NO_ERR
        BNerr(BN_F_BN_CTX_NEW,ERR_R_MALLOC_FAILURE);
#endif
        return(NULL);
        }

    BN_CTX_init(ret);
    ret->flags=BN_FLG_MALLOCED;
    return(ret);
    }
#endif

#ifdef SPLIT_CTX_INIT
/**
 * Initializes a BIGNUM context
 *
 * @param a [In]  BIGNUM context reference
 *
 * @pre <i>a</i> is a valid BIGNUM context.
 */
void BN_CTX_init(ctx)
BN_CTX *ctx;
    {
    (void)Memset(ctx,0,sizeof(BN_CTX));
    ctx->tos=0;
    ctx->flags=0;
    }
#endif

#ifdef SPLIT_BN_CTX_FREE
/**
 * Frees a BIGNUM context
 *
 * @param a [In]  BIGNUM context reference
 *
 * @pre <i>a</i> is a valid BIGNUM context.
 */
void BN_CTX_free(c)
BN_CTX *c;
    {
    int i;

    for (i=0; i<BN_CTX_NUM; i++)
        BN_clear_free(&(c->bn[i]));
    if (c->flags & BN_FLG_MALLOCED)
        Free(c);
    }
#endif

#ifdef SPLIT_BN_EXPAND2
BIGNUM *bn_expand2(b, words)
BIGNUM *b;
int words;
    {
    BN_ULONG *A,*B,*a;

    R_DIAG_CHECK_STACK;

    bn_check_top(b);

    if (words > b->max)
        {
        bn_check_top(b);    
        if (BN_get_flags(b,BN_FLG_STATIC_DATA))
            {
#ifndef NO_ERR
            BNerr(BN_F_BN_EXPAND2,BN_R_EXPAND_ON_STATIC_BIGNUM_DATA);
#endif
            return(NULL);
            }
        a=A=(BN_ULONG *)Malloc(sizeof(BN_ULONG)*(words+1));
        if (A == NULL)
            {
#ifndef NO_ERR
            BNerr(BN_F_BN_EXPAND2,ERR_R_MALLOC_FAILURE);
#endif
            return(NULL);
            }
        /* during development this is a nice way of making sure
         * that we are not relying on the top byte being 0 or
         * other such things
         */
#ifdef BN_DEBUG
        (void)Memset(A,0x5c,sizeof(BN_ULONG)*(words+1));
#endif

        B=b->d;
        if (B != NULL)
            {
#ifndef SMALL_CODE_SIZE
            int i;

            /* for (i=b->top&(~7); i>0; i-=8) */
            /* The above line can induce a SunC compiler bug */
            for (i=(b->top>>3); i>0; i--)
                {
                BN_ULONG T0,T1,T2,T3;

                T0=B[0];
                T1=B[1];
                T2=B[2];
                T3=B[3];
                A[0]=T0;
                A[1]=T1;
                A[2]=T2;
                A[3]=T3;
                T0=B[4];
                T1=B[5];
                T2=B[6];
                T3=B[7];
                A[4]=T0;
                A[5]=T1;
                A[6]=T2;
                A[7]=T3;
                A+=8;
                B+=8;
                }
            switch (b->top&7)
                {
            case 7:
                A[6]=B[6];
            case 6:
                A[5]=B[5];
            case 5:
                A[4]=B[4];
            case 4:
                A[3]=B[3];
            case 3:
                A[2]=B[2];
            case 2:
                A[1]=B[1];
            case 1:
                A[0]=B[0];
            case 0:
                /* I need the 'case 0' entry for utrix cc.
                 * If the optimiser is turned on, it does the
                 * switch table by doing
                 * a=top&7
                 * a--;
                 * goto jump_table[a];
                 * If top is 0, this makes us jump to 0xffffffc 
                 * which is rather bad.
                 * eric 23-Apr-1998
                 */
                ;
                }
#if 0 /* Not needed */
            B= &(b->d[b->top]);
            j=b->max-8;
            for (i=b->top; i<j; i+=8)
                {
                B[0]=0; B[1]=0; B[2]=0; B[3]=0;
                B[4]=0; B[5]=0; B[6]=0; B[7]=0;
                B+=8;
                }
            for (j+=8; i<j; i++)
                {
                B[0]=0;
                B++;
                }
#endif
#else
            (void)Memcpy(A,B,sizeof(BN_ULONG)*b->top);
#endif
            Free(b->d);
            }

        b->d=a;
        b->max=words;
        }
    return(b);
    }
#endif

#ifdef SPLIT_BN_DUP
/**
 * Duplicates a BIGNUM
 *
 * @param a [In] BIGNUM reference
 *
 * @returns pointer to duplicate BIGNUM of <i>a</i>
 */
BIGNUM *BN_dup(a)
BIGNUM *a;
    {
    BIGNUM *r;
    BIGNUM *ret;

    bn_check_top(a);

    r=BN_new();
    if (r == NULL) return(NULL);
    ret = (BIGNUM *)BN_copy(r,a);
    if (ret == NULL)
        {
        BN_free(r);
        }
    return(ret);
    }
#endif

#ifdef SPLIT_BN_COPY
/**
 * Copies a BIGNUM to an existing BIGNUM
 *
 * @param a [In]  To BIGNUM reference
 * @param b [Out] From BIGNUM reference
 *
 * @pre Both <i>a</i> and <i>b</i> are valid BIGNUMs
 *
 * @returns pointer to duplicate BIGNUM of <i>a</i>
 */
BIGNUM *BN_copy(a, b)
BIGNUM *a;
BIGNUM *b;
    {
#ifndef SMALL_CODE_SIZE
    int i;
    BN_ULONG *A,*B;
#endif

    bn_check_top(b);

    if (a == b) return(a);
    if (bn_wexpand(a,b->top) == NULL) return(NULL);
#ifndef SMALL_CODE_SIZE
    A=a->d;
    B=b->d;

/*  for (i=b->top&~0x07; i>0; i-=8) */
/*  The above version of this loop has been removed.  It appeared
    to generate a compiler bug in SunC 4.x and 5.x.  I do not believe
    that the code was to blame.  Compiling with -fast was the problem
    since this would loop unroll the loop below another 3 times.
    The compiler would generate generate
         andcc   %o0,-8,%o1     # load ~0x07
         ld      [%i2],%o3      # load *B
         ble     .L77000125     # exit loop unless 8 or more
         ld      [%i1],%o2      # load *A
         sub     %o1,-7,%o0     #  
         sdivcc  %o0,8,%o0      # when do we clear %y?
         bvs,a   .L900001510
         sethi   %hi(0x80000000),%o0
.L900001510:
         cmp     %o0,3
         bl      .L77000114     # one loop
         ld      [%o2],%o0
                    # three loop code

    The loop would then be unrolled 3 times.  The problem I believe
    I was seeing was that the sdivcc operates is %y:%o0 / 8.
    %y is not being cleared, and so occasionally this would
    cause problems.  The replacement code sequence stops
    the compiler generating the sdiv instruction.
    */
    for (i=(b->top>>3); i>0; i--)
        {
        BN_ULONG T0,T1,T2,T3;
        T0=B[0];
        T1=B[1];
        T2=B[2];
        T3=B[3];
        A[0]=T0;
        A[1]=T1;
        A[2]=T2;
        A[3]=T3;
        T0=B[4];
        T1=B[5];
        T2=B[6];
        T3=B[7];
        A[4]=T0;
        A[5]=T1;
        A[6]=T2;
        A[7]=T3;
        A+=8;
        B+=8;
        }
    switch (b->top&0x07)
        {
    case 7:
        A[6]=B[6];
    case 6:
        A[5]=B[5];
    case 5:
        A[4]=B[4];
    case 4:
        A[3]=B[3];
    case 3:
        A[2]=B[2];
    case 2:
        A[1]=B[1];
    case 1:
        A[0]=B[0];
        case 0:
        /* I need the 'case 0' entry for utrix cc.
         * If the optimiser is turned on, it does the
         * switch table by doing
         * a=top&7
         * a--;
         * goto jump_table[a];
         * If top is 0, this makes us jump to 0xffffffc which is
         * rather bad.
         * eric 23-Apr-1998
         */
        ;
        }
#else
    (void)Memcpy(a->d,b->d,sizeof(b->d[0])*b->top);
#endif

/*  (void)Memset(&(a->d[b->top]),0,sizeof(a->d[0])*(a->max-b->top));*/
    a->top=b->top;
    if ((a->top == 0) && (a->d != NULL))
        a->d[0]=0;
    a->neg=b->neg;
    return(a);
    }
#endif

#ifdef SPLIT_BN_CLEAR
/**
 * Clears a BIGNUM
 *
 * @param a [In]  BIGNUM to clear
 *
 * @pre <i>a</i> is valid
 */
void BN_clear(a)
BIGNUM *a;
    {
    if (a->d != NULL)
        (void)Memset(a->d,0,a->max*sizeof(a->d[0]));
    a->top=0;
    a->neg=0;
    }
#endif

#ifdef SPLIT_BN_GET_WORD
BN_ULONG BN_get_word(a)
BIGNUM *a;
    {
    int i,n;
    BN_ULONG ret=0;

    n=BN_num_bytes(a);
    if (n > ((int) sizeof(BN_ULONG)))
        return(BN_MASK2);
    for (i=a->top-1; i>=0; i--)
        {
#ifndef SIXTY_FOUR_BIT /* the data item > unsigned long */
        ret<<=BN_BITS4; /* stops the compiler complaining */
        ret<<=BN_BITS4;
#else
        ret=0;
#endif
        ret|=a->d[i];
        }
    return(ret);
    }
#endif

#ifdef SPLIT_BN_SET_WORD
/**
 * Set the BIGNUM to the supplied BN_ULONG value.
 *
 * @param a    [In/Out] BIGNUM to be updated
 * @param w    [In]     Value to be set in <i>a</i>
 *
 * @notes Function will expand <i>a</i> to be an
 *        array of 8 BN_ULONGs in size.
 * @todo  Review behaviour of expansion without checking
 *        of size first.
 */
int BN_set_word(a,w)
BIGNUM *a;
BN_ULONG w;
    {
    int i,n;
    
    if (bn_expand(a,(int)(sizeof(BN_ULONG)*8)) == NULL) return(0);

#ifdef BN_BYTES
        /* This is used when a BN_ULONG is greater in size 
         * than the BN_BYTES of the array, eg BN_ULONG 64 bits
         * and actual a->d array of 32 bits.
         */
    n=sizeof(BN_ULONG)/BN_BYTES;
#else
    n=1;
#endif
    a->neg=0;
    a->top=0;
    a->d[0]=(BN_ULONG)w&BN_MASK2;
    if (a->d[0] != 0) a->top=1;
    for (i=1; i<n; i++)
        {
        /* the following is done instead of
         * w>>=BN_BITS2 so compilers don't complain
         * on builds where sizeof(long) == BN_TYPES */
#ifndef SIXTY_FOUR_BIT /* the data item > unsigned long */
        w>>=BN_BITS4;
        w>>=BN_BITS4;
#else
        w=0;
#endif
        a->d[i]=(BN_ULONG)w&BN_MASK2;
        if (a->d[i] != 0) a->top=i+1;
        }
    return(1);
    }
#endif

#ifdef BN_BYTES
#ifdef SPLIT_BN_BIN2BN
/* ignore negative */
BIGNUM *BN_bin2bn(s, len, ret)
unsigned char *s;
int len;
BIGNUM *ret;
    {
    int i,m;
    int n;
    BN_ULONG l;

    if (ret == NULL) ret=BN_new();
    if (ret == NULL) return(NULL);
    l=0;
    n=len;
    if (n == 0)
        {
        ret->top=0;
        return(ret);
        }
    if (bn_expand(ret,(int)(n+2)*8) == NULL)
        return(NULL);
    i=((n-1)/BN_BYTES)+1;
    m=((n-1)%(BN_BYTES));
    ret->top=i;
    while (n-- > 0)
        {
        l=(l<<8L)| *(s++);
        if (m-- == 0)
            {
            ret->d[--i]=l;
            l=0;
            m=BN_BYTES-1;
            }
        }
    /* need to call this due to clear byte at top if avoiding
     * having the top bit set (-ve number) */
    bn_fix_top(ret);
    return(ret);
    }
#endif

#ifdef SPLIT_BN_BN2BIN
/* ignore negative */
int BN_bn2bin(a, to)
BIGNUM *a;
unsigned char *to;
    {
    int n,i;
    BN_ULONG l;

    bn_check_top(a);

    n=i=BN_num_bytes(a);
    while (i-- > 0)
        {
        l=a->d[i/BN_BYTES];
        *(to++)=(unsigned char)((l>>(8*(i%BN_BYTES)))&0xff);
        }
    return(n);
    }
#endif
#else
#ifdef SPLIT_BN_BIN2BN
/* ignore negative */
BIGNUM *BN_bin2bn(s, len, ret)
unsigned char *s;
int len;
BIGNUM *ret;
    {
    int i,w,r,b,j,v;
    int n;
    BN_ULONG l;

    if (ret == NULL) ret=BN_new();
    if (ret == NULL) return(NULL);
    while (len > 0)
        {
        if (*s != 0) break;
        s++;
        len--;
        }
    n=len*8;
    if (bn_expand(ret,n) == NULL)
        return(NULL);
    for (i=0; i<ret->max; i++)
        ret->d[i]=0;
    j=0;
    for (i=len-1; i>=0; i--)
        {
        v=((int)s[i])&0xff;
        w=j/BN_BITS2;
        b=j%BN_BITS2;
        r=BN_BITS2-b;
        j+=8;
        if (r >= 8)
            {
            ret->d[w]|=(((BN_ULONG)v)<<b);
            }
        else
            {
            ret->d[w]  =(ret->d[w]|(((BN_ULONG)v)<<b))&BN_MASK2;
            ret->d[w+1]=(ret->d[w+1]|(((BN_ULONG)v)>>r));
            }
        }

    ret->top=((n-1)/BN_BITS2)+1;
    bn_fix_top(ret);
    return(ret);
    }
#endif

#ifdef SPLIT_BN_BN2BIN
int BN_bn2bin(a, to)
BIGNUM *a;
unsigned char *to;
    {
    int num,i,bi,w,b,r,max;
    BN_ULONG l,*lp;

    bn_check_top(a);

    num=bi=BN_num_bytes(a);
    lp=a->d;
    for (i=0; i<num; i++)
        {
        
        w=(i*8)/BN_BITS2;
        b=(i*8)%BN_BITS2;
        r=BN_BITS2-b;
        bi--;
        if (r >= 8)
            to[bi]=(lp[w]>>b)&0xff;
        else
            {
            to[bi]=(lp[w]>>b);
            if (w+1 < a->top)
                to[bi]|=lp[w+1]<<r;
            }
        }

    return(num);
    }
#endif
#endif

#ifdef SPLIT_BN_UCMP
int BN_ucmp(a, b)
BIGNUM *a;
BIGNUM *b;
    {
    int i;
    BN_ULONG t1,t2,*ap,*bp;

    bn_check_top(a);
    bn_check_top(b);

    i=a->top-b->top;
    if (i != 0) return(i);
    ap=a->d;
    bp=b->d;
    for (i=a->top-1; i>=0; i--)
        {
        t1= ap[i];
        t2= bp[i];
        if (t1 != t2)
            return(t1 > t2?1:-1);
        }
    return(0);
    }
#endif

#ifdef SPLIT_BN_CMP
int BN_cmp(a, b)
BIGNUM *a;
BIGNUM *b;
    {
    int i;
    int gt,lt;
    BN_ULONG t1,t2;

    if ((a == NULL) || (b == NULL))
        {
        if (a != NULL)
            return(-1);
        else if (b != NULL)
            return(1);
        else
            return(0);
        }

    bn_check_top(a);
    bn_check_top(b);

    if (a->neg != b->neg)
        {
        if (a->neg)
            return(-1);
        else    return(1);
        }
    if (a->neg == 0)
        { gt=1; lt= -1; }
    else    { gt= -1; lt=1; }

    if (a->top > b->top) return(gt);
    if (a->top < b->top) return(lt);
    for (i=a->top-1; i>=0; i--)
        {
        t1=a->d[i];
        t2=b->d[i];
        if (t1 > t2) return(gt);
        if (t1 < t2) return(lt);
        }
    return(0);
    }
#endif

#ifdef SPLIT_BN_SET_BIT
int BN_set_bit(a, n)
BIGNUM *a;
int n;
    {
    int i,j,k;

    bn_check_top(a);

    i=n/BN_BITS2;
    j=n%BN_BITS2;
    if (a->top <= i)
        {
        if (bn_wexpand(a,i+1) == NULL) return(0);
        for(k=a->top; k<i+1; k++)
            a->d[k]=0;
        a->top=i+1;
        }

    a->d[i]|=(((BN_ULONG)1)<<j);
    return(1);
    }
#endif

#ifdef SPLIT_BN_CLEAR_BIT
int BN_clear_bit(a, n)
BIGNUM *a;
int n;
    {
    int i,j;

    bn_check_top(a);

    i=n/BN_BITS2;
    j=n%BN_BITS2;
    if (a->top <= i) return(0);

    a->d[i]&=(~(((BN_ULONG)1)<<j));
    bn_fix_top(a);
    return(1);
    }
#endif

#ifdef SPLIT_BN_IS_BIT_SET
int BN_is_bit_set(a, n)
BIGNUM *a;
int n;
    {
    int i,j;

    bn_check_top(a);

    if (n < 0) return(0);
    i=n/BN_BITS2;
    j=n%BN_BITS2;
    if (a->top <= i) return(0);
    return((a->d[i]&(((BN_ULONG)1)<<j))?1:0);
    }
#endif

#ifdef SPLIT_BN_MASK_BITS
int BN_mask_bits(a,n)
BIGNUM *a;
int n;
    {
    int b,w;

    bn_check_top(a);

    w=n/BN_BITS2;
    b=n%BN_BITS2;
    if (w >= a->top) return(0);
    if (b == 0)
        a->top=w;
    else
        {
        a->top=w+1;
        a->d[w]&= ~(((BN_ULONG)BN_MASK2)<<b);
        }
    bn_fix_top(a);
    return(1);
    }
#endif

#ifdef SPLIT_BN_CMP_WORDS
int bn_cmp_words(a,b,n)
BN_ULONG *a,*b;
int n;
    {
    int i;
    BN_ULONG aa,bb;

    aa=a[n-1];
    bb=b[n-1];
    if (aa != bb) return((aa > bb)?1:-1);
    for (i=n-2; i>=0; i--)
        {
        aa=a[i];
        bb=b[i];
        if (aa != bb) return((aa > bb)?1:-1);
        }
    return(0);
    }
#endif
#endif

