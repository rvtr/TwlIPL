#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL
# File:     Makefile
#
# Copyright 2007 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2007-10-03#$
# $Rev: 1319 $
# $Author: kitase_hirotake $
#----------------------------------------------------------------------------

SUBDIRS				=

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL 	?=	True

TWL_PROC			=	ARM7

SRCDIR				=	. ../common
SRCS				=	\
						gcd.c \
						gcd_init.c \
						blowfish.c \
						ds_blowfish.c \
						ds_blowfish_table.c \
						gcdfirm_blowfish_table.c \

TARGET_LIB			=	libgcd_sp$(FIRM_LIBSUFFIX).a
INCDIR				=	../include

#----------------------------------------------------------------------------

include	$(TWLIPL_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR			=	$(FIRM_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLIPL_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
