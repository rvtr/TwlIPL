#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools - WiFiIcon
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

override TARGET_PLATFORM	= TWL
override TWL_CODEGEN		= ARM
override TWL_ARCHGEN		= LIMITED


#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL
TWL_ARCHGEN		:= LIMITED
SRCS			 =	main.c
TARGET_NAME		:=	WiFiIcon
TARGET_BIN		:=	main.tad
TWLNMENU_ROOT	?=	.

TWL_NANDAPP		=	TRUE

#-------------------------
#-- バナーデータを生成します
BANNER          = ./banner/banner.bnr
BANNERSRC      := $(wildcard ./banner/data/Cell/*.nce)
MAKEBANNER      = $(TWL_TOOLSDIR)/bin/makebanner.TWL.exe
BANNERCVTR      = $(TWL_TOOLSDIR)/bin/bannercvtr.exe


#-------------------------
#-- NAND アプリではいくつかのパラメータの指定のために固有の RSF ファイルが必要です。 

ROM_SPEC			=	$(TARGET_NAME).autogen.rsf
ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf
ROM_SPEC_PARAM		=	MakerCode=01				\
						GameCode=4S02				\
						BannerFile=./banner/banner.bnr	\
						TitleName=$(TARGET_NAME)	\
						Media=NAND				\
						WiFiConnectionIcon=TRUE		\
						Secure=TRUE
#						DSWirelessIcon=TRUE			\



include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#-------------------------
# セキュア用 
MAKEROM			:=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe
MAKETAD_OPTION	:=	-s


#-------------------------
# ビルドパラメータ 
INCDIR			=	$(TWLSYSTEM_ROOT)/include	\
					./include

#-------------------------
# インストール指定
ifneq	($(TWL_IPL_RED_ROOT),)
INSTALL_DIR	= $(TWL_IPL_RED_ROOT)/debugsoft/$(TARGET_NAME)
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif


#-------------------------
# ビルド
do-build:		$(TARGETS)

#-------------------------
#-- SRL を作成する前にバナーが作成されるようにします。

#ifdef	MAKE_BANNER
#$(BINDIR)/$(TARGET_NAME)$(ROM_SPEC_NAME_PART).srl: $(BANNER)
#endif

#-------------------------
#-- バナー作成用ターゲット

$(BANNER): $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf)
	$(MAKEBANNER) -A $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf) $@

$(BANNER:.bnr=.bin): $(BANNERSRC)
	$(BANNERCVTR) -o $@ $<

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
