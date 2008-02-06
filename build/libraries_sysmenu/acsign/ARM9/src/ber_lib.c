/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makerom.TWL
  File:     ber_lib.c

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
#define SPLIT_BER_NUM_BITS
#define SPLIT_BER_ITEM_CMP_BYTES
#define SPLIT_BER_ITEM_CMP_TAG
#define SPLIT_BER_ITEM_INIT
#define SPLIT_BER_ITEMS_INIT
#define SPLIT_BER_ITEM_SET_ALL
#define SPLIT_BER_ITEM_SET_HEADER
#define SPLIT_BER_ITEM_SET_PREFIX_BYTE
#define SPLIT_BER_ITEM_SET_DATA
#define SPLIT_BER_READ_ITEM
#endif /* NO_SPLIT */

#ifdef SPLIT_BER_NUM_BITS
/**
 * Calculates the number of used bits in a buffer bytes. This function is
 * useful when dealing with a BER item that is a bit string.
 *
 * @param   buf       [In]  The buffer of data.
 * @param   len       [In]  The length of the buffer in bytes.
 * @param   bigendian [In]  Indicates that the bits in the data is order in
 *                          big endian order.
 *
 * @return  The number of bits.
 */
int R_num_bits(unsigned char *buf, int len, int bigendian)
{
    int ret;
    int i;
    int j;
    int inc;
    int c;

    if (len == 0)
    {
        return(0);
    }
    if (bigendian)
    {
        inc = 1;
        j = 0;
    }
    else
    {
        j = len - 1;
        inc = -1;
    }

    ret = (len-1)*8;
    for (i = 0; i < len; i++)
    {
        c = buf[j];
        if (c == 0)
        {
            ret -= 8;
            j += inc;
        }
        else
        {
            if (c & 0x80)
            {
                return(ret+8);
            }
            if (c & 0x40)
            {
                return(ret+7);
            }
            if (c & 0x20)
            {
                return(ret+6);
            }
            if (c & 0x10)
            {
                return(ret+5);
            }
            if (c & 0x08)
            {
                return(ret+4);
            }
            if (c & 0x04)
            {
                return(ret+3);
            }
            if (c & 0x02)
            {
                return(ret+2);
            }
            /* if (c & 0x01) */
            {
                return(ret+1);
            }
        }
    }
    return(0);
}
#endif /* SPLIT_BER_NUM_BITS */


#ifdef SPLIT_BER_ITEM_CMP_BYTES
/**
 * Compares the data bytes of the BER item with the data of the buffer.
 *
 * @param   a  [In]  A BER item.
 * @param   d  [In]  A buffer of data.
 * @param   l  [In]  The length of the data.
 *
 * @return  Indication of a match:<br>
 *          <li>0 indicates a match</li>
 *          <li>BER_ERR_CMP_BYTES indicates no match</li>
 */
int BER_ITEM_cmp_bytes(BER_ITEM *a, unsigned char *d, unsigned int l)
{
    unsigned char *b;

    if (l != a->data.len)
    {
        return(BER_ERR_CMP_BYTES);
    }

    if (l == 0)
    {
        return(0);
    }

    b = a->data.bytes;
    if (BER_prefix_byte(a))
    {
        if (a->prefix_byte != *(d++))
        {
            return(BER_ERR_CMP_BYTES);
        }

        l--;
    }

    if (Memcmp(d,b,l) != 0)
    {
        return(BER_ERR_CMP_BYTES);
    }

    return(0);
}
#endif /* SPLIT_BER_ITEM_CMP_BYTES */

#ifdef SPLIT_BER_ITEM_CMP_TAG
/**
 * Compares the tag of a BER item with a BER type value for a match.
 *
 * @param   a     [In]  A BER item.
 * @param   type  [In]  A type of BER item.
 *
 * @return  Indication of a match:<br>
 *          <li>0 indicares a match</li>
 *          <li>BER_ERR_CMP_TAG indicates no match</li>
 */
