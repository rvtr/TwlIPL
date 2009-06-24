/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tools - makerom.TWL
  File:     ber.h

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

#ifndef HEADER_COMMON_BER_H
#define HEADER_COMMON_BER_H

#ifdef  __cplusplus
extern "C" {
#endif

//#include "r_com.h"	// local modified.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* The ASN.1 types are in this file */
#include "ber_type.h"

#if defined(VXWORKS) && defined(m_len)
#undef m_len
#endif

/* In the info field */
/**
 * The name-space of the object is universal or global.
 */
#define BER_UNIVERSAL		0x00
/**
 * The name-space of the object is application specific.
 */
#define BER_APPLICATION		0x40
/**
 * The name-space of the object is context specific.
 * Context-specific items are constructed types of a value (cont [1]).
 */
#define BER_CONTEXT_SPECIFIC	0x80
/**
 * The name-space of the object is private.
 */
#define BER_PRIVATE		0xc0
/**
 * Masks the class part of the type of the BER item.
 */
#define BER_CLASS_MASK          0xc0

/**
 * The items has no explicit data. All following items that fit in the length
 * of the constructed item are part of the data. If the constructed item is
 * indefinite encoded, the data finishes with an EOC of 0 length.
 * The items that make up the data of this item are said to have a greater
 * depth than the constructed item.
 */
#define BER_CONSTRUCTED		0x20
/**
 * This mask is used to determine the type of the item.
 */
#define BER_PRIMITIVE_TAG_MASK	0x1f

/* Set in the flags */
/**
 * Flag indicates that the stack of items was dynamically allocated and needs
 * to be freed.
 */
#define BER_FLAG_DYNAMIC	0x01
/**
 * Flag indicates that the items in the #BER_ITEMS_SK were dynamically allocated
 * and need to be freed.
 */
#define BER_FLAG_DYNAMIC_ITEMS	0x02
/**
 * A prefix byte has been set. A bit string requires a byte to precede the data
 * in order to provide information about the number of valid bits.
 */
#define BER_FLAG_PREFIX_BYTE	0x04
/**
 * The item has an invalid length value associated with it. This may be because
 * the length is greater than the parent's length or because the item has a
 * that is required more than five bytes to represent.
 */
#define BER_FLAG_INVALID_LENGTH	0x08
/**
 * The header bytes have been seen for this item and any parents will not have
 * these bytes available.
 */
#define BER_FLAG_SEEN_HEADER    0x10
/**
 * No data for this item has been made available.
 */
#define BER_FLAG_NO_DATA_SEEN   0x20
/**
 * Set if the header of the BER_ITEM has already been encoded.
 */
#define BER_FLAG_HEADER_ENCODED 0x40
/**
 * @fn int BER_prefix_byte(BER_ITEMS *i)
 *
 * Returns whether the prefix byte flag has been set for this item.
 *
 * @param  i   [In]  BER item.
 * @return           Bit is set.
 *                   <li>0 = False, the bit is not set.</li>
 *                   <li>1 = True, the bit is set.</li>
 *
 * @note  This flag is used for the bit string type as it requires an extra
 *        byte to indicate a number of valid bits.
 *
 * @see   BER_ITEM_set_prefix_byte().
 */

#define BER_prefix_byte(i)	((i)->flags & BER_FLAG_PREFIX_BYTE)
/**
 * @fn int BER_invalid_length(BER_ITEMS *i)
 *
 * Returns whether the invalid length flag has been set for this item.
 *
 * @param  i   [In]  BER item.
 * @return           Bit is set.
 *                   <li>0 = False, the bit is not set.</li>
 *                   <li>1 = True, the bit is set.</li>
 */
#define BER_invalid_length(i)	((i)->flags & BER_FLAG_INVALID_LENGTH)

/* In the info field */
/**
 * Mask off the flags of the information field.
 */
#define BER_INFO_MASK		0x0f
/**
 * The encoding is BER rather than DER.
 */
#define BER_BER			0x01
/**
 * The item is indefinite length encoded. This means that there is no length
 * value that can be used to determine the length of the data for this item.
 */
#define BER_ILEN		0x02
/**
 * The contents of this item are to be hidden.
 */
#define BER_HIDE_CONTENTS	0x04
/**
 * The header of this item has not been set yet.
 */
#define BER_NO_HEADER		0x08


/**
 * @fn int BER_hide_contents(BER_ITEMS *i)
 *
 * Returns whether the hide contents flag has been set for this item.
 *
 * @param  i   [In]  BER item.
 * @return           Bit is set.
 *                   <li>0 = False, the bit is not set.</li>
 *                   <li>1 = True, the bit is set.</li>
 */
#define BER_hide_contents(i)	((i)->info & BER_HIDE_CONTENTS)

/**
 * @fn int BER_no_header(BER_ITEMS *i)
 *
 * Returns whether the no header flag has been set for this item.
 *
 * @param  i   [In]  BER item.
 * @return           Bit is set.
 *                   <li>0 = False, the bit is not set.</li>
 *                   <li>1 = True, the bit is set.</li>
 *
 * @note  This could be set when the data has been placed in the item but the
 *        header has not been setup.
 */
#define BER_no_header(i)	((i)->info & BER_NO_HEADER)

/**
 * @fn unsigned long BER_MASK(int a)
 *
 * Returns the BER type as a bit mask.
 *
 * @param  a   [In]  Type of the item.
 * @return           Each ASN.1 type maps to a bit in a 32 bit value.
 *
 * @note  There are less than 32 primative ASN.1 types.  This
 *        means a 32bit word can be used to specify acceptable 'types'.
 */
#define BER_MASK(a)		(1UL << (a))

/**
 * @fn int BER_constructed(BER_ITEMS *a)
 *
 * Returns a value to indicate that the item is constructed.
 *
 * @param  a   [In]  BER item.
 * @return           Constructed bit set.
 *                   <li>0 = False, the bit is not set.</li>
 *                   <li>#BER_CONSTRUCTED = True, the bit is set.</li>
 */
#define BER_constructed(a)		(((a)->info) & BER_CONSTRUCTED)

/**
 * @fn int BER_class(BER_ITEMS *a)
 *
 * Returns a value to indicate the name-space of the item.
 *
 * @param  a   [In]  BER item.
 * @return           Application and/or context specific bit set.
 *                   <li>#BER_UNIVERSAL = Universal.</li>
 *                   <li>#BER_APPLICATION = application specific item.</li>
 *                   <li>#BER_CONTEXT_SPECIFIC = context specific item.</li>
 *                   <li>#BER_PRIVATE = private item.</li>
 *
 * @note   The top two bits of the type are used to specify the name-space of
 *         the item and these are pulled out and put in the info field.
 */
#define BER_class(a)			(((a)->info) & BER_CLASS_MASK)

/**
 * @fn int BER_indefinite_encoding(BER_ITEMS *a)
 *
 * Returns a value to indicate if the item is indefinite encoded. That is,
 * the number of bytes to the data is not known.
 *
 * @param  a   [In]  BER item.
 * @return           Indefinite length bit set.
 *                   <li>0 = False, the bit is not set.</li>
 *                   <li>#BER_ILEN = True, the bit is set.</li>
 */
#define BER_indefinite_encoding(a)	(((a)->info) & BER_ILEN)

/**
 * @fn void BER_ITEMS_SK_clear(BER_ITEMS_SK *sk)
 *
 * Clears the items out of the stack.
 *
 * @param  sk  [In]  Stack of BER items.
 *
 * @note  The items in the stack are not freed by this call.
 */
#define BER_ITEMS_SK_clear(sk)	((sk)->num=0)

/**
 * Maximum length of the tag/type of an item.
 */
#define BER_MAX_TAG_LEN_IN_BITS			(sizeof(int) * 8)
/**
 * Minumum length of the data for a header of an item that streaming can use
 * when decoding.
 */
#define BER_MIN_HEADER_LEN                        2
/**
 * Maximum length of the header of an item.
 */
#define BER_MAX_HEADER_LEN           (1 + (BER_MAX_TAG_LEN_IN_BITS / 8) + \
                                      1 + sizeof(unsigned long))

