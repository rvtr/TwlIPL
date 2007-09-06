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

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))
 
#ifdef NO_SPLIT
#define SPLIT_BN_MUL_RECURSIVE
#define SPLIT_BN_MUL_PART_RECURSIVE
#define SPLIT_BN_MUL_LOW_RECURSIVE
#define SPLIT_BN_MUL_HIGH
#define SPLIT_BN_MUL
#define SPLIT_BN_MUL_NORMAL
#define SPLIT_BN_MUL_LOW_NORMAL
#endif /* NO_SPLIT */

#ifdef SMALL_CODE_SIZE
#undef BN_RECURSION_MUL
#endif

#ifdef BN_RECURSION_MUL
#if 0 /* Replaced by bn_mul_rec_words() */
/* r is 2*n2 words in size,
 * a and b are both n2 words in size.
 * n2 must be a power of 2.
 * We multiply and return the result.
 * t must be 2*n2 words in size
 * We calulate
 * a[0]*b[0]
 * a[0]*b[0]+a[1]*b[1]+(a[0]-a[1])*(b[1]-b[0])
 * a[1]*b[1]
 */
#ifdef SPLIT_BN_MUL_RECURSIVE
void bn_mul_recursive(r,a,b,n2,t)
BN_ULONG *r,*a,*b;
int n2;
BN_ULONG *t;
    {
    int n=n2/2,c1,c2;
    unsigned int neg,zero;
    BN_ULONG ln,lo,*p;

#ifdef BN_COUNT
printf(" bn_mul_recursive %d * %d\n",n2,n2);
#endif
#ifdef BN_MUL_COMBA
/*  if (n2 == 4)
        {
        bn_mul_comba4(r,a,b);
        return;
        }
    else */ if (n2 == 8)
        {
        bn_mul_comba8(r,a,b);
        return; 
        }
#endif
    if (n2 < BN_MUL_RECURSIVE_SIZE_NORMAL)
        {
        /* This should not happen */
        bn_mul_normal(r,a,n2,b,n2);
        return;
        }
    /* r=(a[0]-a[1])*(b[1]-b[0]) */
    c1=bn_cmp_words(a,&(a[n]),n);
    c2=bn_cmp_words(&(b[n]),b,n);
    zero=neg=0;
    switch (c1*3+c2)
        {
    case -4:
        bn_sub_words(t,      &(a[n]),a,      n); /* - */
        bn_sub_words(&(t[n]),b,      &(b[n]),n); /* - */
        break;
    case -3:
        zero=1;
        break;
    case -2:
        bn_sub_words(t,      &(a[n]),a,      n); /* - */
        bn_sub_words(&(t[n]),&(b[n]),b,      n); /* + */
        neg=1;
        break;
    case -1:
    case 0:
    case 1:
        zero=1;
        break;
    case 2:
        bn_sub_words(t,      a,      &(a[n]),n); /* + */
        bn_sub_words(&(t[n]),b,      &(b[n]),n); /* - */
        neg=1;
        break;
    case 3:
        zero=1;
        break;
    case 4:
        bn_sub_words(t,      a,      &(a[n]),n);
        bn_sub_words(&(t[n]),&(b[n]),b,      n);
        break;
        }

#ifdef BN_MUL_COMBA
    if (n == 4)
        {
        if (!zero)
            bn_mul_comba4(&(t[n2]),t,&(t[n]));
        else
            Memset(&(t[n2]),0,8*sizeof(BN_ULONG));
        
        bn_mul_comba4(r,a,b);
        bn_mul_comba4(&(r[n2]),&(a[n]),&(b[n]));
        }
    else if (n == 8)
        {
        if (!zero)
            bn_mul_comba8(&(t[n2]),t,&(t[n]));
        else
            Memset(&(t[n2]),0,16*sizeof(BN_ULONG));
        
        bn_mul_comba8(r,a,b);
        bn_mul_comba8(&(r[n2]),&(a[n]),&(b[n]));
        }
    else
#endif
        {
        p= &(t[n2*2]);
        if (!zero)
            bn_mul_recursive(&(t[n2]),t,&(t[n]),n,p);
        else
            Memset(&(t[n2]),0,n2*sizeof(BN_ULONG));
        bn_mul_recursive(r,a,b,n,p);
        bn_mul_recursive(&(r[n2]),&(a[n]),&(b[n]),n,p);
        }

    /* t[32] holds (a[0]-a[1])*(b[1]-b[0]), c1 is the sign
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     */

    c1=(int)bn_add_words(t,r,&(r[n2]),n2);

    if (neg) /* if t[32] is negative */
        {
        c1-=(int)bn_sub_words(&(t[n2]),t,&(t[n2]),n2);
        }
    else
        {
        /* Might have a carry */
        c1+=(int)bn_add_words(&(t[n2]),&(t[n2]),t,n2);
        }

    /* t[32] holds (a[0]-a[1])*(b[1]-b[0])+(a[0]*b[0])+(a[1]*b[1])
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     * c1 holds the carry bits
     */
    c1+=(int)bn_add_words(&(r[n]),&(r[n]),&(t[n2]),n2);
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

#if 0
#ifdef SPLIT_BN_MUL_PART_RECURSIVE
/* n+tn is the word length
 * t must be n*4 is size, as does r */
void bn_mul_part_recursive(r,a,b,tn,n,t)
BN_ULONG *r,*a,*b;
int tn,n;
BN_ULONG *t;
    {
    int i,j,n2=n*2;
    int c1;
    BN_ULONG ln,lo,*p;

#ifdef BN_COUNT
printf(" bn_mul_part_recursive %d * %d\n",tn+n,tn+n);
#endif
    if (n < 8)
        {
        i=tn+n;
        bn_mul_normal(r,a,i,b,i);
        return;
        }

    /* r=(a[0]-a[1])*(b[1]-b[0]) */
    bn_sub_words(t,      a,      &(a[n]),n); /* + */
    bn_sub_words(&(t[n]),b,      &(b[n]),n); /* - */

#ifdef BN_MUL_COMBA
/*  if (n == 4)
        {
        bn_mul_comba4(&(t[n2]),t,&(t[n]));
        bn_mul_comba4(r,a,b);
        bn_mul_normal(&(r[n2]),&(a[n]),tn,&(b[n]),tn);
        Memset(&(r[n2+tn*2]),0,sizeof(BN_ULONG)*(n2-tn*2));
        }
    else */ if (n == 8)
        {
        bn_mul_comba8(&(t[n2]),t,&(t[n]));
        bn_mul_comba8(r,a,b);
        bn_mul_normal(&(r[n2]),&(a[n]),tn,&(b[n]),tn);
        Memset(&(r[n2+tn*2]),0,sizeof(BN_ULONG)*(n2-tn*2));
        }
    else
#endif
        {
        p= &(t[n2*2]);
        bn_mul_recursive(&(t[n2]),t,&(t[n]),n,p);
        bn_mul_recursive(r,a,b,n,p);
        i=n/2;
        /* If there is only a bottom half to the number,
         * just do it */
        j=tn-i;
        if (j == 0)
            {
            bn_mul_recursive(&(r[n2]),&(a[n]),&(b[n]),i,p);
            Memset(&(r[n2+i*2]),0,sizeof(BN_ULONG)*(n2-i*2));
            }
        else if (j > 0) /* eg, n == 16, i == 8 and tn == 11 */
                {
                bn_mul_part_recursive(&(r[n2]),&(a[n]),&(b[n]),
                    j,i,p);
                Memset(&(r[n2+tn*2]),0,
                    sizeof(BN_ULONG)*(n2-tn*2));
                }
        else /* (j < 0) eg, n == 16, i == 8 and tn == 5 */
            {
            Memset(&(r[n2]),0,sizeof(BN_ULONG)*n2);
            if (tn < BN_MUL_RECURSIVE_SIZE_NORMAL)
                {
                bn_mul_normal(&(r[n2]),&(a[n]),tn,&(b[n]),tn);
                }
            else
                {
                for (;;)
                    {
                    i/=2;
                    if (i < tn)
                        {
                        bn_mul_part_recursive(&(r[n2]),
                            &(a[n]),&(b[n]),
                            tn-i,i,p);
                        break;
                        }
                    else if (i == tn)
                        {
                        bn_mul_recursive(&(r[n2]),
                            &(a[n]),&(b[n]),
                            i,p);
                        break;
                        }
                    }
                }
            }
        }

    /* t[32] holds (a[0]-a[1])*(b[1]-b[0]), c1 is the sign
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     */

    c1=(int)bn_add_words(t,r,&(r[n2]),n2);
    c1-=(int)bn_sub_words(&(t[n2]),t,&(t[n2]),n2);

    /* t[32] holds (a[0]-a[1])*(b[1]-b[0])+(a[0]*b[0])+(a[1]*b[1])
     * r[10] holds (a[0]*b[0])
     * r[32] holds (b[1]*b[1])
     * c1 holds the carry bits
     */
    c1+=(int)bn_add_words(&(r[n]),&(r[n]),&(t[n2]),n2);
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

#if 0
#ifdef SPLIT_BN_MUL_LOW_RECURSIVE
/* a and b must be the same size, which is n2.
 * r must be n2 words and t must be n2*2
 */
void bn_mul_low_recursive(r,a,b,n2,t)
BN_ULONG *r,*a,*b;
int n2;
BN_ULONG *t;
    {
    int n=n2/2;

#ifdef BN_COUNT
printf(" bn_mul_low_recursive %d * %d\n",n2,n2);
#endif

    bn_mul_recursive(r,a,b,n,&(t[0]));
    if (n >= BN_MUL_LOW_RECURSIVE_SIZE_NORMAL)
        {
        bn_mul_low_recursive(&(t[0]),&(a[0]),&(b[n]),n,&(t[n2]));
        bn_add_words(&(r[n]),&(r[n]),&(t[0]),n);
        bn_mul_low_recursive(&(t[0]),&(a[n]),&(b[0]),n,&(t[n2]));
        bn_add_words(&(r[n]),&(r[n]),&(t[0]),n);
        }
    else
        {
        bn_mul_low_normal(&(t[0]),&(a[0]),&(b[n]),n);
        bn_mul_low_normal(&(t[n]),&(a[n]),&(b[0]),n);
        bn_add_words(&(r[n]),&(r[n]),&(t[0]),n);
        bn_add_words(&(r[n]),&(r[n]),&(t[n]),n);
        }
    }
#endif
#endif 

#ifdef SPLIT_BN_MUL_HIGH
#if 0
/* a and b must be the same size, which is n2.
 * r must be n2 words and t must be n2*2
 * l is the low words of the output.
 * t must be n2*3
 */
void bn_mul_high(r,a,b,l,n2,t)
BN_ULONG *r,*a,*b,*l;
int n2;
BN_ULONG *t;
    {
    int i,n;
    int c1,c2;
    int neg,oneg,zero;
    BN_ULONG ll,lc,*lp,*mp;

#ifdef BN_COUNT
printf(" bn_mul_high %d * %d\n",n2,n2);
#endif
    n=n2/2;

    /* Calculate (al-ah)*(bh-bl) */
    neg=zero=0;
    c1=bn_cmp_words(&(a[0]),&(a[n]),n);
    c2=bn_cmp_words(&(b[n]),&(b[0]),n);
    switch (c1*3+c2)
        {
    case -4:
        bn_sub_words(&(r[0]),&(a[n]),&(a[0]),n);
        bn_sub_words(&(r[n]),&(b[0]),&(b[n]),n);
        break;
    case -3:
        zero=1;
        break;
    case -2:
        bn_sub_words(&(r[0]),&(a[n]),&(a[0]),n);
        bn_sub_words(&(r[n]),&(b[n]),&(b[0]),n);
        neg=1;
        break;
    case -1:
    case 0:
    case 1:
        zero=1;
        break;
    case 2:
        bn_sub_words(&(r[0]),&(a[0]),&(a[n]),n);
        bn_sub_words(&(r[n]),&(b[0]),&(b[n]),n);
        neg=1;
        break;
    case 3:
        zero=1;
        break;
    case 4:
        bn_sub_words(&(r[0]),&(a[0]),&(a[n]),n);
        bn_sub_words(&(r[n]),&(b[n]),&(b[0]),n);
        break;
        }
        
    oneg=neg;
    /* t[10] = (a[0]-a[1])*(b[1]-b[0]) */
    /* r[10] = (a[1]*b[1]) */
#ifdef BN_MUL_COMBA
    if (n == 8)
        {
        bn_mul_comba8(&(t[0]),&(r[0]),&(r[n]));
        bn_mul_comba8(r,&(a[n]),&(b[n]));
        }
    else
#endif
        {
#ifdef BN_MUL_RECURSION
        bn_mul_recursive(&(t[0]),&(r[0]),&(r[n]),n,&(t[n2]));
        bn_mul_recursive(r,&(a[n]),&(b[n]),n,&(t[n2]));
#else
        bn_mul_normal(&(t[0]),&(r[0]),n,&(r[n]),n);
        bn_mul_normal(r,&(a[n]),n,&(b[n]),n);
#endif
        }

    /* s0 == low(al*bl)
     * s1 == low(ah*bh)+low((al-ah)*(bh-bl))+low(al*bl)+high(al*bl)
     * We know s0 and s1 so the only unknown is high(al*bl)
     * high(al*bl) == s1 - low(ah*bh+s0+(al-ah)*(bh-bl))
     * high(al*bl) == s1 - (r[0]+l[0]+t[0])
     */
    if (l != NULL)
        {
        lp= &(t[n2+n]);
        c1=(int)bn_add_words(lp,&(r[0]),&(l[0]),n);
        }
    else
        {
        c1=0;
        lp= &(r[0]);
        }

    if (neg)
        neg=(int)bn_sub_words(&(t[n2]),lp,&(t[0]),n);
    else
        {
        bn_add_words(&(t[n2]),lp,&(t[0]),n);
        neg=0;
        }

    if (l != NULL)
        {
        bn_sub_words(&(t[n2+n]),&(l[n]),&(t[n2]),n);
        }
    else
        {
        lp= &(t[n2+n]);
        mp= &(t[n2]);
        for (i=0; i<n; i++)
            lp[i]=((~mp[i])+1)&BN_MASK2;
        }

    /* s[0] = low(al*bl)
     * t[3] = high(al*bl)
     * t[10] = (a[0]-a[1])*(b[1]-b[0]) neg is the sign
     * r[10] = (a[1]*b[1])
     */
    /* R[10] = al*bl
     * R[21] = al*bl + ah*bh + (a[0]-a[1])*(b[1]-b[0])
     * R[32] = ah*bh
     */
    /* R[1]=t[3]+l[0]+r[0](+-)t[0] (have carry/borrow)
     * R[2]=r[0]+t[3]+r[1](+-)t[1] (have carry/borrow)
     * R[3]=r[1]+(carry/borrow)
     */
    if (l != NULL)
        {
        lp= &(t[n2]);
        c1= (int)bn_add_words(lp,&(t[n2+n]),&(l[0]),n);
        }
    else
        {
        lp= &(t[n2+n]);
        c1=0;
        }
    c1+=(int)bn_add_words(&(t[n2]),lp,  &(r[0]),n);
    if (oneg)
        c1-=(int)bn_sub_words(&(t[n2]),&(t[n2]),&(t[0]),n);
    else
        c1+=(int)bn_add_words(&(t[n2]),&(t[n2]),&(t[0]),n);

    c2 =(int)bn_add_words(&(r[0]),&(r[0]),&(t[n2+n]),n);
    c2+=(int)bn_add_words(&(r[0]),&(r[0]),&(r[n]),n);
    if (oneg)
        c2-=(int)bn_sub_words(&(r[0]),&(r[0]),&(t[n]),n);
    else
        c2+=(int)bn_add_words(&(r[0]),&(r[0]),&(t[n]),n);
    
    if (c1 != 0) /* Add starting at r[0], could be +ve or -ve */
        {
        i=0;
        if (c1 > 0)
            {
            lc=c1;
            do  {
                ll=(r[i]+lc)&BN_MASK2;
                r[i++]=ll;
                lc=(lc > ll);
                } while (lc);
            }
        else
            {
            lc= -c1;
            do  {
                ll=r[i];
                r[i++]=(ll-lc)&BN_MASK2;
                lc=(lc > ll);
                } while (lc);
            }
        }
    if (c2 != 0) /* Add starting at r[1] */
        {
        i=n;
        if (c2 > 0)
            {
            lc=c2;
            do  {
                ll=(r[i]+lc)&BN_MASK2;
                r[i++]=ll;
                lc=(lc > ll);
                } while (lc);
            }
        else
            {
            lc= -c2;
            do  {
                ll=r[i];
                r[i++]=(ll-lc)&BN_MASK2;
                lc=(lc > ll);
                } while (lc);
            }
        }
    }
#endif
#endif
#endif

#ifdef SPLIT_BN_MUL
int BN_mul(r,a,b,ctx)
BIGNUM *r,*a,*b;
BN_CTX *ctx;
    {
    int top,al,bl,neg;
    BIGNUM *rr;
#ifdef BN_RECURSION_MUL
    BIGNUM *t;
    int i,j,k,l;
#endif

#ifdef BN_COUNT
printf("BN_mul %d * %d\n",a->top,b->top);
#endif

    bn_check_top(a);
    bn_check_top(b);
    bn_check_top(r);

    al=a->top;
    bl=b->top;

    if ((al == 0) || (bl == 0))
        {
        (void)BN_zero(r);
        return(1);
        }
    top=al+bl;
    neg=a->neg^b->neg;

    if ((r == a) || (r == b))
        rr= &(ctx->bn[ctx->tos+1]);
    else
        rr=r;

    if (bn_wexpand(rr,top) == NULL) return(0);
    rr->top=top;

#if defined(BN_MUL_COMBA) || defined(BN_RECURSION_MUL)
    if (al == bl)
        {
#  ifdef BN_MUL_COMBA
/*      if (al == 4)
            {
            bn_mul_comba4(rr->d,a->d,b->d);
            goto end;
            }
        else */ if (al == 8)
            {
            bn_mul_comba8(rr->d,a->d,b->d);
            goto end;
            }
        else
#  endif
#ifdef BN_RECURSION_MUL
        if (al < BN_MULL_SIZE_NORMAL)
#endif
            {
            bn_mul_normal(rr->d,a->d,al,b->d,bl);
            goto end;
            }
#  ifdef BN_RECURSION_MUL
        goto symetric;
#  endif
        }
#endif
#ifdef BN_RECURSION_MUL
    else if ((al < BN_MULL_SIZE_NORMAL) || (bl < BN_MULL_SIZE_NORMAL))
        {
        bn_mul_normal(rr->d,a->d,al,b->d,bl);
        goto end;
        }
    else
        {
        i=(al-bl);
        if ((i ==  1) && !BN_get_flags(b,BN_FLG_STATIC_DATA))
            {
            bn_wexpand(b,al);
            b->d[bl]=0;
            bl++;
            goto symetric;
            }
        else if ((i ==  -1) && !BN_get_flags(a,BN_FLG_STATIC_DATA))
            {
            bn_wexpand(a,bl);
            a->d[al]=0;
            al++;
            goto symetric;
            }
        }
#endif

#ifdef BN_RECURSION_MUL
normal_mul:
#endif
    bn_mul_normal(rr->d,a->d,al,b->d,bl);

#ifdef BN_RECURSION_MUL
    if (0)
        {
symetric:
        /* symetric and > 4 */
        /* 16 or larger */
        l=BN_num_bits_word((BN_ULONG)al);
        j=1<<(l-1);
        k=j+j;
        t= &(ctx->bn[ctx->tos]);
        if (al == j) /* exact multiple */
            {
            BN_REC rec;
            rec.depth=l-5;
            rec.n=j;
            rec.mul=bn_mul_comba8;
            rec.sqr=bn_sqr_comba8;
            if (bn_wexpand(t,k+k) == NULL)
                return(0);
            if (bn_wexpand(rr,k) == NULL)
                return(0);
            bn_mul_rec_words(rr->d,a->d,b->d,t->d,&rec);
            }
        else
            goto normal_mul;
#if 0
            {
            bn_zexpand(a,k);
            bn_zexpand(b,k);
            bn_wexpand(t,k);
            bn_wexpand(rr,k);
            bn_mul_part_recursive(rr->d,a->d,b->d,al-j,j,t->d);
            }
#endif
        }
#endif
#if defined(BN_MUL_COMBA) || defined(BN_RECURSION_MUL)
end:
#endif

    r->neg=neg;
    bn_fix_top(rr);
    if (r != rr) (void)BN_copy(r,rr);
    return(1);
    }
#endif


#ifdef SPLIT_BN_MUL_NORMAL
void bn_mul_normal(r,a,na,b,nb)
BN_ULONG *r,*a;
int na;
BN_ULONG *b;
int nb;
    {
    BN_ULONG *rr;

#ifdef BN_COUNT
printf(" bn_mul_normal %d * %d\n",na,nb);
#endif
    /* asymetric and >= 4 */ 
#if 0
    if ((na == nb) && (na == 8))
        {
        bn_mul_normal(r,a,na,b,nb);
        return;
        }
#endif

    if (na < nb)
        {
        int itmp;
        BN_ULONG *ltmp;

        itmp=na; na=nb; nb=itmp;
        ltmp=a;   a=b;   b=ltmp;

        }
    rr= &(r[na]);
    rr[0]=bn_mul_words(r,a,na,b[0]);

    for (;;)
        {
        if (--nb <= 0) return;
        rr[1]=bn_mul_add_words(&(r[1]),a,na,b[1]);
        if (--nb <= 0) return;
        rr[2]=bn_mul_add_words(&(r[2]),a,na,b[2]);
        if (--nb <= 0) return;
        rr[3]=bn_mul_add_words(&(r[3]),a,na,b[3]);
        if (--nb <= 0) return;
        rr[4]=bn_mul_add_words(&(r[4]),a,na,b[4]);
        rr+=4;
        r+=4;
        b+=4;
        }
    }
#endif

#ifdef SPLIT_BN_MUL_LOW_NORMAL
void bn_mul_low_normal(r,a,b,n)
BN_ULONG *r,*a,*b;
int n;
    {
#ifdef BN_COUNT
printf(" bn_mul_low_normal %d * %d\n",n,n);
#endif
    (void)bn_mul_words(r,a,n,b[0]);

    for (;;)
        {
        if (--n <= 0) return;
        (void)bn_mul_add_words(&(r[1]),a,n,b[1]);
        if (--n <= 0) return;
        (void)bn_mul_add_words(&(r[2]),a,n,b[2]);
        if (--n <= 0) return;
        (void)bn_mul_add_words(&(r[3]),a,n,b[3]);
        if (--n <= 0) return;
        (void)bn_mul_add_words(&(r[4]),a,n,b[4]);
        r+=4;
        b+=4;
        }
    }
#endif
#endif
