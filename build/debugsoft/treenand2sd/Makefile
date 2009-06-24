#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools - treenand
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

SUBDIR_FLAGS	=	TWL_ARCHGEN=$(TWL_ARCHGEN)
SUBDIRS			=	$(SDLOG_DIR)

TARGET_PLATFORM    := TWL

TWL_PROC           := ARM9
TWL_ARCHGEN        := LIMITED
TARGET_BIN          = treenand2sd.srl

SRCS                = main.cpp text.c MyStrings.c fontdata.c

SDLOG_DIR		= $(ROOT)/build/tests/sdlog/sdlog
LINCLUDES      += $(SDLOG_DIR)/include
LLIBRARY_DIRS   = $(SDLOG_DIR)/lib/$(TWL_BUILDTYPE)
LLIBRARIES      += libsdlog$(NITRO_LIBSUFFIX).a\

ROM_SPEC			=	demo.autogen.rsf
ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf
ROM_SPEC_PARAM		=	MakerCode=01				\
						GameCode=4NTA				\
						NANDAccess=TRUE				\
						SDCardAccess=TRUE			\
						Secure=TRUE					\
						CardRegion=ALL				\


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------
# セキュアアプリ指定

MAKEROM			 := $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

#----------------------------------------------------------------------------


#----------------------------------------------------------------------------
#追加するＣＣオプション
#----------------------------------------------------------------------------
CCFLAGS +=  -w nounwanted

ifeq	($(TWL_BUILD_TYPE),RELEASE)
INSTALL_DIR		= $(ROOT)/bin/ARM9-TS/Release
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
