#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - libraries - aes
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
# $Date::            $
# $Rev:$
# $Author:$
#----------------------------------------------------------------------------

SUBDIRS		=
SUBMAKES	=


#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL 	?= TRUE

SRCDIR			= . ../common

SRCS			= \
				aes_util.c \

TARGET_LIB		= libaes$(FIRM_LIBSUFFIX).a

#----------------------------------------------------------------------------

include	$(TWLIPL_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(FIRM_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLIPL_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
