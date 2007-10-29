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

void BN_RECP_CTX_init(recp)
BN_RECP_CTX *recp;
    {
    BN_init(&(recp->N));
    BN_init(&(recp->Nr));
    recp->num_bits=0;
    recp->flags=0;
    }

BN_RECP_CTX *BN_RECP_CTX_new()
    {
    BN_RECP_CTX *ret;

    if ((ret=(BN_RECP_CTX *)Malloc(sizeof(BN_RECP_CTX))) == NULL)
        return(NULL);

    BN_RECP_CTX_init(ret);
    ret->flags=BN_FLG_MALLOCED;
    return(ret);
    }

void BN_RECP_CTX_free(recp)
BN_RECP_CTX *recp;
    {
    BN_free(&(recp->N));
    BN_free(&(recp->Nr));
    if (recp->flags & BN_FLG_MALLOCED)
        Free(recp);
    }

int BN_RECP_CTX_set(recp,d,ctx)
BN_RECP_CTX *recp;
BIGNUM *d;
BN_CTX *ctx;
    {
    (void)BN_copy(&(recp->N),d);
    (void)BN_zero(&(recp->Nr));
    recp->num_bits=BN_num_bits(d);
    recp->shift=0;
    ctx=ctx;    /* Stop warning */
    return(1);
    }

int BN_mod_mul_reciprocal(r, x, y, recp, ctx)
BIGNUM *r;
BIGNUM *x;
BIGNUM *y;
BN_RECP_CTX *recp;
BN_CTX *ctx;
    {
    int ret=0;
    BIGNUM *a;

    bn_check_top(x);
    bn_check_top(y);

    a= &(ctx->bn[ctx->tos++]);
    if (y != NULL)
        {
        if (x == y)
            { if (!BN_sqr(a,x,ctx)) goto err; }
        else
            { if (!BN_mul(a,x,y,ctx)) goto err; }
        }
    else
        a=x; /* Just do the mod */

    (void)BN_div_recp(NULL,r,a,recp,ctx);
    ret=1;
err:
    ctx->tos--;
    return(ret);
    }

int BN_div_recp(dv,rem,m,recp,ctx)
BIGNUM *dv;
BIGNUM *rem;
BIGNUM *m;
BN_RECP_CTX *recp;
BN_CTX *ctx;
    {
    int i,j,tos,ret=0,ex;
    BIGNUM *a,*b,*d,*r;

    bn_check_top(m);

    tos=ctx->tos;
    a= &(ctx->bn[ctx->tos++]);
    b= &(ctx->bn[ctx->tos++]);
    if (dv != NULL)
        d=dv;
    else
        d= &(ctx->bn[ctx->tos++]);
    if (rem != NULL)
        r=rem;
    else
        r= &(ctx->bn[ctx->tos++]);

    if (BN_ucmp(m,&(recp->N)) < 0)
        {
        (void)BN_zero(d);
        (void)BN_copy(r,m);
        ctx->tos=tos;
        return(1);
        }

    /* We want the remainder
     * Given input of ABCDEF / ab
     * we need multiply ABCDEF by 3 digests of the reciprocal of ab
     *
     */
    i=BN_num_bits(m);

    j=recp->num_bits*2;
    if (j > i)
        {
        i=j;
        ex=0;
        }
    else
        {
        ex=(i-j)/2;
        }

    j=i/2;

    if (i != recp->shift)
        recp->shift=BN_reciprocal(&(recp->Nr),&(recp->N),
            i,ctx);

    if (!BN_rshift(a,m,j-ex)) goto err;
    if (!BN_mul(b,a,&(recp->Nr),ctx)) goto err;
    if (!BN_rshift(d,b,j+ex)) goto err;
    d->neg=0;
    if (!BN_mul(b,&(recp->N),d,ctx)) goto err;
    if (!BN_usub(r,m,b)) goto err;
    r->neg=0;

    j=0;
#if 1
    while (BN_ucmp(r,&(recp->N)) >= 0)
        {
        if (j++ > 2)
            {
#ifndef NO_ERR
            BNerr(BN_F_BN_DIV_RECP,BN_R_BAD_RECIPROCAL);
#endif
            goto err;
            }
        if (!BN_usub(r,r,&(recp->N))) goto err;
        if (!BN_add_word(d,1)) goto err;
        }
#endif

    r->neg=BN_is_zero(r)?0:m->neg;
    d->neg=m->neg^recp->N.neg;
    ret=1;
err:
    ctx->tos=tos;
    return(ret);
    } 

/* len is the expected size of the result
 * We actually calculate with an extra word of precision, so
 * we can do faster division if the remainder is not required.
 */
int BN_reciprocal(r,m,len,ctx)
BIGNUM *r;
BIGNUM *m;
int len;
BN_CTX *ctx;
    {
    int ret= -1;
    BIGNUM t;

    bn_check_top(m);

    BN_init(&t);

    (void)BN_zero(&t);
    if (!BN_set_bit(&t,len)) goto err;

    if (!BN_div(r,NULL,&t,m,ctx)) goto err;
    ret=len;
err:
    BN_free(&t);
    return(ret);
    }

