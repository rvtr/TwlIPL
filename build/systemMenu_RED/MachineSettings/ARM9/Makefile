#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - SystemMenuRED - MachineSettings
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

SUBDIRS					=	\
#							../../../libraries_sysmenu/sysmenu \
#							../../../libraries_sysmenu/settings

#----------------------------------------------------------------------------


# COMPILE SWITCH　※最終ROM作成時は、全てFALSEにセットする。
SYSM_OUTSIDE_UI			?=	FALSE
SYSM_BROADON_UI			?=	FALSE


# MAKEROM SWITCH ※最終ROM作成時は、TRUEにする
DISABLE_DEBUG			?=	FALSE

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

#※RED本体設定はデバッガ対応の都合上、TITLEID_LO=HNAA, CARD_REGION=ALLとしています。
#  UIG本体設定では、リージョンに合わせてTITLEID_LO, CARD_REGIONを設定してください。
##################################################################
TITLEID_LO				=	HNBA
CARD_REGION				=	ALL
##################################################################

TARGET_BIN				=	$(TITLEID_LO).tad

ROM_SPEC				=	main.rsf

MISC_DIR				=	../../misc

BG_DIR					=	../../data

SRCS					=	main.c MachineSetting.c \
							setRTC.c selectLanguage.c calibrationTP.c setOwnerInfo.c selectCountry.c \
							setParentalControl.c setFreeSoftBox.c setWireless.c cleanupMachine.c	\
							setEULA.c \
							CountryNameMappingData.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c \
							$(BG_DIR)/BGData_MachineSettings.c

LINCLUDES				=	$(MISC_DIR)/include

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(DISABLE_DEBUG)'

MAKETAD_OPTION			+= -s

LLIBRARIES				=	libes$(TWL_LIBSUFFIX).a				\
							libboc$(TWL_LIBSUFFIX).a			\
							libsfs$(TWL_LIBSUFFIX).a	\
							libnam$(TWL_LIBSUFFIX).a			\
							libsea$(TWL_LIBSUFFIX).a			\
							libnamut$(TWL_LIBSUFFIX).a

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs


ifeq ($(SYSM_OUTSIDE_UI),TRUE)
MACRO_FLAGS += -DOUTSIDE_UI
endif

ifeq ($(SYSM_BROADON_UI),TRUE)
MACRO_FLAGS += -DBROADON_UI
endif

ifeq ($(ENABLE_AES),TRUE)
MAKEROM_FLAGS += -a
endif

MAKEROM_ARM7_BASE		=	$(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7			=	$(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)
MAKEROM					=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

INSTALL_TARGETS			=	$(BINDIR)/$(TARGET_BIN)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#----------------------------------------------------------------------------

#===== End of Makefile =====
