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
/**
 * @file bn_me.c
 * @brief BN method functions and utilities
 */
#include "bn_lcl.h"
#include "bn_thx.h"

#if !(defined(NO_SPLIT) && defined(SPLIT_FILE))

#ifdef NO_SPLIT
#define SPLIT_BN_MOD_EXP_METH_DEFAULT
#define SPLIT_BN_MONT_CTX_USEIT
#define SPLIT_BN_ME_CTX_NAME
#define SPLIT_BN_ME_CTX_NEW
#define SPLIT_BN_ME_CTX_FREE
#define SPLIT_BN_ME_CTX_SET
#define SPLIT_BN_ME_CTX_MOD_EXP
#endif /* NO_SPLIT */

#ifndef NOPROTO
BN_ME_METH *bn_mod_exp_meth_default(int fallback);
#else
BN_ME_METH *bn_mod_exp_meth_default();
#endif

#ifdef SPLIT_BN_MOD_EXP_METH_DEFAULT

/**
 *            Returns the default mod_exp method, choose first available in the
 *            method table or a hardwired default
 *
 * @param     fallback  [In] Flag for choice of method
 * 			- 0 use the first value in the method table
 *                      - 1 use the all encompassing default method
 *
 * @pre       BN_default_init() has been run setting up library
 *
 * @post
 *
 * @internal  uses global table <i>bnme[]</i> and counter <i>me_num</i>
 *
 * @internal  only read the BN_ME_METH_INFO table no need for locking
 *
 * @relates   BN_library_init
 * @relates   BN_default_init
 * @relates   BN_ME_CTX_new
 * @relates   BN_ME_CTX_free
 *
 */
BN_ME_METH *bn_mod_exp_meth_default(int fallback)
{
    const BN_ME_METH *tmp;

#ifndef BN_LIBRARY_SMALL
    BN_ME_METH_INFO *meth_info;

    if(!fallback && ((meth_info = BN_bnme_get_info(0)) != NULL))
    {
        tmp = meth_info->meth();
    }
    else
#endif
    {
        /* set in bn.h or override bn_thx.h */
        tmp = BN_ME_METH_DEFAULT();
    }
    return((BN_ME_METH *)tmp);
}
#endif

#ifdef SPLIT_BN_MONT_CTX_USEIT

#ifndef NOPROTO
int bn_mont_ctx_useit(int argi);
#else
int bn_mont_ctx_useit();
#endif

/**
 * Montgomery method default useit function
 * 
 * @param argi [In] reserved for future use
 *
 * @return 0 success
 */
int bn_mont_ctx_useit(int argi)
	{
	return(0);
	}
#endif

#ifdef SPLIT_BN_ME_CTX_NAME
/**
 * Retrieve the BN method name string
 * 
 * @param ctx  [In] Method
 * 
 * @pre  ctx is valid and not NULL
 *
 * @return string pointer
 */
char *BN_ME_CTX_name(BN_ME_CTX *ctx)
	{
	return(ctx->meth->name);
	}
#endif

#ifdef SPLIT_BN_ME_CTX_NEW
/**
 * Allocate a new BN_ME_CTX structure and assign method
 *
 * @param meth [In]  Method to assign 
 * @param in   [Out] Result BN_ME_CTX
 * 
 * @note if meth is NULL system method default is used
 *
 * @retval pointer to BN_ME_CTX success
 * @retval NULL failure
 */
BN_ME_CTX *BN_ME_CTX_new(BN_ME_METH *meth, BN_ME_CTX **in)
	{
	int i;
	BN_ME_CTX *ret;

	if (in == NULL)
		in= &ret;

	if (meth == NULL)
		meth=bn_mod_exp_meth_default(0);
	i=meth->init_ctx(meth,in);
	if (i)
		return(NULL);
	else
		return(*in);
	}
#endif

#ifdef SPLIT_BN_ME_CTX_FREE
/**
 * Free context method
 *
 * @param mctx  [In]  Context object to be deallocated
 * 
 * @note it is the callers responsibility to set mctx to NULL after free
 * 
 * @pre mctx is not NULL and is valide BN_ME_CTX
 */
void BN_ME_CTX_free(BN_ME_CTX *mctx)
	{
	mctx->meth->free_ctx(mctx);
	}
#endif

#ifdef SPLIT_BN_ME_CTX_SET
/**
 * Set value to BN_ME_CTX by identifier
 *
 * @param mctx  [In]  Method context object
 * @param b     [In]  Big number 
 * @param cmd   [In]  Command identifier
 * @param flags [In]  Mask operation directives
 * @param ctx   [In]  BN data store
 *
 * @note cmd values:
 * @li BN_ME_SET_MOD will cause a method lookup is method not set
 *
 * @note passes control to set method in mctx 
 *
 * @return result of mctx method set operation
 */
int BN_ME_CTX_set(BN_ME_CTX *mctx, BIGNUM *b, int cmd, int flags, BN_CTX *ctx)
	{
#ifndef BN_LIBRARY_SMALL
#ifndef NOPROTO
	const BN_ME_METH *(*meth)(void);
#else
	const BN_ME_METH *(*meth)();
#endif
#endif  /* BN_LIBRARY_SMALL */

	/* If we are loading the modulus, and we do not have the
	 * no_lookup flag (without a null method)
	 */
	if ((cmd == BN_ME_SET_MOD) && 
		((mctx->meth != NULL) && !(flags & BN_ME_SET_FLG_NO_LOOKUP)))
		{
#ifndef BN_LIBRARY_SMALL
		meth=(const BN_ME_METH*(*)(void))
				BN_bnme_get(BN_num_bits(b),BN_BNME_F_BITS);
		if (meth != NULL)
			mctx->meth=meth();
		else
#endif
			mctx->meth=bn_mod_exp_meth_default(1);
		}
	return(mctx->meth->set(mctx,b,cmd,flags,ctx));
	}
#endif

#ifdef SPLIT_BN_ME_CTX_MOD_EXP
/**
 * Call method mod_exp operation
 *
 * @param mctx   [In]   Method context object
 * @param ret    [Out]  Result
 * @param a      [In]   Base value
 * @param p      [In]   Exponent
 * @param m      [In]   Modulus
 * @param ctx    [In]   Data storage
 * 
 * @pre <i>a</i> must be expandable 
 *
 * @return 0 success
 */
int BN_ME_CTX_mod_exp(BN_ME_CTX *mctx, BIGNUM *ret, BIGNUM *a, BIGNUM *p, 
        BIGNUM *m, BN_CTX *ctx)
	{
	int wtop;

	/* Before we call the method, make sure we are not doing 0^p,
	 * and p is not 0 or 1 */
	if (BN_is_zero(a))
		{ BN_zero(ret); return(0); }
	if (p != NULL)
		{
		if (BN_is_zero(p))
			{ BN_one(ret); return(0); }
		else if (BN_is_one(p))
			{ BN_copy(ret,a); return(0); }
		}

	/* wtop is the size 'a' needs to be for the method */
	wtop=mctx->meth->num;
	bn_zexpand(a,wtop);
	
	return(mctx->meth->mod_exp(mctx,ret,a,p,ctx));
	}
#endif

#endif