#ifndef NO_STREAM
/**
 * The initial state in which a new item is created.
 */
#define BER_STATE_READ_NEXT_ITEM                  1
/**
 * The state in which the item header is read in and processed.
 */
#define BER_STATE_READ_NEXT_ITEM_HEADER           2
/**
 * The state in which the data of the item is read in and processed.
 */
#define BER_STATE_READ_NEXT_ITEM_DATA             3
/**
 * The state in which the current item is the last for the constructed item
 * above.
 */
#define BER_STATE_GO_UP                           4
#endif /* !NO_STREAM */

/**
 * This structure holds the information about the data part of the BER item.
 */
typedef struct ber_bytes_st
{
    /**
     * Length of the data for this item.
     */
    unsigned long len;
    /**
     * A pointer to the start of the valid data of this item.
     */
    unsigned char *bytes;
} BER_BYTES;

/**
 * This structure holds the information about the data part of the BER item.
 */
typedef struct ber_item_st
{
    /**
     * The data of the item.
     */
    BER_BYTES data;
#ifndef NO_STREAM
    /**
     * The depth of this item. The depth is the number of constructed items this
     * item is under.
     */
    unsigned char depth;
    /**
     * The header bytes of the item. They are kept as the buffer it comes from
     * may no longer be valid when the header data is needed.
     */
    unsigned char header[BER_MAX_HEADER_LEN];
    /**
     * Number of data bytes seen of this item. When streaming the number of
     * bytes seen depends on the number of bytes in the buffer that is being
     * parsed.
     */
    unsigned long seen;
    /**
     * Number of data bytes left to get out. When streaming the number of
     * valid bytes available depends on the number of bytes in the buffer that
     * is being parsed.
     */
    unsigned long part_len;
    /**
     * Number of data bytes already encoded of the BER_ITEM. When 
     * streaming the number of bytes encoded depends on how much space
     * is in the output buffer and how many bytes are available to encode.
     */
    unsigned int encoded;
#endif /* !NO_STREAM */
    /**
     * The tag number that indicates the type of the item.
     */
    unsigned int type;
    /**
     * Information about the item. Includes the name-space, constructed bit
     * and whether it is indefinite encoded.
     */
    unsigned char info;
    /**
     * The length of the header bytes. Varies depending on name-space and the
     * number of bytes needed to represent data length.
     */
    unsigned char hlen;
    /**
     * The extra flags of the item. Includes flags to indicate what parts of the
     * item are dynamic.
     */
    unsigned char flags;
    /**
     * Holds an 'extra' prefix byte. Needed for the bit string type as it
     * requires an extra byte to indicate the number of valid bits in the data.
     */
    unsigned char prefix_byte;
} BER_ITEM;

