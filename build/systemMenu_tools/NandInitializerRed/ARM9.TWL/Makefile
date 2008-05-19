#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - systemMenu_RED - NandInitializerRed
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

#----------------------------------------------------------------------------

#============================================================================
#現時点で製品鍵を使った書き込みが行えてはダメなので、デフォルトOFFにしておいてください。
#USE_PRODUCT_KEY			=	TRUE
#============================================================================
#============================================================================
# ワイヤレスの強制Disableフラグを設定可能にする場合定義します。（要:Make Clean）
MACRO_FLAGS				+=	-DUSE_WIRELESS_FORCE_DISABLE_SETTING
#============================================================================
#============================================================================
# フォントデータを書き込み可能にする場合定義します。（要:Make Clean）
MACRO_FLAGS				    +=	-DUSE_WRITE_FONT_DATA
#============================================================================
#============================================================================
# マリクラデバッグ用に製品本体での起動を抑制する場合定義します。（要:Make Clean）
#MACRO_FLAGS				    +=	-DMARIOCLUB_VERSION
#============================================================================

DISABLE_DEBUG	=	FALSE

TARGET_FIRM			= SYSTEMMENU
TARGET_PLATFORM		= TWL
TWL_ARCHGEN    		= LIMITED

#TWL_CODEGEN   =       THUMB

TITLEID_LO				=	0NRA
#TARGET_TAD				=

TARGET_BIN	= NandInitializerRed.srl
LCFILE_SPEC				=	ARM9-TS.lsf
ROM_SPEC	= main.rsf

MAKEROM_ARM7_BASE	=	../ARM7.TWL/bin/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7		=	$(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

SRCS		=	main.c \
				kami_pxi.c \
				font_data.c \
				graphics.c \
				keypad.c \
				kami_font.c \
				kami_write_nandfirm.c \
				cursor.c \
				process_format.c \
				process_topmenu.c \
				process_hw_info.c \
				process_eticket.c \
				process_import.c \
				process_nandfirm.c \
				process_norfirm.c \
				process_auto.c \
				process_fade.c \
				process_font.c \
				process_wireless_setting.c \
				sd_event.c \
				hwi.c

LINCLUDES	=	../common/include \
				$(ROOT)/build/libraries/lcfg/ARM9.TWL/include

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

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DPRIVKEY_PATH='$(TWL_KEYSDIR)/rsa' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(DISABLE_DEBUG)'

MAKETAD_FLAGS			+=	-s

LLIBRARIES		+=	libes$(TWL_LIBSUFFIX).a				\
					libboc$(TWL_LIBSUFFIX).a			\
					libsfs$(TWL_LIBSUFFIX).a	\
					libnam$(TWL_LIBSUFFIX).a			\
					libsea$(TWL_LIBSUFFIX).a			\
					libacsign$(TWL_LIBSUFFIX).a     \
					libacsign_enc$(TWL_LIBSUFFIX).a     \
					libnamut$(TWL_LIBSUFFIX).a

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

GLIBRARIES		:= $(filter-out libcrypto$(TWL_LIBSUFFIX).a,$(GLIBRARIES))

#LDIRT_CLEAN				=
#INSTALL_TARGETS			=
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#include	$(TWLSDK_ROOT)/build/buildtools/modulerules
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
