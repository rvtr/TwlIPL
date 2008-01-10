#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - simpleShoot-1
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

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	MLEP
TARGET_TAD				=	$(BINDIR)/$(TITLEID_LO).$(TWL_BUILD_DIR).tad

TARGET_BIN				=	main.srl

LCFILE_SPEC				=	ARM9-TS.lsf
ROM_SPEC				=	main.rsf

MISC_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/misc

SRCS					=	main.c ExecPreLoadedApp.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c

LINCLUDES				=	$(MISC_DIR)/include

LLIBRARIES				+=	libboc$(TWL_LIBSUFFIX).a \
							libes$(TWL_LIBSUFFIX).a \
							libnam$(TWL_LIBSUFFIX).a \
							libreloc_info$(TWL_LIBSUFFIX).a

ADDRESS_DTCM			=	0x0e000000

DEFAULT_COMP_ARM7		=	hyena

MAKEROM_FLAGS			+=	-F -DTITLEID_LO='$(TITLEID_LO)'

include	$(TWLSDK_ROOT)/add-ins/es/commondefs.es
include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

LDIRT_CLEAN				=	$(TARGET_TAD)
INSTALL_TARGETS			=	$(TARGET_TAD)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
