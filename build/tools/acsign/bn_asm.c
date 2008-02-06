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

#ifndef OPT_BN_ASM

#define BN_MUL_ADD_WORDS

#ifndef BN_MUL_WORDS
#define BN_MUL_WORDS
#endif

#ifndef BN_SQR_WORDS
#define BN_SQR_WORDS
#endif

#ifndef BN_ADD_WORDS
#define BN_ADD_WORDS
#endif

#ifndef BN_SUB_WORDS
#define BN_SUB_WORDS
#endif

#endif


#include "bn_lcl.h"

#ifdef BN_LLONG 

/*
 * bn_mul_add_words
 *    
 *   for(i=0;i<num;i++)
 *     rp[i]+=ap[i]*w (with carry propagation)
 * 
 */

#ifdef BN_MUL_ADD_WORDS

#if 0
BN_ULONG bn_mul_add_words(rp,ap,num,w)
BN_ULONG *rp,*ap;
int num;
BN_ULONG w;
	{
	BN_ULONG c1=0;

	bn_check_num(num);
	if (num <= 0) return(c1);

	for (;;)
		{
		mul_add(rp[0],ap[0],w,c1);
		if (--num == 0) break;
		ap++;
		rp++;
		}
	
	return(c1);
	} 
#endif

BN_ULONG bn_mul_add_words(rp,ap,num,w)
BN_ULONG *rp,*ap;
int num;
BN_ULONG w;
	{
	BN_ULONG c1=0;

	bn_check_num(num);
	if (num <= 0) return(c1);

#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)
	{
	int i,nn=num;

	for (i=num-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,ap[i]);
	fprintf(stderr,"*");
	fprintf(stderr,BN_HEX_FMT,w);
	fprintf(stderr,"+");
	for (i=num-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,rp[i]);
#endif

	for (;;)
		{
		mul_add(rp[0],ap[0],w,c1);
		if (--num == 0) break;
		mul_add(rp[1],ap[1],w,c1);
		if (--num == 0) break;
		mul_add(rp[2],ap[2],w,c1);
		if (--num == 0) break;
		mul_add(rp[3],ap[3],w,c1);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
		}
	
#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)
	fprintf(stderr,"-");
	fprintf(stderr,BN_HEX_FMT,c1);
	for (i=nn-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,rp[i]);
	fprintf(stderr,"\n");
	}
#endif
	return(c1);
	} 
#endif

#ifdef BN_MUL_WORDS
BN_ULONG bn_mul_words(rp,ap,num,w)
BN_ULONG *rp,*ap;
int num;
BN_ULONG w;
	{
	BN_ULONG c1=0;

	bn_check_num(num);
	if (num <= 0) return(c1);

#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)
	{
	int i,nn=num;

	for (i=num-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,ap[i]);
	fprintf(stderr,"*");
	fprintf(stderr,BN_HEX_FMT,w);
#endif

	for (;;)
		{
		mul(rp[0],ap[0],w,c1);
		if (--num == 0) break;
		mul(rp[1],ap[1],w,c1);
		if (--num == 0) break;
		mul(rp[2],ap[2],w,c1);
		if (--num == 0) break;
		mul(rp[3],ap[3],w,c1);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
		}

#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)
	fprintf(stderr,"-");
	fprintf(stderr,BN_HEX_FMT,c1);
	for (i=nn-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,rp[i]);
	fprintf(stderr,"\n");
	}
#endif
	return(c1);
	} 
#endif

