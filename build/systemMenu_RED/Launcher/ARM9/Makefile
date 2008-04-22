#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - simpleShoot-1
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
# $Date:: $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

# ※コンパイルスイッチの都合上、../../../libraries_sysmenu/sysmenu　は有効にしておく
SUBDIRS					=	\
							../../../libraries_sysmenu/sysmenu \
#							../../../libraries_sysmenu/boot \
#							../../../components/hyena.TWL
#							../../../libraries_sysmenu/hotsw \
#							../../../libraries_sysmenu/reloc_info \
#							../../../libraries_sysmenu/mcu \
#							../../../libraries_sysmenu/ds \
#							$(ROOT)/build/libraries/reboot \

#----------------------------------------------------------------------------

# COMPILE SWITCH　※最終ROM作成時は、全てFALSEにセットする。SUBDIRのビルド時にも影響するようexportで環境変数設定する。
export DO_NOT_SHOW_LAUNCHER	=	FALSE
export DISABLE_WDS_SCAN		=	TRUE
export DISABLE_WLFIRM_LOAD	=	TRUE
export DISABLE_SLEEP		=	TRUE

# COMPILE SWITCH ※最終ROM作成時は、全てTRUEにセットする
export LOAD_APP_VIA_WRAM	=	FALSE

# MAKEROM SWITCH ※最終ROM作成時は、全てTRUEにセットする
DISABLE_DEBUG			=	FALSE
ENABLE_AES				=	FALSE

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_CODEGEN				=	ARM
TWL_ARCHGEN				=	LIMITED

#※REDランチャーはデバッガ対応の都合上、TITLEID_LO=HNAA, CARD_REGION=ALLとしています。
#  UIGランチャーでは、リージョンに合わせてTITLEID_LO, CARD_REGIONを設定してください。
##################################################################
TITLEID_LO				=	HNAA
CARD_REGION				=	ALL
##################################################################


TARGET_TAD				=	$(BINDIR)/$(TITLEID_LO).$(TWL_BUILD_DIR)$(CODEGEN_ARCH).tad

TARGET_BIN				=	main.srl

LCFILE_SPEC				=	ARM9-TS.lsf
#LCFILE_TEMPLATE			=	ARM9-TS.lcf.template
ROM_SPEC				=	main.rsf

LOGO_DIR				=	Logo
MISC_DIR				=	../../misc

BG_DIR					=	../../data

SRCS_LOGO				=	logoDemo.c logoData.c
SRCS					=	main.c launcher.c sound.c bannerCounter.c loadWlanFirm.c scanWDS.c \
							$(addprefix $(LOGO_DIR)/, $(SRCS_LOGO)) \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c \
							$(BG_DIR)/BGData_Launcher.c

LINCLUDES				=	$(MISC_DIR)/include \
							$(SRCDIR)/$(LOGO_DIR) \
							$(TWLSDK_ROOT)/build/libraries/wm/common.TWL/inc \
							$(TWLSDK_ROOT)/build/libraries/wm/ARM9.TWL/nwm/inc

LLIBRARIES				+=	libes$(TWL_LIBSUFFIX).a		\
							libboc$(TWL_LIBSUFFIX).a	\
							libsfs$(TWL_LIBSUFFIX).a	\
							libnam$(TWL_LIBSUFFIX).a	\
							libsea$(TWL_LIBSUFFIX).a	\
							libreloc_info$(TWL_LIBSUFFIX).a \
							WDS$(TWL_LIBSUFFIX).a

ADDRESS_DTCM			=	0x0e000000

DEFAULT_COMP_ARM7		=	hyena

MAKEROM_FLAGS			+=	-F \
							-DTITLEID_LO='$(TITLEID_LO)' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(DISABLE_DEBUG)'

MAKETAD_FLAGS			+= -s

ifeq ($(DO_NOT_SHOW_LAUNCHER),TRUE)
MACRO_FLAGS += -DDO_NOT_SHOW_LAUNCHER
endif

ifeq ($(DISABLE_WDS_SCAN),TRUE)
MACRO_FLAGS += -DDISABLE_WDS_SCAN
endif

ifeq ($(DISABLE_WLFIRM_LOAD),TRUE)
MACRO_FLAGS += -DDISABLE_WLFIRM_LOAD
endif

ifeq ($(DISABLE_SLEEP),TRUE)
MACRO_FLAGS += -DDISABLE_SLEEP
endif

ifeq ($(ENABLE_AES),TRUE)
MAKEROM_FLAGS += -a
endif

ifneq ($(DHT_TEST),)
MACRO_FLAGS += -DDHT_TEST
MAKEROM_FLAGS += -DDS_HASH_TABLE_FILE=DSHashTable.bin
else
MAKEROM_FLAGS += -DDS_HASH_TABLE_DIR=data
endif

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/libraries/reboot/commondefs.reboot

MAKEROM			 = $(SYSMENU_TOOLSDIR)/bin/makerom.TWL.launcher.exe

LDIRT_CLEAN				=	$(TARGET_TAD)
INSTALL_TARGETS			=	$(TARGET_TAD)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
