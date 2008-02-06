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

#if     0   //RSA
#include <stdio.h>
#endif
#include "bn_lcl.h"

/* #ifdef MONT_MUL_MOD */
/**
 * Generic form of Montgomery Exponentiation, can be called directly
 * out side of a BN_ME_METH
 *
 * @param rr_in    BIGNUM pointer for result 
 * @param a_in     BIGNUM pointer for base 
 * @param p        BIGNUM pointer to the exponent
 * @param m        BIGNUM pointer to the modulus
 * @param ctx      BN_CTX pointer for temp values are argument storage
 * @param in_mont  BN_MONT_CTX pointer to structure of montgomery values
 *
 * @pre            arguments are all initialised and not NULL or zero
 * @post           rr_in contains evaluated result
 *
 * @note           
 */
int BN_mod_exp_mont(rr_in,a_in,p,m,ctx,in_mont)
BIGNUM *rr_in;
BIGNUM *a_in;
BIGNUM *p;
BIGNUM *m;
BN_CTX *ctx;
BN_MONT_CTX *in_mont;
        {
        int i,j,k,ret=0,tos,top;
        int mul,sqr,num;
        BIGNUM *d,*aa,*r,*a,*t;
        BN_ULONG *vall[BN_EXP_TABLE_SIZE],*dp,*rp,n0,*rr,*tmp,*np;
        BN_MONT_CTX *mont=NULL;
        unsigned char *str=NULL;
#ifdef BN_SURRENDER
    R_SURRENDER *surrender=NULL;
    int count=0;
#endif

        bn_check_top(a_in);
        bn_check_top(p);
        bn_check_top(m);

        if (!(m->d[0] & 1))
                {
#ifndef NO_ERR
                BNerr(BN_F_BN_MOD_EXP_MONT,BN_R_CALLED_WITH_EVEN_MODULUS);
#endif
                return(0);
                }

        tos=ctx->tos;

        if (BN_is_zero(a_in))
                { (void)BN_zero(rr_in); return(1); }
        if (BN_is_zero(p))
                { (void)BN_one(rr_in); return(1); }
        if (BN_is_one(p))
                { (void)BN_copy(rr_in,a_in); return(1); }

        /* If this is not done, things will break in the montgomery
         * part */

#if 1
        if (in_mont != NULL)
                mont=in_mont;
        else
#endif
                {
                if ((mont=BN_MONT_CTX_new()) == NULL) goto err;
                if (!BN_MONT_CTX_set(mont,m,ctx)) goto err;
                }

#ifdef BN_SURRENDER
        surrender=ctx->surrender;
#endif

        if (!BN_gen_exp_bits(p,&str,0,ctx)) goto err;
        /* Remember this function uses another element from the passed ctx */
        ctx->tos++;
        num=str[3];
        i=(BN_BITS+str[2]-1)/str[2];
        str+=4;


        top=mont->N.top;
        if ((a_in->top == top) && (a_in->d[top-1] < m->d[top-1]))
                {
                a=a_in;
                }
        else if (a_in->top < top)
                {
                a= &(ctx->bn[ctx->tos++]);
                a->top=a_in->top;
                bn_zexpand(a,top);
                for (j=0; j<a_in->top; j++)
                        a->d[j]=a_in->d[j];
                }
        else /* if (a_in->top > i) */
                {
                a= &(ctx->bn[ctx->tos++]);
                if (!BN_mod(a,a_in,m,ctx)) goto err;
                bn_zexpand(a,top);
                }
        
        /* At this point a is the size of the modulus and is 0 padded
         * out to its size if needed.
         */

        /* Number of tmp words */
        k=num*top;
        j=k+(p->top*i*2+BN_BYTES-1+4)/BN_BYTES;

        aa=&(ctx->bn[ctx->tos++]);
        d= &(ctx->bn[ctx->tos++]);
        r= &(ctx->bn[ctx->tos++]);
        t= &(ctx->bn[ctx->tos++]);

        if (bn_wexpand(rr_in,top) == NULL) goto err;
        if (bn_wexpand(d,top*4) == NULL) goto err; /* *2? */
        if (bn_wexpand(r,top*2) == NULL) goto err;
        if (bn_wexpand(aa,j) == NULL) goto err;
        if (bn_wexpand(t,top*2) == NULL) goto err;
        dp=d->d;
        rp=r->d;
        tmp=t->d;
        rr=mont->RR.d;

        n0=mont->n0;
        np=mont->N.d;

        vall[0]=aa->d;

        bn_mul_normal(tmp,a->d,top,rr,top);
        bn_from_montgomery_words(vall[0],tmp,np,top,n0);

        if (num > 1)
        {
            bn_sqr_normal(tmp,vall[0],top,dp);
            bn_from_montgomery_words(dp,tmp,np,top,n0);

            for (i=1; i<num; i++)
            {
                vall[i]= &(vall[i-1][top]);

                bn_mul_normal(tmp,vall[i-1],top,dp,top);
                bn_from_montgomery_words(vall[i],tmp,np,top,n0);
            }
        }

        mul= *(str++);
        sqr= *(str++);
        /* add all of the 255s together plus next less than 255 
         * 255 is a special case and should be treated as 256
         * which is too big to fit in an unsigned char 
         */
        if(sqr == 255 && mul == 0)
        {
            mul = *(str++);        /* grab next mul in case it is last */
            while(*str == 255 && mul == 0)
            {
                sqr += 256;  
        str++;
                mul = *(str++);     /* increment the str pointer */
            }
            sqr+= *(str++) + 1;  /* add last sqr value < 255 + 1 for 1st 255 */
        }
        /* Get the first element */
        (void)Memcpy(rp,vall[mul>>1],sizeof(BN_ULONG)*top);
        while (sqr != 0)
        {
#ifdef BN_SURRENDER
            /*
             * Check this at the start of the loop rather than the
             * end so we avoid as many of the bn operations as possible
             */
            if (surrender != NULL)
            {
                if (surrender->callback(surrender,0xff, count++) != 0)
                {
                    goto err;
                }
            }
#endif /* BN_SURRENDER */

            /*
             * check the abort flag at the start of the loop so we avoid
             * as many of the bn operations as possible
             */
            if (BN_CTX_get_flags(ctx, BN_CTX_FLG_ABORT))
            {
                goto err;
            }

            for (i=0; i<sqr; i++)
            {
                bn_sqr_normal(tmp,rp,top,dp);
                bn_from_montgomery_words(rp,tmp,np,top,n0);
            }
            mul= *(str++);
            sqr= *(str++);
            /* add all of the 255s together plus next less than 255 */
            if(sqr == 255 && mul == 0)
            {
                mul = *(str++);        /* grab next mul */
                while(str[0] == 255 && mul == 0)
                {
                    sqr += 256;
                    str++; 
                    mul = *(str++);   /* increment the str pointer */
                }
                sqr+= *(str++) + 1;  /* add last sqr value < 255 + 1 */
            }
            if ((mul == 0) && (sqr == 0)) break;
            if ((sqr != 0) || (mul != 1))
            {
                bn_mul_normal(tmp,rp,top,vall[mul>>1],top);
                bn_from_montgomery_words(rp,tmp,np,top,n0);
            }        
            else
            {
                bn_mul_normal(tmp,rp,top,a->d,top);
                bn_from_montgomery_words(rr_in->d,tmp,np,top,n0);
                goto end;
            }
        }

        /* if (mul != 1) */
                {
                for (i=top; i<top+top; i++)
                        rp[i]=0;
                bn_from_montgomery_words(rr_in->d,rp,np,top,n0);
                }
end:        /* Use this to avoid an if */

        /*
         * Even if the operation has completed successfully, if the
         * abort flag is set we want to abort so that anything
         * calling this function will know not to continue.
         */
        if (BN_CTX_get_flags(ctx, BN_CTX_FLG_ABORT))
        {
            goto err;
        }

        rr_in->top=top;
        bn_fix_top(rr_in);
        ret=1;
err:
#ifdef BN_SURRENDER
        /*
         * Even if the operation completed successfully we want to abort
         * if the abort flag has been is set, so that subsequent operations
         * do not continue. Allow the caller change to return value and 
         * cause the function to fail.
         */
    if (surrender != NULL)
    {
            if (surrender->callback(surrender, 0xff, -1) != 0)
            {
        ret = 0;
            }
    }
#endif
        if ((in_mont == NULL) && (mont != NULL)) BN_MONT_CTX_free(mont);
        ctx->tos=tos;
        return(ret);
        }
/* #endif */

