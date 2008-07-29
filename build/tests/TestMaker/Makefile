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
# sub make �N��

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
ROMPARAM     = ./romparam.yaml
CODESLIST    = ./codeslist

-include $(CODESLIST)

build:  $(GAMECODES)

.PHONY: $(GAMECODES) clean config clstest fonttest parentaltest

#---- CLS �e�X�g�ݒ�
clstest:
	cp -f $(CLSROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "CLS"

#---- fontcode �e�X�g�ݒ�
fonttest:
	$(GENFONT) $(FONTCODES) $(ROMPARAM) "Reverse"
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "FONT"

#---- parentarlcontrol �e�X�g�ݒ�
parentaltest:
	$(GENPARENTAL) $(PCROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "PARENTAL"

#---- eula �e�X�g�ݒ�
eulatest:
	cp -f $(EULAROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "EULA"

#---- shop �e�X�g�ݒ�
shoptest:
	cp -f $(SHOPROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "SHOP"

#---- savedate �e�X�g�ݒ�
savetest:
	cp -f $(SAVEROMPARAM) $(ROMPARAM)
	$(CONFIG) $(ROMPARAM) $(CODESLIST) "SAVE"

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
# sub make �{��

SUBDIRS		=


#---- default 
BASEDIR	= $(ROOT)/build/demos/os/consoleType-1
SRCDIR	= $(BASEDIR)/src
INCDIR	= $(BASEDIR)/include
SRCS	= main.c font.c screen.c
BANNERSRC := $(wildcard ../banner/dummy/Cell/*.nce)


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

BANNER     = ./banner.bnr
MAKEBANNER = $(TWL_TOOLSDIR)/bin/makebanner.TWL.exe
BANNERCVTR = $(TWL_TOOLSDIR)/bin/bannercvtr.exe

#----------------------------------------------------------------------------

TARGET_BIN	=	twl_$(GAMECODE).$(TARGET_SUFFIX)
ROM_SPEC	=	twl_$(GAMECODE).rsf

#SRCDIR		=	# using default
#LCFILE		=	# using default

INSTALL_DIR		= ../roms
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib
include $(GAMECODE).param

#----------------------------------------------------------------------------

do-build:	$(TARGETS)

$(TARGETS): $(BANNER)

$(BANNER): $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf)
	$(MAKEBANNER) -A $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf) $@

$(BANNER:.bnr=.bin): $(BANNERSRC)
	$(BANNERCVTR) -o $@ $<

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


# sub make �{��
#------------------------------------------------------------------------------------
endif

#===== End of Makefile =====