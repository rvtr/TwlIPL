#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos.TWL - nandApp - ExecPreLoadedApp
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
# $Date:: $
# $Rev:  $
# $Author:  $
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------

TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	MLEP

TARGET_BIN				=	ExecPreLoadedApp.tad

LCFILE_SPEC				=	ARM9-TS.lsf
ROM_SPEC				=	main.rsf

SRCS					=	main.c ExecPreLoadedApp.c misc_simple.c data.c

LINCLUDES	=	$(TWLSDK_ROOT)/build/demos/gx/UnitTours/DEMOLib/include

LLIBRARY_DIRS	+=	$(TWLSDK_ROOT)/build/demos/gx/UnitTours/DEMOLib/lib/$(TWL_BUILDTYPE)

LLIBRARIES				+=	libboc$(TWL_LIBSUFFIX).a \
							libes$(TWL_LIBSUFFIX).a \
							libnam$(TWL_LIBSUFFIX).a \
							libDEMO$(TWL_LIBSUFFIX).a

ADDRESS_DTCM			=	0x0e000000

MAKEROM_FLAGS			+=	-F -DTITLEID_LO='$(TITLEID_LO)'

include	$(TWLSDK_ROOT)/add-ins/es/commondefs.es
include $(TWLSDK_ROOT)/build/buildtools/commondefs

DEFAULT_COMP_ARM7		= my_racoon
DEFAULT_MAKEROM_ARM7_BASE	= ../ARM7/bin/$(TWL_BUILDTYPE_ARM7)/$(DEFAULT_COMP_ARM7)
DEFAULT_MAKEROM_ARM7		= $(DEFAULT_MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

SDK_NMENU_DATADIR				=  $(ROOT)/build/tools/TwlNMenu/data
INSTALL_TARGETS			=	$(BINDIR)/$(TARGET_BIN)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
