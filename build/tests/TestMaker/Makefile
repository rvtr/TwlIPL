#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - GX - demos - UnitTours/3D_Shininess
# File:     Makefile
#
# Copyright 2003 Nintendo.  All rights reserved.
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

ifeq ($(GAMECODE),)
#------------------------------------------------------------------------------------
# sub make 起動

CONFIG       = ./script/config.py
GENCLS       = ./script/clsdir.py
GENFONT      = ./script/fontcode.py
GENPARENTAL  = ./script/parentalcontrol.py
FONTCODES    = ./doc/20080617_DS_fontcodeList_NOE_FIANL.xls
CLSROMPARAM  = ./config/romparam_forCLStest.yaml
PCROMPARAM   = ./config/twl_parentalcontrol_spec.yaml
EULAROMPARAM = ./config/romparam_forEULAtest.yaml
SHOPROMPARAM = ./config/romparam_forShoptest.yaml
SAVEROMPARAM = ./config/romparam_forSavetest.yaml
FATFSPERMISSIONROMPARAM = ./config/romparam_forFATFSPermissiontest.yaml
BOOTROMPARAM = ./config/romparam_forBoottest.yaml
ROMPARAM     = ./romparam.yaml
CODESLIST    = ./codeslist

-include $(CODESLIST)

build:  $(GAMECODES)

.PHONY: $(GAMECODES) clean config clstest fonttest parentaltest

