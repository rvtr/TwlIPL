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

#ifndef HEADER_COMMON_BN_H
#define HEADER_COMMON_BN_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifdef FLAT_INC
#include "r_types.h"
#else
#include "../include/r_types.h"
#endif

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

#if defined(OPT_BN_MUL_LATENCY)
#define BN_MUL_LATENCY
#endif

/* by default we have the following switched on - unless we have an option
 * that switched them off for a specific platform
 */
#ifdef OPT_NO_COMBA
#define OPT_NO_BN_MUL_COMBA
#define OPT_NO_BN_SQR_COMBA
#endif

#ifdef OPT_NO_REC
#define OPT_NO_BN_RECURSION_MUL
#define OPT_NO_BN_RECURSION_SQR
#endif

#ifndef OPT_NO_BN_MUL_COMBA
#define BN_MUL_COMBA
#endif
#ifndef OPT_NO_BN_SQR_COMBA
#define BN_SQR_COMBA
#endif
#ifndef OPT_NO_BN_RECURSION_MUL
#define BN_RECURSION_MUL
#endif
#ifndef OPT_NO_BN_RECURSION_SQR
#define BN_RECURSION_SQR
#endif
#ifndef OPT_NO_BN_RECURSION_MONT
#undef  BN_RECURSION_MONT /* DO NOT TURN THIS ON, IT IS BROKEN */
#endif

#if (!defined(OPT_NO_BN_MUL_COMBA) && !defined(OPT_NO_BN_SQR_COMBA))
#if OPT_MONT_REDUCE_COMBA /* TEMP UNTIL C IS IMPLEMENTED-DEF'd ON PLATFORMS W/ ASM */
#define BN_REDUCE_COMBA
#endif
#endif

#define RECP_MUL_MOD
#define MONT_MUL_MOD

#ifndef OPT_NO_BN_SURRENDER
#define BN_SURRENDER
#else
#undef BN_SURRENDER
#endif

#ifdef SMALL_CODE_SIZE
#undef BN_MUL_COMBA     /* stop modification */
#undef BN_SQR_COMBA     /* stop modification */
#undef BN_REDUCE_COMBA  /* stop modification */
#undef BN_RECURSION_MUL     /* stop modification */
#undef BN_RECURSION_SQR     /* stop modification */
#undef BN_RECURSION_MONT    /* stop modification */
#endif



/* This next option uses the C libraries (2 word)/(1 word) function.
 * If it is not defined, I use my C version (which is slower).
 * The reason for this flag is that when the particular C compiler
 * library routine is used, and the library is linked with a different
 * compiler, the library is missing.  This mostly happens when the
 * library is built with gcc and then linked using nornal cc.  This would
 * be a common occurance because gcc normally produces code that is
 * 2 times faster than system compilers for the big number stuff.
 * For machines with only one compiler (or shared libraries), this should
 * be on.  Again this in only really a problem on machines
 * using "long long's", are 32bit, and are not using my assember code. */
#if defined(MSDOS) || defined(WINDOWS) || defined(linux)
#define BN_DIV2W
#endif

/* Only one for the following should be defined */
/* The prime number generation stuff may not work when
 * EIGHT_BIT but I don't care since I've only used this mode
 * for debuging the bignum libraries */
#undef SIXTY_FOUR_BIT_LONG
#undef SIXTY_FOUR_BIT
#undef SIXTY_BIT
#undef THIRTY_TWO_BIT
#undef THIRTY_BIT
#undef SIXTEEN_BIT
#undef EIGHT_BIT
#undef TEST_EIGHT_BIT

#if defined(OPT_64_BIT_LONG)
#define SIXTY_FOUR_BIT_LONG
#endif
#if defined(OPT_64_BIT)
#define SIXTY_FOUR_BIT
#endif
#if defined(OPT_60_BIT)
#define SIXTY_BIT
#endif
#if defined(OPT_32_BIT)
#define THIRTY_TWO_BIT
#endif
#if defined(OPT_32_BIT_INT)
#define THIRTY_TWO_BIT
#endif
#if defined(OPT_30_BIT)
#define THIRTY_BIT
#endif
#if defined(OPT_16_BIT)
#define SIXTEEN_BIT
#endif
#if defined(OPT_8_BIT)
#define EIGHT_BIT
#endif
#if defined(OPT_8_BIT_TEST)
#define TEST_EIGHT_BIT
#endif

/* This define is used for those few functions that 'break' when
 * things are compiled for 8 bit words.  Basically the size of an
 * integer.
 */
#define BN_ILONG    BN_ULONG

/* assuming long is 64bit - this is the DEC Alpha
 * unsigned long long is only 64 bits, don't define
 * BN_LLONG for the DEC Alpha */
#ifdef SIXTY_FOUR_BIT_LONG
#undef BN_LLONG
#define BN_ULLONG   unsigned long long
#define BN_ULONG    unsigned long
#define BN_LONG     long
#define BN_BITS     128
#define BN_BYTES    8
#define BN_BITS2    64
#define BN_BITS4    32
#define BN_MASK     (0xffffffffffffffffffffffffffffffffLL)
#define BN_MASK2    (0xffffffffffffffffL)
#define BN_MASK2l   (0xffffffffL)
#define BN_MASK2lh  (0xffffffffL)
#define BN_MASK2h   (0xffffffff00000000L)
#define BN_MASK2h1  (0xffffffff80000000L)
#define BN_TBIT     (0x8000000000000000L)
#define BN_DEC_CONV (10000000000000000000UL)
#define BN_DEC_FMT1 "%lu"
#define BN_DEC_FMT2 "%019lu"
#define BN_DEC_NUM  19
#define BN_HEX_FMT  "%016lX"
#endif

/* This is where the long long data type is 64 bits, but long is 32.
 * For machines where there are 64bit registers, this is the mode to use.
 * IRIX, on R4000 and above should use this mode, along with the relevent
 * assember code.  Do NOT define BN_LLONG.
 */
