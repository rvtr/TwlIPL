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

#include "bn_lcl.h"

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))
 
#ifdef NO_SPLIT
#define SPLIT_BN_MOD_WORD
#define SPLIT_BN_DIV_WORD
#define SPLIT_BN_ADD_WORD
#define SPLIT_BN_SUB_WORD
#define SPLIT_BN_MUL_WORD
#endif /* NO_SPLIT */

#ifdef SPLIT_BN_MOD_WORD
BN_ULONG BN_mod_word(a, w)
BIGNUM *a;
BN_ULONG w;
    {
#ifndef BN_LLONG
    BN_ULONG ret=0;
#else
    BN_ULLONG ret=0;
#endif
    int i;

    w=w&BN_MASK2;//w&=BN_MASK2;
    for (i=a->top-1; i>=0; i--)
        {
#ifndef BN_LLONG
        ret=((ret<<BN_BITS4)|((a->d[i]>>BN_BITS4)&BN_MASK2l))%w;
        ret=((ret<<BN_BITS4)|(a->d[i]&BN_MASK2l))%w;
#else
        ret=(BN_ULLONG)(((ret<<(BN_ULLONG)BN_BITS2)|a->d[i])%
            (BN_ULLONG)w);
#endif
        }
    return((BN_ULONG)ret);
    }
#endif

#ifdef SPLIT_BN_DIV_WORD
BN_ULONG BN_div_word(a, w)
BIGNUM *a;
BN_ULONG w;
    {
    BN_ULONG ret;
    int i;

    if (a->top == 0) return(0);
    ret=0;
    w=w&BN_MASK2;//w&=BN_MASK2;
    for (i=a->top-1; i>=0; i--)
        {
        BN_ULONG l,d;
        
        l=a->d[i];
        d=bn_div_words(ret,l,w);
        ret=(l-((d*w)&BN_MASK2))&BN_MASK2;
        a->d[i]=d;
        }
    if ((a->top > 0) && (a->d[a->top-1] == 0))
        a->top--;
    return(ret);
    }
#endif

#ifdef SPLIT_BN_ADD_WORD
int BN_add_word(a, w)
BIGNUM *a;
BN_ULONG w;
    {
    BN_ULONG l;
    int i;

    if (w == 0) return(1);
    bn_check_top(a);

    w=w&BN_MASK2;//w&=BN_MASK2;
    if (a->neg)
        {
        if(a->top > 1)
            {
            a->neg=0;
            i=BN_sub_word(a,w);
            a->neg=1;
            return(i);
            }
        else
            { /* a->top == 1, it cannot be 0 */
            l=a->d[0];
            if (l > w)
                a->d[0]=l-w;
            else if (l < w)
                {
                a->neg=0;
                a->d[0]=w-l;
                }
            else
                {
                a->neg=0;
                a->top=0;
                }
            return(1);
            }
        }
    w=w&BN_MASK2;//w&=BN_MASK2;
    if (bn_wexpand(a,a->top+1) == NULL) return(0);
    a->d[a->top]=0;
    i=0;
    for (;;)
        {
        l=(a->d[i]+(BN_ULONG)w)&BN_MASK2;
        a->d[i]=l;
        if (w > l)
            w=1;
        else
            break;
        i++;
        }
    if (i >= a->top)
        a->top++;
    return(1);
    }
#endif

#ifdef SPLIT_BN_SUB_WORD
int BN_sub_word(a, w)
BIGNUM *a;
BN_ULONG w;
    {
    int i;

    bn_check_top(a);
    if (w == 0) return(1);

    w=w&BN_MASK2;//w&=BN_MASK2;
    if (a->neg)
        {
        a->neg=0;
        i=BN_add_word(a,w);
        a->neg=1;
        return(i);
        }

    w=w&BN_MASK2;//w&=BN_MASK2;
    if (a->top <= 1)
        {
        BN_ULONG l;

        if (a->top == 0)
            {
            if (bn_wexpand(a,1) == NULL) return(0);
            a->d[0]=w;
            a->neg = 1;
            a->top = 1;
            return(1);
            }
        l=a->d[0];
        if (l == w)
            a->top=0;
        else if (l > w)
            a->d[0]=l-w;
        else
            {
            a->neg=1;
            a->d[0]=w-l;
            }
        return(1);
        }
    i=0;
    for (;;)
        {
        if (a->d[i] >= w)
            {
            a->d[i]-=w;
            break;
            }
        else
            {
            a->d[i]=(a->d[i]-w)&BN_MASK2;
            i++;
            w=1;
            }
        }
    if ((a->d[i] == 0) && (i == (a->top-1)))
        a->top--;
    return(1);
    }
#endif

#ifdef SPLIT_BN_MUL_WORD
int BN_mul_word(a,w)
BIGNUM *a;
BN_ULONG w;
    {
    BN_ULONG ll;

    w=w&BN_MASK2;//w&=BN_MASK2;
    if (a->top)
        {
        ll=bn_mul_words(a->d,a->d,a->top,w);
        if (ll)
            {
            if (bn_wexpand(a,a->top+1) == NULL) return(0);
            a->d[a->top++]=ll;
            }
        }
    return(1);
    }
#endif

#endif
