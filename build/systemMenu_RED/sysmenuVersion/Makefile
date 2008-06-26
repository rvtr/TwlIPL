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

VER_TIMESTAMP	=	08062300

# システムメニューバージョン
MAJOR_VERSION		=	0
MINOR_VERSION		=	1

# ユーザー領域サイズ(=128MB)
USER_AREA_SIZE		=	134217728

VERSION_FILE	=	SysmenuVersion.dat

GEN_VERSION_FILE	=	$(SYSMENU_TOOLSDIR)/bin/genVersion.plx

VERSION_MAKETAD_OPTION		+=	-s -d 0003000F484E4c41 3031 0 SYSM_VERSION -v 0 -p

VERSION_TAD			=	HNLA.tad

ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
	VERSION_DAT		=	$(VERSION_FILE)
endif

TARGETS				+=	$(VERSION_TAD)

#----------------------------------------------------------------------------

LDIRT_CLEAN		=	$(VERSION_DAT) $(VERSION_TAD) properties

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build :  $(VERSION_TAD)

$(VERSION_DAT):
	$(GEN_VERSION_FILE) $(VER_TIMESTAMP) $(MAJOR_VERSION) $(MINOR_VERSION) $(USER_AREA_SIZE)

$(VERSION_TAD) : $(VERSION_FILE)
	$(MAKETAD) $(call empath,$<) $(VERSION_MAKETAD_OPTION) -o $@

#===== End of Makefile =====
