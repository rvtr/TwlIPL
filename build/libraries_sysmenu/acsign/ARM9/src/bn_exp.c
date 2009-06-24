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
#define SPLIT_BN_MOD_EXP_ORIG
#define SPLIT_BN_MOD_EXP
#endif /* NO_SPLIT */

#define TABLE_SIZE  16

#ifdef SPLIT_BN_MOD_EXP_ORIG
#if 0
/* this one works - simple but works */
int BN_mod_exp_orig(r,a,p,m,ctx)
BIGNUM *r,*a,*p,*m;
BN_CTX *ctx;
    {
    int i,bits,ret=0;
    BIGNUM *v,*tmp;

    v= &(ctx->bn[ctx->tos++]);
    tmp= &(ctx->bn[ctx->tos++]);

    if (BN_copy(v,a) == NULL) goto err;
    bits=BN_num_bits(p);

    if (BN_is_odd(p))
        { if (BN_copy(r,a) == NULL) goto err; }
    else    { if (!BN_one(r)) goto err; }

    for (i=1; i<bits; i++)
        {
        if (!BN_sqr(tmp,v,ctx)) goto err;
        if (!BN_mod(v,tmp,m,ctx)) goto err;
        if (BN_is_bit_set(p,i))
            {
            if (!BN_mul(tmp,r,v,ctx)) goto err;
            if (!BN_mod(r,tmp,m,ctx)) goto err;
            }
        }
    ret=1;
err:
    ctx->tos-=2;
    return(ret);
    }

#endif

/* this one works - simple but works */
int BN_exp(r,a,p,ctx)
BIGNUM *r,*a,*p;
BN_CTX *ctx;
    {
    int i,bits,ret=0,tos;
    BIGNUM *v,*rr;

    bn_check_top(a);
    bn_check_top(p);

    tos=ctx->tos;
    v= &(ctx->bn[ctx->tos++]);
    if ((r == a) || (r == p))
        rr= &(ctx->bn[ctx->tos++]);
    else
        rr=r;

    if (BN_copy(v,a) == NULL) goto err;
    bits=BN_num_bits(p);

    if (BN_is_odd(p))
        { if (BN_copy(rr,a) == NULL) goto err; }
    else    { if (!BN_one(rr)) goto err; }

    for (i=1; i<bits; i++)
        {
        if (!BN_sqr(v,v,ctx)) goto err;
        if (BN_is_bit_set(p,i))
            {
            if (!BN_mul(rr,rr,v,ctx)) goto err;
            }
        }
    ret=1;
err:
    ctx->tos=tos;
    if (r != rr) (void)BN_copy(r,rr);
    return(ret);
    }
#endif

#ifdef SPLIT_BN_MOD_EXP
/**
 * Perform mod exp on BIGNUM
 * @param   r Pointer to return value BIGNUM
 * @param   a Pointer to data value BIGNUM
 * @param   p Pointer to Public exponent BIGNUM
 * @param   m Pointer to modulus BIGNUM
 * @param   ctx Pointer to BN_CTX
 * @return  0 success
 * @pre     BIGNUMs a, p, m and BN_CTX ctx exist and are valid
 * @post    BIGNUM r points to evaluated mod_exp
 * @note    BN_mod_exp_mont is only available if library compiled
 *      with define BN_MONT_MUL, and will only be used to compute
 *      mod_exp operation where the modulus m is odd.
 *      BN_mod_exp_recp is only available if library is compiled
 *      with define RECP_MUL_MOD.
 *      BN_mod_exp_simple is the default method, not available if
 *      library is compiled with define RECP_MUL_MOD.
 *      if BN_mod_exp_mont is available, which ever of BN_mod_exp_simple
 *      or BN_mod_exp_recp is available will only be used for even
 *      modulus m values.
 * @relates BN_mod_exp_mont     
 * @relates BN_mod_exp_recp     
 * @relates BN_mod_exp_simple       
 */
int BN_mod_exp(r,a,p,m,ctx)
BIGNUM *r;
BIGNUM *a;
BIGNUM *p;
BIGNUM *m;
BN_CTX *ctx;
    {
    int ret;

    bn_check_top(a);
    bn_check_top(p);
    bn_check_top(m);

#ifdef MONT_MUL_MOD

    if (BN_is_odd(m))
        { ret=BN_mod_exp_mont(r,a,p,m,ctx,NULL); }
    else
#endif
#ifdef RECP_MUL_MOD
        { ret=BN_mod_exp_recp(r,a,p,m,ctx); }
#else
        { ret=BN_mod_exp_simple(r,a,p,m,ctx); }
#endif

    return(ret);
    }
#endif

#endif
