#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - ETicketViewer
# File:     Makefile
#
# Copyright 2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 
# $Rev: 
# $Author: 
#----------------------------------------------------------------------------
override TARGET_PLATFORM	=	TWL

TARGET_BIN		=	TicketViwer.srl

INCDIR			=	./include $(ROOT)/build/libraries/os/common/include
SRCDIR			=	./src

SRCS			=	main.c screen.c font.c common.c

LLIBRARIES		=	liblcfg$(TWL_LIBSUFFIX).a \
					libes$(TWL_LIBSUFFIX).a		\
					libboc$(TWL_LIBSUFFIX).a	\
					libsfs$(TWL_LIBSUFFIX).a	\
					libnam$(TWL_LIBSUFFIX).a	\
					libsea$(TWL_LIBSUFFIX).a	\
					libna$(TWL_LIBSUFFIX).a

# libnamut$(TWL_LIBSUFFIX).a\

ROM_SPEC		 =	./ROM-TS_sys.rsf

COMPONENT_NAME		=	armadillo_SYSMENU
MAKEROM_ARM7_BASE   =   $(TWL_COMPONENTSDIR)/$(COMPONENT_NAME)/$(TWL_BUILDTYPE_ARM7)/$(COMPONENT_NAME)
MAKEROM_ARM7        =   $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

MAKEROM			:=		$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====


