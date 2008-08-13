#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - libraries - pm
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
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS		=
SUBMAKES	=


#----------------------------------------------------------------------------

# build ARM & THUMB libraries
override TWL_CODEGEN 	= ALL

# Codegen for sub processer
TWL_PROC		= ARM7

INCDIR			= $(ROOT)/build/libraries/spi/ARM7/pm/include \

SRCDIR			= .

SRCS			= \
				pm_init.c \
				pm_pmic_ex.c \

TARGET_LIB		= libpm_sp$(FIRM_LIBSUFFIX).a

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(FIRM_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
