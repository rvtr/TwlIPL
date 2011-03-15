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

TARGET_FIRM			=	SYSTEMMENU

override TARGET_PLATFORM	:= TWL
override TARGET_CODEGEN		:= ARM
override TWL_ARCHGEN		:= LIMITED
TWL_NANDAPP			=	TRUE

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include ./commondefs.DSHashTable

MY_DATA				=	$(DS_HASH_TABLE_DATA)
MY_DATA_EX			=	$(DS_HASH_TABLE_EX_DATA)
MY_DATA_ADHOC			=	$(DS_HASH_TABLE_ADHOC_DATA)
MY_APPEND			=	revision.bin
MY_DATA2			=	data.bin

MY_TITLE			=	$(DS_HASH_TABLE_TITLE)

#----------------------------------------------------------------------------
MY_TAD				=	$(MY_TITLE)-$(DS_HASH_TABLE_VERSION)-$(DS_HASH_TABLE_EX_VERSION).tad
MY_PADDED_DATA			=	$(MY_TAD:.tad=.bin)

#INSTALL_TARGETS	=	$(MY_TAD)
#INSTALL_DIR		=

LDIRT_CLEAN			=	$(MY_TAD) $(MY_APPEND) $(MY_DATA2)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build : $(MY_TAD)

$(MY_TAD): $(MY_DATA2)
	$(MAKETAD) $(call empath,$<) $(DS_HASH_TABLE_MAKETAD_OPTION) -o $@

$(MY_DATA2): $(MY_DATA) $(MY_DATA_EX) $(MY_DATA_ADHOC) $(MY_APPEND)
	cat $(MY_DATA) $(MY_DATA_EX) $(MY_DATA_ADHOC) $(MY_APPEND) > $@
	# for CTR
	dd if=$@ of=$(MY_PADDED_DATA) bs=320k conv=sync

$(MY_APPEND)::
	@if test -e $(SYSMENU_ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(SYSMENU_ROOT)) | grep 'Revision' | sed s/[^0-9]//g > $(MY_APPEND); \
	else \
		date > $(MY_APPEND); \
	fi
	@if test -e $(ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(ROOT)) | grep 'Revision' | sed s/[^0-9]//g >> $(MY_APPEND); \
	else \
		echo "" >> $(MY_APPEND); \
	fi

#===== End of Makefile =====
