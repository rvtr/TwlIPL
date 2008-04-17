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

# workaround for small srl probrem
WORKAROUND_SMALL_SRL	= TRUE

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MY_TITLE		=	HNHA
DS_HASH_TABLE			=	DSHashTable.bin

# workaround for small srl probrem
ifneq ($(WORKAROUND_SMALL_SRL),)
DS_HASH_TABLE2			=	DSHashTable2.bin
endif

#----------------------------------------------------------------------------
MY_SRL			=	$(MY_TITLE).srl
MY_TAD			=	$(MY_TITLE).tad
ATTACH_HEADER_COMMAND	=	$(SYSMENU_TOOLSDIR)/bin/attach_dummyromheader.exe

INSTALL_TARGETS		=	$(MY_TAD)
INSTALL_DIR		=	$(ROOT)/build/tools/TwlNMenu/data

LDIRT_CLEAN		=	$(MY_SRL) $(MY_TAD)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build : $(MY_TAD)

ifneq ($(WORKAROUND_SMALL_SRL),)
$(DS_HASH_TABLE2): $(DS_HASH_TABLE)
	cat $< $< > $@

$(MY_SRL): $(DS_HASH_TABLE2)
	$(ATTACH_HEADER_COMMAND) -s -l -i $(call empath,$<) -c $(MY_TITLE) -o $@
else
$(MY_SRL): $(DS_HASH_TABLE)
	$(ATTACH_HEADER_COMMAND) -s -l -i $(call empath,$<) -c $(MY_TITLE) -o $@
endif

$(MY_TAD): $(MY_SRL)
	$(MAKETAD) -s $< $(MAKETAD_FLAGS) -o $@

#===== End of Makefile =====