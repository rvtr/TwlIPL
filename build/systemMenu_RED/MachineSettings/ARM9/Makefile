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

SUBDIRS					=
SUBMAKES				=	\
							$(ROOT)/build/libraries/os/ARM9.TWL/Makefile.sharedFont

#----------------------------------------------------------------------------


# COMPILE SWITCH　※最終ROM作成時は、全てFALSEにセットする。
SYSM_OUTSIDE_UI			?=	FALSE

# MAKEROM SWITCH ※最終ROM作成時は、TRUEにする
SYSM_DISABLE_DEBUG		?=	FALSE

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TARGET_CODEGEN			?=	THUMB
TWL_ARCHGEN				=	LIMITED
TWL_NANDAPP				=	TRUE

#※RED本体設定はデバッガ対応の都合上、TITLEID_LO=HNAA, CARD_REGION=ALLとしています。
#  UIG本体設定では、リージョンに合わせてTITLEID_LO, CARD_REGIONを設定してください。
##################################################################
TITLEID_LO				=	HNBA
CARD_REGION				=	ALL
##################################################################

TARGET_BIN				=	$(TITLEID_LO).tad

ROM_HEADER_DIR			=	$(SYSMENU_ROOT)/RomHeader/$(TITLEID_LO)
SMALL_TITLEID			=	$(shell echo $(TITLEID_LO) | tr '[A-Z]' '[a-z]' )
ROM_HEADER_TEMPLATE		=	$(ROM_HEADER_DIR)/rom_header_$(SMALL_TITLEID).template.sbin
LIBSYSCALL				=	$(ROM_HEADER_DIR)/libsyscall.a

ROM_SPEC				=	main.rsf

MISC_DIR				=	../../misc

BG_DIR					=	../../data

SRCS					=	main.c MachineSetting.c \
							setRTC.c selectLanguage.c calibrationTP.c setOwnerInfo.c selectCountry.c \
							setParentalControl.c setFreeSoftBox.c setWireless.c cleanupMachine.c	\
							setEULA.c getSysMenuVersion.c \
							CountryNameMappingData.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c \
							$(BG_DIR)/BGData_MachineSettings.c

LINCLUDES				=	$(MISC_DIR)/include

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(SYSM_DISABLE_DEBUG)' \
							-DROM_HEADER_TEMPLATE='$(call empath,$(ROM_HEADER_TEMPLATE))'

MAKETAD_OPTION			+= -s

# libsysmenu.a　は、RED本体設定でSVNのリビジョンを取得するためだけに使用しているので、
# UIG本体設定でリンクする必要はありません。
SYSMENU_LIBS			=	\
							libnamut$(TWL_LIBSUFFIX).a \
							libsysmutil$(TWL_LIBSUFFIX).a \
							libsysmenu$(TWL_LIBSUFFIX).a

SDK_APPEND_LIBS			=	\
							liblcfg$(TWL_LIBSUFFIX).a \
							libes$(TWL_LIBSUFFIX).a		\
							libboc$(TWL_LIBSUFFIX).a	\
							libsfs$(TWL_LIBSUFFIX).a	\
							libnam$(TWL_LIBSUFFIX).a	\
							libsea$(TWL_LIBSUFFIX).a	\
							libsharedfont$(TWL_LIBSUFFIX).a \
							libna$(TWL_LIBSUFFIX).a

LLIBRARIES				+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs


ifeq ($(SYSM_OUTSIDE_UI),TRUE)
MACRO_FLAGS += -DOUTSIDE_UI
endif

MAKEROM_ARM7_BASE		=	$(TWL_COMPONENTSDIR)/armadillo_SYSMENU/$(TWL_BUILDTYPE_ARM7)/armadillo_SYSMENU
MAKEROM_ARM7			=	$(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)
MAKEROM					=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

INSTALL_TARGETS			=	$(BINDIR)/$(TARGET_BIN)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
