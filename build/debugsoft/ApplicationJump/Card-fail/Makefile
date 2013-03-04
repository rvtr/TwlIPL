#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - appjumpTest - Card
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

TARGET_PLATFORM := TWL

TARGET_BIN	= appjumpTestForCard_Fail.srl
INCDIR		= ../include
SRCDIR		= ./src ../src
SRCS		= main.c screen.c font.c common.c

ROM_SPEC	=	appjumptest_card.rsf

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/commondefs

# インストール指定
ifneq	($(TWL_IPL_RED_ROOT),)
INSTALL_DIR	= $(TWL_IPL_RED_ROOT)/debugsoft/ApplicationJumpTest
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif


do-build:	$(TARGETS)

#----------------------------------------------------------------------------

LIBSYSCALL			 = ./460A/libsyscall.a

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
