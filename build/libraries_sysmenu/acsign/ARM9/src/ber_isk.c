/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makerom.TWL
  File:     ber_isk.c

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

#include "ber_lcl.h"

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))
 
#ifdef NO_SPLIT
#define SPLIT_BER_ITEMS_SK_INIT
#define SPLIT_BER_ITEMS_SK_GET
#define SPLIT_BER_ITEMS_APPEND
#define SPLIT_BER_ITEMS_UNDER
#define SPLIT_BER_ITEMS_SK_GROW
#define SPLIT_BER_ITEMS_SK_FREE
#endif /* NO_SPLIT */

/* BER_ITEMS_SK_EXPAND_COUNT - number of items to expand to given that
 *                   there are "X" items currently.
 *
 * note: each item typically costs 28 bytes so 20 is around .5k which 
 *       is a reasonable growth size
 */
#define BER_ITEMS_SK_EXPAND_COUNT(X)    ((X)+20)

#ifdef SPLIT_BER_ITEMS_SK_INIT
/**
 * Initialises the stack of BER items. The array of items, the current number
 * of items and the maximum number of items allocated may be set.
 *
 * @param   sk     [In]  A stack of BER items.
 * @param   items  [In]  An array of BER items.
 * @param   num    [In]  The number of BER items in the array.
 * @param   max    [In]  The number of BER items allocated in the array.
 */
void BER_ITEMS_SK_init(BER_ITEMS_SK *sk, BER_ITEMS *items, unsigned int num,
    unsigned int max)
{
    sk->num=num;
    sk->max=max;
    if (items == NULL)
    {
        sk->flags=BER_FLAG_DYNAMIC_ITEMS;
    }
    else
    {
        sk->flags=0;
    }
    sk->items=items;

#ifndef NO_STREAM
    sk->state=BER_STATE_READ_NEXT_ITEM;
    sk->inf=0;
    sk->pinf=0;
    sk->con=0;
    sk->down=1;
    sk->this_idx=-1;
    sk->next=NULL;
#endif /* !NO_STREAM */
}
#endif /* SPLIT_BER_ITEMS_SK_INIT */

#ifdef SPLIT_BER_ITEMS_SK_GET
/**
 * Obtains the next available BER item in the stack.
 *
 * @param  sk    [In]  Stack of BER items.
 * @param  index [Out] Index into array of new item.
 * @return             Indication of successful completion of function.
 *                     Values are:
 *                     <li>0 indicates the function succeeded.</li>
 *                     <li>>0 indicates the function failed.</li>
 *
 * @note   If there is no item available in the array then the array is grown.
 *         This means that an existing external pointer to an item may not be
 *         valid after a call to this function.
 */
int BER_ITEMS_SK_get(BER_ITEMS_SK *sk, int *index)
{
    int ret;        /* return value */
    BER_ITEMS *it;  /* next item */

    /* Ensure there are enough spare items. */
    if (sk->num >= sk->max)
    {
        /* Reallocate the array. */
        ret = BER_ITEMS_SK_grow(sk, BER_ITEMS_SK_EXPAND_COUNT(sk->num));
        if (ret != 0)
        {
            return(ret);
        }
    }

    /* Set the new item index for return. */
    *index = (int)sk->num;

    /* Initialize the new item. */
    it = &(sk->items[sk->num++]);
    Memset((char *)it, 0, sizeof(BER_ITEMS));

    /* return success */
    return(0);
}

#endif /* SPLIT_BER_ITEMS_SK_GET */

#ifdef SPLIT_BER_ITEMS_APPEND
/**
 * Places the BER item <i>b</i> next to the BER item <i>a</i> in the tree
 * structure.
 *
 * @param   a  [In]  A BER item.
 * @param   b  [In]  A BER item.
 */
void BER_ITEMS_append(a,b)
BER_ITEMS *a,*b;
{
    b->parent=a->parent;
    if (b->parent != NULL)
    {
        b->parent->item.flags|=BER_FLAG_INVALID_LENGTH;
    }
    b->next=a->next;
    a->next=b;
}
#endif /* SPLIT_BER_ITEMS_APPEND */

#ifdef SPLIT_BER_ITEMS_UNDER
/**
 * Places the BER item <i>b</i> below the BER item <i>a</i> in the tree
 * structure.
 *
 * @param   a  [In]  A BER item.
 * @param   b  [In]  A BER item.
 *
 * @return  An indication of success:<br>
 *          <li>0 indicates success</li>
 *          <li>>0 indicates failure</li>
 */
