#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - tmpjumpTest - AppforTmpJump
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
# $Rev$
# $Author$
#----------------------------------------------------------------------------

override TARGET_CODEGEN		=		ARM
TWL_ARCHGEN					=		ALL

TWL_NANDAPP		=	TRUE

TARGET_BIN		=	TestApp$(TEST_ARCH).srl

INCDIR	=	../include
SRCDIR	=	./ ../src
SRCS	=	common.c font.c screen.c main_2.c

ifeq ($(ROMTYPE),MASTER)
MAKEROM_ROMROOT		=	../mas_data
else
MAKEROM_ROMROOT		=	../data
endif
MAKEROM_ROMFILES	=	dummy.srl

#----------------------------------------------------------------------------
include $(TWLSDK_ROOT)/build/buildtools/commondefs

ifeq ($(TARGET_PLATFORM),TWL)
TEST_ARCH			=	_$(TWL_ARCHGEN)

ifeq ($(TWL_ARCHGEN),HYBRID)

ROM_SPEC			=	main_434a.rsf
LIBSYSCALL			=	../434A/libsyscall.a

else

ROM_SPEC			=	main_435a.rsf
LIBSYSCALL			=	../435A/libsyscall.a

endif
# TARGET_PLATFORM = NITRO
endif

ifeq ($(TARGET_PLATFORM),NITRO)

TEST_ARCH			=	
ROM_SPEC			=	main_433a.rsf
LIBSYSCALL			=	../433A/libsyscall.a

endif

INSTALL_TARGETS		=	$(BINDIR)/$(TARGET_BIN)

ifeq ($(ROMTYPE),MASTER)
INSTALL_DIR		=	../mas_data
else
INSTALL_DIR			=	../data
endif
#----------------------------------------------------------------------------

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
