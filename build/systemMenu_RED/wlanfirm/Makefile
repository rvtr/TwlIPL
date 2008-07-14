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

TARGET_FIRM					=	SYSTEMMENU
override TARGET_PLATFORM	:= TWL
TWL_NANDAPP		=	TRUE


include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include ./commondefs.wlanfirm


MY_TITLE		=	HNCA

#----------------------------------------------------------------------------
MY_TAD			=	$(MY_TITLE).tad

#INSTALL_TARGETS	=
#INSTALL_DIR		=

LDIRT_CLEAN		=	$(MY_TAD) properties

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build : $(MY_TAD)

$(MY_TAD): $(MY_WLANFIRM)
	$(MAKETAD) $(call empath,$<) $(WLANFIRM_MAKETAD_OPTION) -o $@

#===== End of Makefile =====
