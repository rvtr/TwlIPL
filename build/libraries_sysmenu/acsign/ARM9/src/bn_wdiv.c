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

#ifndef OPT_BN_ASM

#ifndef BN_DIV_WORDS
#define BN_DIV_WORDS
#endif

#endif

#ifdef BN_DIV_WORDS
#if defined(BN_LLONG)
BN_ULONG bn_div_words(h,l,d)
BN_ULONG h,l,d;
	{
	return((BN_ULONG)(((((BN_ULLONG)h)<<BN_BITS2)|l)/(BN_ULLONG)d));
	}

#else

/* Divide h-l by d and return the result. */
/* I need to test this some more */
BN_ULONG bn_div_words(h,l,d)
BN_ULONG h,l,d;
	{
	BN_ULONG dh,dl,q,ret=0,th,tl,t;
	int i,count=2;

	if (d == 0) return(BN_MASK2);

	i=BN_num_bits_word(d);
	if ((i != BN_BITS2) && (h > ((BN_ULONG)1)<<i))
		{
                return(0);
		}
	i=BN_BITS2-i;
	if (h >= d) h-=d;

	if (i)
		{
		d=(d<<i)&BN_MASK2;
		h=((h<<i)|(l>>(BN_BITS2-i)))&BN_MASK2;
		l=(l<<i)&BN_MASK2;
		}
	dh=(d>>BN_BITS4)&BN_MASK2lh;
	dl=(d&BN_MASK2l);
	for (;;)
		{
		if ((h>>BN_BITS4) == dh)
			q=BN_MASK2l;
		else
			q=h/dh;

		for (;;)
			{
			t=(h-q*dh)&BN_MASK2;
			if ((t&BN_MASK2h) ||
				(((dl*q)&BN_MASK2) <= ((
					(t<<BN_BITS4)+
					((l>>BN_BITS4)&BN_MASK2lh))&BN_MASK2)))
				break;
			q--;
			}
		th=(q*dh)&BN_MASK2;
		tl=(q*dl)&BN_MASK2;
		t=(tl>>BN_BITS4);
		tl=(tl&BN_MASK2lh)<<BN_BITS4;
		th=(th+t)&BN_MASK2;

		if (l < tl) th++;
		l=(l-tl)&BN_MASK2;
		if (h < th)
			{
			h=(h+d)&BN_MASK2;
			q--;
			}
		h=(h-th)&BN_MASK2;

		if (--count == 0) break;

		ret=(q&BN_MASK2lh)<<BN_BITS4;
		h=((h<<BN_BITS4)|(l>>BN_BITS4))&BN_MASK2;
		l=(l&BN_MASK2l)<<BN_BITS4;
		}
	ret|=q;
	return(ret);
	}
#endif
#endif /* BN_DIV_WORDS */
