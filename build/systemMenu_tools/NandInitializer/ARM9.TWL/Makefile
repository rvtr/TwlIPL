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
#�����_�Ő��i�����g�����������݂��s���Ă̓_���Ȃ̂ŁA�f�t�H���gOFF�ɂ��Ă����Ă��������B
#USE_PRODUCT_KEY			=	TRUE
#============================================================================
#============================================================================
# NandInitializerRed����ꕔ�̋@�\���폜���邽�߂ɒ�`��ǉ����܂��B
MACRO_FLAGS				+=	-DNAND_INITIALIZER_LIMITED_MODE
#============================================================================
#============================================================================
# ���C�����X�̋���Disable�t���O��ݒ�\�ɂ���ꍇ��`���܂��B�i�v:Make Clean�j
#MACRO_FLAGS				+=	-DUSE_WIRELESS_FORCE_DISABLE_SETTING
#============================================================================
#============================================================================
# NightlyBuild �p�Ɏ������s������ꍇ��`���܂��B�i�v:Make Clean�j
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
TARGET_BIN	= NandInitializer.srl
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
				sd_event.c \
				hwi.c

LINCLUDES	=	../../NandInitializerRed/common/include \
				../../NandInitializerRed/ARM9.TWL/include \
				$(ROOT)/build/libraries/lcfg/ARM9.TWL/include

SRCDIR		=	../../NandInitializerRed/ARM9.TWL/src \
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

# �C���X�g�[���ݒ�
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