/**
 * An item and pointer to other items linked to it.
 */
typedef struct ber_items_st
{
    /**
     * The current item data.
     */
    BER_ITEM item;
    /**
     * The constructed item that this item is the data for.
     */
     struct ber_items_st *parent;
    /**
     * The next item that is also in this constructed item.
     */
    struct ber_items_st *next;
    /**
     * When the item is constructed, this points to the first item that is
     * part of the data for this item.
     */
    struct ber_items_st *down;
} BER_ITEMS;

/**
 * Stack of BER items. The stack is the context of the BER items.
 */
typedef struct ber_items_sk_st
{
    /**
     * The number of items stored in the stack.
     */
    unsigned int num;
    /**
     * The maximum number of items that can be stored in the stack.
     */
    unsigned int max;
    /**
     * The array of items.
     */
    BER_ITEMS *items;
    /**
     * Flags of the structure. Includes dynamic allocation of data.
     * If the data is dynamic, this means that it can grow.
     */
    unsigned int flags;
#ifndef NO_STREAM
    /**
     * Current streaming parsing state.
     */
    int state;
    /**
     * Current item is indefinite encoded.
     */
    int inf;
    /**
     * The parent or the constructed item this item belongs to is indefinite
     * encoded.
     */
    int pinf;
    /**
     * The current item is a constructed item.
     */
    int con;
    /**
     * The next item needs to be placed under the current item.
     * Alternatively, the previous item was a constructed item.
     */
    int down;
    /**
     * The index into the array of items of the current item.
     * Due to the fact that the items may be reallocated an index needs to be
     * kept rather than a pointer.
     */
    int this_idx;
    /**
     * The index into the array of items of the next/new item.
     * Due to the fact that the items may be reallocated an index needs to be
     * kept rather than a pointer.
     */
    int next_idx;
    /**
     * Holds a pointer to the next/new item for the next call to the streaming
     * parsing function.
     */
    BER_ITEMS *next;
#endif /* !NO_STREAM */
} BER_ITEMS_SK;