#ifdef BN_SQR_WORDS
void bn_sqr_words(r,a,n)
BN_ULONG *r,*a;
int n;
        {
	bn_check_num(n);
	if (n <= 0) return;

#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)
	{
	int i,nn=n;

	for (i=n-1; i>=0; i--)
		{
		fprintf(stderr,BN_HEX_FMT,a[i]);
		if (i != 0)
			fprintf(stderr,"^2*2^%X+",i*2*BN_BITS2);
		else
			fprintf(stderr,"^2");
		}
#endif

	for (;;)
		{
		BN_ULLONG t;

		t=(BN_ULLONG)(a[0])*(a[0]);
		r[0]=Lw(t); r[1]=Hw(t);
		if (--n == 0) break;

		t=(BN_ULLONG)(a[1])*(a[1]);
		r[2]=Lw(t); r[3]=Hw(t);
		if (--n == 0) break;

		t=(BN_ULLONG)(a[2])*(a[2]);
		r[4]=Lw(t); r[5]=Hw(t);
		if (--n == 0) break;

		t=(BN_ULLONG)(a[3])*(a[3]);
		r[6]=Lw(t); r[7]=Hw(t);
		if (--n == 0) break;

		a+=4;
		r+=8;
		}
#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)

	fprintf(stderr,"-");
	for (i=nn+nn-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,r[i]);
	fprintf(stderr,"\n");
	}
#endif
	}
#endif

#else

#ifdef BN_MUL_ADD_WORDS

BN_ULONG bn_mul_add_words(rp,ap,num,w)
BN_ULONG *rp,*ap;
int num;
BN_ULONG w;
	{
	BN_ULONG c=0;
	BN_ULONG bl,bh,b_hl;

	bn_check_num(num);
	if (num <= 0) return((BN_ULONG)0);

	bl=LBITS(w);
	bh=HBITS(w);
	b_hl=bh-bl;
/*
{ int i,nnum=num;
BN_ULONG *rrp=rp;
for (i=num-1; i>=0; i--) printf("%02X",rp[i]);
printf("+");
for (i=num-1; i>=0; i--) printf("%02X",ap[i]);
printf("*%02X - ",w);
*/
	for (;;)
		{
		mul_add(rp[0],ap[0],bl,bh,b_hl,c);
		if (--num == 0) break;
		mul_add(rp[1],ap[1],bl,bh,b_hl,c);
		if (--num == 0) break;
		mul_add(rp[2],ap[2],bl,bh,b_hl,c);
		if (--num == 0) break;
		mul_add(rp[3],ap[3],bl,bh,b_hl,c);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
		}

/*
printf("%02X",c);
for (i=nnum-1; i>=0; i--) printf("%02X",rrp[i]);
printf("\n");
}
*/
	return(c);
	} 

#endif

#ifdef BN_MUL_WORDS
BN_ULONG bn_mul_words(rp,ap,num,w)
BN_ULONG *rp,*ap;
int num;
BN_ULONG w;
	{
	BN_ULONG carry=0;
	BN_ULONG bl,bh;

	bn_check_num(num);
	if (num <= 0) return((BN_ULONG)0);

	bl=LBITS(w);
	bh=HBITS(w);

	for (;;)
		{
		mul(rp[0],ap[0],bl,bh,carry);
		if (--num == 0) break;
		mul(rp[1],ap[1],bl,bh,carry);
		if (--num == 0) break;
		mul(rp[2],ap[2],bl,bh,carry);
		if (--num == 0) break;
		mul(rp[3],ap[3],bl,bh,carry);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
		}
	return(carry);
	} 
#endif

#ifdef BN_SQR_WORDS
void bn_sqr_words(r,a,n)
BN_ULONG *r,*a;
int n;
        {
	bn_check_num(n);
	if (n <= 0) return;
	for (;;)
		{
		sqr64(r[0],r[1],a[0]);
		if (--n == 0) break;

		sqr64(r[2],r[3],a[1]);
		if (--n == 0) break;

		sqr64(r[4],r[5],a[2]);
		if (--n == 0) break;

		sqr64(r[6],r[7],a[3]);
		if (--n == 0) break;

		a+=4;
		r+=8;
		}
	}
#endif

#endif

