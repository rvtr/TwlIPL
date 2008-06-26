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
# WL_IPL_RED_PRIVATE_ROOTが設定されていない場合は
# 既存のバージョンファイルからTADのみ生成する

TARGET_FIRM			=	SYSTEMMENU

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include ./commondefs.sysmenuVersion

VER_TIMESTAMP	=	08062300

# システムメニューバージョン
MAJOR_VERSION		=	0
MINOR_VERSION		=	1

# ユーザー領域サイズ(=128MB)
USER_AREA_SIZE		=	134217728

GEN_VERSION_FILE	=	$(SYSMENU_TOOLSDIR)/bin/genVersion.plx

VERSION_TAD			=	HNLA.tad

ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
	VERSION_DAT		=	$(SYSMENU_VERSION_FILE)
endif

TARGETS				+=	$(VERSION_TAD)

#----------------------------------------------------------------------------

LDIRT_CLEAN		=	$(VERSION_DAT) $(VERSION_TAD) properties

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build :  $(VERSION_TAD)

$(VERSION_DAT):
	$(GEN_VERSION_FILE) $(VER_TIMESTAMP) $(MAJOR_VERSION) $(MINOR_VERSION) $(USER_AREA_SIZE)

$(VERSION_TAD) : $(SYSMENU_VERSION_FILE)
	$(MAKETAD) $(call empath,$<) $(SYSMENU_VERSION_MAKETAD_OPTION) -o $@

#===== End of Makefile =====