#ifdef SIXTY_FOUR_BIT
#undef BN_LLONG     /* Protect against config */
/* #define BN_ULLONG    unsigned long long */
#ifdef WIN64
#define BN_ULONG    unsigned _int64
#define BN_LONG     _int64
#else
#define BN_ULONG    unsigned long long
#define BN_LONG     long long
#endif
#define BN_BITS     128
#define BN_BYTES    8
#define BN_BITS2    64
#define BN_BITS4    32
#define BN_MASK2    (0xffffffffffffffffLL)
#define BN_MASK2l   (0xffffffffL)
#define BN_MASK2lh  (0xffffffffL)
#define BN_MASK2h   (0xffffffff00000000LL)
#define BN_MASK2h1  (0xffffffff80000000LL)
#define BN_TBIT     (0x8000000000000000LL)
#define BN_DEC_CONV (10000000000000000000ULL)
#ifdef WIN64
#define BN_DEC_FMT1 "%I64u"
#define BN_DEC_FMT2 "%019I64u"
#define BN_DEC_NUM  19
#define BN_HEX_FMT  "%016I64X"
#else
#define BN_DEC_FMT1 "%llu"
#define BN_DEC_FMT2 "%019llu"
#define BN_DEC_NUM  19
#define BN_HEX_FMT  "%016llX"
#endif
#endif

#ifdef SIXTY_BIT
#undef SIXTY_FOUR_BIT
#undef BN_LLONG     /* Protect against config */
/* #define BN_ULLONG    unsigned long long */
#define BN_ULONG    unsigned long long
#define BN_LONG     long long
#define BN_BITS     120
#define BN_BITS2    60
#define BN_BITS4    30
#define BN_MASK2    (0x0fffffffffffffffLL)
#define BN_MASK2l   (        0x3fffffffL)
#define BN_MASK2lh  (        0x3fffffffL)
#define BN_MASK2h   (0x0fffffffc0000000LL)
#define BN_MASK2h1  (0x0fffffffe0000000LL)
#define BN_TBIT     (0x0800000000000000LL)
#define BN_DEC_CONV (100000000000000000LL)
#define BN_DEC_FMT1 "%9u"
#define BN_DEC_FMT2 "%017llu"
#define BN_DEC_NUM  17
#define BN_HEX_FMT  "%016llX"
#endif

#ifdef THIRTY_TWO_BIT
#ifdef WIN32
#if defined(__BORLANDC__)
#define BN_ULLONG   unsigned __int64
#else /* !__BORLANDC__ */
#if defined(__MINGW32__) || defined(__DJGPP__)
#define BN_ULLONG   unsigned long long
#else
#define BN_ULLONG   unsigned _int64
#endif /* __MINGW32__ || __DJGPP__ */
#endif /* __BORLANDC__ */
#else /* !WIN32 */
#define BN_ULLONG   unsigned long long
#endif /* WIN32 */
#ifdef OPT_32_BIT_INT
#define BN_ULONG    unsigned int
#define BN_LONG     int
#else
#define BN_ULONG    unsigned long
#define BN_LONG     long
#endif
#define BN_BITS     64
#define BN_BYTES    4
#define BN_BITS2    32
#define BN_BITS4    16
/* This is needed because the Watcom compiler pre-processor
 * under QNX tries to parses the 'LL' part even though it is 
 * never used.
 */
#ifdef BN_LLONG
#ifndef WIN32
#define BN_MASK     (0xffffffffffffffffLL)
#else
#define BN_MASK     (0xffffffffffffffffL)
#endif
#endif
#define BN_MASK2    (0xffffffffL)
#define BN_MASK2l   (0xffff)
#define BN_MASK2lh  (0xffff)
#define BN_MASK2h1  (0xffff8000L)
#define BN_MASK2h   (0xffff0000L)
#define BN_TBIT     (0x80000000L)
#define BN_DEC_CONV (1000000000L)
#define BN_DEC_FMT1 "%lu"
#define BN_DEC_FMT2 "%09lu"
#define BN_DEC_NUM  9
#define BN_HEX_FMT  "%08lX"
#endif

#ifdef THIRTY_BIT
#ifdef WIN32
#if defined(__MINGW32__) || defined(__DJGPP__) 
#define BN_ULLONG   unsigned long long
#else /* ! (__MINGW32__ || __DJGPP__) */
#define BN_ULLONG   unsigned _int64
#endif /* __MINGW32__ || __DJGPP__ */
#else /* !WIN32 */
#define BN_ULLONG   unsigned long long
#endif /* WIN32 */
#define BN_ULONG    unsigned long
#define BN_LONG     long
#define BN_BITS     60
#define BN_BITS2    30
#define BN_BITS4    15
/* This is needed because the Watcom compiler pre-processor
 * under QNX tries to parses the 'LL' part even though it is 
 * never used.
 */
#ifdef BN_LLONG
#ifndef WIN32
#define BN_MASK     (0x0fffffffffffffffLL)
#else
#define BN_MASK     (0x0fffffffffffffffL)
#endif
#endif
#define BN_MASK2    (0x3fffffffL)
#define BN_MASK2l   (0x7fff)
#define BN_MASK2lh  (0x7fff)
#define BN_MASK2h1  (0x3fffc000L)
#define BN_MASK2h   (0x3fff8000L)
#define BN_TBIT     (0x20000000L)
#define BN_DEC_CONV (1000000000L)
#define BN_DEC_FMT1 "%lu"
#define BN_DEC_FMT2 "%09lu"
#define BN_DEC_NUM  9
#define BN_HEX_FMT  "%08lX"
#endif

#ifdef SIXTEEN_BIT
#ifndef BN_DIV2W
#define BN_DIV2W
#endif
#define BN_ULLONG   unsigned long
#define BN_ULONG    unsigned short
#define BN_LONG     short
#define BN_BITS     32
#define BN_BYTES    2
#define BN_BITS2    16
#define BN_BITS4    8
#define BN_MASK     (0xffffffff)
#define BN_MASK2    (0xffff)
#define BN_MASK2l   (0xff)
#define BN_MASK2lh  (0xff)
#define BN_MASK2h1  (0xff80)
#define BN_MASK2h   (0xff00)
#define BN_TBIT     (0x8000)
#define BN_DEC_CONV (10000)
#define BN_DEC_FMT1 "%u"
#define BN_DEC_FMT2 "%04u"
#define BN_DEC_NUM  4
#define BN_HEX_FMT  "%04X"
#endif

