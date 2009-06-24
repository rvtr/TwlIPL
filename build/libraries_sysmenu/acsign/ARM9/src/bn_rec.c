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

#if !(defined(NO_SPLIT) &&  defined(SPLIT_FILE))
#ifdef NO_SPLIT
#define SPLIT_BN_2s_COMP
#define SPLIT_BN_MUL_REC_WORDS
#define SPLIT_BN_SQR_REC_WORDS
#endif /* NO_SPLIT */

#ifdef SPLIT_BN_2s_COMP
void bn_2s_comp(r,a,n)
BN_ULONG *r,*a;
int n;
	{
	BN_ULONG l;
	int i;

	for (i=0; i<n; i++)
		{
		l=a[i];
		l= ~l+1;
		r[i]=l;
		if (l != 0) break;
		}
#if 1
	for (i++; i<n; i++)
		{
		r[i]= ~a[i];
		}
#else
	for (i+=4; i<n; i+=4)
		{
		r[i-3]= ~a[i-3];
		r[i-2]= ~a[i-2];
		r[i-1]= ~a[i-1];
		r[i  ]= ~a[i  ];
		}
	for (i-=3; i<n; i++)
		{
		r[i]= ~a[i];
		}
#endif
	}
#endif

#ifdef SPLIT_BN_MUL_REC_WORDS
/* t needs to be 2*n words */
void bn_mul_rec_words(r,a,b,tt,rec)
BN_ULONG *r,*a,*b,*tt;
BN_REC *rec;
	{
	int n=rec->n;
	int n2=n/2;
	int neg=0;
	BN_ULONG c1;


	if (rec->depth == 0)
		{
		/* t needs to have space for 4*n words
		 * The multiply needs to be a n2*n2 -> n
		 */
#ifndef NOPROTO
		void (PRE_CCONV CCONV *rmul)(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b);
#else
		void (PRE_CCONV CCONV *rmul)();
#endif
		rmul=rec->mul;

#ifdef _EXTRA_ARG_
                /* THIS IS WRONG - there is no extra int arg on the end */
		rmul(&(r[0]),&(a[0]),&(b[0]),n2);
		rmul(&(r[n]),&(a[n2]),&(b[n2]),n2);
#else
		rmul(&(r[0]),&(a[0]),&(b[0]));
		rmul(&(r[n]),&(a[n2]),&(b[n2]));
#endif

		if (bn_sub_words(&(tt[n+0]), &(a[0]),&(a[n2]),n2))
			{
			bn_2s_comp(&(tt[n+ 0]),&(tt[n+ 0]),n2);
			neg=1;
			}
		else
			neg=0;
		if (bn_sub_words(&(tt[n+n2]),&(b[n2]),&(b[0]),n2))
			{
			neg^=1;
			bn_2s_comp(&(tt[n+n2]),&(tt[n+n2]),n2);
			}

#ifdef _EXTRA_ARG_
		rmul(&(tt[0]),&(tt[n+0]),&(tt[n+n2]),n2);
#else
		rmul(&(tt[0]),&(tt[n+0]),&(tt[n+n2]));
#endif
		}
	else	/* Leaf node */
		{
		/* t has 4*n words taken per call.
		 * Since n is half the size per call, we need
		 * 6*n words for our current level and all sub levels
		 * The multiply needs to be a n2*n2 -> n
		 */
		rec->depth--;
		rec->n=n2;
		bn_mul_rec_words(&(r[0]), &(a[0]), &(b[0]),    &(tt[n+n]),rec);
		bn_mul_rec_words(&(r[n]), &(a[n2]),&(b[n2]),   &(tt[n+n]),rec);

		neg=0;
		if (bn_sub_words(&(tt[n+0]), &(a[0]),&(a[n2]),n2))
			{
			bn_2s_comp(&(tt[n+ 0]),&(tt[n+ 0]),n2);
			neg=1;
			}
		else
			neg=0;
		if (bn_sub_words(&(tt[n+n2]),&(b[n2]),&(b[0]),n2))
			{
			neg^=1;
			bn_2s_comp(&(tt[n+n2]),&(tt[n+n2]),n2);
			}

		bn_mul_rec_words(&(tt[0]),&(tt[n]),&(tt[n+n2]),&(tt[n+n]),rec);
		rec->n=n;
		rec->depth++;
		}

	c1=bn_add_words(&(tt[n]),&(r[0]),&(r[n]),n);

	if (neg)
		c1-=bn_sub_words(&(tt[0]),&(tt[n]),&(tt[0]),n);
	else
		c1+=bn_add_words(&(tt[0]),&(tt[n]),&(tt[0]),n);
	tt[n]=c1;
	c1=bn_add_words(&(r[n2]),&(r[n2]),&(tt[0]),n+1); 
	if (c1)
		{
		tt= &(r[n+n2+1]);
		do	{
			} while (++(*(tt++)) == 0);
		}
	}
