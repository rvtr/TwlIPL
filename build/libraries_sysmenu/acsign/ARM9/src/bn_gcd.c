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

#ifndef NOPROTO
BIGNUM *bn_euclid(BIGNUM *a, BIGNUM *b);
#else
BIGNUM *bn_euclid();
#endif

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))
 
#ifdef NO_SPLIT
#define SPLIT_BN_GCD
#define SPLIT_BN_MOD_INVERSE
#define SPLIT_BN_MOD_INVERSE_WORD
#endif /* NO_SPLIT */

#ifdef SPLIT_BN_GCD
int BN_gcd(r,in_a,in_b,ctx)
BIGNUM *r,*in_a,*in_b;
BN_CTX *ctx;
    {
    BIGNUM *a,*b,*t;
    int ret=0;

    bn_check_top(in_a);
    bn_check_top(in_b);

    a= &(ctx->bn[ctx->tos]);
    b= &(ctx->bn[ctx->tos+1]);

    if (BN_copy(a,in_a) == NULL) goto err;
    if (BN_copy(b,in_b) == NULL) goto err;

    if (BN_cmp(a,b) < 0) { t=a; a=b; b=t; }
    t=bn_euclid(a,b);
    if (t == NULL) goto err;

    if (BN_copy(r,t) == NULL) goto err;
    ret=1;
err:
    return(ret);
    }

BIGNUM *bn_euclid(a,b)
BIGNUM *a,*b;
    {
    BIGNUM *t;
    int shifts=0;

    bn_check_top(a);
    bn_check_top(b);

    for (;;)
        {
        if (BN_is_zero(b))
            break;

        if (BN_is_odd(a))
            {
            if (BN_is_odd(b))
                {
                if (!BN_sub(a,a,b)) goto err;
                if (!BN_rshift1(a,a)) goto err;
                if (BN_cmp(a,b) < 0)
                    { t=a; a=b; b=t; }
                }
            else        /* a odd - b even */
                {
                if (!BN_rshift1(b,b)) goto err;
                if (BN_cmp(a,b) < 0)
                    { t=a; a=b; b=t; }
                }
            }
        else            /* a is even */
            {
            if (BN_is_odd(b))
                {
                if (!BN_rshift1(a,a)) goto err;
                if (BN_cmp(a,b) < 0)
                    { t=a; a=b; b=t; }
                }
            else        /* a even - b even */
                {
                if (!BN_rshift1(a,a)) goto err;
                if (!BN_rshift1(b,b)) goto err;
                shifts++;
                }
            }
        }
    if (shifts)
        {
        if (!BN_lshift(a,a,shifts)) goto err;
        }
    return(a);
err:
    return(NULL);
    }
#endif

#ifdef SPLIT_BN_MOD_INVERSE
/* solves ax == 1 (mod n) */
BIGNUM *BN_mod_inverse(in, a, n, ctx)
BIGNUM *in;
BIGNUM *a;
BIGNUM *n;
BN_CTX *ctx;
    {
    BIGNUM *A,*B,*X,*Y,*M,*D,*R;
    BIGNUM *T,*ret=NULL;
    int sign;

    bn_check_top(a);
    bn_check_top(n);

    A= &(ctx->bn[ctx->tos]);
    B= &(ctx->bn[ctx->tos+1]);
    X= &(ctx->bn[ctx->tos+2]);
    D= &(ctx->bn[ctx->tos+3]);
    M= &(ctx->bn[ctx->tos+4]);
    Y= &(ctx->bn[ctx->tos+5]);
    ctx->tos+=6;
    if (in == NULL)
        R=BN_new();
    else
        R=in;
    if (R == NULL) goto err;

    (void)BN_zero(X);
    (void)BN_one(Y);
    if (BN_copy(A,a) == NULL) goto err;
    if (BN_copy(B,n) == NULL) goto err;
    sign=1;

    while (!BN_is_zero(B))
        {
        if (!BN_div(D,M,A,B,ctx)) goto err;
        T=A;
        A=B;
        B=M;
        /* T has a struct, M does not */

        if (!BN_mul(T,D,X,ctx)) goto err;
        if (!BN_add(T,T,Y)) goto err;
        M=Y;
        Y=X;
        X=T;
        sign= -sign;
        }
    if (sign < 0)
        {
        if (!BN_sub(Y,n,Y)) goto err;
        }

    if (BN_is_one(A))
        { if (!BN_mod(R,Y,n,ctx)) goto err; }
    else
        {
#ifndef NO_ERR
        BNerr(BN_F_BN_MOD_INVERSE,BN_R_NO_INVERSE);
#endif
        goto err;
        }
    ret=R;
err:
    if ((ret == NULL) && (in == NULL)) BN_free(R);
    ctx->tos-=6;
    return(ret);
    }
#endif

#ifdef SPLIT_BN_MOD_INVERSE_WORD
/* solves 1  == (ret*(1<<BN_BITS2)) mod n */
BN_ULONG BN_mod_inverse_word(n)
BN_ULONG n;
    {
    BN_ULONG A,B,X,Y,M,D,R;
    BN_ULONG T;
    int sign;

    X=0;
    Y=1;
    A=0; /* 1 00000000 */
    B=n;
    sign=1;

    T=((BN_ULONG)0-B)&BN_MASK2;
    D=T/B+1;
    M=T%B;
    T=A; A=B; B=M;

    /* T=D*X; X == 0; so leave out */
    T=Y;
    M=Y; Y=X; X=T;
    sign= -sign;

    while (B != 0)
        {
        D=A/B;
        M=A%B;
        T=D*X+Y;

        Y=X; X=T;
        A=B; B=M;
        sign= -sign;
        }
    if (sign < 0)
        Y=(n-Y)&BN_MASK2;

    if (A == 1)
        R=Y%n;
    else
        R=0;
    return(R);
    }

#endif

#endif