/**
 * @fn unsigned int BER_ITEMS_SK_num(BER_ITEMS_SK *sk)
 *
 * Returns the number of items in the stack.
 *
 * @param  sk  [In]  Stack of BER items.
 * @return           Number of items in the stack.
 *
 * @note  Useful in calculating the index of the next new item.
 */
#define BER_ITEMS_SK_num(sk)            ((sk)->num)
/**
 * @fn unsigned BER_ITEMS *BER_ITEMS_SK_items(BER_ITEMS_SK *sk, unsigned int n)
 *
 * Returns a pointer to a data item in the stack.
 *
 * @param  sk  [In]  Stack of BER items.
 * @param  n   [In]  Array position of the item to obtain.
 * @return           Pointer to a BER item.
 */
#define BER_ITEMS_SK_items(sk,n)        (&((sk)->items[n]))


/**
 * Return value indicating no error occurred.
 */
#define BER_OK				0
/**
 * Return value indicating no error occurred.
 */
#define BER_ERR_OK			0
/**
 * Error indicating that the type/tag of the item was too long.
 * This normally indicates that the encoding is invalid.
 * Some valid private name-space items may cause this error.
 */
#define BER_ERR_TAG_TOO_LONG		1
/**
 * Error indicating that there are not enough bytes in the buffer to complete
 * the message. This is not a fatal error when streaming.
 */
#define BER_ERR_NOT_ENOUGH_BYTES	2
/**
 * Error indicating that the value of the length of data is too long.
 * A length value can be infinitely long but only lengths that can fit in an
 * <i>unsigned long</i> are supported.
 */
#define BER_ERR_LENGTH_TOO_LARGE	3
/**
 * Error indicating that the item is not constructed as expected or required.
 */
#define BER_ERR_NOT_CONSTRUCTED		4
/**
 * Error indicating that the function was unable to allocate the memory
 * required.
 */
#define BER_ERR_OUT_OF_MEMORY		5
/**
 * Error indicating that the function needed space for more items but was
 * unable to grow the stack. This is mostly likely to occur when the array
 * of items is static and more items than allocated are required.
 */
#define BER_ERR_OUT_OF_ITEMS_STORAGE	6
/**
 * Error indicating that the comparison of the tag/type failed.
 */
#define BER_ERR_CMP_TAG			7
/**
 * Error indicating that the item is not an integer as expected or required.
 */
#define BER_ERR_NOT_AN_INTEGER		8
/**
 * Error indicating that the length of the data is too large to return in an
 * <i>unsigned long</i>.
 */
#define BER_ERR_NUMBER_TOO_LARGE	9
/**
 * Error indicating that the bytes of data did not match in the comparison.
 */
#define BER_ERR_CMP_BYTES		10
/**
 * Error indicating an end of content item is present when not in an indefinite
 * encoded item. Normally occurs when buffer is not filled properly.
 */ 
#define BER_ERR_UNEXPECTED_EOC		11 
/**
 * Error indicating that the value of the length is invalid. This occurs when
 * the length is an indefinite length value and the item is not constructed.
 */
#define BER_ERR_INVALID_LENGTH_ENCODING	12
/**
 * Error indicating that the partial length of the value is not zero. This
 * means that there is data in the items that has not been used yet that will
 * be invalidated by parsing the new data. Streaming only.
 */
#define BER_ERR_PARTIAL                	13
/**
 * Error indicating that the various length fields of a BER_ITEM do
 * not make sense and therefore processing cannot continue. In this
 * case an unidentified logic error earlier in the code has occured.
 */
#define BER_ERR_PAR_ENC_LENGTH		14
/**
 * Invalid state in parsing or encoding.
 */
#define BER_ERR_INVALID_STATE           15


/* A description of the BER item header:
 * First byte definition
 * 5 1 -  search for a class
 *   0 -  search for a number
 * for class
 * 7-6 -  for class, it class field
 * 4-0 -  tag, normal taging convention
 * for number
 * 7-6 -  0 - do next operation
 *     -  0x40 - finished
 *     -  0x80 - operate on the contents of this item
 *     -  0xc0 - operate on the contents and return at end
 * 4-0 - num to scan, can only do 31 at a time, extend with subsequent calls
 * Number counts skip explicit and implicit tags, they only count on
 * universal types.
 * If bit 5 is set, and 6-7 are 0, then we have 'special' comands
 * which we can use WRT class elements.
 * These two are needed when we are 'sitting' on a non-primative tag
 * item since the normal 'counting' commands will skip the element before
 * they perform their command. 
 * 0x20 return
 * 0x21 operate on the contents
 */
