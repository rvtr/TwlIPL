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

/*
 * ***************************************************************************
 *
 *  Purpose:
 *
 * 	This file defines an API for performing stack diagnostics.
 *
 * 	It provides:
 * 		Peek Stack Usage statistics
 * 	To use it:
 * 		1. Build the library without "NO_R_DIAG" defined
 * 		2. Make a call to R_DIAG_set_stack_datum() prior to
 * 		   calling the first library function.
 * 		3. Add calls to the macro R_DIAG_CHECK_STACK where potential
 * 		   high stack usage points are.
 * 		4. Call R_DIAG_get_stack_low_water_mark() at then end of the
 * 		   library calls to return the maximum stack usage OR
 * 		5. Call R_DIAG_print_stack_depth() to display the peek stack
 * 		   usage
 *
 *     	
 *
 * History:
 *   3-Aug-00	mjs	development
 *
 * **************************************************************************
 */


#ifndef HEADER_COMMON_R_STDIAG_H
#define HEADER_COMMON_R_STDIAG_H

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef NO_R_DIAG

#ifdef NOPROTO
int R_DIAG_set_stack_datum(char *ptr);
int R_DIAG_check_stack(char *ptr, char *file, int line, int hit);
int R_DIAG_get_stack_low_water_mark(int *depth, char **file, int *line,
	int *hit);
int R_DIAG_print_stack_depth(BIO *bio);

#else

int R_DIAG_set_stack_datum(); 
int R_DIAG_check_stack();
int R_DIAG_get_stack_low_water_mark();
int R_DIAG_print_stack_depth();

#endif

#define R_DIAG_CHECK_STACK {\
	char r_diag_stack_check_var = 0;\
	static int r_diag_stack_check_cnt = 0;\
\
	R_DIAG_check_stack(&r_diag_stack_check_var, __FILE__,\
		__LINE__,++r_diag_stack_check_cnt);\
	}
#else
#define R_DIAG_CHECK_STACK
#endif 


#ifdef  __cplusplus
}
#endif
#endif /* HEADER_COMMON_R_STDIAG_H */

