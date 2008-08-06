#! make -f
#----------------------------------------------------------------------------
# Project:  TwlWiFi - NCFG - demos - ncfg-1
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

SUBDIRS			 =	$(ROOT)/build/demos/gx/UnitTours/DEMOLib

#----------------------------------------------------------------------------
TARGET_PLATFORM		= TWL
TWL_ARCHGEN		:= LIMITED
TWL_NANDAPP	:=	TRUE
TARGET_NAME	= NCFGCrusher
TARGET_BIN	= $(TARGET_NAME).srl
#TARGET_BIN	= $(TARGET_NAME).tad
SRCS		= main.c


ROM_SPEC			=	$(TARGET_NAME).autogen.rsf
ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf
ROM_SPEC_PARAM		=	MakerCode=01				\
						BannerFile=./banner/banner.bnr	\
						GameCode=4S03				\
						TitleName=$(TARGET_NAME)	\
						AppType=USER		\
#						Media=NAND			\


MAKEROM			:=	$(TWL_TOOLSDIR)/bin/makerom.TWL.exe


#-------------------------
# インストール指定
ifneq	($(TWL_IPL_RED_ROOT),)
INSTALL_DIR	= $(TWL_IPL_RED_ROOT)/debugsoft/$(TARGET_NAME)
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif




#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLWIFI_ROOT)/build/buildtools/commondefs.demos
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib


#----------------------------------------------------------------------------

do-build:	$(TARGETS)

include	$(TWLWIFI_ROOT)/build/buildtools/modulerules
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