/* To get a public key and the algorithm the following strings would work
 * 0x80|0,0x40|5 - this will return the pubkey structure
 * 0x80,0xc0	- Return the pubkey algorithm (from the pubkey)
 * 0x80,0xc1	- Return the pubkey parameters
 * 0x81		- Return the pubkey bitstring
 * 0x80,0x20|0x80|3,0xc0 - return the first extension
 */

int BER_read_item(BER_ITEM *item,unsigned char *p,unsigned long max);
int BER_find(BER_ITEM *ret,BER_ITEM *in,unsigned char *find);
int BER_ITEMS_SK_get(BER_ITEMS_SK *sk,int *items_idx);
void BER_ITEMS_append(BER_ITEMS *a,BER_ITEMS *b);
int BER_ITEMS_under(BER_ITEMS *a,BER_ITEMS *b);
int BER_ITEMS_SK_grow(BER_ITEMS_SK *ks,unsigned int num);
int BER_parse(BER_ITEMS_SK *ks,unsigned char *in,unsigned long max,
    unsigned long *num_used);
int BER_parse_stream(BER_ITEMS_SK *ks,unsigned char **in,long mlen,
    long *num_used);

#ifdef HEADER_COMMON_BIO_H
int BER_print(BIO *bio,BER_ITEMS *items);
int BER_out(BIO *out,BER_ITEMS *start);
#else /* !HEADER_COMMON_BIO_H */
int BER_print(char *,BER_ITEMS *items);
int BER_out(char *out,BER_ITEMS *start);
#endif /* !HEADER_COMMON_BIO_H */

void BER_ITEM_init(BER_ITEM *item);
void BER_ITEMS_init(BER_ITEMS *item);
void BER_ITEM_set_all(BER_ITEM *item,unsigned int sclass,unsigned int tag,
    unsigned char *data,unsigned int len,unsigned int info, unsigned int flags);
void BER_ITEM_set_header(BER_ITEM *item, unsigned int sclass,unsigned int tag,
    unsigned int flags);
void BER_ITEM_set_data(BER_ITEM *item, unsigned char *data, unsigned int len);
int BER_ITEM_cmp_tag(BER_ITEM *a,unsigned int tag);
int BER_ITEM_get_long(BER_ITEM *a,long *l);
int BER_ITEM_set_long(BER_ITEM *a, long lret, unsigned char *buf);

size_t BER_ITEM_header_len(BER_ITEM *item);
unsigned int BER_ITEM_header_write(BER_ITEM *item, unsigned char *out);
unsigned int BER_ITEM_header_swrite(BER_ITEM *item, unsigned char *out);

int BER_ITEM_cmp_bytes(BER_ITEM *a,unsigned char *d, unsigned int l);
void BER_ITEMS_SK_init(BER_ITEMS_SK *sk,BER_ITEMS *items,unsigned int num,
    unsigned int max);
void BER_ITEMS_SK_free(BER_ITEMS_SK *sk);
unsigned long BER_ITEMS_recalc_length(BER_ITEMS *a);
int BER_ITEMS_encode(BER_ITEMS *a, unsigned char *out, unsigned long *olen,
    unsigned long max);
int BER_ITEMS_encode_stream(BER_ITEMS **a, unsigned char *out,
	unsigned long *olen, unsigned long max);
int BER_load_level(BER_ITEMS_SK *sk, unsigned char *ino, unsigned long m_len);

/* We have to make the second parameter 'unsigned int' instead of
 * 'unsigned char' because both the Sun and HP compilers complain 
 * when using a mix of ANSI prototypes and K&R function implementations
 */
void BER_ITEM_set_prefix_byte(BER_ITEM *item, unsigned int byte);

/* Used to count bits in an array of chars, strip leading 0, counts bits */
int R_num_bits(unsigned char *buf, int len, int bigendian);
 
#ifdef  __cplusplus
}
#endif

#endif /* !HEADER_COMMON_BER_H */

