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

#undef BN_DEBUG
#include "bn_lcl.h"

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))
 
#ifdef NO_SPLIT
#define SPLIT_BN_MOD_MUL_MONTGOMERY
#define SPLIT_BN_FROM_MONTGOMERY
#define SPLIT_BN_MONT_CTX_NEW
#define SPLIT_BN_MONT_CTX_INIT
#define SPLIT_BN_MONT_CTX_FREE
#define SPLIT_BN_MONT_CTX_COPY
#endif /* NO_SPLIT */

/* #define DEBUG */

#ifdef SPLIT_BN_MOD_MUL_MONTGOMERY
/* We multiply a by b, generating a number 2n words long.
 * This is then reduced by the montogmery number, which is
 * n words. */
int BN_mod_mul_montgomery(r,a,b,mont,ctx)
BIGNUM *r,*a,*b;
BN_MONT_CTX *mont;
BN_CTX *ctx;
    {
    BIGNUM *tmp,*tmp2;
    int ret=0,w;
    int i;

    bn_check_top(a);
    bn_check_top(b);

    w=mont->riw;

    tmp= &(ctx->bn[ctx->tos++]);
    tmp2= &(ctx->bn[ctx->tos++]);
    bn_check_top(tmp);
    bn_check_top(tmp2);
    if (bn_wexpand(tmp,w+w) == NULL) goto err;
    if (bn_wexpand(r,w+w) == NULL) goto err;

    if (a == b)
        {
        if (bn_wexpand(tmp2,w+w) == NULL) goto err;
#ifdef BN_SQR_COMBA
        if ((a->top == 8) && (a->top == b->top))
            bn_sqr_comba8(tmp->d,a->d);
        else
#endif
            {
            bn_sqr_normal(tmp->d,a->d,a->top,tmp2->d);
            }
        }
    else
        {
#ifdef BN_MUL_COMBA
        if (a->top == 8)
            bn_mul_comba8(tmp->d,a->d,b->d);
        else
#endif
            {
            bn_mul_normal(tmp->d,a->d,a->top,b->d,b->top);
            }
        }
        
    i=a->top+b->top;
    while  (i<(w+w))
        {
        tmp->d[i]=0;
        i++;
        }
    tmp->top=w+w;
    tmp->neg=0;

    /* reduce from aRR to aR */
    /* tmp->d is 2w words */
#if 0
    bn_fix_top(tmp);
    BN_from_montgomery_words(r,tmp,mont,ctx);
#else
    bn_from_montgomery_words(r->d,tmp->d,mont->N.d,w,mont->n0);
#endif
    r->top=w;
    r->neg=0;
    bn_fix_top(r);

    ret=1;
err:
    ctx->tos-=2;
    return(ret);
    }
#endif

#ifdef SPLIT_BN_FROM_MONTGOMERY
int BN_from_montgomery(ret,a,mont,ctx)
BIGNUM *ret;
BIGNUM *a;
BN_MONT_CTX *mont;
BN_CTX *ctx;
    {
    BIGNUM *n,*r;
    BN_ULONG *np,*rp,n0,v0,v1,*nrp;
    int al,nl,max,i;
    int retn=0;

    bn_check_top(a);

    r= &(ctx->bn[ctx->tos]);

    if (!BN_copy(r,a)) goto err1;
    n= &(mont->N);

    /* mont->riw is the size of mont->N in bits/words */
    al=mont->riw;

    nl=n->top;
    if ((al == 0) || (nl == 0)) { r->top=0; return(1); }

    max=(nl+al+1); /* allow for overflow (no?) XXX */
    if (bn_wexpand(r,max) == NULL) goto err1;
    if (bn_wexpand(ret,max) == NULL) goto err1;

    r->neg=a->neg^n->neg;
    np=n->d;
    rp=r->d;
    nrp= &(r->d[nl]);

    /* clear the top words of T */
    for (i=r->top; i<max; i++) /* Memset? XXX */
        r->d[i]=0;

    r->top=max;
    n0=mont->n0;

    v1=0;
    for (i=0; i<nl; i++)
        {
        v0=bn_mul_add_words(rp,np,nl,(rp[0]*n0)&BN_MASK2);
        v0=(v1+v0)&BN_MASK2;
        v1=(BN_ULONG)( (v0 < v1)?1:0 );
        if ((nrp[0]=(nrp[0]+v0)&BN_MASK2) < v0)
            v1++;
        nrp++;
        rp++;
        }
    nrp[0]=v1;
    bn_fix_top(r);

    /* mont->riw will be a multiple of the word size */
    (void)BN_rshift(ret,r,mont->riw*BN_BITS2);

    if (BN_ucmp(ret, &(mont->N)) >= 0)
        {
        (void)BN_usub(ret,ret,&(mont->N)); /* XXX */
        }
    retn=1;

err1:
    return(retn);
    }
#endif

#ifdef SPLIT_BN_MONT_CTX_NEW
BN_MONT_CTX *BN_MONT_CTX_new()
    {
    BN_MONT_CTX *ret;

    if ((ret=(BN_MONT_CTX *)Malloc(sizeof(BN_MONT_CTX))) == NULL)
        return(NULL);

    BN_MONT_CTX_init(ret);
    ret->flags=BN_FLG_MALLOCED;
    return(ret);
    }
#endif

#ifdef SPLIT_BN_MONT_CTX_INIT
void BN_MONT_CTX_init(ctx)
BN_MONT_CTX *ctx;
    {
    ctx->use_word=0;
    ctx->riw=0;
    BN_init(&(ctx->RR));
    BN_init(&(ctx->N));
    BN_init(&(ctx->Ni));
    ctx->flags=0;
    }
#endif

#ifdef SPLIT_BN_MONT_CTX_FREE
void BN_MONT_CTX_free(mont)
BN_MONT_CTX *mont;
    {
    BN_free(&(mont->RR));
    BN_free(&(mont->N));
    BN_free(&(mont->Ni));
    if (mont->flags & BN_FLG_MALLOCED)
        Free(mont);
    }
#endif

#ifdef SPLIT_BN_MONT_CTX_COPY
BN_MONT_CTX *BN_MONT_CTX_copy(to, from)
BN_MONT_CTX *to, *from;
    {
    if (to == from) return(to);

    (void)BN_copy(&(to->RR),&(from->RR));
    (void)BN_copy(&(to->N),&(from->N));
    (void)BN_copy(&(to->Ni),&(from->Ni));
    bn_zexpand(&(to->RR),from->riw);
    bn_zexpand(&(to->Ni),from->riw);
    to->use_word=from->use_word;
    to->riw=from->riw;
    to->n0=from->n0;
    return(to);
    }
#endif

#endif
