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
# FontWriter用としてビルドする場合定義します。（要:Make Clean）
MACRO_FLAGS				+=	-DTWL_FONT_WRITER
#============================================================================
#============================================================================
# tadを作成する場合はTRUEを指定
FONT_WRITER_TAD     = FALSE
#============================================================================


SYSM_DISABLE_DEBUG	= TRUE

TARGET_FIRM			= SYSTEMMENU
TARGET_PLATFORM		= TWL
TWL_ARCHGEN    		= LIMITED

#TARGET_CODEGEN   	= THUMB

TITLEID_LO			= 0FWA
#TARGET_TAD			=

ifeq ($(FONT_WRITER_TAD),TRUE)
TWL_NANDAPP         = TRUE
TARGET_BIN  = FontWriter.tad
MAKETAD_OPTION	+=	-s
FONT_WRITER_MEDIA = NAND
else
TARGET_BIN	= FontWriter.srl
FONT_WRITER_MEDIA = GameCard
endif

LCFILE_SPEC	= ../../NandInitializerRed/ARM9.TWL/ARM9-TS.lsf
ROM_SPEC	= ./main.rsf

MAKEROM_ARM7_BASE   = $(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7        = $(MAKEROM_ARM7_BASE).TWL.FLX.sbin

SRCS		=	./local_src/main.c \
				./local_src/process_write_data.c \
				kami_pxi.c \
				font_data.c \
				graphics.c \
				keypad.c \
				kami_font.c \
				cursor.c \
				process_fade.c

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


MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(SYSM_DISABLE_DEBUG)' \
							-DFONT_WRITER_MEDIA='$(FONT_WRITER_MEDIA)'

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
							libna$(TWL_LIBSUFFIX).a

LLIBRARIES				+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

# インストール設定
#ifeq	($(TWL_BUILD_TYPE),FINALROM)
#INSTALL_DIR	= $(ROOT)/bin/ARM9-TS/Rom
#INSTALL_TARGETS	= $(BINDIR)/$(TARGET_NAME).srl
#endif


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#include	$(TWLSDK_ROOT)/build/buildtools/modulerules
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
