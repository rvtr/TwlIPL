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
/*****************************************************************************
 * Copyright (c). 2001 RSA Security Inc. All rights reserved.
 * This work contains proprietary information of RSA Security.
 * Distribution is limited to authorized licensees of RSA
 * Security. Any unauthorized reproduction, distribution or
 * modification of this work is strictly prohibited.
 ****************************************************************************/
/**
 * @file  bn_thx.h
 * @brief header file created for used as interface defines for systems
 *        containing hetergeneous environments where some bn operations
 *        will be performed in isolation from the rest of the library
 */
#ifndef HEADER_COMMON_BN_THX_H
#define HEADER_COMMON_BN_THX_H

#ifdef  __cplusplus
extern "C" {
#endif

/* These defines are required on systems so that the mod_exp code
 * to be used in isolation from the bn operation with library will work,
 * if these are not available, the header files with equivalent functionality
 * should be added inside a CPU or Operating system define
 */
#ifndef NO_STDIO_H
#include <stdio.h>
#endif

#ifndef NO_STDLIB_H
#include <stdlib.h>
#endif

#ifndef NO_STRING_H
#include <string.h>
#endif

/*  Interface for ARM/DSP systems, this interface requires only
 *  the standard defines and includes at the moment
 */
#ifdef CPU_TMS320 
#define THX_RECIPIENT
#define CPU_DSP
#endif

#ifdef  THX_RECIPIENT

#ifndef restrict
#define restrict
#endif

#ifndef Malloc
#define Malloc malloc
#endif

#ifndef Memcpy
#define Memcpy memcpy
#endif

#ifndef Memmove
#define Memmove memmove
#endif

#ifndef Memset
#define Memset memset
#endif

#ifndef Free
#define Free free
#endif

#endif /* THX_RECIPIENT */

#ifndef THX_RECIPIENT  /* not the recipient, we must be the caller */

#include "bn_lcl.h"

#endif /* ! THX_RECIPIENT */

/* The follow define protected inclusions are excerpts from the 
 * header file bn.h, this has been done, so that code written for the
 * THX systems can be used in isolation from the library
 * This code may need to be updated in line with changes to the bn.h
 * header file.
 */
#ifndef HEADER_COMMON_BN_H
#define HEADER_COMMON_BN_H

#if !defined(CCONV)
#define CCONV
#endif

#ifndef PRE_CCONV
#define PRE_CCONV
#endif

/* convert from the new to the old option names */
#if defined(OPT_BN_LLONG)
#define BN_LLONG          /* comment to make sure Configure leaves this alone */
#endif

#if defined(OPT_32_BIT_INT) || defined(OPT_32_BIT)
#define THIRTY_TWO_BIT
#endif

#define BN_ILONG       BN_ULONG

#ifdef THIRTY_TWO_BIT
#define BN_ULLONG      unsigned long long
#ifdef OPT_32_BIT_INT
#define BN_ULONG       unsigned int
#define BN_LONG        int
#else
#define BN_ULONG       unsigned long
#define BN_LONG        long
#endif
#define BN_BITS        64
#define BN_BYTES       4
#define BN_BITS2       32
#define BN_BITS4       16
/* This is needed because the Watcom compiler pre-processor
 * under QNX is perverted and tries to parses the 'LL'
 * part even though it is never used.
 */
#ifdef BN_LLONG
#define BN_MASK        (0xffffffffffffffffL)
#endif
#define BN_MASK2       (0xffffffffL)
#define BN_MASK2l      (0xffff)
#define BN_MASK2lh     (0xffff)
#define BN_MASK2h1     (0xffff8000L)
#define BN_MASK2h      (0xffff0000L)
#define BN_TBIT        (0x80000000L)
#define BN_DEC_CONV    (1000000000L)
#define BN_DEC_FMT1    "%lu"
#define BN_DEC_FMT2    "%09lu"
#define BN_DEC_NUM     9
#define BN_HEX_FMT     "%08lX"
#endif

#define BN_EXP_TABLE_SIZE                      16

typedef struct bignum_st BIGNUM;

struct bignum_st
        {
        BN_ULONG *d;    /* Pointer to an array of 'BN_BITS2' bit chunks. */
        int top;        /* Index of last used d +1. */
        /* The next are internal book keeping for bn_expand. */
        int max;        /* Size of the d array. */
        int neg;        /* one if the number is negative */
        int flags;
        };

PRE_CCONV BN_ULONG CCONV bn_mul_add_words(BN_ULONG *rp,BN_ULONG *ap,
        int num, BN_ULONG w);
PRE_CCONV BN_ULONG CCONV bn_mul_words(BN_ULONG *rp, BN_ULONG *ap,
        int num, BN_ULONG w);
PRE_CCONV void     CCONV bn_sqr_words(BN_ULONG *rp, BN_ULONG *ap, int num);
BN_ULONG CCONV bn_div_words(BN_ULONG h, BN_ULONG l, BN_ULONG d);
PRE_CCONV BN_ULONG CCONV bn_add_words(BN_ULONG *rp, BN_ULONG *ap,
        BN_ULONG *bp,int num);
PRE_CCONV BN_ULONG CCONV bn_sub_words(BN_ULONG *rp, BN_ULONG *ap,
        BN_ULONG *bp,int num);

#ifndef BN_ME_METH
#define BN_ME_METH void
#endif

#endif /* ! HEADER_COMMON_BN_H */

/* The follow define protected inclusions are excerpts from the 
 * header file r_error.h, this has been done, so that code written for the
 * THX systems can be used in isolation from the library
 * This code may need to be updated in line with changes to the r_error.h
 * header file.
 */
#ifndef HEADER_COMMON_R_ERROR_H
#define HEADER_COMMON_R_ERROR_H

/* The FATAL_INTERNAL_ERROR is a flag that is set with the following
 * error codes:
 *    R_ERROR_INVALID_STATE
 *    R_ERROR_INIT_NOT_CALLED
 *    R_ERROR_SHOULD_NOT_HAVE_BEEN_CALLED
 */
#define R_ERROR_FATAL_INTERNAL_ERROR    64

/* The BAD_PARAMETER is a flag that is set with the following error
 * codes:
 *    R_ERROR_NULL_ARG
 *    R_ERROR_BUFFER_TOO_SMALL
 *    R_ERROR_BAD_VALUE
 *    R_ERROR_BAD_RANGE
 *    R_ERROR_BAD_FORMAT
 *    R_ERROR_BAD_TYPE
 *    R_ERROR_BAD_DATA
 *    R_ERROR_BAD_LENGTH
 */
#define R_ERROR_BAD_PARAMETER        32

/* Base value for all general errors used through all products */
#define R_ERROR_BASE            10000

#define R_ERROR_NONE            0

#define R_ERROR_FAILED            (R_ERROR_BASE+1)
#define R_ERROR_IO                (R_ERROR_BASE+2)
#define R_ERROR_PROTOCOL          (R_ERROR_BASE+3)
#define R_ERROR_EOF               (R_ERROR_BASE+4)
#define R_ERROR_ALLOC_FAILURE     (R_ERROR_BASE+5)
#define R_ERROR_EVAL_RESTRICTION  (R_ERROR_BASE+6)
#define R_ERROR_EVAL_EXPIRED      (R_ERROR_BASE+7)
#define R_ERROR_NOT_FOUND         (R_ERROR_BASE+8)
#define R_ERROR_NOT_AVAILABLE     (R_ERROR_BASE+9)
#define R_ERROR_NOT_IMPLEMENTED   (R_ERROR_BASE+10)
#define R_ERROR_NOT_SUPPORTED     (R_ERROR_BASE+11)
#define R_ERROR_INVALID_STATE            \
            ((R_ERROR_BASE+12) | R_ERROR_FATAL_INTERNAL_ERROR)
#define R_ERROR_INIT_NOT_CALLED           \
            ((R_ERROR_BASE+13) | R_ERROR_FATAL_INTERNAL_ERROR)
#define R_ERROR_SHOULD_NOT_HAVE_BEEN_CALLED    \
            ((R_ERROR_BASE+14) | R_ERROR_FATAL_INTERNAL_ERROR)
#define R_ERROR_METHOD_UNDEFINED        \
            ((R_ERROR_BASE+15) | R_ERROR_FATAL_INTERNAL_ERROR)
#define R_ERROR_BUFFER_TOO_SMALL        \
            ((R_ERROR_BASE+16) | R_ERROR_BAD_PARAMETER)
#define R_ERROR_NULL_ARG          ((R_ERROR_BASE+17) | R_ERROR_BAD_PARAMETER)
#define R_ERROR_BAD_VALUE         ((R_ERROR_BASE+18) | R_ERROR_BAD_PARAMETER)
#define R_ERROR_BAD_RANGE         ((R_ERROR_BASE+19) | R_ERROR_BAD_PARAMETER)
#define R_ERROR_BAD_FORMAT        ((R_ERROR_BASE+20) | R_ERROR_BAD_PARAMETER)
#define R_ERROR_BAD_TYPE          ((R_ERROR_BASE+21) | R_ERROR_BAD_PARAMETER)
#define R_ERROR_BAD_DATA          ((R_ERROR_BASE+22) | R_ERROR_BAD_PARAMETER)
#define R_ERROR_BAD_LENGTH        ((R_ERROR_BASE+23) | R_ERROR_BAD_PARAMETER)
#define R_ERROR_RCOM_LIBRARY_NOT_SUPPORTED (R_ERROR_BASE+24)

/* Resource Manager base for errors */
#define R_COM_ERR_LIB_CTX_BASE        10100

#endif    /* HEADER_COMMON_R_ERROR_H */


const BN_ME_METH *BN_ME_METH_thxc(void);
/**
 * @note THX code only needs to store rr, n0, str
 *
 */
typedef struct thxc_mont_ctx_st
    {
    BIGNUM         *rr;
    unsigned char  *str;
    unsigned int   str_len;
    BN_ULONG       n0;
    } THXC_MONT_CTX;


int Rx_thxr_mod_exp_mont( BN_ULONG *result, BN_ULONG *ap, BN_ULONG *np,
    BN_ULONG *rrp, BN_ULONG n0, int top, int tmp_len, unsigned char *str);

int Ri_thxr_mod_exp_mont( BN_ULONG *result, BN_ULONG *ap, BN_ULONG *np,
    BN_ULONG *rrp, BN_ULONG *dp, BN_ULONG *aap, BN_ULONG *rp, BN_ULONG *tmp,
    BN_ULONG n0, int top, unsigned char *str);

void Ri_thxr_mul_normal(BN_ULONG *r,BN_ULONG *a,int na,BN_ULONG *b,int nb);
void Ri_thxr_sqr_normal(BN_ULONG *r, BN_ULONG *a, int n, BN_ULONG *tmp);

void Ri_thxr_from_montgomery_words(BN_ULONG *ret,BN_ULONG *ap,BN_ULONG *np, 
                                   int w, BN_ULONG n0);

BN_ULONG Ri_thxr_from_mont_words(BN_ULONG *ap,BN_ULONG *wap,
                      BN_ULONG *np, int w, BN_ULONG n0);

#ifdef CPU_TMS320

/* Method and functions required for the ARM/DSP interface */

const BN_ME_METH *BN_ME_METH_dspc(void);

int Rx_dsp_mod_exp_mont( BN_ULONG *result, BN_ULONG *ap, BN_ULONG *np,
    BN_ULONG *rrp, BN_ULONG n0, int top, int tmp_len, unsigned char *str);

int Ri_dsp_mod_exp_mont( BN_ULONG *result, BN_ULONG *ap, BN_ULONG *np,
    BN_ULONG *rrp, BN_ULONG *dp, BN_ULONG *aap, BN_ULONG *rp, BN_ULONG *tmp,
    BN_ULONG n0, int top, unsigned char *str);

#endif

#ifdef  __cplusplus
}
#endif
#endif /* HEADER_COMMON_BN_THX_H */
