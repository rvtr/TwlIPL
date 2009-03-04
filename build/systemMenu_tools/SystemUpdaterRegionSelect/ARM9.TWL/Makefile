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

# SystemMenuファイルの置かれているフォルダを指定(相対もしくは絶対）
UPDATER_HOST_ROOT_DIR = ./v1_2

# JPリージョンしか選択できなくする場合はTRUEを指定
JP_REGION_ONLY			?= FALSE

# COMPILE SWITCH for build SystemUpdater（TRUEにすると、ダミーdev.kpを生成しないようになります。）
FOR_LOTCHECK			?=	FALSE

# COMPILE SWITCH for build SystemUpdater
IGNORE_VERSION_CHECK	?=	FALSE
SKIP_WRITE_HWINFO       ?=	FALSE
USE_NORMAL_COMMON_KEY   ?=	FALSE

# MAKEROM SWITCH for build SystemUpdater
SYSM_DISABLE_DEBUG		?=	TRUE

TARGET_FIRM		= SYSTEMMENU
TARGET_PLATFORM	= TWL
TWL_ARCHGEN    	= LIMITED

#TARGET_CODEGEN = THUMB

TITLEID_LO		= 0SUA
#TARGET_TAD		=

TARGET_BIN		= SystemUpdater.srl
LCFILE_SPEC		= ARM9-TS.lsf
ROM_SPEC		= main.rsf

MAKEROM_ARM7_BASE   = ../ARM7.TWL/bin/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7        = $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

SRCS		=	main.c \
				font.c \
				font_data.c \
				graphics.c \
				kami_font.c \
				hwi.c \
				kami_pxi.c \
				kami_write_nandfirm.c \
				hw_info.c \
				keypad.c \
				kami_copy_file.c \
				debugger_hw_reset_control.c \
				debugger_card_rom.c \
				process_select_region.c \
				process_cancel.c \
				process_check_log.c \
				process_note.c \
				process_check_console.c \
				process_format.c \
				process_hwinfo.c \
				process_log.c \
				process_finish.c \
				process_import.c \
				process_write_font.c \
				process_write_cert.c \
				process_write_dummy.c \
				process_nandfirm.c \
				process_namut_format.c \
				process_delete_other_region_sysmenu.c \
				fade.c \
				sort_title.c

LINCLUDES	=	include \
				../common/include \
				$(ROOT)/build/libraries/lcfg/ARM9.TWL/include \
				$(ROOT)/build/libraries/fs/common/include \
				../../common/ARM9/include

SRCDIR		=	src \
				../../HWInfoWriter/ARM9/src \
				../../common/ARM9/src

#LCFILE		=	# using default

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

ifeq ($(SKIP_WRITE_HWINFO),TRUE)
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='' \
							-DHWID_PRIVKEY=''
else
ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
ifdef FIRM_USE_PRODUCT_KEYS
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo.der' \
							-DHWID_PRIVKEY='private_HWID.der'
else
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo_dev.der' \
							-DHWID_PRIVKEY='private_HWID_dev.der'
endif
endif
endif

ifeq ($(JP_REGION_ONLY),TRUE)
MACRO_FLAGS += -DJP_REGION_ONLY
endif

ifeq ($(FOR_LOTCHECK),TRUE)
MACRO_FLAGS += -DFOR_LOTCHECK
endif

ifeq ($(IGNORE_VERSION_CHECK),TRUE)
MACRO_FLAGS += -DIGNORE_VERSION_CHECK
endif

ifeq ($(SKIP_WRITE_HWINFO),TRUE)
MACRO_FLAGS += -DSKIP_WRITE_HWINFO
endif

ifeq ($(USE_NORMAL_COMMON_KEY),TRUE)
MAKEROM_FLAGS			+=	-DComClientKey='TRUE' \
							-DComClientKeyDebugger='FALSE'
else
MAKEROM_FLAGS			+=	-DComClientKey='FALSE' \
							-DComClientKeyDebugger='TRUE'
endif

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DPRIVKEY_PATH='$(TWL_KEYSDIR)/rsa' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(SYSM_DISABLE_DEBUG)' \
							-DUPDATER_HOST_ROOT_DIR='$(UPDATER_HOST_ROOT_DIR)'

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
