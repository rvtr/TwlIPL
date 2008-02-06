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
 * @file bn_add.c
 * @brief BIGNUM addition and subtraction functions
 */

#include "bn_lcl.h"

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))
 
#ifdef NO_SPLIT
#define SPLIT_BN_ADD
#define SPLIT_BN_UADD
#define SPLIT_BN_USUB
#define SPLIT_BN_SUB
#endif /* NO_SPLIT */

#ifdef SPLIT_BN_ADD
/* r can == a or b */
int BN_add(r, a, b)
BIGNUM *r;
BIGNUM *a;
BIGNUM *b;
    {
    BIGNUM *tmp;

    bn_check_top(a);
    bn_check_top(b);

    /*  a +  b  a+b
     *  a + -b  a-b
     * -a +  b  b-a
     * -a + -b  -(a+b)
     */
    if (a->neg ^ b->neg)
        {
        /* only one is negative */
        if (a->neg)
            { tmp=a; a=b; b=tmp; }

        /* we are now a - b */

        if (BN_ucmp(a,b) < 0)
            {
            if (!BN_usub(r,b,a)) return(0);
            r->neg=1;
            }
        else
            {
            if (!BN_usub(r,a,b)) return(0);
            r->neg=0;
            }
        return(1);
        }

    if (a->neg) /* both are neg */
        r->neg=1;
    else
        r->neg=0;

    if (!BN_uadd(r,a,b)) return(0);
    return(1);
    }
#endif

#ifdef SPLIT_BN_UADD
/* unsigned add of b to a, r must be large enough */
int BN_uadd(r,a,b)
BIGNUM *r;
BIGNUM *a;
BIGNUM *b;
    {
    register int i;
    int max,min;
    BN_ULONG *ap,*bp,*rp,carry,t1;
    BIGNUM *tmp;

    bn_check_top(a);
    bn_check_top(b);

    if (a->top < b->top)
        { tmp=a; a=b; b=tmp; }
    max=a->top;
    min=b->top;

    if (bn_wexpand(r,max+1) == NULL)
        return(0);

    r->top=max;


    ap=a->d;
    bp=b->d;
    rp=r->d;
    carry=0;

    carry=bn_add_words(rp,ap,bp,min);
    rp+=min;
    ap+=min;
    bp+=min;
    i=min;

    if (carry)
        {
        while (i < max)
            {
            i++;
            t1= *(ap++);
            if ((*(rp++)=(t1+1)&BN_MASK2) >= t1)
                {
                carry=0;
                break;
                }
            }
        if ((i >= max) && carry)
            {
            *(rp++)=1;
            r->top++;
            }
        }
    if (rp != ap)
        {
        for (; i<max; i++)
            *(rp++)= *(ap++);
        }
    /* Memcpy(rp,ap,sizeof(*ap)*(max-i));*/
    return(1);
    }
#endif

/**
 * perform unsigned subtraction of <tt>b</tt> from <tt>a</tt>
 * @param r result of subtraction
 * @param a pointer to a BIGNUM
 * @param b pointer to a BIGNUM
 * @pre   <tt>a</tt> must be larger than <tt>b</tt>
 * @return 1 on success, 0 on failure
 * @todo On next IRIX port check the validity of IRIX_CC_BUG.
 */
#ifdef SPLIT_BN_USUB
int BN_usub(r, a, b)
BIGNUM *r;
BIGNUM *a;
BIGNUM *b;
    {
    int max,min;
    register BN_ULONG t1,t2,*ap,*bp,*rp;
    int i,carry;
#if defined(IRIX_CC_BUG) && !defined(LINT)
    int dummy;
#endif

    bn_check_top(a);
    bn_check_top(b);

    /*
     * check for pre-condition violation
     */
    if (a->top < b->top)
        {
#ifndef NO_ERR
        BNerr(BN_F_BN_USUB,BN_R_ARG2_LT_ARG3);
#endif
        return(0);
        }

    max=a->top;
    min=b->top;
    if (bn_wexpand(r,max) == NULL) return(0);

    ap=a->d;
    bp=b->d;
    rp=r->d;

    /*
     * perform the subtraction and see if we
     * have a (final) carry
     */
    carry=0;
    for (i=0; i<min; i++)
        {
        t1= *(ap++);
        t2= *(bp++);
        if (carry)
            {
            carry=(t1 <= t2);
            t1=(t1-t2-1)&BN_MASK2;
            }
        else
            {
            carry=(t1 < t2);
            t1=(t1-t2)&BN_MASK2;
            }
#if defined(IRIX_CC_BUG) && !defined(LINT)
        dummy=t1;
#endif
        *(rp++)=t1&BN_MASK2;
        }

    if (carry) /* subtracted */
        {
        while (i < max)
            {
            i++;
            t1= *(ap++);
            t2=(t1-1)&BN_MASK2;
            *(rp++)=t2;
            if (t1 > t2) break;
            }
        }

    if (rp != ap)
    {
#ifdef BN_LIBRARY_SMALL
        Memcpy(rp,ap,sizeof(*rp)*(max-i));
#else /* BN_LIBRARY_SMALL */
        for (;;)
        {
            if (i++ >= max) break;
            rp[0]=ap[0];
            if (i++ >= max) break;
            rp[1]=ap[1];
            if (i++ >= max) break;
            rp[2]=ap[2];
            if (i++ >= max) break;
            rp[3]=ap[3];
            rp+=4;
            ap+=4;
        }
#endif /* BN_LIBRARY_SMALL */
    }

    r->top=max;
    bn_fix_top(r);
    return(1);
    }
#endif

#ifdef SPLIT_BN_SUB
int BN_sub(r, a, b)
BIGNUM *r;
BIGNUM *a;
BIGNUM *b;
    {
    int max;
    int add=0,neg=0;
    BIGNUM *tmp;

    bn_check_top(a);
    bn_check_top(b);

    /*  a -  b  a-b
     *  a - -b  a+b
     * -a -  b  -(a+b)
     * -a - -b  b-a
     */
    if (a->neg)
        {
        if (b->neg)
            { tmp=a; a=b; b=tmp; }
        else
            { add=1; neg=1; }
        }
    else
        {
        if (b->neg) { add=1; neg=0; }
        }

    if (add)
        {
        if (!BN_uadd(r,a,b)) return(0);
        r->neg=neg;
        return(1);
        }

    /* We are actually doing a - b */

    max=(a->top > b->top)?a->top:b->top;
    if (bn_wexpand(r,max) == NULL) return(0);
    if (BN_ucmp(a,b) < 0)
        {
        if (!BN_usub(r,b,a)) return(0);
        r->neg=1;
        }
    else
        {
        if (!BN_usub(r,a,b)) return(0);
        r->neg=0;
        }
    return(1);
    }
#endif

#endif
