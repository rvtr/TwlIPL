#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - FS - overlay
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
TARGET_PLATFORM	 = TWL
TWL_ARCHGEN   = LIMITED

SUBDIRS          = 

#----------------------------------------------------------------------------

TARGET_BIN       = main.srl

SRCS             = main.c loadSharedFont.c

LLIBRARIES		+=	libsharedfont$(TWL_LIBSUFFIX).a

ROM_SPEC		=	ROM-TS_nand.rsf

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