#---- CLS テスト設定
clstest:
	cp -f $(CLSROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "CLS"

#---- fontcode テスト設定
fonttest:
	$(GENFONT) $(FONTCODES) $(ROMPARAM) "Reverse"
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "FONT"

#---- parentarlcontrol テスト設定
parentaltest:
	$(GENPARENTAL) $(PCROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "PARENTAL"

#---- eula テスト設定
eulatest:
	cp -f $(EULAROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "EULA"

#---- shop テスト設定
shoptest:
	cp -f $(SHOPROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "SHOP"

#---- savedate テスト設定
savetest:
	cp -f $(SAVEROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "SAVE"

#---- fatfspermissiontest テスト設定
fatfspermissiontest:
	cp -f $(FATFSPERMISSIONROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "FATFSPERMISSION"

#---- boottest テスト設定
boottest:
	cp -f $(BOOTROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "BOOT"

#---- config
config:
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "DEFAULT"

$(GAMECODES): $(CODESLIST)
	install -d $@ && $(MAKE) -k -C $@ -f ../Makefile GAMECODE=$@ TEST_TYPE=$(TEST_TYPE)

forcls:
	$(GENCLS) $(ROMPARAM)

clean:
	rm -Rf $(GAMECODES)
	rm -f  $(CODESLIST)
	rm -Rf roms
	rm -Rf forCLS
	
else
#------------------------------------------------------------------------------------
# sub make 本体

SUBDIRS		=

#---- sysm_red_misc
MISC_DIR	= ../../../systemMenu_RED/misc

#---- default 
BASEDIR	= $(ROOT)/build/demos/os/consoleType-1
SRCDIR	= $(BASEDIR)/src
INCDIR	= $(BASEDIR)/include
SRCS	= main.c font.c screen.c
BANNERSRC := $(wildcard ../banner/dummy/Cell/*.nce)
INSTALL_DIR		= ../roms

TARGET_PLATFORM := TWL
TWL_ARCHGEN     := LIMITED
TWL_NANDAPP		:= TRUE

ifeq ($(TEST_TYPE),CLS)
BASEDIR	= $(ROOT)/build/demos/os/consoleType-1
SRCDIR	= $(BASEDIR)/src
INCDIR	= $(BASEDIR)/include
SRCS	= main.c font.c screen.c
BANNERSRC := $(wildcard ../banner/dummy/Cell/*.nce)
endif

ifeq ($(TEST_TYPE),FONT)
BASEDIR	= $(ROOT)/build/demos/gx/UnitTours/3D_BoxTest
SRCDIR	= $(BASEDIR)/src
INCDIR	= $(BASEDIR)/include
SRCS	= main.c data.c
BANNERSRC := $(wildcard ../banner/font/Cell/*.nce)
endif

ifeq ($(TEST_TYPE),PARENTAL)
BASEDIR	= $(ROOT)/build/tests/ogntest/
SRCDIR	= $(BASEDIR)/src
INCDIR	= $(BASEDIR)/include
SRCS	= main.c
BANNERSRC := $(wildcard ../banner/parental/Cell/*.nce)
endif

ifeq ($(TEST_TYPE),EULA)
BASEDIR	= $(ROOT)/build/demos/gx/UnitTours/3D_BoxTest
SRCDIR	= $(BASEDIR)/src
INCDIR	= $(BASEDIR)/include
SRCS	= main.c data.c
BANNERSRC := $(wildcard ../banner/eula/Cell/*.nce)
endif

ifeq ($(TEST_TYPE),SHOP)
BASEDIR	= $(ROOT)/build/tests/nandApp/40M
SRCDIR	= $(BASEDIR)/src
INCDIR	= $(BASEDIR)/include
SRCS	= main.c
BANNERSRC := $(wildcard ../banner/shop/Cell/*.nce)
MAKEROM_ROMROOT	 = $(ROOT)/build/tests/nandApp/40M/rom_data
MAKEROM_ROMFILES = *.*
endif

ifeq ($(TEST_TYPE),SAVE)
BASEDIR	= $(ROOT)/build/tests/save_data/DisplayVersion
SRCDIR	= $(BASEDIR)/src
INCDIR	= $(BASEDIR)/include
SRCS	= dataver.c font.c main.c menu.c screen.c
BANNERSRC := $(wildcard $(BASEDIR)/banner/data/Cell/*.nce)
endif

ifeq ($(TEST_TYPE),FATFSPERMISSION)
TARGET_FIRM				=	SYSTEMMENU
BASEDIR	= ../../RomTypeTest
SRCDIR	= $(BASEDIR)/ARM9/src
INCDIR	= $(BASEDIR)/ARM9/src $(MISC_DIR)/include
SRCS	= main.c RomTypeTest.c $(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c
BANNERSRC := $(wildcard ../banner/shop/Cell/*.nce)
INSTALL_DIR	= $(BASEDIR)/test_roms
USE_IPL_RED_LIB	= TRUE
MAKEROM_ROMROOT		=	../../../systemMenu_RED/
MAKEROM_ROMFILES	=	./data/NTR_IPL_font_m.NFTR
endif

ifeq ($(TEST_TYPE),BOOT)
TARGET_FIRM				=	SYSTEMMENU
BASEDIR	= ../../HashCheckTemplate
SUBDIRS	= $(BASEDIR)
SRCDIR	= $(BASEDIR)/ARM9/src
INCDIR	= $(BASEDIR)/ARM9/src $(MISC_DIR)/include
SRCS	= main.c HashCheckTemplate.c $(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c
BANNERSRC := $(wildcard ../banner/shop/Cell/*.nce)
INSTALL_DIR	= $(BASEDIR)/test_roms
USE_IPL_RED_LIB	= TRUE
MAKEROM_ROMROOT		=	../../../systemMenu_RED/
MAKEROM_ROMFILES	=	./data/NTR_IPL_font_m.NFTR
endif

BANNER     = ./banner.bnr
MAKEBANNER = $(TWL_TOOLSDIR)/bin/makebanner.TWL.exe
BANNERCVTR = $(TWL_TOOLSDIR)/bin/bannercvtr.exe

#----------------------------------------------------------------------------

TARGET_BIN	=	twl_$(GAMECODE).$(TARGET_SUFFIX)
ROM_SPEC	=	twl_$(GAMECODE).rsf

#SRCDIR		=	# using default
#LCFILE		=	# using default

INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)

ifeq ($(USE_IPL_RED_LIB),TRUE)
include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
else
include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib
endif
include $(GAMECODE).param

# boottestの時だけARM7コンポーネントの指定
# このあたりの指定が苦肉すぎるので妙案募集中
ifeq ($(TEST_TYPE),BOOT)
DEFAULT_COMP_ARM7		= racoon_hct
DEFAULT_MAKEROM_ARM7_BASE	= $(BASEDIR)/ARM7/bin/$(TWL_BUILDTYPE_ARM7)/$(DEFAULT_COMP_ARM7)
DEFAULT_MAKEROM_ARM7		= $(DEFAULT_MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)
endif

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

$(TARGETS): $(BANNER)

$(BANNER): $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf)
	$(MAKEBANNER) -A $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf) $@

$(BANNER:.bnr=.bin): $(BANNERSRC)
	$(BANNERCVTR) -o $@ $<

ifeq ($(USE_IPL_RED_LIB),TRUE)
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules
else
include	$(TWLSDK_ROOT)/build/buildtools/modulerules
endif

# sub make 本体
#------------------------------------------------------------------------------------
endif

#===== End of Makefile =====