#ifdef TEST_EIGHT_BIT
#define EIGHT_BIT   /* comment to stop editing */
#endif

#ifdef EIGHT_BIT
#undef BN_ILONG
#define BN_ILONG    unsigned int
#ifndef BN_DIV2W
#define BN_DIV2W
#endif
#ifdef TEST_EIGHT_BIT
#define BN_ULLONG   unsigned int
#define BN_ULONG    unsigned int
#define BN_LONG     int
#else
#define BN_ULLONG   unsigned short
#define BN_ULONG    unsigned char
#define BN_LONG     char
#endif
#define BN_BITS     16
#define BN_BYTES    1
#define BN_BITS2    8
#define BN_BITS4    4
#define BN_MASK     (0xffff)
#define BN_MASK2    (0xff)
#define BN_MASK2lh  (0xf)
#define BN_MASK2l   (0xf)
#define BN_MASK2h1  (0xf8)
#define BN_MASK2h   (0xf0)
#define BN_TBIT     (0x80)
#define BN_DEC_CONV (100)
#define BN_DEC_FMT1 "%u"
#define BN_DEC_FMT2 "%02u"
#define BN_DEC_NUM  2
#define BN_HEX_FMT  "%02X"
#endif

#ifdef BIGNUM
#undef BIGNUM
#endif

#define BN_FLG_MALLOCED     0x01
#define BN_FLG_STATIC_DATA  0x02
#define BN_FLG_FREE     0x8000  /* used for debuging */
#define BN_CTX_FLG_ABORT        0x4000  /* used for aborting RSA operations*/
#define BN_set_flags(b,n)   ((b)->flags|=(n))
#define BN_get_flags(b,n)   ((b)->flags&(n))
#define BN_CTX_set_flags(b,n)   ((b)->flags|=(n))
#define BN_CTX_get_flags(b,n)   ((b)->flags&(n))

#ifndef HEADER_COMMON_BN_H_TYPEDEF_DEF
#define HEADER_COMMON_BN_H_TYPEDEF_DEF
typedef struct bignum_st BIGNUM;
typedef struct bignum_ctx BN_CTX;
typedef struct bn_mont_ctx_st BN_MONT_CTX;
typedef struct bn_recp_ctx_st BN_RECP_CTX;
typedef struct bn_prime_ctx_st BN_PRIME_CTX;
typedef struct bn_blind_ctx_st BN_BLIND_CTX;
typedef struct bn_blind_meth_st BN_BLIND_METH;
#endif

typedef struct bn_mod_exp_meth_st BN_ME_METH;
typedef struct bn_mod_exp_ctx_st BN_ME_CTX;


/* The data array d must always have an extra 'valid' word in location
 * bn->d[bn->max].  It must be ok for reading.  This is needed if
 * BN_MUL_LATENCY is defined.
 */
struct bignum_st
    {
    BN_ULONG *d;    /* Pointer to an array of 'BN_BITS2' bit chunks. */
    int top;    /* Index of last used d +1. */
    /* The next are internal book keeping for bn_expand. */
    int max;    /* Size of the d array. */
    int neg;    /* one if the number is negative */
    int flags;
    };

/* Used for temp variables */
#define BN_CTX_NUM  12
struct bignum_ctx
    {
    int tos;
    BIGNUM bn[BN_CTX_NUM+1];
    int flags;
    R_SURRENDER *surrender;
    };

#if defined(WIN64) || defined(__ia64__) || defined(CPU_IA64)
#define BN_USHORT       unsigned int
#else
#define BN_USHORT       unsigned short
#endif

/* Used for prime number generation */
struct bn_prime_ctx_st
    {
    BN_USHORT *primes;
    BN_USHORT *mods;
    int num_primes;
    int prime_checks;
    R_SURRENDER *surrender;
    R_RANDOM *random;
    int flags;
    };

struct bn_blind_ctx_st
    {
    int init;
    BN_BLIND_METH *meth;
    BIGNUM B;
    BIGNUM Bi;
    BIGNUM mod;
    };

struct bn_blind_meth_st
    {
    void (*init)    (BN_BLIND_CTX *);
    void (*ctx_free)(BN_BLIND_CTX *);
    int (*set)  (BN_BLIND_CTX *,R_RANDOM *,BIGNUM *,BIGNUM *, BN_ME_CTX *,BN_CTX *);
    int (*convert)  (BN_BLIND_CTX *,BIGNUM *,BN_CTX *);
    int (*invert)   (BN_BLIND_CTX *,BIGNUM *,BN_CTX *);
    int (*update)   (BN_BLIND_CTX *,BN_CTX *);
    int (*copy) (BN_BLIND_CTX *,BN_BLIND_CTX *);
    };

/* Used for montgomery multiplication */
struct bn_mont_ctx_st
        {
    int use_word;   /* 0 for word form, 1 for long form */
        int ri;         /* number of bits in R */
        int riw;        /* number of words in R */
        BIGNUM RR;     /* used to convert to montgomery form */
        BIGNUM N;      /* The modulus */
        BIGNUM Ni;     /* The inverse of N */
    BN_ULONG n0;    /* word form of inverse, normally only one of
             * Ni or n0 is defined */
    int flags;
        };

/* Used by the recursive exponentiation implementations */
typedef struct bn_mod_exp_rec_meth_st
    {
#ifndef NOPROTO
    void (PRE_CCONV CCONV *mul)(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b);
    void (PRE_CCONV CCONV *sqr)(BN_ULONG *r, BN_ULONG *a);
    void (PRE_CCONV CCONV *low_mul)();
#else
    void (PRE_CCONV CCONV *mul)();
    void (PRE_CCONV CCONV *sqr)();
    void (PRE_CCONV CCONV *low_mul)();
#endif
    } BN_MOD_EXP_REC_METH;

