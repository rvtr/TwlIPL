#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - wm
# File:     Makefile
#
# Copyright 2003 Nintendo.  All rights reserved.
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


TARGET_FIRM			=	SYSTEMMENU

TARGET_PLATFORM		:= 	TWL

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

#SUBDIRS    =   $(GSUBDIRS)

ifneq	($(filter TWL,$(TARGET_PLATFORM_LIST)),)
ifeq    ($(TWLSDK_PRIVATE),TRUE)
SUBDIRS_P   +=  ARM7
endif
endif

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
