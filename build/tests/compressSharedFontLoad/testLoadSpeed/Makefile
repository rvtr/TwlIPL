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


ifeq	($(TEST_TWLSDK_REBUILD),TRUE)
export TWLSDK_SYSMENU_PRIVATE = TRUE
SUBDIRS          =	copy_to_sdk \
					$(ROOT)
endif	# ifeq ($(TEST_TWLSDK_REBUILD),TRUE)

#----------------------------------------------------------------------------

TARGET_BIN       = main.srl

SRCS             = main.c loadSharedFont.c

LLIBRARIES		+=	libsharedfont$(TWL_LIBSUFFIX).a

ROM_SPEC		=	ROM-TS_nand.rsf

include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

MAKEROM_ARM7_BASE	=	$(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7		=	$(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)
MAKEROM				=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

#----------------------------------------------------------------------------

do-build:		oslib $(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

oslib:
	touch	$(ROOT)/build/libraries/os/ARM9.TWL/src/os_sharedFont.c

#===== End of Makefile =====
