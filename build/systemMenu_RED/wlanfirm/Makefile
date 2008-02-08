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



include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MY_TITLE		=	WFW0
MY_FIRM			=	$(TWLWIRELESS_ROOT)/binfile/nwm_firm.bin
#----------------------------------------------------------------------------
MY_SRL			=	$(MY_TITLE).srl
MY_TAD			=	$(MY_TITLE).tad
ATTACH_HEADER_COMMAND	=	$(TWL_IPL_RED_ROOT)/tools/bin/attach_dummyromheader.exe

INSTALL_TARGETS		=	$(MY_TAD)
INSTALL_DIR		=	$(TWLSDK_ROOT)/build/tools/TwlNMenu/data

LDIRT_CLEAN		=	$(MY_SRL) $(MY_TAD)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build : $(MY_TAD)

$(MY_SRL): $(MY_FIRM)
	$(ATTACH_HEADER_COMMAND) -s -l -i $< -c $(MY_TITLE) -o $@

$(MY_TAD): $(MY_SRL)
	$(MAKETAD) -s $< $(MAKETAD_FLAGS) -o $@

#===== End of Makefile =====
