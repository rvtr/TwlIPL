/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makerom.TWL
  File:     ber_par.c

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

/* We need to 'parse' the input data into an BER_ITEMS_SK, which is composed of
 * BER_ITEMS.
 * For definite length encoding, we read a full 'level' at a time, and then
 * process the sub-elements.
 * For indefinite encoding, we go as 'deep' as we can, updating the
 * length info of the indefinite encodings as we go.
 * This is easy to do because when we go back up to our parent,
 * just compare it's start data byte with the current pointer.
 */

#define READ_NEXT_ITEM        1
#define GO_UP                 2

int BER_parse(BER_ITEMS_SK *sk, unsigned char *ino, unsigned long m_len,
    unsigned long *n_used)
{
    int this_idx;
    int next_idx;
    BER_ITEMS *this = NULL;
    BER_ITEMS *next = NULL;
    int ret=999;
    int state;
    unsigned char *in = ino;
    unsigned char *max = in + m_len;
    unsigned char *end = max;
    int inf = 0;
    int pinf = 0;
    int con = 0;
    int down;

    state = READ_NEXT_ITEM;
    down = 1;
    this_idx = -1;
    pinf = 0;
    for (;;)
    {
        switch (state)
        {
        case READ_NEXT_ITEM:
            /* Read the item into 'next' */
            if ((ret=BER_ITEMS_SK_get(sk,&next_idx)) != 0)
            {
                goto err;
            }
            next=BER_ITEMS_SK_items(sk,next_idx);
            ret = BER_read_item(&(next->item), in, (unsigned long)(end - in));
            if (ret != 0)
            {
                goto err;
            }
            /* Check the length of the data against what is available */
            if (next->item.data.len > (unsigned long)(end - in))
            {
                ret = BER_ERR_NOT_ENOUGH_BYTES;
                goto err;
            }
            inf = BER_indefinite_encoding(&(next->item));
            con = BER_constructed(&(next->item));
            in += next->item.hlen;

            if (this_idx == -1)
            {
                this = NULL;
            }
            else
            {
                this = BER_ITEMS_SK_items(sk, this_idx);
            }

            /* If we are pushing it under, do so */
            if (down)
            {
                next->parent = this;
                if (this != NULL)
                {
                    this->down = next;
                    if (!pinf)
                    {
                        end = this->item.data.bytes+ this->item.data.len;
                    }
                    pinf = BER_indefinite_encoding(&this->item);
                }
                else
                {
                    pinf = 0;
                }
                next->next = NULL;
            }
            else
            {
                next->parent = this->parent;
                next->next = NULL;
                this->next = next;
            }
#ifndef NO_STREAM
            next->item.seen = next->item.data.len;
            next->item.part_len = next->item.data.len;
#endif /* !NO_STREAM */
            down = 0;
            this = next;
            this_idx = next_idx;
            if ((this->item.type == BER_EOC) && (this->item.data.len == 0) &&
                (BER_class(&(this->item)) == BER_UNIVERSAL))
            {
                if (pinf)
                {
                    this->parent->item.data.len=(unsigned long)
                        (in - this->parent->item.data.bytes);
#ifndef NO_STREAM
                    this->parent->item.seen = this->parent->item.data.len;
                    this->parent->item.part_len = this->parent->item.data.len;
#endif /* !NO_STREAM */
                    state = GO_UP;
                    break;
                }
                else
                {
                    ret = BER_ERR_UNEXPECTED_EOC;
                    goto err;
                }
            }

            if (con && (inf || (this->item.data.len > 0)))
            {
                pinf = inf;
                down = 1;
            }
            else
            {
                in += this->item.data.len;
            }

            if (in > end)
            {
                ret = BER_ERR_NOT_ENOUGH_BYTES;
                goto err;
            }

            if (in == end)
            {
                state = GO_UP;
            }
            else
            {
                state = READ_NEXT_ITEM;
            }

            if (inf && !con)
            {
                ret = BER_ERR_INVALID_LENGTH_ENCODING;
                goto err;
            }

            break;
        case GO_UP:
            /* this is valid and is the 'end'.  We need to
             * go back up one and then see if we contiune or
             * go_up again. */
            this_idx +=(int)(this->parent-this);
            this = this->parent;
            down = 0;
            if (this == NULL)
            {
                ret = 0;
                goto end; /* we have finished */
            }

            /* Must have finished this one */
            /* inf = BER_indefinite_encoding(&this->item); */

            if (this->parent != NULL)
            {
                pinf = BER_indefinite_encoding(&this->parent->item);
                if (pinf)
                {
                    end = max;
                }
                else
                {
                    end = this->parent->item.data.bytes +
                        this->parent->item.data.len;
                }
            }
            else
            {
                if (BER_indefinite_encoding(&this->item))
                {
                    ret = 0;
                    goto end;
                }
                pinf = 0;
                end = max;
            }
            
            if (end < in)
            {
                ret = BER_ERR_NOT_ENOUGH_BYTES;
                goto err;
            }
            if (end == in)
            {
                state = GO_UP;
            }
            else
            {
                state = READ_NEXT_ITEM;
            }
            break;
        default:
            break;
        }
    }
end:
    /* if there were no errors we set the length of data we consumed
     * if the user wants to know that information.
     */
    if (n_used != NULL)
        *n_used = (unsigned long)( in - ino );
err:
    return(ret);
}

