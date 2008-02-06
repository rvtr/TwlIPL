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
#define SPLIT_BN_RSHIFT1
#define SPLIT_BN_RSHIFT
#endif /* NO_SPLIT */

#ifdef SPLIT_BN_RSHIFT1
#ifdef SMALL_CODE_SIZE
int BN_rshift1(r, a)
BIGNUM *r;
BIGNUM *a;
    {
    return(BN_rshift(r,a,1));
    }

#else

int BN_rshift1(r, a)
BIGNUM *r;
BIGNUM *a;
    {
    BN_ULONG *ap,*rp,t,c;
    int i;

    bn_check_top(a);

    if (BN_is_zero(a))
        {
        BN_zero(r);
        return(1);
        }
    if (a != r)
        {
        if (bn_wexpand(r,a->top) == NULL) return(0);
        r->top=a->top;
        r->neg=a->neg;
        }
    ap=a->d;
    rp=r->d;
    c=0;
    for (i=a->top-1; i>=0; i--)
        {
        t=ap[i];
        rp[i]=((t>>1)&BN_MASK2)|c;
        c=(t&1)?BN_TBIT:0;
        }
    bn_fix_top(r);
    return(1);
    }
#endif
#endif

#ifdef SPLIT_BN_RSHIFT
int BN_rshift(r, a, n)
BIGNUM *r;
BIGNUM *a;
int n;
    {
    int i,j,nw,lb,rb;
    BN_ULONG *t,*f;
    BN_ULONG l,tmp;

    bn_check_top(a);

#ifndef SMALL_CODE_SIZE
    if (n == 1) return(BN_rshift1(r,a));
#endif
    nw=n/BN_BITS2;
    rb=n%BN_BITS2;
    lb=BN_BITS2-rb;
    if (nw > a->top)
        {
        (void)BN_zero(r);
        return(1);
        }
    if (r != a)
        {
        if (bn_wexpand(r,a->top-nw+1+1/*???*/) == NULL) return(0);
        r->neg=a->neg;
        }

    f= &(a->d[nw]);
    t=r->d;
    j=a->top-nw;
    r->top=j;

    if (rb == 0)
        {
        for (i=j+1; i > 0; i--)
            *(t++)= *(f++);
        }
    else
        {
        l= *(f++);
        for (i=1; i<j; i++)
            {
            tmp =(l>>rb)&BN_MASK2;
            l= *(f++);
            *(t++) =(tmp|(l<<lb))&BN_MASK2;
            }
        *(t++) =(l>>rb)&BN_MASK2;
        *t=0;
        }
    bn_fix_top(r);
    return(1);
    }
#endif

#endif
