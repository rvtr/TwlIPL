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
 * @file bn_div.c
 * @brief Division and modulus functions
 */
#include "bn_lcl.h"

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))
 
#ifdef NO_SPLIT
#define SPLIT_BN_DIV
#define SPLIT_BN_MOD
#endif /* NO_SPLIT */

#ifdef SPLIT_BN_DIV
/* The old slow way */
#ifdef OLD_BN_DIVISION
int BN_div(dv, rem, m, d,ctx)
BIGNUM *dv;
BIGNUM *rem;
BIGNUM *m;
BIGNUM *d;
BN_CTX *ctx;
    {
    int i,nm,nd;
    BIGNUM *D;

    bn_check_top(m);
    bn_check_top(d);
    if (BN_is_zero(d))
        {
#ifndef NO_ERR
        BNerr(BN_F_BN_DIV,BN_R_DIV_BY_ZERO);
#endif
        return(0);
        }

    if (BN_ucmp(m,d) < 0)
        {
        if (rem != NULL)
            { if (BN_copy(rem,m) == NULL) return(0); }
        if (dv != NULL) BN_zero(dv);
        return(1);
        }

    D= &(ctx->bn[ctx->tos]);
    if (dv == NULL) dv= &(ctx->bn[ctx->tos+1]);
    if (rem == NULL) rem= &(ctx->bn[ctx->tos+2]);

    nd=BN_num_bits(d);
    nm=BN_num_bits(m);
    if (BN_copy(D,d) == NULL) return(0);
    if (BN_copy(rem,m) == NULL) return(0);

    /* The next 2 are needed so we can do a dv->d[0]|=1 later
     * since BN_lshift1 will only work once there is a value 
     */
    BN_zero(dv);
    bn_wexpand(dv,1);
    dv->top=1;

    if (!BN_lshift(D,D,nm-nd)) return(0);
    for (i=nm-nd; i>=0; i--)
        {
        if (!BN_lshift1(dv,dv)) return(0);
        if (BN_ucmp(rem,D) >= 0)
            {
            dv->d[0]|=1;
            if (!BN_usub(rem,rem,D)) return(0);
            }
/* CAN IMPROVE (and have now :=) */
        if (!BN_rshift1(D,D)) return(0);
        }
    rem->neg=BN_is_zero(rem)?0:m->neg;
    dv->neg=m->neg^d->neg;
    return(1);
    }

#else
/**
 * Performs Big number division  
 * 
 * @param dv       [In]    division
 * @param rm       [Out]   remainder
 * @param num      [In]    number
 * @param divisor  [In]    divisor
 * @param ctx      [In]    Temporary data storage
 *
 * @todo add more comments to this function
 */
