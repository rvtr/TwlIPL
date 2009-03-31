#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - build
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

# 製品技術部用リリース作成Makefile

TARGET_FIRM = SYSTEMMENU

# --------------------------------
# ツリーへのExport

ifdef TWLSYSTEM_ROOT
export NITROSYSTEM_ROOT	=	$(TWLSYSTEM_ROOT)
endif

export TARGET_CODEGEN	=	ALL

# --------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

# check mandatory packages

ifneq ($(TWLSDK_PRIVATE),TRUE)
$(error "TwlSDK-private is needed.")
endif

ifneq ($(TWLSDK_SYSMENU_PRIVATE),TRUE)
$(error "TwlSDK-SysMenu-private is needed.")
endif

ifneq ($(TWLSDK_SECURE7_PRIVATE),TRUE)
$(error "TwlSDK-Secure7-private is needed.")
endif

ifneq ($(TWLSDK_TWL_WIRELESS_PRIVATE),TRUE)
$(error "TwlSDK-TwlWireless-private is needed.")
endif

#----------------------------------------------------------------------------

SUBDIRS =	\
			$(TWL_IPL_RED_ROOT)/build/libraries_sysmenu/sysmenu/ARM9		\
			$(TWL_IPL_RED_ROOT)/build/libraries_sysmenu/namut/ARM9			\
			$(TWL_IPL_RED_ROOT)/build/libraries_sysmenu/reloc_info/ARM7		\
			$(TWL_IPL_RED_ROOT)/build/libraries_sysmenu/hotsw/ARM7			\
			$(TWL_IPL_RED_ROOT)/build/tests/TestComponent/ARM7.TWL

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

FIND				:= find
ARCHIVE_ROOT		?=	.
PACKING_DATE_AND_TIME	?= $(shell date +"%Y%m%d %H%M")
PACKING_DATE		?= $(word 1,$(PACKING_DATE_AND_TIME))
PACKING_TIME		?= $(word 2,$(PACKING_DATE_AND_TIME))
ARCHIVE				?= TwlIPL-uji-$(PACKING_DATE)-$(PACKING_TIME).zip
PACKAGE_TMP			?= ./TwlIPL

.PHONY: package

package:
	@echo --- $@ ---;														\
	mkdir -p $(PACKAGE_TMP);												\
	$(CP) $(TWL_IPL_RED_ROOT)/setup $(PACKAGE_TMP)/setup;					\
	mkdir -p $(PACKAGE_TMP)/lib;											\
	$(CP) $(TWL_IPL_RED_ROOT)/lib/ARM9-TS $(PACKAGE_TMP)/lib/ARM9-TS;		\
	$(CP) $(TWL_IPL_RED_ROOT)/include $(PACKAGE_TMP)/include;				\
	$(CP) $(TWL_IPL_RED_ROOT)/components $(PACKAGE_TMP)/components;			\
	mkdir -p $(PACKAGE_TMP)/build;												\
	$(CP) $(TWL_IPL_RED_ROOT)/build/buildtools $(PACKAGE_TMP)/build/buildtools;	\
	$(FIND) $(PACKAGE_TMP) -name ".svn" -a -type d -print -prune -exec $(RM) $(VERBOSE) {} \;
	zip -r $(ARCHIVE_ROOT)/$(ARCHIVE) $(PACKAGE_TMP)


#===== End of Makefile =====
