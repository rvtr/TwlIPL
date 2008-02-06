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

int BN_MONT_CTX_set_word(mont,mod,ctx)
BN_MONT_CTX *mont;
BIGNUM *mod;
BN_CTX *ctx;
    {
    BIGNUM Ri,*R;
#if 1
    BN_ULONG tmod,rr;
#else
    BN_ULONG buf[2];
    BIGNUM tmod;
#endif


    bn_check_top(mod);
    if (mod->top == 0) return(0);

    R= &(mont->RR);                 /* grab RR as a temp */
    if (BN_copy(&(mont->N),mod) == NULL)        /* Set N */
        return(0);

    BN_init(&Ri);

    mont->use_word=1;

/* EAY is this number of words to shift, or the number to shift to end up
 * with a '1' in the next word?
 * for 8 bit words, is 0x01ab == 1 or 2
 */
    mont->riw=(BN_num_bits(mod)+(BN_BITS2-1))/BN_BITS2;
    if (!BN_zero(R)) return(0);

        /* We are now setting a number which is larger than our current 
     * one after we do the shift 
     */
    if (!BN_set_bit(R,BN_BITS2))
        goto err;

#if 0
    tmod.d=buf;
    tmod.top=1;
    tmod.max=mod->max;
    tmod.neg=mod->neg;
    buf[0]=mod->d[0]&BN_MASK2;
    buf[1]=0;

    if ((BN_mod_inverse(&Ri,R,&tmod,ctx)) == NULL)
        goto err;
#else
    tmod=mod->d[0];
    rr=BN_mod_inverse_word(tmod);
    if (!BN_set_word(&Ri,rr))
        goto err;
#endif

    /* R*Ri */
    if (!BN_lshift(&Ri,&Ri,BN_BITS2))
        goto err;

    if (!BN_is_zero(&Ri))
        (void)BN_sub_word(&Ri,1);
    else
        {
        /* This is not common..., 1 in BN_MASK2,
         * It happens when buf[0] was == 1.  So for 8 bit,
         * this is 1/256, 16bit, 1 in 2^16 etc.
         */
        if (!BN_set_word(&Ri,BN_MASK2))
            goto err;
        }
#if 0
    BN_div(&Ri,NULL,&Ri,&tmod,ctx);
    //BN_div_word(&Ri,tmod);
    mont->n0=Ri.d[0];
#else
#if 0
    {
    BN_ULONG h,l;

    h=l=0;
    if (Ri.top >= 2) h=Ri.d[1];
    if (Ri.top >= 1) l=Ri.d[0];
    mont->n0=bn_div_words(h,l,tmod);
    }
#endif
    mont->n0=bn_div_words(
        (Ri.top >= 2)?Ri.d[1]:0,
        (Ri.top >= 1)?Ri.d[0]:0,
        tmod);
#endif
    /* mod->top=z; */

    /* setup RR for conversions */
    (void)BN_zero(&(mont->RR));
    if (!BN_set_bit(&(mont->RR),mont->riw*2*BN_BITS2))
        goto err;
    (void)BN_mod(&(mont->RR),&(mont->RR),&(mont->N),ctx);
    bn_zexpand(&(mont->RR),mont->riw);
#if 0
    bn_zexpand(&(mont->Ni),mont->riw); /*This is not used? */
#endif

err:
    BN_free(&Ri);
    return(1);
    }


