#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL
# File:     commondefs.systemmenuVersion.A -
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

#---------------------------------------------------------
# リージョン固有パラメータ

SYSMENU_VERSION_REGION		=	E
SYSMENU_VERSION_STRING_REGION	=	U
SYSMENU_VERSION_TIMESTAMP	=	11062200
SYSMENU_VERSION_MAJOR		=	1
SYSMENU_VERSION_MINOR		=	4
SYSMENU_VERSION_TAD_VERSION	=	7

STR_VERSION			=	$(SYSMENU_VERSION_MAJOR).$(SYSMENU_VERSION_MINOR).3$(SYSMENU_VERSION_STRING_REGION)$(DEV_STRING)

do-build :  $(TARGET_BIN)