int BER_ITEM_cmp_tag(BER_ITEM *a, unsigned int type)
{
    int i;
    int j;

    if (a->type != type)
    {
        return(BER_ERR_CMP_TAG);
    }

    if ((a->info & BER_PRIVATE) != 0)
    {
        return(BER_ERR_CMP_TAG);
    }

    i = ((a->type == BER_SEQUENCE) || (a->type == BER_SET));
    j = (a->info&BER_CONSTRUCTED) ? 1 : 0;

    return((i == j) ? 0: BER_ERR_CMP_TAG);
}
#endif /* SPLIT_BER_ITEM_CMP_TAG */

#ifdef SPLIT_BER_ITEM_INIT
/**
 * Initializes a BER item.
 *
 * @param   item  [In]  A BER item.
 *
 * @pre     The BER item is not a NULL pointer.
 */
void BER_ITEM_init(BER_ITEM *item)
{
    Memset(item, 0, sizeof(BER_ITEM));
}
#endif /* SPLIT_BER_ITEM_INIT */

#ifdef SPLIT_BER_ITEMS_INIT
/**
 * Initializes a BER items.
 *
 * @param   items  [In]  A BER items.
 *
 * @pre     The BER item is not a NULL pointer.
 */
void BER_ITEMS_init(BER_ITEMS *items)
{
    Memset(items, 0, sizeof(BER_ITEMS));
}
#endif /* SPLIT_BER_ITEMS_INIT */

#ifdef SPLIT_BER_ITEM_SET_ALL
/**
 * Sets the data against a BER item.
 *
 * @param   item    [In]  A BER item.
 * @param   sclass  [In]  The class of a BER item.
 * @param   tag     [In]  The tag of a BER item.
 * @param   data    [In]  The data of a BER item.
 * @param   len     [In]  The length of the data of a BER item.
 * @param   info    [In]  The info of a BER item.
 * @param   flags   [In]  The flags of a BER item.
 */
void BER_ITEM_set_all(BER_ITEM *item, unsigned int sclass, unsigned int tag,
    unsigned char *data, unsigned int len, unsigned int info,
    unsigned int flags)
{
    BER_ITEM_set_header(item, sclass, tag, info);
    BER_ITEM_set_data(item, data, len);
    item->flags |= flags;
}
#endif /* SPLIT_BER_ITEM_SET_ALL */

#ifdef SPLIT_BER_ITEM_SET_HEADER
/**
 * Sets the header data against a BER item.
 *
 * @param   item    [In]  A BER item.
 * @param   sclass  [In]  The class of a BER item.
 * @param   tag     [In]  The tag of a BER item.
 * @param   info    [In]  The info of a BER item.
 */
void BER_ITEM_set_header(BER_ITEM *item, unsigned int sclass, unsigned int tag,
    unsigned int info)
{
    item->info = (unsigned char)( (sclass & BER_PRIVATE) |
                 (info & (BER_INFO_MASK | BER_CONSTRUCTED)) );
    item->flags |= BER_FLAG_INVALID_LENGTH;

    if (((sclass & BER_PRIVATE) == 0) && 
        ((tag == BER_SEQUENCE) || (tag == BER_SET)))
    {
        item->info |= BER_CONSTRUCTED;
    }

    item->type = tag;
}
#endif /* SPLIT_BER_ITEM_SET_HEADER */

#ifdef SPLIT_BER_ITEM_SET_PREFIX_BYTE
/**
 * Places a byte at the front of the data of a BER item. This is useful for
 * handling bit strings that require a bytes to be placed before the actual
 * data.
 *
 * @param   item   [In]  A BER item.
 * @param   abyte  [In]  A byte.
 */
