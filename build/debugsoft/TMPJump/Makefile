#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - tmpjumpTest
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

SUBDIRS		=	

#----------------------------------------------------------------------------

override TARGET_PLATFORM		=	TWL

# GameCard, NAND の両方を作成する

ifndef APP_TYPE

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:	do-something
clean:		do-something
clobber:	do-something
install:	do-something
do-build:	MY_GOAL=default
clean:		MY_GOAL=clean
clobber:	MY_GOAL=clobber
install:	MY_GOAL=install

do-something:
	+$(REMAKE) APP_TYPE=CARD SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) APP_TYPE=NAND SKIP_SUBDIR=FALSE $(MY_GOAL)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

else		# ifndef APP_TYPE

TWL_NANDAPP		=	TRUE
TARGET_NAME		= tmpjumpTest

ifeq ($(APP_TYPE),NAND)
TARGET_BIN	=	$(TARGET_NAME).tad
ROM_SPEC			=	main_437a.rsf

else
TARGET_BIN	=	$(TARGET_NAME).srl
ROM_SPEC			=	main_436a.rsf
endif

endif		# ifndef APP_TYPE

INCDIR		=	./include
SRCDIR		=	./src

SRCS		=	main.c common.c screen.c font.c


MAKEROM_ROMROOT		=	./data
MAKEROM_ROMFILES	=	TestApp.srl			\
						TestApp_HYBRID.srl	\
						TestApp_LIMITED.srl

#----------------------------------------------------------------------------
include $(TWLSDK_ROOT)/build/buildtools/commondefs

# インストール指定
ifneq	($(TWL_IPL_RED_ROOT),)
INSTALL_DIR	= $(TWL_IPL_RED_ROOT)/debugsoft/TMPJumpTest
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
