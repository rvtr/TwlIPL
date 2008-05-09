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

SUBDIRS					=	

#----------------------------------------------------------------------------

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TARGET_NUMBER	= f
TARGET_NAME		= RomTypeTest$(TARGET_NUMBER)
BINDIR			= ./bin/$(TWL_BUILDTYPE)/$(TARGET_NAME)
TARGET_BIN				=	$(TARGET_NAME).srl
TITLEID_LO				=	RT$(TARGET_NUMBER)A
TARGET_TAD				=	$(BINDIR)/$(TITLEID_LO).$(TWL_BUILD_DIR)$(CODEGEN_ARCH).tad

ROM_SPEC				=	main$(TARGET_NUMBER).rsf

MISC_DIR				=	../../../systemMenu_RED/misc

SRCS					=	main.c RomTypeTest.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c

LINCLUDES				=	$(MISC_DIR)/include

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)'
MAKETAD_FLAGS			+= -s

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.sys.exe

LDIRT_CLEAN				=	$(TARGET_TAD)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