/* If this flag is set, if there is a custom method for a modulus
 * two times larger, use it.  This is mostly mean for use on the
 * itanium where a 512*512 routine is 10 times faster than the C
 * code version, so we need to detect that we can turn of CRT
 * for RSA if there is an native IA64 512*512 present.
 */
#define BN_ME_FLG_FAST_ASM      0x0001

struct bn_mod_exp_meth_st
    {
    int num;    /* Word size we target */
    char *name; /* Identify the method */
#ifndef NOPROTO
    int (*useit)(const BN_ME_METH *meth);
    /* If 'power' is null, reuse the old one */
    int (*mod_exp)(BN_ME_CTX *mctx,BIGNUM *r,BIGNUM *a,BIGNUM *p,
        BN_CTX *ctx);
    /* Create the BN_ME_CTX */
    int (*init_ctx)(const BN_ME_METH *meth,BN_ME_CTX **mctx);
    /* Get rid of it */
    int (*free_ctx)(BN_ME_CTX *mctx);
    /* Assign the modulus */
    int (*set)(BN_ME_CTX *mctx,BIGNUM *n,int cmd,int flags,BN_CTX *ctx);
#else
    int (*useit)();
    int (*mod_exp)();
    int (*init_ctx)();
    int (*free_ctx)();
    int (*set)();
#endif
    int argi;
    char *argp; /* 'Extra stuff' */
    };

#define BN_ME_METH_TABLE_MAX    32
/* Used in BN_library_init */
#define BN_INIT_LAST        0x00
#define BN_INIT_BN_ME_METH  0x01

#define BN_BNME_F_DEFAULT   0x01
/* Used only when loading, kept as bits internally, normally loaded as
 * words */
#define BN_BNME_F_BITS      0x02
typedef struct bn_me_meth_info_st
    {
#ifndef NOPROTO
    const BN_ME_METH *(*meth)(void);
#else
    const BN_ME_METH *(*meth)();
#endif
    int min;
    int max;
    int flags;
    } BN_ME_METH_INFO;

#define BN_ME_SET_MOD       0x01
#define BN_ME_SET_BASE      0x02
#define BN_ME_SET_EXP       0x03
#define BN_ME_SET_FLG_NO_LOOKUP 0x01 /* Passed to BN_ME_CTX_set() */

#define BN_FLG_CACHE        0x10    /* Cache values */
#define BN_ME_FLG_BASE      0x40    /* base has been cached */
#define BN_ME_FLG_EXP       0x80    /* power has been cached */
/* When one of these structures is setup, it is intended that the
 * base or power, if not null, will be used for the current calculation.
 * If the base and/or power are cached, they will only be used if the input
 * value is null.
 * For RSA, the power would normally be set.
 * For DH, the base and power would be set.  The initial value generated
 * would be the public key, which is exchanged.  The phase2 part would
 * change the base but not the power.
 * For DSA signing, a fixed base is used, but a random power.
 * For DSA verification, there are two bases and two powers,
 * a^p%m * b^q%m
 * One way to implement this is to use a special function to
 * generate a composite power value, and or a special base form.
 * The other option is to have a special function to generate
 * the 'base' array.  The problem is that we are exponentiating with
 * a 3 value power value, instead of the normal two.  It should be possible
 * to use the standard a^b%m function.
 */
struct bn_mod_exp_ctx_st
    {
    const BN_ME_METH *meth;
    char *callback;
    char *cb_arg;
    int flags;
    /* Evil hack for storage of data values, it should really be here
     * other than the first value */
    char *modulus;  /* eg BN_MONT_CTX */
    char *power;    /* eg power representation */ 
    char *base; /* eg base representation */ 
    char *arg;
    };

/* Used for reciprocal division/mod functions
 * It cannot be shared between threads
 */
struct bn_recp_ctx_st
    {
    BIGNUM N;   /* the divisor */
    BIGNUM Nr;  /* the reciprocal */
    int num_bits;
    int shift;
    int flags;
    };

typedef struct bn_rec_st
    {
    int depth;
    int n;
#ifndef NOPROTO
    void (PRE_CCONV CCONV *mul)(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b);
    void (PRE_CCONV CCONV *sqr)(BN_ULONG *r, BN_ULONG *a);
    void (PRE_CCONV CCONV *low_mul)();
#else
    void (PRE_CCONV CCONV *mul)();
    void (PRE_CCONV CCONV *sqr)();
    void (PRE_CCONV CCONV *low_mul)();
#endif
    } BN_REC;

#define BN_to_montgomery(r,a,mont,ctx)  BN_mod_mul_montgomery(\
    r,a,&((mont)->RR),(mont),ctx)

#define BN_prime_checks     (5)

#define BN_num_bytes(a) ((BN_num_bits(a)+7)/8)
#define BN_length(a)    ((a)->top * BN_BYTES)
#define BN_is_word(a,w) (((a)->top == 1) && ((a)->d[0] == (BN_ULONG)(w)))
#define BN_is_zero(a)   (((a)->top == 0) || BN_is_word(a,0))
#define BN_is_one(a)    (BN_is_word((a),1))
#define BN_is_odd(a)    (((a)->top > 0) && ((a)->d[0] & 1))
#define BN_one(a)   (BN_set_word((a),(BN_ULONG)1))
#define BN_zero(a)  (BN_set_word((a),(BN_ULONG)0))

/*#define BN_ascii2bn(a)    BN_hex2bn(a) */
/*#define BN_bn2ascii(a)    BN_bn2hex(a) */

#define bn_expand(n,b) ((((((b+BN_BITS2-1))/BN_BITS2)) <= (n)->max)?\
    (n):bn_expand2((n),(b)/BN_BITS2+1))
#define bn_wexpand(n,b) (((b) <= (n)->max)?(n):bn_expand2((n),(b)))

