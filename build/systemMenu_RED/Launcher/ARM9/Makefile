
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
							../../../libraries_sysmenu/boot \
							../../../libraries_sysmenu/hotsw \
#							../../../libraries_sysmenu/sharedFont \
#							../../../libraries_sysmenu/reloc_info \
#							../../../libraries_sysmenu/mcu \
#							../../../libraries_sysmenu/ds \
#							$(ROOT)/build/libraries/reboot \

#----------------------------------------------------------------------------
# コンポーネント選択 ( hyena or jackal )

export DEFAULT_COMP_ARM7	=	jackal

COMPONENT_DIR			=	$(TWL_IPL_RED_ROOT)/build/components/$(DEFAULT_COMP_ARM7).TWL
SUBDIRS					+=	$(COMPONENT_DIR)
WRAM_MAP_FILE			=	$(COMPONENT_DIR)/wram_regs/wram_regs.rbin

#----------------------------------------------------------------------------

ifneq ($(UPDATER_RELEASE),TRUE)

# COMPILE SWITCH　※最終ROM作成時は、全てFALSEにセットする。SUBDIRのビルド時にも影響するようexportで環境変数設定する。
export DO_NOT_SHOW_LAUNCHER		=	FALSE
export DISABLE_WDS_SCAN			=	TRUE
export DISABLE_WLFIRM_LOAD		=	FALSE
export DISABLE_INITIAL_SETTINGS	=	TRUE
export IGNORE_WLFIRM_SIGNCHECK	=	FALSE
endif

# MAKEROM SWITCH ※最終ROM作成時は、全てTRUEにセットする
DISABLE_DEBUG			?=	FALSE


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


TARGET_BIN				=	$(TITLEID_LO).tad

LCFILE_SPEC				=	ARM9-TS.lsf
LCFILE_TEMPLATE			=	ARM9-TS.lcf.template
ROM_SPEC				=	main.rsf

LOGO_DIR				=	Logo
MISC_DIR				=	../../misc

BG_DIR					=	../../data

SRCS_LOGO				=	logoDemo.c logoData.c
SRCS					=	main.c launcher.c sound.c bannerCounter.c loadWlanFirm.c \
							loadSharedFont.c scanWDS.c loadSysmVersion.c \
							$(addprefix $(LOGO_DIR)/, $(SRCS_LOGO)) \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c \
							$(BG_DIR)/BGData_Launcher.c

LINCLUDES				=	$(MISC_DIR)/include \
							$(SRCDIR)/$(LOGO_DIR) \
							$(ROOT)/build/libraries/wm/common.TWL/inc \
							$(ROOT)/build/libraries/wm/ARM9.TWL/nwm/inc \
							$(ROOT)/build/libraries/reboot/common/include

SYSMENU_LIBS			=	\
							liblcfg$(TWL_LIBSUFFIX).a \
							libsysmenu$(TWL_LIBSUFFIX).a \
							libsysmmcu$(TWL_LIBSUFFIX).a \
							libsysmutil$(TWL_LIBSUFFIX).a \
							libbanner$(TWL_LIBSUFFIX).a \
							libboot$(TWL_LIBSUFFIX).a \
							libds$(TWL_LIBSUFFIX).a \
							libhotsw$(TWL_LIBSUFFIX).a \
							libdht$(TWL_LIBSUFFIX).a \
							libreloc_info$(TWL_LIBSUFFIX).a \
							libnamut$(TWL_LIBSUFFIX).a \
							libsharedfont_launcher$(TWL_LIBSUFFIX).a \
							WDS$(TWL_LIBSUFFIX).a

SDK_APPEND_LIBS		=		\
							libes$(TWL_LIBSUFFIX).a		\
							libboc$(TWL_LIBSUFFIX).a	\
							libsfs$(TWL_LIBSUFFIX).a	\
							libnam$(TWL_LIBSUFFIX).a	\
							libsea$(TWL_LIBSUFFIX).a	\
							libreboot$(TWL_LIBSUFFIX).a	\

LLIBRARIES				+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)

ADDRESS_DTCM			=	0x0e000000


#----------------------------------------------------------------------------
# Append Flags


MAKEROM_FLAGS			+=	-F \
							-DTITLEID_LO='$(TITLEID_LO)' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(DISABLE_DEBUG)' \
							-DWRAM_MAP_FILE='$(WRAM_MAP_FILE)'

MAKETAD_OPTION			+= -s

ifeq ($(DO_NOT_SHOW_LAUNCHER),TRUE)
MACRO_FLAGS += -DDO_NOT_SHOW_LAUNCHER
endif

ifeq ($(DISABLE_WDS_SCAN),TRUE)
MACRO_FLAGS += -DDISABLE_WDS_SCAN
endif

ifeq ($(DISABLE_WLFIRM_LOAD),TRUE)
MACRO_FLAGS += -DDISABLE_WLFIRM_LOAD
endif

ifeq ($(IGNORE_WLFIRM_SIGNCHECK),TRUE)
MACRO_FLAGS += -DIGNORE_WLFIRM_SIGNCHECK
endif

ifneq ($(DHT_TEST),)
MACRO_FLAGS += -DDHT_TEST
MAKEROM_FLAGS += -DDS_HASH_TABLE_FILE=DSHashTable.bin
else
MAKEROM_FLAGS += -DDS_HASH_TABLE_DIR=data
endif

ifeq ($(DEFAULT_COMP_ARM7),hyena)
MACRO_FLAGS += -DUSE_HYENA_COMPONENT
endif

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs


# ローカルでビルドしたlibsharedfont_launcherを使用するので、SDKのlibsharedfontは除外する。
GLIBRARIES		:= $(filter-out libsharedfont$(TWL_LIBSUFFIX).a,$(GLIBRARIES))

MAKEROM			 = $(SYSMENU_TOOLSDIR)/bin/makerom.TWL.launcher.exe

INSTALL_TARGETS			=	$(BINDIR)/$(TARGET_BIN)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