#ifdef BN_ADD_WORDS
#ifdef BN_LLONG
BN_ULONG bn_add_words(r,a,b,n)
BN_ULONG *r,*a,*b;
int n;
        {
	BN_ULLONG ll=0;

	bn_check_num(n);
	if (n <= 0) return((BN_ULONG)0);

#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)
	{
	int i,nn=n;

	for (i=n-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,a[i]);
	fprintf(stderr,"+");
	for (i=n-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,b[i]);
#endif

	for (;;)
		{
		ll+=(BN_ULLONG)a[0]+b[0];
		r[0]=(BN_ULONG)ll&BN_MASK2;
		ll>>=BN_BITS2;
		if (--n <= 0) break;

		ll+=(BN_ULLONG)a[1]+b[1];
		r[1]=(BN_ULONG)ll&BN_MASK2;
		ll>>=BN_BITS2;
		if (--n <= 0) break;

		ll+=(BN_ULLONG)a[2]+b[2];
		r[2]=(BN_ULONG)ll&BN_MASK2;
		ll>>=BN_BITS2;
		if (--n <= 0) break;

		ll+=(BN_ULLONG)a[3]+b[3];
		r[3]=(BN_ULONG)ll&BN_MASK2;
		ll>>=BN_BITS2;
		if (--n <= 0) break;

		a+=4;
		b+=4;
		r+=4;
		}
#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)
	fprintf(stderr,"-");
	fprintf(stderr,BN_HEX_FMT,(BN_ULONG)ll);
	for (i=nn-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,r[i]);
	fprintf(stderr,"\n");
	}
#endif
	return((BN_ULONG)ll);
	}
#else
BN_ULONG bn_add_words(r,a,b,n)
BN_ULONG *r,*a,*b;
int n;
        {
	BN_ULONG c,l,t;

	bn_check_num(n);
	if (n <= 0) return((BN_ULONG)0);

	c=0;
	for (;;)
		{
		t=a[0];
		t=(t+c)&BN_MASK2;
		c=(t < c);
		l=(t+b[0])&BN_MASK2;
		c+=(l < t);
		r[0]=l;
		if (--n <= 0) break;

		t=a[1];
		t=(t+c)&BN_MASK2;
		c=(t < c);
		l=(t+b[1])&BN_MASK2;
		c+=(l < t);
		r[1]=l;
		if (--n <= 0) break;

		t=a[2];
		t=(t+c)&BN_MASK2;
		c=(t < c);
		l=(t+b[2])&BN_MASK2;
		c+=(l < t);
		r[2]=l;
		if (--n <= 0) break;

		t=a[3];
		t=(t+c)&BN_MASK2;
		c=(t < c);
		l=(t+b[3])&BN_MASK2;
		c+=(l < t);
		r[3]=l;
		if (--n <= 0) break;

		a+=4;
		b+=4;
		r+=4;
		}

	return((BN_ULONG)c);
	}
#endif
#endif

#ifdef BN_SUB_WORDS
BN_ULONG bn_sub_words(r,a,b,n)
BN_ULONG *r,*a,*b;
int n;
        {
	BN_ULONG c,t1,t2;

	bn_check_num(n);
	if (n <= 0) return((BN_ULONG)0);

#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)
	{
	int i,nn=n;

	for (i=n-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,a[i]);
	fprintf(stderr,"-");
	for (i=n-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,b[i]);
#endif

	c=0;
	for (;;)
		{
		t1=a[0]; t2=b[0];
		r[0]=(t1-t2-c)&BN_MASK2;
		if (t1 != t2) c=(t1 < t2);
		if (--n <= 0) break;

		t1=a[1]; t2=b[1];
		r[1]=(t1-t2-c)&BN_MASK2;
		if (t1 != t2) c=(t1 < t2);
		if (--n <= 0) break;

		t1=a[2]; t2=b[2];
		r[2]=(t1-t2-c)&BN_MASK2;
		if (t1 != t2) c=(t1 < t2);
		if (--n <= 0) break;

		t1=a[3]; t2=b[3];
		r[3]=(t1-t2-c)&BN_MASK2;
		if (t1 != t2) c=(t1 < t2);
		if (--n <= 0) break;

		a+=4;
		b+=4;
		r+=4;
		}

#if defined(BN_ASM_DEBUG) && !defined(NO_FP_API)
	fprintf(stderr,"- ");
	if (c) fprintf(stderr,"-");
	for (i=nn-1; i>=0; i--)
		fprintf(stderr,BN_HEX_FMT,r[i]);
	fprintf(stderr,"\n");
	}
#endif
	return(c);
	}
#endif