#ifdef SMALL_CODE_SIZE
void bn_zexpand(BIGNUM *a,int n);
void bn_fix_top(BIGNUM *a);
#else
#define bn_zexpand(a,n) \
    if ((a)->top < n) \
        { \
        int i; \
        bn_wexpand((a),n); \
        if ((a)->d!=NULL) \
            { \
            for (i=(a)->top; i<n; i++) \
                (a)->d[i]=0; \
            } \
        }

#define bn_fix_top(a) \
        { \
        BN_ULONG *ftl; \
    if ((a)->top > 0) \
        { \
        for (ftl= &((a)->d[(a)->top-1]); (a)->top > 0; (a)->top--) \
        if (*(ftl--)) break; \
        } \
    }
#endif

#define BN_MONT_CTX_set(a,b,c)  BN_MONT_CTX_set_word((a),(b),(c))

#ifndef NOPROTO


const BN_ME_METH *BN_ME_METH_word(void);
const BN_ME_METH *BN_ME_METH_full(void);

#if (defined(BN_MUL_COMBA) || defined(BN_SQR_COMBA))

/* word4 - used for 64bit multiprime*/
const BN_ME_METH *BN_ME_METH_word4(void); 
const BN_ME_METH *BN_ME_METH_rec4_word8(void);
const BN_ME_METH *BN_ME_METH_rec4_word16(void);
const BN_ME_METH *BN_ME_METH_rec4_word32(void);
const BN_ME_METH *BN_ME_METH_rec4_word64(void);
const BN_ME_METH *BN_ME_METH_rec4_word128(void);

/* word6 - used for 64bit multiprime*/
const BN_ME_METH *BN_ME_METH_word6(void); 
const BN_ME_METH *BN_ME_METH_rec6_word12(void);
const BN_ME_METH *BN_ME_METH_rec6_word24(void);
const BN_ME_METH *BN_ME_METH_rec6_word48(void);
const BN_ME_METH *BN_ME_METH_rec6_word96(void);

const BN_ME_METH *BN_ME_METH_word8(void);
const BN_ME_METH *BN_ME_METH_rec8_word16(void);
const BN_ME_METH *BN_ME_METH_rec8_word32(void);
const BN_ME_METH *BN_ME_METH_rec8_word64(void);
const BN_ME_METH *BN_ME_METH_rec8_word128(void);
const BN_ME_METH *BN_ME_METH_rec8_word256(void);

const BN_ME_METH *BN_ME_METH_word11(void);
const BN_ME_METH *BN_ME_METH_rec11_word22(void);
const BN_ME_METH *BN_ME_METH_rec11_word44(void);
const BN_ME_METH *BN_ME_METH_rec11_word88(void);

const BN_ME_METH *BN_ME_METH_word16(void);
const BN_ME_METH *BN_ME_METH_rec16_word32(void);
const BN_ME_METH *BN_ME_METH_rec16_word64(void);
const BN_ME_METH *BN_ME_METH_rec16_word128(void);
const BN_ME_METH *BN_ME_METH_rec16_word256(void);

#endif

#ifdef CPU_IA64
const BN_ME_METH *BN_ME_METH_ia64_384(void);
const BN_ME_METH *BN_ME_METH_ia64_512(void);
const BN_ME_METH *BN_ME_METH_ia64_1024(void);
#endif

#if (defined(CPU_SPARC_V8PLUS) || defined (CPU_SPARC_V9))
const BN_ME_METH *BN_ME_METH_usparc(void);
const BN_ME_METH *BN_ME_METH_usparc_352(void);
const BN_ME_METH *BN_ME_METH_usparc_512(void);
#endif

#if 1 || defined(CPU_X86)
const BN_ME_METH *BN_ME_METH_pentium4_29(void);
const BN_ME_METH *BN_ME_METH_pentium4_28(void);
#endif

/* NCipher Nfast hardware accelerator method prototype */
const BN_ME_METH *BN_ME_METH_nfast(void);  


BIGNUM *BN_value_one(void);
const char *    BN_options(void);
BN_CTX *BN_CTX_new(void);
void    BN_CTX_init(BN_CTX *c);
void    BN_CTX_free(BN_CTX *c);
#ifndef NO_BN_RAND
int     BN_rand(BIGNUM *rnd, R_RANDOM *rand, int bits, int top,int bottom);
#endif
int BN_num_bits(BIGNUM *a);
int BN_num_bits_word(BN_ILONG);
BIGNUM *BN_new(void);
void    BN_init(BIGNUM *);
void    BN_clear_free(BIGNUM *a);
BIGNUM *BN_copy(BIGNUM *a, BIGNUM *b);
BIGNUM *BN_bin2bn(unsigned char *s,int len,BIGNUM *ret);
int BN_bn2bin(BIGNUM *a, unsigned char *to);
BIGNUM *BN_mpi2bn(unsigned char *s,int len,BIGNUM *ret);
int BN_bn2mpi(BIGNUM *a, unsigned char *to);
int BN_sub(BIGNUM *r, BIGNUM *a, BIGNUM *b);
int BN_usub(BIGNUM *r, BIGNUM *a, BIGNUM *b);
int BN_uadd(BIGNUM *r, BIGNUM *a, BIGNUM *b);
int BN_add(BIGNUM *r, BIGNUM *a, BIGNUM *b);
int BN_mod(BIGNUM *rem, BIGNUM *m, BIGNUM *d, BN_CTX *ctx);
int BN_div(BIGNUM *dv, BIGNUM *rem, BIGNUM *m, BIGNUM *d, BN_CTX *ctx);
int BN_mul(BIGNUM *r, BIGNUM *a, BIGNUM *b,BN_CTX *ctx);
int BN_sqr(BIGNUM *r, BIGNUM *a,BN_CTX *ctx);
BN_ULONG BN_mod_word(BIGNUM *a, BN_ULONG w);
BN_ULONG BN_div_word(BIGNUM *a, BN_ULONG w);
int BN_mul_word(BIGNUM *a, BN_ULONG w);
int BN_add_word(BIGNUM *a, BN_ULONG w);
int BN_sub_word(BIGNUM *a, BN_ULONG w);
int BN_set_word(BIGNUM *a, BN_ULONG w);
BN_ULONG BN_get_word(BIGNUM *a);
int BN_cmp(BIGNUM *a, BIGNUM *b);
void    BN_free(BIGNUM *a);
int BN_is_bit_set(BIGNUM *a, int n);
int BN_lshift(BIGNUM *r, BIGNUM *a, int n);
int BN_lshift1(BIGNUM *r, BIGNUM *a);
int BN_exp(BIGNUM *r, BIGNUM *a, BIGNUM *p,BN_CTX *ctx);
int BN_mod_exp(BIGNUM *r, BIGNUM *a, BIGNUM *p, BIGNUM *m,BN_CTX *ctx);
int BN_mod_exp_mont(BIGNUM *r, BIGNUM *a, BIGNUM *p, BIGNUM *m,BN_CTX *ctx,
        BN_MONT_CTX *m_ctx);
