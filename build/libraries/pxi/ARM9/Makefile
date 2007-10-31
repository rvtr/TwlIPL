#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - libraries - pxi
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
FIRM_CODEGEN_ALL 	?= TRUE

SRCDIR		= ../common .

SRCS			= \
				pxi_misc.c \

TARGET_LIB		= libpxi$(FIRM_LIBSUFFIX).a

include	$(TWLIPL_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(FIRM_INSTALL_LIBDIR)

ifdef NITRO_CALLTRACE
CCFLAGS			+= -DOS_PROFILE_AVAILABLE -DOS_PROFILE_CALL_TRACE
endif

ifdef NITRO_FUNCTIONCOST
CCFLAGS			+= -DOS_PROFILE_AVAILABLE -DOS_PROFILE_FUNCTION_COST
endif

ifdef NITRO_TCM_APPLY
CCFLAGS			+= -DSDK_TCM_APPLY
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLIPL_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
