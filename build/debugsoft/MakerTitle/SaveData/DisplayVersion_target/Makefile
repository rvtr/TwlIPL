#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - save_data - DisplayVersion
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
# $Date::$
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS		=


#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL
TWL_ARCHGEN		:= LIMITED
TWL_NANDAPP		:= TRUE

SRCS		=	main.c font.c screen.c menu.c dataver.c
MASTER_NAME	=	dispver
APP_NAME        =	$(MASTER_NAME)_TARGET
TARGET_BIN	=	$(APP_NAME).tad

#-------------------------
#-- NAND アプリではいくつかのパラメータの指定のために固有の RSF ファイルが必要です。

ROM_SPEC			=	test.autogen.rsf
ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_nand.rsf
ROM_SPEC_PARAM		=	MakerCode=01					\
						TitleName=VER_CHECKER			\
						GameCode=4VDA					\
						BannerFile=./banner/banner_TARGET.bnr	\
						PublicSaveDataSize=16K			\
						PrivateSaveDataSize=16K		


#-------------------------
#-- カードアプリと同様に ROM-FS にファイルを持たせます。

#MAKEROM_ROMROOT	 = ./rom_data
#MAKEROM_ROMFILES = *.*

#-------------------------
# インストール指定

ifneq	($(TWL_IPL_RED_ROOT),)
INSTALL_DIR	= $(TWL_IPL_RED_ROOT)/debugsoft/$(MASTER_NAME)
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif


#-------------------------
#-- バナーデータを生成します
BANNER          = ./banner/banner.bnr
BANNERSRC      := $(wildcard ./banner/data/Cell/*.nce)
MAKEBANNER      = $(TWL_TOOLSDIR)/bin/makebanner.TWL.exe
BANNERCVTR      = $(TWL_TOOLSDIR)/bin/bannercvtr.exe

#-------------------------
# rom 内のファイルは全て自動生成なので clean で削除されるようにします
LDIRT_CLEAN		 =	$(MAKEROM_ROMROOT)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
#include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

#.PHONY:	build_time

do-build:		$(TARGETS)


$(BINDIR)/$(TARGET_BIN_BASENAME).$(TWL_ELF_EXT):	make_dir
#build_time

#-------------------------
#-- rom_data/build_time.txt を作成します。

make_dir:
	$(INSTALL) -d $(MAKEROM_ROMROOT)
#build_time:
#	$(INSTALL) -d $(MAKEROM_ROMROOT)
#	echo -n "build at `date \"+%Y/%m/%d %H:%M:%S\"` on \"`hostname`\"" > $(MAKEROM_ROMROOT)/build_time.txt

#-------------------------
#-- バナー作成用ターゲット

$(BANNER): $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf)
	$(MAKEBANNER) -A $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf) $@

$(BANNER:.bnr=.bin): $(BANNERSRC)
	$(BANNERCVTR) -o $@ $<


include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