int BER_ITEMS_under(BER_ITEMS *a, BER_ITEMS *b)
{
    if (!(BER_constructed(&a->item) || BER_hide_contents(&a->item)))
    {
        return(BER_ERR_NOT_CONSTRUCTED);
    }
    b->parent=a;
    b->next=a->down;
    a->down=b;

    a->item.flags|=BER_FLAG_INVALID_LENGTH;
    return(0);
}
#endif /* SPLIT_BER_ITEMS_UNDER */

#ifdef SPLIT_BER_ITEMS_SK_GROW
/**
 * Grows the number of allocated BER items.
 *
 * @param   sk   [In]  A stack of BER items.
 * @param   num  [In]  The number of BER items to allocate.
 *
 * @return  An indication of success:<br>
 *          <li>0 indicates success</li>
 *          <li>>0 indicates failure</li>
 */
int BER_ITEMS_SK_grow(BER_ITEMS_SK *sk, unsigned int num)
{
    int ptr_fix;
    unsigned int i;
    unsigned int j;
    BER_ITEMS *nai;
    BER_ITEMS *oai;
    BER_ITEMS *oend;

    if (num <= sk->num)
    {
        return(0);
    }

    if (sk->flags & BER_FLAG_DYNAMIC_ITEMS)
    {
        if (sk->items == NULL)
        {
            ptr_fix=0;
            nai=(BER_ITEMS *)Malloc(sizeof(BER_ITEMS)*num);
            if (nai == NULL)
            {
                return(BER_ERR_OUT_OF_MEMORY);
            }
            Memset(nai,0,sizeof(BER_ITEMS)*num);
        }
        else
        {
            ptr_fix=1;
            nai=(BER_ITEMS *)Realloc(sk->items, sizeof(BER_ITEMS)*num,
                sizeof(BER_ITEMS)*sk->max);
            if (nai == NULL)
            {
                return(BER_ERR_OUT_OF_MEMORY);
            }
            Memset(&(nai[sk->max]),0, sizeof(BER_ITEMS)*(num-sk->max));
        }
        if (nai == NULL)
        {
            return(BER_ERR_OUT_OF_MEMORY);
        }

        /* If we have realloced, and the Memory has moved, we
         * need to fix the pointers */
        if ((sk->items != nai) && ptr_fix)
        {
            oai=sk->items;
            oend= &(sk->items[sk->num]);
            for (i=0; i<sk->num; i++)
            {
                if ((nai[i].parent >= oai) &&
                    (nai[i].parent <= oend))
                {
                    j=(unsigned int)( nai[i].parent-oai );
                    nai[i].parent= &(nai[j]);
                }
                if ((nai[i].next >= oai) &&
                    (nai[i].next <= oend))
                {
                    j=(unsigned int)( nai[i].next-oai );
                    nai[i].next= &(nai[j]);
                }
                if ((nai[i].down >= oai) &&
                    (nai[i].down <= oend))
                {
                    j=(unsigned int)( nai[i].down-oai );
                    nai[i].down= &(nai[j]);
                }
            }
        }

        sk->max=num;
        sk->items=nai;
        return(0);
    }

    return(BER_ERR_OUT_OF_ITEMS_STORAGE);
}
#endif /* SPLIT_BER_ITEMS_SK_GROW */

#ifdef SPLIT_BER_ITEMS_SK_FREE
void BER_ITEMS_SK_free(sk)
BER_ITEMS_SK *sk;
    {
    unsigned int i;

    for (i=0; i<sk->max; i++)
        {
        if ((sk->items[i].item.data.bytes != NULL) &&
            (sk->items[i].item.flags & BER_FLAG_DYNAMIC))
            {
            Free(sk->items[i].item.data.bytes);
            sk->items[i].item.data.bytes=NULL;
            }
        }
    if ((sk->flags & BER_FLAG_DYNAMIC_ITEMS) && (sk->items != NULL))
        {
        Free(sk->items);
        sk->items=NULL;
        }
    sk->num=0;
    if (sk->flags & BER_FLAG_DYNAMIC)
        Free(sk);
    }
#endif /* SPLIT_BER_ITEMS_SK_FREE */

#endif /* !(defined(NO_SPLIT) && defined(SPLIT_FILE)) */