int BN_mod_exp2_mont(BIGNUM *r, BIGNUM *a1, BIGNUM *p1,BIGNUM *a2,
        BIGNUM *p2,BIGNUM *m,BN_CTX *ctx,BN_MONT_CTX *m_ctx);
int BN_mod_exp_simple(BIGNUM *r, BIGNUM *a, BIGNUM *p,
    BIGNUM *m,BN_CTX *ctx);
int BN_mask_bits(BIGNUM *a,int n);
int BN_mod_mul(BIGNUM *ret, BIGNUM *a, BIGNUM *b, BIGNUM *m,
    BN_CTX *ctx);
#if     !defined(WIN16) && !defined(NO_FP_API)
int BN_print_fp(FILE *fp, BIGNUM *a);
#endif
#ifdef HEADER_COMMON_BIO_H
int BN_print(BIO *fp, BIGNUM *a);
#else
/* int  BN_print(char *fp, BIGNUM *a); */
#endif
int BN_reciprocal(BIGNUM *r, BIGNUM *m, int len, BN_CTX *ctx);
int BN_rshift(BIGNUM *r, BIGNUM *a, int n);
int BN_rshift1(BIGNUM *r, BIGNUM *a);
void    BN_clear(BIGNUM *a);
BIGNUM *bn_expand2(BIGNUM *b, int bits);
BIGNUM *BN_dup(BIGNUM *a);
int BN_ucmp(BIGNUM *a, BIGNUM *b);
int BN_set_bit(BIGNUM *a, int n);
int BN_clear_bit(BIGNUM *a, int n);
char *  BN_bn2hex(BIGNUM *a);
char *  BN_bn2dec(BIGNUM *a);
int     BN_hex2bn(BIGNUM **a,char *str);
int     BN_dec2bn(BIGNUM **a,char *str);
int BN_gcd(BIGNUM *r,BIGNUM *in_a,BIGNUM *in_b,BN_CTX *ctx);
BIGNUM *BN_mod_inverse(BIGNUM *ret,BIGNUM *a, BIGNUM *n,BN_CTX *ctx);
/* Return R, where it is R*(1<<BN_BITS2)% m == 1 */
BN_ULONG BN_mod_inverse_word(BN_ULONG m);
BIGNUM *BN_generate_prime(BN_PRIME_CTX *pctx,BIGNUM *ret,int bits);
int BN_is_prime(BN_PRIME_CTX *pctx,BIGNUM *p,int nchecks,BN_CTX *ctx);
void    ERR_load_BN_strings(void );

BN_PRIME_CTX *BN_PRIME_CTX_new(void);
void BN_PRIME_CTX_init(BN_PRIME_CTX *pctx);
int BN_PRIME_CTX_setup(BN_PRIME_CTX *pctx, R_SURRENDER *surrender,
    R_RANDOM *random, int num);
void BN_PRIME_CTX_free(BN_PRIME_CTX *pctx);

PRE_CCONV BN_ULONG CCONV bn_mul_add_words(BN_ULONG *rp,BN_ULONG *ap,
    int num, BN_ULONG w);
PRE_CCONV BN_ULONG CCONV bn_mul_words(BN_ULONG *rp, BN_ULONG *ap, int num,
    BN_ULONG w);
PRE_CCONV void     CCONV bn_sqr_words(BN_ULONG *rp, BN_ULONG *ap, int num);
BN_ULONG CCONV bn_div_words(BN_ULONG h, BN_ULONG l, BN_ULONG d);
PRE_CCONV BN_ULONG CCONV bn_add_words(BN_ULONG *rp, BN_ULONG *ap,
    BN_ULONG *bp,int num);
PRE_CCONV BN_ULONG CCONV bn_sub_words(BN_ULONG *rp, BN_ULONG *ap,
    BN_ULONG *bp,int num);

BN_MONT_CTX *BN_MONT_CTX_new(void );
void BN_MONT_CTX_init(BN_MONT_CTX *ctx);
int BN_mod_mul_montgomery(BIGNUM *r,BIGNUM *a,BIGNUM *b,BN_MONT_CTX *mont,
    BN_CTX *ctx);

void BN_MONT_CTX_free(BN_MONT_CTX *mont);
/* int BN_MONT_CTX_set(BN_MONT_CTX *mont,BIGNUM *modulus,BN_CTX *ctx); */
BN_MONT_CTX *BN_MONT_CTX_copy(BN_MONT_CTX *to,BN_MONT_CTX *from);
int BN_MONT_CTX_set_full(BN_MONT_CTX *mont,BIGNUM *mod,BN_CTX *ctx);

void BN_set_params(int mul,int high,int low,int mont);

/* BN_get_params - which 0=mul, 1=high, 2=low, 3=mont */
int BN_get_params(int which); 

void bn_mul_normal(BN_ULONG *r,BN_ULONG *a,int na,BN_ULONG *b,int nb);
void bn_sqr_normal(BN_ULONG *r, BN_ULONG *a, int n, BN_ULONG *tmp);

#if (defined(BN_MUL_COMBA) || defined(BN_SQR_COMBA))

