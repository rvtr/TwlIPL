#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - systemMenu_RED - NandInitializer
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

SUBDIRS		=	

#----------------------------------------------------------------------------

#============================================================================
#製品鍵を使った書き込みを行う場合は、TwlIPL/build/buildtools/commondefsの"FIRM_USE_PRODUCT_KEYS=TRUE"を有効にしてください。
#============================================================================
#============================================================================
# NandInitializerRedから一部の機能を削除するために定義を追加します。
MACRO_FLAGS				+=	-DNAND_INITIALIZER_LIMITED_MODE
#============================================================================
#============================================================================
# ワイヤレスの強制Disableフラグを設定可能にする場合定義します。（要:Make Clean）
#MACRO_FLAGS				+=	-DUSE_WIRELESS_FORCE_DISABLE_SETTING
#============================================================================
#============================================================================
# フォントデータを書き込み可能にする場合定義します。（要:Make Clean）
#USE_WRITE_FONT_DATA = TRUE

ifdef USE_WRITE_FONT_DATA
MACRO_FLAGS				    +=	-DUSE_WRITE_FONT_DATA
endif
#============================================================================
#============================================================================
# NightlyBuild 用に自動実行させる場合定義します。（要:Make Clean）
#USE_FOR_NIGHTLY_AUTO_TEST = TRUE

ifdef USE_FOR_NIGHTLY_AUTO_TEST
MACRO_FLAGS				+=	-DUSE_FOR_NIGHTLY_AUTO_TEST
endif
#============================================================================

ifdef USE_FOR_NIGHTLY_AUTO_TEST
DISABLE_DEBUG		= FALSE
else
DISABLE_DEBUG		= TRUE
endif

TARGET_FIRM			= SYSTEMMENU
TARGET_PLATFORM		= TWL
TWL_ARCHGEN    		= LIMITED

#TWL_CODEGEN   		= THUMB

TITLEID_LO			= 0NIA
#TARGET_TAD			=

ifdef USE_FOR_NIGHTLY_AUTO_TEST
TARGET_BIN	= NandInitializerForAutoTest.srl
else
ifdef USE_WRITE_FONT_DATA
TARGET_BIN	= NandInitializerWithSharedFont.srl
else
TARGET_BIN	= NandInitializer.srl
endif
endif

LCFILE_SPEC	= ../../NandInitializerRed/ARM9.TWL/ARM9-TS.lsf
ROM_SPEC	= ./main.rsf

MAKEROM_ARM7_BASE   = $(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7        = $(MAKEROM_ARM7_BASE).TWL.FLX.sbin

SRCS		=	main.c \
				kami_pxi.c \
				font_data.c \
				graphics.c \
				keypad.c \
				kami_font.c \
				cursor.c \
				process_format.c \
				process_topmenu.c \
				process_hw_info.c \
				process_wireless_setting.c \
				process_fade.c \
				process_font.c \
				sd_event.c \
				hwi.c \
				debugger_hw_reset_control.c

LINCLUDES	=	../../NandInitializerRed/common/include \
				../../NandInitializerRed/ARM9.TWL/include \
				$(ROOT)/build/libraries/lcfg/ARM9.TWL/include \
				../../common/ARM9/include

SRCDIR		=	../../NandInitializerRed/ARM9.TWL/src \
				../../HWInfoWriter/ARM9/src \
				../../common/ARM9/src

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

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DPRIVKEY_PATH='$(TWL_KEYSDIR)/rsa' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(DISABLE_DEBUG)'

MAKETAD_FLAGS			+=	-s

SYSMENU_LIBS			=	\
							libnamut$(TWL_LIBSUFFIX).a

SDK_APPEND_LIBS			=	\
							libes$(TWL_LIBSUFFIX).a \
							libboc$(TWL_LIBSUFFIX).a \
							libsfs$(TWL_LIBSUFFIX).a \
							libnam$(TWL_LIBSUFFIX).a \
							libsea$(TWL_LIBSUFFIX).a \
							liblcfg$(TWL_LIBSUFFIX).a

LLIBRARIES				+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

# インストール設定
ifeq	($(TWL_BUILD_TYPE),FINALROM)
INSTALL_DIR	= $(ROOT)/bin/ARM9-TS/Rom
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_NAME).srl
endif

ifdef USE_FOR_NIGHTLY_AUTO_TEST
ifeq	($(TWL_BUILD_TYPE),RELEASE)
INSTALL_DIR	= $(ROOT)/bin/ARM9-TS/Release
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_NAME).srl
endif
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#include	$(TWLSDK_ROOT)/build/buildtools/modulerules
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
