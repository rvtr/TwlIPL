#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools - NandFiler
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
override TARGET_CODEGEN		= ARM
override TWL_ARCHGEN		= LIMITED



#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL
TWL_ARCHGEN		:= LIMITED
SRCS			 =	main.cpp
TARGET_NAME		=	sd2photo
TWLNMENU_ROOT	?=	.


#-------------------------
#-- バナーデータを生成します
BANNER          = ./banner/banner.bnr
BANNERSRC      := $(wildcard ./banner/data/Cell/*.nce)
MAKEBANNER      = $(TWL_TOOLSDIR)/bin/makebanner.TWL.exe
BANNERCVTR      = $(TWL_TOOLSDIR)/bin/bannercvtr.exe


#-------------------------
#-- NAND アプリではいくつかのパラメータの指定のために固有の RSF ファイルが必要です。 

ROM_SPEC			=	$(TARGET_NAME)$(ROM_SPEC_NAME_PART).autogen.rsf
ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf
ROM_SPEC_PARAM		=	MakerCode=01				\
						GameCode=4S01				\
						Secure=TRUE					\
						BannerFile=./banner/banner.bnr	\
						NANDAccess=TRUE				\
						SDCardAccess=TRUE	\
						TitleName=$(TARGET_NAME)

#-------------------------
#-- CARD, NANDによるオプションの指定

TWL_NANDAPP		=	TRUE

ifeq	($(APP_TYPE),)
APP_TYPE	=	CARD
endif

ifeq	($(APP_TYPE),CARD)
MEDIA	=	GameCard
SUFFIX	=	.srl
endif

ifeq	($(APP_TYPE),NAND)
MEDIA	=	NAND
SUFFIX	=	.tad
ROM_SPEC_NAME_PART	:=	$(ROM_SPEC_NAME_PART).nand
endif

ROM_SPEC_PARAM	+=	Media=$(MEDIA)
TARGET_BIN	=	$(TARGET_NAME)$(SUFFIX)

#-------------------------
# 対象拡張子のためのインクルードファイルを決定
ifneq	($(SUFFIX_DEFINE),)
SUFFIX_FILE	=	$(SUFFIX_DEFINE)
else
SUFFIX_FILE	=	default_suffix.h
endif


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
# 対象拡張子のためのインクルードファイルをビルドディレクトリにコピー
.PHONY: suffix

suffix:
	cp $(SUFFIX_FILE) ./include/suffix.h

#-------------------------
# ビルド
do-build:		suffix $(TARGETS)

#-------------------------
#-- SRL を作成する前にバナーが作成されるようにします。

ifdef	MAKE_BANNER
$(BINDIR)/$(TARGET_NAME)$(ROM_SPEC_NAME_PART).srl: $(BANNER)
endif


#-------------------------
#-- バナー作成用ターゲット

$(BANNER): $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf)
	$(MAKEBANNER) -A $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf) $@

$(BANNER:.bnr=.bin): $(BANNERSRC)
	$(BANNERCVTR) -o $@ $<

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
