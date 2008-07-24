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
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS					=	

#----------------------------------------------------------------------------

TWL_NANDAPP				=	TRUE

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TARGET_NAME		= HashCheckTemplate
TARGET_BIN				=	$(TARGET_NAME).tad

TITLEID_LO				=	CTMP

ROM_SPEC				=	main.rsf
LCFILE_TEMPLATE			=	hct.lcf.template

MISC_DIR				=	$(SYSMENU_ROOT)/build/systemMenu_RED/misc

SRCS					=	main.c HashCheckTemplate.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c

LINCLUDES				=	$(MISC_DIR)/include
LLIBRARY_DIRS				+=	../crt/ARM9/obj/ARM9-TS.HYB/$(TWL_BUILD_DIR)

CRT0_O		=	crt0_hct.FLX.TWL.o

MAKEROM_FLAGS			+=	-F -DTITLEID_LO='$(TITLEID_LO)'

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

DEFAULT_COMP_ARM7		= racoon_hct
DEFAULT_MAKEROM_ARM7_BASE	= ../ARM7/bin/$(TWL_BUILDTYPE_ARM7)/$(DEFAULT_COMP_ARM7)
DEFAULT_MAKEROM_ARM7		= $(DEFAULT_MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)


LDIRT_CLEAN				=	$(TARGET_TAD)
INSTALL_TARGETS			=	$(TARGET_TAD)

#----------------------------------------------------------------------------

do-build : $(TARGETS)

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