PRE_CCONV void CCONV bn_mul_comba11(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b);
PRE_CCONV void CCONV bn_mul_comba6(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b);
#ifndef PLATFORM_HPUX_64
PRE_CCONV void CCONV bn_mul_comba16(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b);
#endif

PRE_CCONV void CCONV bn_sqr_comba11(BN_ULONG *r,BN_ULONG *a);
PRE_CCONV void CCONV bn_sqr_comba6(BN_ULONG *r,BN_ULONG *a);
#ifndef PLATFORM_HPUX_64
PRE_CCONV void CCONV bn_sqr_comba16(BN_ULONG *r,BN_ULONG *a);
#endif

#endif

PRE_CCONV void CCONV bn_mul_comba8(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b);
PRE_CCONV void CCONV bn_mul_comba4(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b);
PRE_CCONV void CCONV bn_sqr_comba8(BN_ULONG *r,BN_ULONG *a);
PRE_CCONV void CCONV bn_sqr_comba4(BN_ULONG *r,BN_ULONG *a);


int BN_from_montgomery(BIGNUM *r,BIGNUM *a,BN_MONT_CTX *mont,BN_CTX *ctx);
PRE_CCONV void r0_bn_mont_comba4(BN_ULONG *r,BN_ULONG *a,BN_ULONG *n,
    int num,BN_ULONG n0);
PRE_CCONV void r0_bn_mont_comba6(BN_ULONG *r,BN_ULONG *a,BN_ULONG *n,
    int num,BN_ULONG n0);
PRE_CCONV void r0_bn_mont_comba8(BN_ULONG *r,BN_ULONG *a,BN_ULONG *n,
    int num,BN_ULONG n0);
PRE_CCONV void r0_bn_mont_comba11(BN_ULONG *r,BN_ULONG *a,BN_ULONG *n,
    int num,BN_ULONG n0);
PRE_CCONV void r0_bn_mont_comba12(BN_ULONG *r,BN_ULONG *a,BN_ULONG *n,
    int num,BN_ULONG n0);
PRE_CCONV void r0_bn_mont_comba16(BN_ULONG *r,BN_ULONG *a,BN_ULONG *n,
    int num,BN_ULONG n0);

int bn_cmp_words(BN_ULONG *a,BN_ULONG *b,int n);
void bn_mul_recursive(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b,int n2,BN_ULONG *t);
void bn_mul_part_recursive(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b,
    int tn, int n,BN_ULONG *t);
void bn_sqr_recursive(BN_ULONG *r,BN_ULONG *a, int n2, BN_ULONG *t);
void bn_mul_low_normal(BN_ULONG *r,BN_ULONG *a,BN_ULONG *b, int n);

void    BN_RECP_CTX_init(BN_RECP_CTX *recp);
BN_RECP_CTX *BN_RECP_CTX_new(void);
void    BN_RECP_CTX_free(BN_RECP_CTX *recp);
int BN_RECP_CTX_set(BN_RECP_CTX *recp,BIGNUM *rdiv,BN_CTX *ctx);
int BN_mod_mul_reciprocal(BIGNUM *r, BIGNUM *x, BIGNUM *y,
        BN_RECP_CTX *recp,BN_CTX *ctx);
int BN_mod_exp_recp(BIGNUM *r, BIGNUM *a, BIGNUM *p, BIGNUM *m,BN_CTX *ctx);
int BN_div_recp(BIGNUM *dv, BIGNUM *rem, BIGNUM *m,
        BN_RECP_CTX *recp, BN_CTX *ctx);

BN_ME_CTX *BN_ME_CTX_new(BN_ME_METH *meth,BN_ME_CTX **ret);
void BN_ME_CTX_free(BN_ME_CTX *ctx);
int BN_ME_CTX_set(BN_ME_CTX *mctx, BIGNUM *m, int cmd,int flags,BN_CTX *ctx);
char *BN_ME_CTX_name(BN_ME_CTX *ctx);
char *BN_ME_name(BN_ME_CTX *ctx);
int BN_ME_CTX_mod_exp(  BN_ME_CTX *mctx, BIGNUM *ret, BIGNUM *a,
    BIGNUM *p, BIGNUM *m, BN_CTX *ctx);
#if 0
int BN_ME_CTX_set_power(BN_ME_CTX *mctx,BIGNUM *p, int flags,BN_CTX *ctx);
int BN_ME_CTX_set_base( BN_ME_CTX *mctx, BIGNUM *b, int flags,BN_CTX *ctx);
#endif
int BN_gen_exp_bits(BIGNUM *p,unsigned char **strp,int flags,BN_CTX *ctx);
int BN_MONT_CTX_set_word(BN_MONT_CTX *mont,BIGNUM *mod,BN_CTX *ctx);

void bn_from_montgomery_words(BN_ULONG *ret,BN_ULONG *ap,BN_ULONG *np, int w,
    BN_ULONG n0);

void bn_from_montgomery_full(BN_ULONG *ret,BN_ULONG *ap,BN_ULONG *np, int w,
    BN_ULONG *nip,BN_ULONG *tmp);

void bn_mul_rec_words(BN_ULONG *rp,BN_ULONG *a,BN_ULONG *b,BN_ULONG *t,
    BN_REC *rec);
void bn_sqr_rec_words(BN_ULONG *r,BN_ULONG *a,BN_ULONG *t,BN_REC *rec);
void bn_2s_comp(BN_ULONG *r,BN_ULONG *a, int n);

int BN_library_init(BN_ME_METH_INFO *);
int BN_default_init(void);

BN_ME_METH_INFO *BN_get_default_exp_table(void);
BN_ME_METH_INFO *BN_bnme_get_info(int index);
const BN_ME_METH *  (*BN_bnme_get(int size,int flags))(void);
int     BN_bnme_set(const BN_ME_METH *(*meth)(void), int min, int max, int flags);
void        BN_bnme_clear(void);
int     BN_bnme_delete(int index);
int     BN_bnme_insert(const BN_ME_METH *(*meth)(void),int min,int max,int flagss,
            int index);

