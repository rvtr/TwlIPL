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

#----------------------------------------------------------------------------

#============================================================================
#���i�����g�����������݂��s���ꍇ�́ATwlIPL/build/buildtools/commondefs��"FIRM_USE_PRODUCT_KEYS=TRUE"��L���ɂ��Ă��������B
#============================================================================
#============================================================================
# NandInitializerRed�Ɠ��l�̓�������܂��������t�H�[�}�b�g�������I�Ɏ��s�����~���܂��B
MACRO_FLAGS				+=	-DAUTO_FORMAT_MODE
#============================================================================
#============================================================================
# ���C�����X�̋���Disable�t���O��ݒ�\�ɂ���ꍇ��`���܂��B�i�v:Make Clean�j
#MACRO_FLAGS				+=	-DUSE_WIRELESS_FORCE_DISABLE_SETTING
#============================================================================
#============================================================================
# �e��f�[�^���������݉\�ɂ���ꍇ��`���܂��B�i�v:Make Clean�j
# ��̓I�ɂ� TWLFontTable.dat wrap.bin cert.sys ��3�ł��B
#MACRO_FLAGS				+=	-DUSE_WRITE_VARIOUS_DATA
#============================================================================
#============================================================================
# dev.kp���������݉\�ɂ���ꍇ��`���܂��B�i�v:Make Clean�j
#MACRO_FLAGS				+=	-DWRITE_DEVKP_ENABLE
#============================================================================
#============================================================================
# �}���N���f�o�b�O�p�Ƃ��č쐬����ꍇ��`���܂��B�i�v:Make Clean�j
# �E���������ɂ�����"nand:/shared2/launcher/wrap.bin"���쐬���܂��B
# �E���������ɂ����ĊJ���pCLS�̏ؖ���(cert.sys)���������݂܂��B
# �E���������̍Ō�ɖ{�̏��������s���܂��B
#MACRO_FLAGS				+=	-DMARIOCLUB_VERSION
#============================================================================

MACRO_FLAGS				+=	-DTWL_CAPTURE_VERSION

SYSM_DISABLE_DEBUG	=	FALSE

TARGET_FIRM			= SYSTEMMENU
TARGET_PLATFORM		= TWL
TWL_ARCHGEN    		= LIMITED

#TARGET_CODEGEN   =       THUMB

TITLEID_LO				=	0NOA
#TARGET_TAD				=

TARGET_BIN		= TWLCaptureSystemWriter.srl
LCFILE_SPEC	=	ARM9-TS.lsf
ROM_SPEC	= main.rsf

MAKEROM_ARM7_BASE	=	../../NandInitializerRed/ARM7.TWL/bin/$(TWL_BUILDTYPE_ARM7)/armadillo
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
				process_eticket.c \
				process_import_twlc.c \
				process_nandfirm_twlc.c \
				process_norfirm.c \
				process_auto.c \
				process_fade.c \
				process_mcu.c \
				process_wireless_setting.c \
				hwi.c \
				debugger_hw_reset_control.c

LINCLUDES	=	./include	\
				../../NandInitializerRed/common/include \
				../../NandInitializerRed/ARM9.TWL/include \
				$(ROOT)/build/libraries/lcfg/ARM9.TWL/include \
				../../common/ARM9/include

SRCDIR		=	./src	\
				../../NandInitializerRed/ARM9.TWL/src \
				../../HWInfoWriter/ARM9/src \
				../../common/ARM9/src

#LCFILE		=	# using default

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs


ifdef FIRM_USE_PRODUCT_KEYS
DATA_DIR				=	prod
else
DATA_DIR				=	dev
endif

MAKELCF_FLAGS			+=	-DNANDFIRM_OBJ=./data/$(DATA_DIR)/nandfirm.o \
							-DHNAA_OBJ=./data/$(DATA_DIR)/HNAA.o \
							-DHNBA_OBJ=./data/$(DATA_DIR)/HNBA.o \
							-DHNCA_OBJ=./data/$(DATA_DIR)/HNCA.o \
							-DHNHA_OBJ=./data/$(DATA_DIR)/HNHA.o \
							-DHNLA_OBJ=./data/$(DATA_DIR)/HNLA.o

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
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
							libna$(TWL_LIBSUFFIX).a

LLIBRARIES				+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

#LDIRT_CLEAN				=
#INSTALL_TARGETS			=
#INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#include	$(TWLSDK_ROOT)/build/buildtools/modulerules
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====