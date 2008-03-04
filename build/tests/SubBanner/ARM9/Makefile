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

#----------------------------------------------------------------------------

TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	SBNR

TARGET_BIN				=	SubBanner.tad

ROM_SPEC				=	main.rsf

MISC_DIR				=	../../misc
DATA_DIR				=	../../data

SRCS					=	main.c SubBanner.c \
							$(MISC_DIR)/misc_simple.c \
							$(DATA_DIR)/data.c

LINCLUDES	=	$(TWLSDK_ROOT)/build/demos/gx/UnitTours/DEMOLib/include \
				$(MISC_DIR) \
				$(DATA_DIR)

LLIBRARY_DIRS	+=	$(TWLSDK_ROOT)/build/demos/gx/UnitTours/DEMOLib/lib/$(TWL_BUILDTYPE)

LLIBRARIES				+=	libDEMO$(TWL_LIBSUFFIX).a

MAKEROM_FLAGS			+=	-F -DTITLEID_LO='$(TITLEID_LO)'

include $(TWLSDK_ROOT)/build/buildtools/commondefs

SDK_NMENU_DATADIR				=  $(ROOT)/build/tools/TwlNMenu/data
INSTALL_TARGETS			=	$(BINDIR)/$(TARGET_BIN)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