#endif

#ifdef SPLIT_BN_SQR_REC_WORDS
/* tt needs to be 3*n words */
void bn_sqr_rec_words(r,a,tt,rec)
BN_ULONG *r,*a,*tt;
BN_REC *rec;
	{
	int n=rec->n;
	int n2=n/2;
	BN_ULONG c1;

	if (rec->depth == 0)
		{
		/* t needs to have space for 4*n words
		 * The multiply needs to be a n2*n2 -> n
		 */
#ifndef NOPROTO
		void (PRE_CCONV CCONV *rsqr)(BN_ULONG *r, BN_ULONG *a);
#else
		void (PRE_CCONV CCONV *rsqr)();
#endif

		rsqr=rec->sqr;

#ifdef _EXTRA_ARG_
                /* THIS IS WRONG - there is no extra int arg on the end */
		rsqr(&(r[0]), &(a[0]), n2);
		rsqr(&(r[n]), &(a[n2]),n2);
#else
		rsqr(&(r[0]), &(a[0]));
		rsqr(&(r[n]), &(a[n2]));
#endif

		if (bn_sub_words(&(tt[n+0]), &(a[0]),&(a[n2]),n2))
			bn_2s_comp(&(tt[n+0]),&tt[n+0], n2);

#ifdef _EXTRA_ARG_
		rsqr(&(tt[0]),&(tt[n]), n2);
#else
		rsqr(&(tt[0]),&(tt[n]));
#endif

		}
	else	/* Leaf node */
		{
		/* t has 4*n words taken per call.
		 * Since n is half the size per call, we need
		 * 6*n words for our current level and all sub levels
		 * The multiply needs to be a n2*n2 -> n
		 */
		rec->depth--;
		rec->n=n2;
		bn_sqr_rec_words(&(r[0]), &(a[0]), &(tt[n+n]),rec);
		bn_sqr_rec_words(&(r[n]), &(a[n2]),&(tt[n+n]),rec);

		if (bn_sub_words(&(tt[n+0]), &(a[0]),&(a[n2]),n2))
			bn_2s_comp(&(tt[n+0]),&tt[n+0], n2);
		bn_sqr_rec_words(&(tt[0]),&(tt[n]),&(tt[n+n]),rec);

		rec->n=n;
		rec->depth++;
		}

	c1=bn_add_words(&(tt[n]),&(r[0]),&(r[n]),n);
	c1-=bn_sub_words(&(tt[0]),&(tt[n]),&(tt[0]),n);

	tt[n]=c1;
	c1=bn_add_words(&(r[n2]),&(r[n2]),&(tt[0]),n+1); 
	if (c1)
		{
		tt= &(r[n+n2+1]);
		do	{
			} while (++(*(tt++)) == 0);
		}
	}
#endif

#endif /* !NO_SPLIT || !SPLIT_FILE */
