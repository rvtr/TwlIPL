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
FIRM_CODEGEN_ALL 	?=	True



SRCDIR				=	. ../common
SRCS				=	\
						gcd.c \
						gcd_init.c \
						blowfish.c \
						ds_blowfish.c \
						ds_blowfish_table.c \
						gcdfirm_blowfish_table.c \

TARGET_LIB			=	libgcd$(FIRM_LIBSUFFIX).a
INCDIR				=	../include

#----------------------------------------------------------------------------

# DEBUG版ビルドの場合、RELEASE版でビルドして
# DEBUG版のライブラリを装います。

ifdef	NITRO_DEBUG
NITRO_BUILD_TYPE	=	RELEASE
endif

include	$(TWLFIRM_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR			=	$(FIRM_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLFIRM_ROOT)/build/buildtools/modulerules

LDEPENDS_OBJ      := $(MAKEFIRM_RSA_PUBKEY) \

gcd_acsign.c : $(LDEPENDS_OBJ)
	touch gcd_acsign.c

#===== End of Makefile =====
