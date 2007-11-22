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
# $Rev:  $
# $Author:  $
#----------------------------------------------------------------------------

SUBDIRS					=	

#----------------------------------------------------------------------------

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	MSET
TARGET_WAD				=	$(BINDIR)/$(TITLEID_LO).wad

TARGET_BIN				=	main.srl

ROM_SPEC				=	main.rsf

MISC_DIR				=	../../misc

BG_DIR					=	../../data

SRCS					=	main.c MachineSetting.c \
							setRTC.c selectLanguage.c calibrationTP.c setOwnerInfo.c selectRegion.c selectCountry.c \
						CountryNameMappingData.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c \
						$(BG_DIR)/BGData_MachineSettings.c

LINCLUDES				=	$(MISC_DIR)/include

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)'

include $(TWLIPL_ROOT)/build/buildtools/commondefs


LDIRT_CLEAN				=	$(TARGET_WAD)
INSTALL_TARGETS			=	$(TARGET_WAD)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)

include $(TWLIPL_ROOT)/build/buildtools/modulerules


#----------------------------------------------------------------------------

#===== End of Makefile =====
