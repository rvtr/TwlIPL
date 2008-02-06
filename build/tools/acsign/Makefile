#! make -f
#----------------------------------------------------------------------------
# Project:  NitroSDK - libraries - acsign
# File:     Makefile
#
# Copyright 2003,2004 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $NoKeywords: $
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
# Codegen for sub processer
NITRO_PROC	    	=	ARM9

# build ARM & THUMB libraries
NITRO_CODEGEN_ALL	=	True

#----------------------------------------------------------------------------

SUBDIRS		=


#----------------------------------------------------------------------------

SRCDIR		= 	./src
INCDIR		= 	./include $(ROOT)/build/libraries/mb/include


SRCS		=	acsign.c acmemory.c acsign_util.c

TARGET_LIB	=	libacsign_x86.a


include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

# CCFLAGS	        +=	-DOPT_32_BIT -DSTANDALONE -DNO_SPLIT -DNO_FP_API -DNO_R_DIAG -DNO_STDIO_H -DNO_STDLIB_H
CCFLAGS	+=	-DSMALL_CODE_SIZE \
		-DSTANDALONE \
		-DOPT_32_BIT \
		-DNO_SPLIT \
		-DNO_FP_API \
		-DNO_R_DIAG \
		-DNO_STDIO_H \
		-DNO_STDLIB_H

INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR		=	.


do-build:	$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