void BER_ITEM_set_prefix_byte(BER_ITEM *item, unsigned int abyte)
{
    if ((item->flags & BER_FLAG_PREFIX_BYTE) == 0)
    {
        item->flags |= BER_FLAG_PREFIX_BYTE;
        item->data.len++;
    }

    item->prefix_byte = (unsigned char)( abyte & 0xff );
}
#endif /* SPLIT_BER_ITEM_SET_PREFIX_BYTE */

#ifdef SPLIT_BER_ITEM_SET_DATA
/**
 * Sets data against a BER item.
 *
 * @param   item    [In]  A BER item.
 * @param   data    [In]  The data of a BER item.
 * @param   len     [In]  The length of the data of a BER item.
 */
void BER_ITEM_set_data(BER_ITEM *item, unsigned char *data, unsigned int len)
{
    item->flags |= BER_FLAG_INVALID_LENGTH;
    item->data.bytes = data;
    item->data.len = len;
}
#endif /* SPLIT_BER_ITEM_SET_DATA */

#ifdef SPLIT_BER_READ_ITEM
/**
 * Reads the data from the buffer as a BER item and stores the header
 * information into <i>item</i>.
 *
 * @param   item  [In]  A BER item.
 * @param   p     [In]  A buffer of data.
 * @param   max   [In]  The number of bytes in buffer.
 */
int BER_read_item(BER_ITEM *item, unsigned char *p, unsigned long max)
{
    unsigned int i=0;
    unsigned int tag;
    unsigned int j;
    unsigned int x;
    unsigned int tclass;
    unsigned long len;

    if (2 > max)
    {
        return(BER_ERR_NOT_ENOUGH_BYTES);
    }

    tclass = p[i++];
	
    /* First get the class and constructed flags */
    item->info = (unsigned char)( tclass & (BER_PRIVATE | BER_CONSTRUCTED) );

    /* Get the tag */
    tag = tclass & BER_PRIMITIVE_TAG_MASK;
    if (tag == BER_PRIMITIVE_TAG_MASK) 
    {
        /* This code handles tags greater than 30 */
        tag = 0;
        j =0;
        for (;;)
        {
            if ((unsigned long)i >= max)
            {
                return(BER_ERR_NOT_ENOUGH_BYTES);
            }

            x = p[i++];
            tag |= x & 0x7f;
            if (x & 0x80)
            {
                break;
            }
            j += 7;
            if (j > BER_MAX_TAG_LEN_IN_BITS)
            {
                return(BER_ERR_TAG_TOO_LONG);
            }
            tag<<=7;
        }
    }
    item->type = tag;

    /* Now get the length */
    if ((unsigned long)i >= max)
    {
        return(BER_ERR_NOT_ENOUGH_BYTES);
    }

    len = p[i++];
    if (len & 0x80)
    { /* Long or extended */
        len &= 0x7f;
        if (len == 0)
        {
            item->info |= BER_ILEN;
	    /* Don't set this flag if indefinite length encoding.
	     * Assume if the Constructed bit is set that it is indef encoding.
	     */
	    if (!(tclass & BER_CONSTRUCTED))
                item->flags |= BER_FLAG_INVALID_LENGTH;
        }
        else
        {
            if (len > sizeof(unsigned long))
            {
                return(BER_ERR_LENGTH_TOO_LARGE);
            }
            if ((unsigned long)i+len >= max)
            {
                return(BER_ERR_NOT_ENOUGH_BYTES);
            }

            j = len;
            len = 0;
            for (;;)
            {
                len |= p[i++];
                if (--j <= 0)
                {
                    break;
                }
                len <<= 8;
            }
        }
    }
    item->data.len = len;
    item->data.bytes = &(p[i]);
    item->hlen = (unsigned char)i;

#ifndef NO_STREAM
    /* Keep a copy of the header for an outer sequence. */
    Memcpy(item->header, p, i);
#endif /* !NO_STREAM */

    return(BER_OK);
}
#endif /* SPLIT_BER_READ_ITEM */

#endif /* !(defined(NO_SPLIT) && defined(SPLIT_FILE)) */

