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
#現時点で製品鍵を使った書き込みが行えてはダメなので、デフォルトOFFにしておいてください。
#USE_PRODUCT_KEY			=	TRUE
#============================================================================

# COMPILE SWITCH for build SystemUpdater
IGNORE_VERSION_CHECK	?=	FALSE

# MAKEROM SWITCH for build SystemUpdater
DISABLE_DEBUG			?=	TRUE

TARGET_FIRM		= SYSTEMMENU
TARGET_PLATFORM	= TWL
TWL_ARCHGEN    	= LIMITED

#TWL_CODEGEN   	= THUMB

TITLEID_LO		=	0SUA
#TARGET_TAD		=

TARGET_BIN		= SystemUpdater.srl
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
				kami_copy_file.c

LINCLUDES	=	include \
				../common/include \
				$(ROOT)/build/libraries/lcfg/ARM9.TWL/include \
				$(ROOT)/build/libraries/fs/common/include

SRCDIR		=	src \
				../../HWInfoWriter/ARM9/src

#LCFILE		=	# using default

ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
ifdef USE_PRODUCT_KEY
MACRO_FLAGS				+=	-DUSE_PRODUCT_KEY
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo.der'
else
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo_dev.der'
endif
endif

ifeq ($(IGNORE_VERSION_CHECK),TRUE)
MACRO_FLAGS += -DIGNORE_VERSION_CHECK
endif


MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DPRIVKEY_PATH='$(TWL_KEYSDIR)/rsa' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(DISABLE_DEBUG)'
MAKETAD_FLAGS			+=	-s

LLIBRARIES		+=	libes$(TWL_LIBSUFFIX).a				\
					libboc$(TWL_LIBSUFFIX).a			\
					libsfs$(TWL_LIBSUFFIX).a			\
					libnam$(TWL_LIBSUFFIX).a			\
					libsea$(TWL_LIBSUFFIX).a			\
					libacsign$(TWL_LIBSUFFIX).a     	\
					libacsign_enc$(TWL_LIBSUFFIX).a     \
					libnamut$(TWL_LIBSUFFIX).a

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

GLIBRARIES		:= $(filter-out libcrypto$(TWL_LIBSUFFIX).a,$(GLIBRARIES))

#LDIRT_CLEAN			=
#INSTALL_TARGETS		=
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#include	$(TWLSDK_ROOT)/build/buildtools/modulerules
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
