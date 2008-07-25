#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL
# File:     Makefile -
#
# Copyright 2007 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 
# $Rev: 
# $Author: 
#----------------------------------------------------------------------------

# バージョンファイルとTADを生成する
# TWL_IPL_RED_PRIVATE_ROOTが設定されていない場合は
# 既存のバージョンファイルからTADのみ生成する

# 現在、製品版の証明書を含んだものは生成しない

override TARGET_PLATFORM  := TWL
override TWL_ARCHGEN    := LIMITED

TARGET_FIRM     = SYSTEMMENU
TWL_NANDAPP     = TRUE

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include ./commondefs.sysmenuVersion

REGION        ?=  A
DST_DIR       = dev

# タイムスタンプ
VER_TIMESTAMP   = 08062300

# システムメニューバージョン
MAJOR_VERSION   = 0
MINOR_VERSION   = 1
STR_VERSION     = $(MAJOR_VERSION).$(MINOR_VERSION)$(REGION)

# ユーザー領域サイズ(=128MB)
USER_AREA_SIZE    = 134217728

# NUPホストネーム
NUP_HOSTNAME    = nus.t.shop.nintendowifi.net:443

# EULAのURL
EULA_URL      = https://cfh.t.app.nintendowifi.net/eula/

GEN_VERSION_PARAM = $(DST_DIR) $(VER_TIMESTAMP) $(STR_VERSION) $(MAJOR_VERSION) $(MINOR_VERSION) \
            $(USER_AREA_SIZE) $(NUP_HOSTNAME) $(EULA_URL)

GEN_VERSION_FILE  = $(SYSMENU_TOOLSDIR)/bin/genVersion.plx

VERSION_TAD     = HNL$(REGION).tad

ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
  VERSION_DAT   = $(SYSMENU_VERSION_FILE)
endif

TARGETS       +=  $(VERSION_TAD)

#----------------------------------------------------------------------------

LDIRT_CLEAN     = $(VERSION_TAD) properties

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build :  $(VERSION_TAD)

$(VERSION_DAT): ./Makefile ./commondefs.sysmenuVersion
	$(GEN_VERSION_FILE) $(GEN_VERSION_PARAM)

$(VERSION_TAD) : $(SYSMENU_VERSION_FILE)
	$(MAKETAD) $(call empath,$<) $(SYSMENU_VERSION_MAKETAD_OPTION) -o $@

#===== End of Makefile =====
