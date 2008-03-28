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

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	HMSA
TARGET_TAD				=	$(BINDIR)/$(TITLEID_LO).$(TWL_BUILD_DIR)$(CODEGEN_ARCH).tad

TARGET_BIN				=	main.srl

ROM_SPEC				=	main.rsf

MISC_DIR				=	../../misc

BG_DIR					=	../../data

SRCS					=	main.c MachineSetting.c \
							setRTC.c selectLanguage.c calibrationTP.c setOwnerInfo.c selectCountry.c \
							setParentalControl.c setFreeSoftBox.c\
						CountryNameMappingData.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c \
						$(BG_DIR)/BGData_MachineSettings.c

LINCLUDES				=	$(MISC_DIR)/include

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)'

MAKETAD_FLAGS			+= -s

LLIBRARIES				=	libsea$(TWL_LIBSUFFIX).a

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEROM_ARM7_BASE		=	$(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7			=	$(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)
MAKEROM					=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

LDIRT_CLEAN				=	$(TARGET_TAD)
INSTALL_TARGETS			=	$(TARGET_TAD)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#----------------------------------------------------------------------------

#===== End of Makefile =====
