#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - CheckPreloadParameters
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

TARGET_PLATFORM	=	TWL
TWL_ARCHGEN   = LIMITED

# nandapp Ç…Ç∑ÇÈèÍçáÇÕÇ±ÇÃï”Ç∆Ç©ÇÃÉRÉÅÉìÉgÇÇÕÇ∏Ç∑
# rsfÇÃï˚Ç‡mediaÇNandÇ…Ç∑ÇÈi

TWL_NANDAPP = TRUE


SRCS			=	main.c font.c screen.c keypad.c
LINCLUDES		=	./include
SRCDIR			=	./src
 #TARGET_BIN		=	NewApp.srl
TARGET_BIN	=	JumpToSetting.tad
ROM_SPEC		=	main.rsf


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

MAKEROM			=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe
DEFAULT_MAKEROM_ARM7_BASE	= $(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
DEFAULT_MAKEROM_ARM7		= $(DEFAULT_MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

 MAKETAD_OPTION	+=	-s
#----------------------------------------------------------------------------

SUBDIRS			=	

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
