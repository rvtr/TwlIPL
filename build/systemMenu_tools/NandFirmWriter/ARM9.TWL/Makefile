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
# $Date:: 2009-03-11#$
# $Rev: 2763 $
# $Author: aoki_ryoma $
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------

#============================================================================
#製品鍵を使った書き込みを行う場合は、TwlIPL/build/buildtools/commondefsの"FIRM_USE_PRODUCT_KEYS=TRUE"を有効にしてください。
#============================================================================
#============================================================================
# ワイヤレスの強制Disableフラグを設定可能にする場合定義します。（要:Make Clean）
# MACRO_FLAGS				+=	-DUSE_WIRELESS_FORCE_DISABLE_SETTING
#============================================================================
#============================================================================
# 各種データを書き込み可能にする場合定義します。（要:Make Clean）
# 具体的には TWLFontTable.dat wrap.bin cert.sys の3つです。
# MACRO_FLAGS				+=	-DUSE_WRITE_VARIOUS_DATA
#============================================================================
#============================================================================
# dev.kpを書き込み可能にする場合定義します。（要:Make Clean）
#MACRO_FLAGS				+=	-DWRITE_DEVKP_ENABLE
#============================================================================
#============================================================================
# マリクラデバッグ用として作成する場合定義します。（要:Make Clean）
# ・自動処理において"nand:/shared2/launcher/wrap.bin"を作成します。
# ・自動処理において開発用CLSの証明書(cert.sys)を書き込みます。
# ・自動処理の最後に本体初期化を行います。
#MACRO_FLAGS				+=	-DMARIOCLUB_VERSION
#============================================================================

SYSM_DISABLE_DEBUG	= FALSE

TARGET_FIRM			= SYSTEMMENU
TARGET_PLATFORM		= TWL
TWL_ARCHGEN    		= LIMITED

#TARGET_CODEGEN   	= THUMB

TITLEID_LO			= 0NRA
#TARGET_TAD			=

TARGET_BIN			= NandFirmWriter.srl
LCFILE_SPEC			= ARM9-TS.lsf
ROM_SPEC			= main.rsf

MAKEROM_ARM7_BASE	= ../ARM7.TWL/bin/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7		= $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

SRCS				=	main.c \
						common.c \
						font.c \
						screen.c \
						hwi.c \
						kami_pxi.c \
						kami_write_nandfirm.c \

LINCLUDES			=	../common/include \
						$(ROOT)/build/libraries/lcfg/ARM9.TWL/include \
						../../common/ARM9/include

SRCDIR				=	src \
						../../HWInfoWriter/ARM9/src \
						../../common/ARM9/src

#LCFILE				=	# using default

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs


#ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
#ifdef FIRM_USE_PRODUCT_KEYS
#MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo.der' \
#							-DHWID_PRIVKEY='private_HWID.der'
#else
#MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo_dev.der' \
#							-DHWID_PRIVKEY='private_HWID_dev.der'
#endif
#endif

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DPRIVKEY_PATH='$(TWL_KEYSDIR)/rsa' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(SYSM_DISABLE_DEBUG)'

MAKETAD_FLAGS			+=	-s

SYSMENU_LIBS			=	\
							libnamut$(TWL_LIBSUFFIX).a

SDK_APPEND_LIBS			=	\
							libes$(TWL_LIBSUFFIX).a \
							libboc$(TWL_LIBSUFFIX).a \
							libsfs$(TWL_LIBSUFFIX).a \
							libnam$(TWL_LIBSUFFIX).a \
							libsea$(TWL_LIBSUFFIX).a \
							liblcfg$(TWL_LIBSUFFIX).a \
							libna$(TWL_LIBSUFFIX).a \
							liberrorlog$(TWL_LIBSUFFIX).a

LLIBRARIES				+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)

MAKEROM			 		= $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

#LDIRT_CLEAN			=
#INSTALL_TARGETS		=
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#include	$(TWLSDK_ROOT)/build/buildtools/modulerules
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
