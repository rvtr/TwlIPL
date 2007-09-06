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

/* #ifdef RECP_MUL_MOD */
int BN_mod_exp_recp(r,a,p,m,ctx)
BIGNUM *r;
BIGNUM *a;
BIGNUM *p;
BIGNUM *m;
BN_CTX *ctx;
    {
    int i,j,bits,ret=0,wstart,wend,window,wvalue;
    int start=1,ts=0;
    BIGNUM *aa;
    BIGNUM val[BN_EXP_TABLE_SIZE];
    BN_RECP_CTX recp;

    bits=BN_num_bits(p);

    if (BN_is_zero(a))
        { (void)BN_zero(r); return(1); }
    if (BN_is_zero(p))
        { (void)BN_one(r); return(1); }
    if (BN_is_one(p))
        { (void)BN_copy(r,a); return(1); }

    BN_RECP_CTX_init(&recp);
    if (BN_RECP_CTX_set(&recp,m,ctx) <= 0) goto err;

    BN_init(&(val[0]));
    ts=1;

    aa= &(ctx->bn[ctx->tos++]);

    if (!BN_mod(&(val[0]),a,m,ctx)) goto err;       /* 1 */
    if (!BN_mod_mul_reciprocal(aa,&(val[0]),&(val[0]),&recp,ctx))
        goto err;               /* 2 */

    if (bits <= 17) /* This is probably 3 or 0x10001, so just do singles */
        window=1;
    else if (bits >= 256)
        window=5;   /* max size of window */
    else if (bits >= 128)
        window=4;
    else
        window=3;

    j=1<<(window-1);
    for (i=1; i<j; i++)
        {
        BN_init(&val[i]);
        if (!BN_mod_mul_reciprocal(&(val[i]),&(val[i-1]),aa,&recp,ctx))
            goto err;
        }
    ts=i;

    start=1;    /* This is used to avoid multiplication etc
             * when there is only the value '1' in the
             * buffer. */
    wvalue=0;   /* The 'value' of the window */
    wstart=bits-1;  /* The top bit of the window */
    wend=0;     /* The bottom bit of the window */

    if (!BN_one(r)) goto err;

    for (;;)
        {
        if (BN_is_bit_set(p,wstart) == 0)
            {
            if (!start)
                if (!BN_mod_mul_reciprocal(r,r,r,&recp,ctx))
                goto err;
            if (wstart == 0) break;
            wstart--;
            continue;
            }
        /* We now have wstart on a 'set' bit, we now need to work out
         * how bit a window to do.  To do this we need to scan
         * forward until the last set bit before the end of the
         * window */
        j=wstart;
        wvalue=1;
        wend=0;
        for (i=1; i<window; i++)
            {
            if (wstart-i < 0) break;
            if (BN_is_bit_set(p,wstart-i))
                {
                wvalue<<=(i-wend);
                wvalue|=1;
                wend=i;
                }
            }

        /* wend is the size of the current window */
        j=wend+1;
        /* add the 'bytes above' */
        if (!start)
            for (i=0; i<j; i++)
                {
                if (!BN_mod_mul_reciprocal(r,r,r,&recp,ctx))
                    goto err;
                }
        
        /* wvalue will be an odd number < 2^window */
        if (!BN_mod_mul_reciprocal(r,r,&(val[wvalue>>1]),&recp,ctx))
            goto err;

        /* move the 'window' down further */
        wstart-=wend+1;
        wvalue=0;
        start=0;
        if (wstart < 0) break;
        }
    ret=1;
err:
    ctx->tos--;
    for (i=0; i<ts; i++)
        BN_clear_free(&(val[i]));
    BN_RECP_CTX_free(&recp);
    return(ret);
    }
/* #endif */

