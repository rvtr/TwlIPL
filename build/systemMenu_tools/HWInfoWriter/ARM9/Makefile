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

#============================================================================
#製品鍵を使った書き込みを行う場合は、TwlIPL/build/buildtools/commondefsの"FIRM_USE_PRODUCT_KEYS=TRUE"を有効にしてください。
#============================================================================


TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	0HWA
#TARGET_TAD				=

TARGET_BIN				=	main.srl

ROM_SPEC				=	main.rsf

MAKEROM_ARM7_BASE   = $(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7        = $(MAKEROM_ARM7_BASE).TWL.FLX.sbin

MISC_DIR				=	../../../systemMenu_RED/misc

SRCS					=	main.c HWInfoWriter.c hwi.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c

LINCLUDES				=	$(MISC_DIR)/include \
							$(ROOT)/build/libraries/lcfg/ARM9.TWL/include

SDK_APPEND_LIBS			=	\
							liblcfg$(TWL_LIBSUFFIX).a

LLIBRARIES				+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS) 	\
							libnamut$(TWL_LIBSUFFIX).a 			\
							libes$(TWL_LIBSUFFIX).a				\
							libnam$(TWL_LIBSUFFIX).a			\
							libsea$(TWL_LIBSUFFIX).a			\
							libboc$(TWL_LIBSUFFIX).a			\
							libsfs$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs


ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
ifdef FIRM_USE_PRODUCT_KEYS
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo.der' \
							-DHWID_PRIVKEY='private_HWID.der'
else
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo_dev.der' \
							-DHWID_PRIVKEY='private_HWID_dev.der'
endif
endif

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DPRIVKEY_PATH='$(TWL_KEYSDIR)/rsa'

MAKETAD_FLAGS			+=	-s

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

#LDIRT_CLEAN				=
#INSTALL_TARGETS			=
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
