#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - libraries - init
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

# Codegen for sub processer
override TWL_PROC	= 	ARM7

# Avoid to use pre-compile-header of "nitro.h"
TWL_NO_STD_PCHDR        = 	True

# Avoid to build THUMB version object
override TWL_CODEGEN	= 	ARM

SRCS			= 	crt0_firm.c

TARGET_OBJ		= 	crt0_firm.o

MACRO_FLAGS		+=	-DSDK_NOINIT

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(FIRM_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