void BN_BLIND_CTX_init(BN_BLIND_CTX *blind);
void BN_BLIND_CTX_free(BN_BLIND_CTX *blind);
int BN_BLIND_CTX_set(BN_BLIND_CTX *b_ctx, R_RANDOM *rand, BIGNUM *e,
    BIGNUM *m,BN_ME_CTX *m_ctx, BN_CTX *ctx);
int BN_BLIND_CTX_convert(BN_BLIND_CTX *b, BIGNUM *a,BN_CTX *ctx);
int BN_BLIND_CTX_invert(BN_BLIND_CTX *b, BIGNUM *a, BN_CTX *ctx);
int BN_BLIND_CTX_update(BN_BLIND_CTX *b, BN_CTX *ctx);
int BN_BLIND_CTX_copy(BN_BLIND_CTX *b1, BN_BLIND_CTX *b2);

BN_BLIND_METH *BN_BLIND_METH_blinding();

#else

BIGNUM *BN_value_one();
const char *    BN_options();
BN_CTX *BN_CTX_new();
void    BN_CTX_init();
void    BN_CTX_free();
#ifndef NO_BN_RAND
int     BN_rand();
#endif
int BN_num_bits();
int BN_num_bits_word();
BIGNUM *BN_new();
void    BN_init();
void    BN_clear_free();
BIGNUM *BN_copy();
BIGNUM *BN_bin2bn();
int BN_bn2bin();
BIGNUM *BN_mpi2bn();
int BN_bn2mpi();
int BN_sub();
int BN_usub();
int BN_uadd();
int BN_add();
int BN_mod();
int BN_div();
int BN_mul();
int BN_sqr();
BN_ULONG BN_mod_word();
BN_ULONG BN_div_word();
int BN_add_word();
int BN_sub_word();
int BN_mul_word();
int BN_set_word();
unsigned long BN_get_word();
int BN_cmp();
void    BN_free();
int BN_is_bit_set();
int BN_lshift();
int BN_lshift1();
int BN_exp();
int BN_mod_exp();
int BN_mod_exp_mont();
int BN_mod_exp_recp();
int BN_mod_exp_simple();
int BN_mask_bits();
int BN_mod_mul_reciprocal();
int BN_mod_mul();
#ifndef WIN16
int BN_print_fp();
#endif
int BN_print();
int BN_reciprocal();
int BN_rshift();
int BN_rshift1();
void    BN_clear();
BIGNUM *bn_expand2();
BIGNUM *BN_dup();
int BN_ucmp();
int BN_set_bit();
int BN_clear_bit();
char *  BN_bn2hex();
char *  BN_bn2dec();
int     BN_hex2bn();
int     BN_dec2bn();
int BN_gcd();
BIGNUM *BN_mod_inverse();
BIGNUM *BN_generate_prime();
int BN_is_prime();
void    ERR_load_BN_strings();

PRE_CCONV BN_ULONG CCONV bn_mul_add_words();
PRE_CCONV BN_ULONG CCONV bn_mul_words();
PRE_CCONV void     CCONV bn_sqr_words();
BN_ULONG bn_div_words();
PRE_CCONV BN_ULONG CCONV bn_add_words();
PRE_CCONV BN_ULONG CCONV bn_sub_words();

int BN_mod_mul_montgomery();
int BN_from_montgomery();
BN_MONT_CTX *BN_MONT_CTX_new();
void BN_MONT_CTX_init();
void BN_MONT_CTX_free();
int BN_MONT_CTX_set();

void bn_mul_normal();
PRE_CCONV void bn_mul_comba8();
PRE_CCONV void bn_mul_comba4();
void bn_sqr_normal();
PRE_CCONV void bn_sqr_comba8();
PRE_CCONV void bn_sqr_comba4();
int bn_cmp_words();
void bn_mul_recursive();
void bn_mul_part_recursive();
void bn_sqr_recursive();
void bn_mul_low_normal();

void    BN_RECP_CTX_init();
BN_RECP_CTX *BN_RECP_CTX_new();
void    BN_RECP_CTX_free();
int BN_RECP_CTX_set();
int BN_mod_mul_reciprocal();
int BN_mod_exp_recp();
int BN_div_recp();

int BN_library_init();
int BN_library_init_small();
int BN_default_init();

BN_ME_METH_INFO *BN_get_default_exp_table();
BN_ME_METH_INFO *BN_bnme_get_info();
const BN_ME_METH *  (*BN_bnme_get())();
int     BN_bnme_set();
void        BN_bnme_clear();
int     BN_bnme_delete();
int     BN_bnme_insert();

#endif

#ifdef CPU_TMS320
#define BN_ME_METH_DEFAULT BN_ME_METH_dspc
#else
#define BN_ME_METH_DEFAULT BN_ME_METH_word
#endif
/* BEGIN ERROR CODES */
/* Error codes for the BN functions. */

/* Function codes. */
#define BN_F_BN_BN2DEC                   100
#define BN_F_BN_BN2HEX                   101
#define BN_F_BN_CTX_NEW                  102
#define BN_F_BN_DIV                  103
#define BN_F_BN_DIV_RECP                 104
#define BN_F_BN_EXPAND2                  105
#define BN_F_BN_MOD_EXP2_MONT                106
#define BN_F_BN_MOD_EXP_MONT                 107
#define BN_F_BN_MOD_INVERSE              108
#define BN_F_BN_MPI2BN                   109
#define BN_F_BN_NEW                  110
#define BN_F_BN_RAND                     111
#define BN_F_BN_USUB                     112

/* Reason codes. */
#define BN_R_ARG2_LT_ARG3                100
#define BN_R_BAD_RECIPROCAL              101
#define BN_R_CALLED_WITH_EVEN_MODULUS            102
#define BN_R_DIV_BY_ZERO                 103
#define BN_R_ENCODING_ERROR              104
#define BN_R_EXPAND_ON_STATIC_BIGNUM_DATA        105
#define BN_R_INVALID_LENGTH              106
#define BN_R_NO_INVERSE                  122
 
#ifdef  __cplusplus
}
#endif
#endif /* HEADER_COMMON_BN_H */

