#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - tmpjumpTest - AppforFailedTmpJump
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
# $Date::            $
# $Rev:$
# $Author:$
#----------------------------------------------------------------------------

override TARGET_CODEGEN		=		ARM
TWL_ARCHGEN					=		LIMITED

TWL_NANDAPP		=	TRUE

TARGET_BIN		=	Test_$(TARGET_PLATFORM)fail.srl

INCDIR	=	../include
SRCDIR	=	../src
SRCS	=	common.c font.c screen.c main_f.c

MAKEROM_ROMROOT		=	../data
MAKEROM_ROMFILES	=	dummy.srl

#----------------------------------------------------------------------------
include $(TWLSDK_ROOT)/build/buildtools/commondefs

ifeq ($(TARGET_PLATFORM),TWL)
ROM_SPEC			=	main_459a.rsf
# LIBSYSCALL			=	../459A/libsyscall.a

endif

ifeq ($(TARGET_PLATFORM),NITRO)
ROM_SPEC			=	main_458a.rsf
# LIBSYSCALL			=	../458A/libsyscall.a

endif

INSTALL_TARGETS		=	$(BINDIR)/$(TARGET_BIN)
INSTALL_DIR			=	../data

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
