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

/* rp has w words, the top w words are 0 */
void bn_from_montgomery_words(ret,a,np,w,n0)
BN_ULONG *ret;
BN_ULONG *a;
BN_ULONG *np;
int w;
BN_ULONG n0;
    {
    BN_ULONG v0,v1,*ap,*wap;
    int i;

    v1=0;

    ap=a;
    wap= &(a[w]);
    /* Consider putting this loop in ASM */
    for (i=0; i<w; i++)
        {
        v0=bn_mul_add_words(ap,np,w,((*ap)*n0)&BN_MASK2);
        v0=(v1+v0)&BN_MASK2;
        v1= (BN_ULONG)( (v0 < v1)?1:0 );
        if (((*wap= *wap+v0)&BN_MASK2) < v0) v1++;
        ap++; wap++;
        }

    /* ap is now wap (&a[w]) */
    i=w-1;
    if (v1 == 0)
        {
        if (ap[i] == np[i]) /* GTCHECK */
            {
            /* i=w-1 */
            for (; i>0; i--)
                if (ap[i] != np[i]) break;
            }
        v1=(ap[i] >= np[i]);
        }
    if (v1)
        (void)bn_sub_words(ret,ap,np,w);
    else
        {
#if 0 /* Alpha does not like Memcpy */
        Memcpy(ret,ap,sizeof(BN_ULONG)*w);
#else
        for (i=0; i<w; i++)
            ret[i]=ap[i];
#endif
        }
    }