int BN_div(BIGNUM *dv, BIGNUM *rm, BIGNUM *num, BIGNUM *divisor, BN_CTX *ctx)
    {
    int norm_shift,i,j,loop;
    BIGNUM *tmp,wnum,*snum,*sdiv,*res;
    BN_ULONG *resp,*wnump;
    BN_ULONG d0,d1;
    int num_n,div_n;

    bn_check_top(num);
    bn_check_top(divisor);

    if (BN_is_zero(divisor))
        {
#ifndef NO_ERR
        BNerr(BN_F_BN_DIV,BN_R_DIV_BY_ZERO);
#endif
        return(0);
        }

    if (BN_ucmp(num,divisor) < 0)
        {
        if (rm != NULL)
            { if (BN_copy(rm,num) == NULL) return(0); }
        if (dv != NULL) (void)BN_zero(dv);
        return(1);
        }

    tmp= &(ctx->bn[ctx->tos]);
    tmp->neg=0;
    snum= &(ctx->bn[ctx->tos+1]);
    sdiv= &(ctx->bn[ctx->tos+2]);
    if (dv == NULL)
        res= &(ctx->bn[ctx->tos+3]);
    else    res=dv;

    /* First we normalise the numbers */
    norm_shift=BN_BITS2-((BN_num_bits(divisor))%BN_BITS2);
    if (!BN_lshift(sdiv,divisor,norm_shift)) return(0);
    sdiv->neg=0;
    norm_shift+=BN_BITS2;
    if (!BN_lshift(snum,num,norm_shift)) return(0);
    snum->neg=0;
    div_n=sdiv->top;
    num_n=snum->top;
    loop=num_n-div_n;

    /* Lets setup a 'window' into snum
     * This is the part that corresponds to the current
     * 'area' being divided */
    BN_init(&wnum);
    wnum.d=  &(snum->d[loop]);
    wnum.top= div_n;
    wnum.max= snum->max+1; /* a bit of a lie */

    /* Get the top 2 words of sdiv */
    /* i=sdiv->top; */
    d0=sdiv->d[div_n-1];
    d1=(div_n == 1)?0:sdiv->d[div_n-2];

    /* pointer to the 'top' of snum */
    wnump= &(snum->d[num_n-1]);

    /* Setup to 'res' */
    if (!bn_wexpand(res,(loop+1))) goto err;
    res->neg= (num->neg^divisor->neg);
    res->top=loop;
    resp= &(res->d[loop-1]);

    /* space for temp */
    if (!bn_wexpand(tmp,(div_n+1))) goto err;

    if (BN_ucmp(&wnum,sdiv) >= 0)
        {
        if (!BN_usub(&wnum,&wnum,sdiv)) goto err;
        *resp=1;
        res->d[res->top-1]=1;
        }
    else
        res->top--;
    resp--;

    for (i=0; i<loop-1; i++)
        {
        BN_ULONG q,n0,n1;
        BN_ULONG l0;

        wnum.d--; wnum.top++;
        n0=wnump[0];
        n1=wnump[-1];
        if (n0 == d0)
            q=BN_MASK2;
        else
            q=bn_div_words(n0,n1,d0);
        {
#ifdef BN_LLONG
        BN_ULLONG t1,t2,rem;
        t1=((((BN_ULLONG)n0)<<BN_BITS2)|n1)&BN_MASK;
        for (;;)
            {
            t2=((BN_ULLONG)d1*q)&BN_MASK;
            rem=(t1-(BN_ULLONG)q*d0)&BN_MASK;
            if ((rem>>BN_BITS2) ||
                (t2 <= ((BN_ULLONG)(rem<<BN_BITS2)+wnump[-2])))
                break;
            q--;
            }
#else
        BN_ULONG t1l,t1h,t2l,t2h,t3l,t3h,ql,qh,t3t;
        t1h=n0;
        t1l=n1;
        for (;;)
            {
            t2l=LBITS(d1); t2h=HBITS(d1);
            ql =LBITS(q);  qh =HBITS(q);
            mul64(t2l,t2h,ql,qh); /* t2=(BN_ULLONG)d1*q; */

            t3t=LBITS(d0); t3h=HBITS(d0);
            mul64(t3t,t3h,ql,qh); /* t3=t1-(BN_ULLONG)q*d0; */
            t3l=(t1l-t3t)&BN_MASK2;
            if (t3l > t1l) t3h++;
            t3h=(t1h-t3h)&BN_MASK2;

            /*if ((t3>>BN_BITS2) ||
                (t2 <= ((t3<<BN_BITS2)+wnump[-2])))
                break; */
            if (t3h) break;
            if (t2h < t3l) break;
            if ((t2h == t3l) && (t2l <= wnump[-2])) break;

            q--;
            }
#endif
        }
        l0=bn_mul_words(tmp->d,sdiv->d,div_n,q);
        tmp->d[div_n]=l0;
        for (j=div_n+1; j>0; j--)
            if (tmp->d[j-1]) break;
        tmp->top=j;

        j=wnum.top;
#ifdef BN_DEBUG
        /* Sometimes this is 0 now (wnum.top is not).
         * We need to look at this some time, I am quite
         * sure it has no affect.
         */
        bn_fix_top(&wnum);
#endif
        (void)BN_sub(&wnum,&wnum,tmp);

        snum->top=snum->top+wnum.top-j;

        if (wnum.neg)
            {
            q--;
            j=wnum.top;
            (void)BN_add(&wnum,&wnum,sdiv);
            snum->top+=wnum.top-j;
            }
        *(resp--)=q;
        wnump--;
        }
    bn_fix_top(snum);
    if (rm != NULL)
        {
        i=num->neg; /* just in case num == rm */
        if (BN_rshift(rm,snum,norm_shift) == 0)
            return(0);
        rm->neg=i;
        }
    return(1);
err:
    return(0);
    }
#endif
#endif

#ifdef SPLIT_BN_MOD
/**
 * Calculate the remainder where rem = m mod d
 *
 * @param   rem    [Out] Modulus result
 * @param   m      [In]  Base value
 * @param   d      [In]  Divisor
 * @param   ctx    [In]  BN_CTX for data space
 *
 * @pre     variables are initialised and valid
 * @post    rem contained modulus
 *
 * @retval  1 success
 * @retval  0 failure
 *
 * @note    when BN_LIBRARY_SMALL define uses following algorithm
 *          otherwise calls through to BN_div
 *
 * @note    m < d   rem = m
 *          (length m = length of d) and (m > d) , rem = m - d
 *          alg otherwise
 *          let rem = m
 *          loop while rem > d
 *             let dv = shift left d, n bits to = length rem
 *             if dv > rem
 *                let dv = shift left d, n - 1 bits ( = length rem - 1)
 *             end if
 *             let rem = rem - dv;
 *          end loop
 * 
 * @relates BN_div
 */
int BN_mod(BIGNUM *rem, BIGNUM *m, BIGNUM *d, BN_CTX *ctx)
{
#ifdef BN_LIBRARY_SMALL
    int nm,nd;
    BIGNUM *dv, *dv2;

    /* if m < d , mod = m */  
    if (BN_ucmp(m,d) < 0)
        return((BN_copy(rem,m) == NULL)?0:1);

    dv= &(ctx->bn[ctx->tos]);
    dv2 = &(ctx->bn[ctx->tos+1]);

    nm=BN_num_bits(m);
    nd=BN_num_bits(d);

    /* if bits m = bits d and m >= d (from above) 
     * rem = m - d
     */
    if(nm == nd)
    {
        BN_usub(rem, m, d);
        goto end;
    }

    if (BN_copy(rem, m) == NULL)
    {
        return 0;
    }

    /* while the rem > d */
    while(BN_ucmp(rem,d) > 0)
    {
        nm=BN_num_bits(rem);
        if(!BN_lshift(dv2,d,nm-nd))
        {
            return(0);
        }

        if(BN_ucmp(dv2, rem) >0)
        {
            if(!BN_lshift(dv2,d,nm-nd-1))
            {
                return(0);
            }
        }

        BN_usub(rem,rem,dv2);
    }
end:    
    return(1);
#else
    return(BN_div(NULL,rem,m,d,ctx));
#endif
}
#endif

#endif
