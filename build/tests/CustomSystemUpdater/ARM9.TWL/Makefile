#! make -f
#----------------------------------------------------------------------------
# Project:  SystemUpdater
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

#============================================================================
#製品鍵を使った書き込みを行う場合は、TwlIPL/build/buildtools/commondefsの"FIRM_USE_PRODUCT_KEYS=TRUE"を有効にしてください。
#============================================================================

# COMPILE SWITCH for build SystemUpdater
IGNORE_VERSION_CHECK	?=	TRUE

# MAKEROM SWITCH for build SystemUpdater
SYSM_DISABLE_DEBUG			?=	TRUE

TARGET_FIRM		= SYSTEMMENU
TARGET_PLATFORM	= TWL
TWL_ARCHGEN    	= LIMITED

#TARGET_CODEGEN   	= THUMB

TITLEID_LO		=	0SUA
#TARGET_TAD		=

TARGET_BIN		= CustomSystemUpdater.srl
LCFILE_SPEC		= ARM9-TS.lsf
ROM_SPEC		= main.rsf

MAKEROM_ARM7_BASE   = ../ARM7.TWL/bin/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7        = $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

SRCS		=	main.c \
				font_data.c \
				graphics.c \
				kami_font.c \
				import.c \
				hwi.c \
				kami_pxi.c \
				kami_write_nandfirm.c \
				hw_info.c \
				keypad.c \
				kami_copy_file.c \
				debugger_hw_reset_control.c \
				debugger_card_rom.c

LINCLUDES	=	include \
				../common/include \
				$(ROOT)/build/libraries/lcfg/ARM9.TWL/include \
				$(ROOT)/build/libraries/fs/common/include \
				../../../systemMenu_tools/common/ARM9/include \

SRCDIR		=	src \
				../../../systemMenu_tools/HWInfoWriter/ARM9/src \
				../../../systemMenu_tools/common/ARM9/src \

#LCFILE		=	# using default

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
ifdef FIRM_USE_PRODUCT_KEYS
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo.der' \
							-DHWID_PRIVKEY='private_HWID.der'
else
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo_dev.der' \
							-DHWID_PRIVKEY='private_HWID_dev.der'
endif
endif

ifeq ($(IGNORE_VERSION_CHECK),TRUE)
MACRO_FLAGS += -DIGNORE_VERSION_CHECK
endif


MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DPRIVKEY_PATH='$(TWL_KEYSDIR)/rsa' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(SYSM_DISABLE_DEBUG)'

MAKETAD_FLAGS			+=	-s

SYSMENU_LIBS			=	\
							libnamut$(TWL_LIBSUFFIX).a \
							libsysmenu$(TWL_LIBSUFFIX).a

SDK_APPEND_LIBS			=	\
							libes$(TWL_LIBSUFFIX).a \
							libboc$(TWL_LIBSUFFIX).a \
							libsfs$(TWL_LIBSUFFIX).a \
							libnam$(TWL_LIBSUFFIX).a \
							libsea$(TWL_LIBSUFFIX).a \
							liblcfg$(TWL_LIBSUFFIX).a

LLIBRARIES				+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

#LDIRT_CLEAN			=
#INSTALL_TARGETS		=
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#include	$(TWLSDK_ROOT)/build/buildtools/modulerules
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
