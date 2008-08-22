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

override TARGET_PLATFORM  := TWL

TARGET_FIRM     = SYSTEMMENU
TWL_NANDAPP     = TRUE

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include ./commondefs.sysmenuVersion
include ./commondefs.sysmenuVersion.local

#---------------------------------------------------------
# リージョン固有パラメータ

SYSMENU_VERSION_REGION		=	J
SYSMENU_VERSION_STRING_REGION	=	J
SYSMENU_VERSION_TIMESTAMP	=	08082200
SYSMENU_VERSION_MAJOR		=	0
SYSMENU_VERSION_MINOR		=	0

do-build :  $(TARGET_BIN)

#----------------------------------------------------------------------------
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules
include ./modulerules.sysmenuVersion

#===== End of Makefile =====
