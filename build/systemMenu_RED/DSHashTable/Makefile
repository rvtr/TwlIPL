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

override TARGET_PLATFORM := TWL

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include ./commondefs.DSHashTable

MY_DATA				=	$(DS_HASH_TABLE_DATA)
MY_APPEND			=	revision.bin
MY_DATA2			=	data.bin

MY_TITLE			=	$(DS_HASH_TABLE_TITLE)

#----------------------------------------------------------------------------
MY_TAD				=	$(MY_TITLE).tad

#INSTALL_TARGETS	=	$(MY_TAD)
#INSTALL_DIR		=

LDIRT_CLEAN			=	$(MY_TAD) $(MY_APPEND) $(MY_DATA2)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

.PHONY: revision

do-build : $(MY_TAD)

$(MY_TAD): $(MY_DATA2)
	$(MAKETAD) $(call empath,$<) $(DS_HASH_TABLE_MAKETAD_OPTION) -o $@

$(MY_DATA2): $(MY_DATA) revision
	cat $(MY_DATA) $(MY_APPEND) > $@

revision:
	@if test -e $(SYSMENU_ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(SYSMENU_ROOT)) | grep 'Revision' | sed "s|Revision: \(.*\)|\1|" > $(MY_APPEND); \
	else \
		echo `date` > $(MY_APPEND); \
	fi
	@if test -e $(ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(ROOT)) | grep 'Revision' | sed "s|Revision: \(.*\)|\1|" >> $(MY_APPEND); \
	else \
		echo "" >> $(MY_APPEND); \
	fi

revision:
	echo `(LANG=C svn info $(TWL_IPL_RED_ROOT) | grep 'Revision') || date` > $(MY_APPEND)

#===== End of Makefile =====
