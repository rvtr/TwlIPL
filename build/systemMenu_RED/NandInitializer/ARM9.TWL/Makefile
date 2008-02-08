#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - camera-test
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

USE_PRODUCT_KEY		=	TRUE

# if USE_PRODUCT_KEY is set, set USE_SDBOOT_KEY to boot from SDMC, otherwise to boot from CARD/NAND
#USE_SDBOOT_KEY		=	TRUE

KEYS_DIR				=	$(shell $(CYGPATH) -m $(TWL_KEYSDIR))

TARGET_FIRM			= SYSTEMMENU
TARGET_PLATFORM		= TWL
TWL_ARCHGEN    		= LIMITED

#TWL_CODEGEN   =       THUMB

TITLEID_LO				=	NINI
TARGET_TAD				=	$(BINDIR)/$(TITLEID_LO).$(TWL_BUILD_DIR)$(CODEGEN_ARCH).tad

TARGET_BIN	= main.srl
LCFILE_SPEC				=	ARM9-TS.lsf
ROM_SPEC	= main.rsf

MAKEROM_ARM7_BASE	=	../ARM7.TWL/bin/$(TWL_BUILDTYPE_ARM7)/main
MAKEROM_ARM7		=	$(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

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
				process_eticket.c \
				process_import.c \
				process_nandfirm.c \
				process_norfirm.c \
				process_auto.c \
				process_fade.c \
				sd_event.c

LINCLUDES	=	../common/include \
				$(ROOT)/build/libraries/lcfg/ARM9.TWL/include

SRCDIR		=	src \
				$(ROOT)/build/demos.TWL/camera/saveShoot-1/src
#LCFILE		=	# using default

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DHWINFO_PRIVKEY='private_HWInfo.der'
MAKETAD_FLAGS			+=	-s

ifdef USE_PRODUCT_KEY
MAKEROM_FLAGS			+=	-DPRIVKEY_PATH='$(KEYS_DIR)/rsa'
MACRO_FLAGS				+=	-DUSE_PRODUCT_KEY
ifdef USE_SDBOOT_KEY
MAKEROM_FLAGS			+=	-s -DSYS_PRIV_KEY='$(KEYS_DIR)/rsa/private_sdboot.der' \
					-DSYS_PUB_KEY='$(KEYS_DIR)/rsa/pubkey_sdboot.der'
TARGET_TAD=
endif
else
MAKEROM_FLAGS			+=	-DPRIVKEY_PATH='../../../../keys/dummy/rsa'
endif

LLIBRARIES		+=	libes$(TWL_LIBSUFFIX).a				\
					libboc$(TWL_LIBSUFFIX).a			\
					libnam$(TWL_LIBSUFFIX).a			\
					libacsign_enc$(TWL_LIBSUFFIX).a     \
					libnamut$(TWL_LIBSUFFIX).a


#include	$(TWLSDK_ROOT)/build/buildtools/commondefs
include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib

LDIRT_CLEAN				=	$(TARGET_TAD)
INSTALL_TARGETS			=	$(TARGET_TAD)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#include	$(TWLSDK_ROOT)/build/buildtools/modulerules
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
