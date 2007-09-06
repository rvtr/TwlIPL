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
#define SPLIT_BN_LSHIFT1
#define SPLIT_BN_LSHIFT
#endif /* NO_SPLIT */

#ifdef SPLIT_BN_LSHIFT1
#ifdef SMALL_CODE_SIZE

int BN_lshift1(r, a)
BIGNUM *r,*a;
    {
    return(BN_lshift(r, a, 1));
    }

#else

int BN_lshift1(r, a)
BIGNUM *r;
BIGNUM *a;
    {
    register BN_ULONG *ap,*rp,t,c;
    int i;

    bn_check_top(a);

    if (r != a)
        {
        if (bn_wexpand(r,a->top+1) == NULL) return(0);
        r->neg=a->neg;
        r->top=a->top;
        }
    else
        {
        if (bn_wexpand(r,a->top+1) == NULL) return(0);
        }
    ap=a->d;
    rp=r->d;
    c=0;
    for (i=0; i<a->top; i++)
        {
        t= *(ap++);
        *(rp++)=((t<<1L)|c)&BN_MASK2;
        c=(t & BN_TBIT)?1:0;
        }
    if (c)
        {
        *rp=1;
        r->top++;
        }
    return(1);
    }
#endif
#endif

#ifdef SPLIT_BN_LSHIFT
int BN_lshift(r, a, n)
BIGNUM *r;
BIGNUM *a;
int n;
    {
    int i,nw,lb,rb;
    BN_ULONG *t,*f;
    BN_ULONG l;

    bn_check_top(a);

#ifndef SMALL_CODE_SIZE
    if (n == 1) return(BN_lshift1(r,a));
#endif
    if (bn_wexpand(r,a->top+(n/BN_BITS2)+1) == NULL) return(0);
    r->neg=a->neg;
    nw=n/BN_BITS2;
    lb=n%BN_BITS2;
    rb=BN_BITS2-lb;
    f=a->d;
    t=r->d;
    t[a->top+nw]=0;
    if (lb == 0)
        for (i=a->top-1; i>=0; i--)
            t[nw+i]=f[i];
    else
        for (i=a->top-1; i>=0; i--)
            {
            l=f[i];
            t[nw+i+1]|=(l>>rb)&BN_MASK2;
            t[nw+i]=(l<<lb)&BN_MASK2;
            }
    (void)Memset(t,0,nw*sizeof(t[0]));
/*  for (i=0; i<nw; i++)
        t[i]=0;*/
    r->top=a->top+nw+1;
    bn_fix_top(r);
    return(1);
    }
#endif

#endif
