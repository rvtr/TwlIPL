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

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
MY_FIRM_ROOT	=	$(TWLWIRELESS_ROOT)/binfile

MY_FIRM_ROOT_CYG =	$(call eupath,$(MY_FIRM_ROOT))


MY_TITLE		=	HNCA
MY_FIRM			=	$(MY_FIRM_ROOT_CYG)/nwm_firm.bin

MAKETAD_FLAGS	+=	0003000F484E4341 3031 0 WIRELESS_FW -p

#----------------------------------------------------------------------------
MY_TAD			=	$(MY_TITLE).tad

INSTALL_TARGETS	=	$(MY_TAD)
INSTALL_DIR		=	$(ROOT)/build/tools/TwlNMenu/data

LDIRT_CLEAN		=	$(MY_TAD)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build : $(MY_TAD)

$(MY_TAD): $(MY_FIRM)
	$(MAKETAD) $(call empath,$<) -s -d $(MAKETAD_FLAGS) -o $@

#===== End of Makefile =====
