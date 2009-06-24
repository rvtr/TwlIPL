#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - DisableDebugFlgChecker
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
TARGET_FIRM	= SYSTEMMENU
TARGET_PLATFORM	=	TWL
TWL_ARCHGEN   = LIMITED
TWL_NANDAPP	=	TRUE

TARGET_BIN		=	DisableDebugFlgChecker.srl

INCDIR			=	./include $(ROOT)/build/libraries/os/common/include
SRCDIR			=	./src

SRCS			=	main.c screen.c font.c common.c

LLIBRARIES		+=	libes$(TWL_LIBSUFFIX).a		\
					libboc$(TWL_LIBSUFFIX).a	\
					libsfs$(TWL_LIBSUFFIX).a
					
#----------------------------------
# sea を使用

LLIBRARIES		+= libsea$(TWL_LIBSUFFIX).a

#----------------------------------
# nam を使用

LLIBRARIES		+=	libnam$(TWL_LIBSUFFIX).a


ROM_SPEC		 =	main.rsf

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

MAKETAD_OPTION	:=	-s

MAKEROM			=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe
DEFAULT_MAKEROM_ARM7_BASE	= $(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
DEFAULT_MAKEROM_ARM7		= $(DEFAULT_MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====


