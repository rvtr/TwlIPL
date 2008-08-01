#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - appjumpTest - Nand-2
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
TWL_NANDAPP		:= TRUE

TARGET_BIN	= appjumpTestForNand2.tad
INCDIR		= ../include
SRCDIR		= ./src ../src
SRCS		= main.c screen.c font.c common.c

ROM_SPEC	=	main.autogen.rsf
ROM_SPEC_TEMPLATE	= $(ROOT)/include/twl/specfiles/ROM-TS_nand.rsf
ROM_SPEC_PARAM		=	MakerCode=03					\
						GameCode=432A					\
						RomVersion=0					\
						PermitLandingNormalJump=TRUE	\
						TitleName=AppJmpTestN2		\
						BannerFile=./banner/banner.bnr 

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/commondefs

#-------------------------
# インストール指定
ifneq	($(TWL_IPL_RED_ROOT),)
INSTALL_DIR	= $(TWL_IPL_RED_ROOT)/debugsoft/ApplicationJumpTest
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif

#-------------------------

do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
