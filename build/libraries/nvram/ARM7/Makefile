#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - spi
# File:     Makefile
#
# Copyright 2003 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date::            $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS				=

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
override TWL_CODEGEN 	= ALL

TWL_PROC			=	ARM7

SRCDIR				=	. ../common
SRCS				=	\
						nvram_misc.c \

TARGET_LIB			=	libnvram_sp$(FIRM_LIBSUFFIX).a
INCDIR				=	../include
LINCLUDES		=	$(ROOT)/build/libraries/spi/ARM7/include ../common/include

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR			=	$(FIRM_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
