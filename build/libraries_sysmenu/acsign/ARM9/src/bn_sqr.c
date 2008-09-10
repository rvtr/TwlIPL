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

#ifdef SMALL_CODE_SIZE
#undef BN_RECURSION_SQR
#endif

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))
 
#ifdef NO_SPLIT
#define SPLIT_BN_SQR
#define SPLIT_BN_SQR_NORMAL
#define SPLIT_BN_RECURSION_SQR
#endif /* NO_SPLIT */

#ifdef SPLIT_BN_SQR
/* r must not be a */
int BN_sqr(r, a, ctx)
BIGNUM *r;
BIGNUM *a;
BN_CTX *ctx;
    {
    int max,al;
    BIGNUM *tmp,*rr;

#ifdef BN_COUNT
printf("BN_sqr %d * %d\n",a->top,a->top);
#endif
    bn_check_top(a);
    tmp= &(ctx->bn[ctx->tos]);
    rr=(a != r)?r: (&ctx->bn[ctx->tos+1]);

    al=a->top;
    if (al <= 0)
        {
        r->top=0;
        return(1);
        }

    max=(al+al);
    if (bn_wexpand(rr,max) == NULL) return(0);

    rr->top=max;
    rr->neg=0;

    if (al == 4)
        {
#ifndef BN_SQR_COMBA
        BN_ULONG t[8];
        bn_sqr_normal(rr->d,a->d,4,t);
#else
        bn_sqr_comba4(rr->d,a->d);
#endif
        }
    else if (al == 8)
        {
#ifndef BN_SQR_COMBA
        BN_ULONG t[16];
        bn_sqr_normal(rr->d,a->d,8,t);
#else
        bn_sqr_comba8(rr->d,a->d);
#endif
        }
    else 
        {
#if 1 && defined(BN_RECURSION_SQR)
        if (al < BN_SQR_RECURSIVE_SIZE_NORMAL)
            {
            BN_ULONG t[BN_SQR_RECURSIVE_SIZE_NORMAL*2];
            bn_sqr_normal(rr->d,a->d,al,t);
            }
        else
            {
            int j,l,k;

            l=BN_num_bits_word((BN_ULONG)al);
            j=1<<(l-1);
            k=j+j;
            if ((al == j) && !BN_get_flags(a,BN_FLG_STATIC_DATA))
                {
                BN_REC rec;
                if (bn_wexpand(tmp,k*2) == NULL) return(0);
                rec.depth=l-5;
                rec.n=j;
                rec.mul=bn_mul_comba8;
                rec.sqr=bn_sqr_comba8;

                bn_sqr_rec_words(rr->d,a->d,tmp->d,&rec);
                }
            else
                {
                if (bn_wexpand(tmp,max) == NULL) return(0);
                bn_assert(al*2 <= max);
                bn_sqr_normal(rr->d,a->d,al,tmp->d);
                }
            }
#else
        if (bn_wexpand(tmp,max) == NULL) return(0);
        bn_assert(al*2 <= max);
        bn_sqr_normal(rr->d,a->d,al,tmp->d);
#endif
#ifdef BN_DEBUG
        tmp->top=0;
#endif
        }

    if ((max > 0) && (rr->d[max-1] == 0)) rr->top--;
    if (rr != r) (void)BN_copy(r,rr);
    return(1);
    }
#endif

#ifdef SPLIT_BN_SQR_NORMAL
/* tmp must have 2*n words */
void bn_sqr_normal(r, a, n, tmp)
BN_ULONG *r;
BN_ULONG *a;
int n;
BN_ULONG *tmp;
    {
    int i,j,max;
    BN_ULONG *ap,*rp,m;

    max=n*2;
    ap=a;
    rp=r;
    rp[0]=rp[max-1]=0;
    rp++;
    j=n;

    if (--j > 0)
        {
        m= (*ap++);
        rp[j]=bn_mul_words(rp,ap,j,m);
        rp+=2;
        }

    for (i=n-2; i>0; i--)
        {
        j--;
        m= *(ap++);
        rp[j]=bn_mul_add_words(rp,ap,j,m);
        rp+=2;
        }

    (void)bn_add_words(r,r,r,max);

    /* There will not be a carry */

    bn_sqr_words(tmp,a,n);

    (void)bn_add_words(r,r,tmp,max);
    }
#endif

#if 0 /* replaced by bn_sqr_rec_words() AND this has bugs */
#ifdef SPLIT_BN_RECURSION_SQR
#ifdef BN_RECURSION_SQR
/* r is 2*n words in size,
 * a and b are both n words in size.
 * n must be a power of 2.
 * We multiply and return the result.
 * t must be 2*n words in size
 * We calulate
 * a[0]*b[0]
 * a[0]*b[0]+a[1]*b[1]+(a[0]-a[1])*(b[1]-b[0])
 * a[1]*b[1]
 */
void bn_sqr_recursive(r,a,n2,t)
BN_ULONG *r,*a;
int n2;
BN_ULONG *t;
    {
    int n=n2/2;
    int zero,c1;
    BN_ULONG ln,lo,*p;

#ifdef BN_COUNT
printf(" bn_sqr_recursive %d * %d\n",n2,n2);
#endif
    if (n2 == 4)
        {
#ifndef BN_SQR_COMBA
        bn_sqr_normal(r,a,4,t);
#else
        bn_sqr_comba4(r,a);
#endif
        return;
        }
    else if (n2 == 8)
        {
#ifndef BN_SQR_COMBA
        bn_sqr_normal(r,a,8,t);
#else
        bn_sqr_comba8(r,a);
#endif
        return;
        }
    if (n2 < BN_SQR_RECURSIVE_SIZE_NORMAL)
        {
        bn_sqr_normal(r,a,n2,t);
        return;
        }
    /* r=(a[0]-a[1])*(a[1]-a[0]) */
    c1=bn_cmp_words(a,&(a[n]),n);
    zero=0;
    if (c1 > 0)
        bn_sub_words(t,a,&(a[n]),n);
    else if (c1 < 0)
        bn_sub_words(t,&(a[n]),a,n);
    else
        zero=1;

    /* The result will always be negative unless it is zero */
    p= &(t[n2*2]);

    if (!zero)
        bn_sqr_recursive(&(t[n2]),t,n,p);
    else
        Memset(&(t[n2]),0,n*sizeof(BN_ULONG));
    bn_sqr_recursive(r,a,n,p);
    bn_sqr_recursive(&(r[n2]),&(a[n]),n,p);

    /* t[32] holds (a[0]-a[1])*(a[1]-a[0]), it is negative or zero
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     */

    c1=(int)(bn_add_words(t,r,&(r[n2]),n2));

    /* t[32] is negative */
    c1-=(int)(bn_sub_words(&(t[n2]),t,&(t[n2]),n2));

    /* t[32] holds (a[0]-a[1])*(a[1]-a[0])+(a[0]*a[0])+(a[1]*a[1])
     * r[10] holds (a[0]*a[0])
     * r[32] holds (a[1]*a[1])
     * c1 holds the carry bits
     */
    c1+=(int)(bn_add_words(&(r[n]),&(r[n]),&(t[n2]),n2));
    if (c1)
        {
        p= &(r[n+n2]);
        lo= *p;
        ln=(lo+c1)&BN_MASK2;
        *p=ln;

        /* The overflow will stop before we over write
         * words we should not overwrite */
        if (ln < (BN_ULONG)c1)
            {
            do  {
                p++;
                lo= *p;
                ln=(lo+1)&BN_MASK2;
                *p=ln;
                } while (ln == 0);
            }
        }
    }
#endif
#endif
#endif
#endif
