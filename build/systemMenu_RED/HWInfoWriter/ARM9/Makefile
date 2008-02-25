#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - SystemMenuRED - HWInfoWriter
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

SUBDIRS					=	\

#----------------------------------------------------------------------------

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	HIWR
TARGET_TAD				=	$(BINDIR)/$(TITLEID_LO).$(TWL_BUILD_DIR)$(CODEGEN_ARCH).tad

TARGET_BIN				=	main.srl

ROM_SPEC				=	main.rsf

MISC_DIR				=	../../misc

SRCS					=	main.c HWInfoWriter.c hwi.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c

LINCLUDES				=	$(MISC_DIR)/include \
							$(ROOT)/build/libraries/lcfg/ARM9.TWL/include
LLIBRARIES				=	libacsign_enc$(TWL_LIBSUFFIX).a

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DHWINFO_PRIVKEY='private_HWInfo.der'
MAKETAD_FLAGS			+=	-s

TWL_KEYSDIR			+=	-DPRIVKEY_PATH='$(TWL_KEYSDIR)/rsa'
ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
MACRO_FLAGS				+=	-DUSE_PRODUCT_KEY
endif

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEROM			 = $(SYSMENU_TOOLSDIR)/bin/makerom.TWL.sys.exe

LDIRT_CLEAN				=	$(TARGET_TAD)
INSTALL_TARGETS			=	$(TARGET_TAD)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
