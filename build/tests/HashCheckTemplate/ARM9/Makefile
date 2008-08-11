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

TITLEID_LO				=	0DTA

ROM_SPEC				=	main.rsf
ROM_HEADER_TEMPLATE		=	$(SYSMENU_ROM_HEADER_DIR)/$(TITLEID_LO)/rom_header_$(call toLower,$(TITLEID_LO)).template.sbin
LIBSYSCALL				=	$(SYSMENU_ROM_HEADER_DIR)/$(TITLEID_LO)/libsyscall.a

MISC_DIR				=	$(SYSMENU_ROOT)/build/systemMenu_RED/misc

SRCS					=	main.c HashCheckTemplate.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c

LINCLUDES				=	$(MISC_DIR)/include

MAKEROM_FLAGS			+=	-F -DTITLEID_LO='$(TITLEID_LO)' \
							-DROM_HEADER_TEMPLATE='$(call empath,$(ROM_HEADER_TEMPLATE))'

#デバッグ用パラメータ追加
MAKEROM_FLAGS			+=	-DSYSCALL_C='$(call empath,$(LIBSYSCALL:.a=_c.bin))' \
							-DARM9FLX_EXPAND=0x27c000 \
							-DARM7FLX_EXPAND=0x40000 \
							-DARM9LTD_EXPAND=0x280000 \
							-DARM7LTD_EXPAND=0x108000

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEROM			=	$(TWL_TOOLSDIR)/bin/makerom.TWL.DEBUG.exe

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
