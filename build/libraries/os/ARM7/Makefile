#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - libraries_sp - os
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

SUBDIRS			=

#----------------------------------------------------------------------------

# build ARM & THUMB libraries
TWL_CODEGEN_ALL 	?= TRUE

# Codegen for sub processer
TWL_PROC		= ARM7

SRCDIR			= ../common .

SRCS			= \
				os_init_firm.c \
				os_boot.c \

TARGET_LIB		= libos_sp$(FIRM_LIBSUFFIX).a


#----------------------------------------------------------------------------

include	$(TWLIPL_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(FIRM_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLIPL_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
