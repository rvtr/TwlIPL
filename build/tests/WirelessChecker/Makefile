#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools - Wireless Checker
# File:     Makefile
#
# Copyright 2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date::             $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

override TARGET_PLATFORM	= TWL
override TARGET_CODEGEN		= ARM
override TWL_ARCHGEN		= LIMITED

SUBDIRS          = libraries components rsf banner

RES_DIR			 = ./resources

SRCDIR           = src
INCDIR           = include $(ROOT)/build/libraries/wm/common.TWL/inc \
				   $(ROOT)/build/libraries/wm/ARM9.TWL/nwm/inc

ifeq	($(TWL_NANDAPP),TRUE)
ROM_SPEC		 = rsf/wireless_checker_nand.rsf
else
ROM_SPEC		 = rsf/wireless_checker.rsf
endif

BANNER_FILE		 = ./banner/000A_WLCK.bnr
MAKEROM_FLAGS	 =	-DBANNER_FILE='$(BANNER_FILE)'

#----------------------------------
# Font

FONTCVTR_DIR	= $(call empath,$(TWLSYSTEM_ROOT))/tools/fontcvtr
FONT_BMP	= $(RES_DIR)/ds_ipl10.bmp

TARGET_OBJ	+=	$(FONT_O)
EXT_OBJS	+=	$(OBJDIR)/$(FONT_O)


FONT_RES_LC		= $(RES_DIR)/font.NFTR
FONT_O_LC		= font.o

FONT_RES	= $(FONT_RES_LC)
FONT_O		= $(FONT_O_LC)

MACRO_FLAGS	+= -DTWLNMENU_USE_LC_FONT


LDIRT_CLEAN	+=	$(FONT_RES_LC) $(FONT_RES_NO_LC) *.rsf $(REVISION_C)

#-----------------------------------
# For CTR compatibility check
#
# To build this imeage, add CHECK_ON_CTR=TRUE.
# make CHECK_ON_CTR=TRUE

ifeq	($(CHECK_ON_CTR),TRUE)
BASENAME    += WirelessChecker_for_CTRcheck
MACRO_FLAGS += -DCHECK_ON_CTR
else
BASENAME    += WirelessChecker
endif

#----------------------------------------------------------------------------

ifeq	($(TWL_NANDAPP),TRUE)
TARGET_BIN       = $(BASENAME)_nand.tad
else
TARGET_BIN       = $(BASENAME).srl
endif

ifeq	($(TWL_NANDAPP),TRUE)
LCFILE_AUTOGEN	= $(BINDIR)/$(notdir $(LCFILE_SPEC:.lsf=.autogen.nand.lcf))
LDRES_AUTOGEN	= $(BINDIR)/$(notdir $(LCFILE_SPEC:.lsf=.autogen.nand.response))
endif

SRCS             = main.cpp				\
				   frame.cpp			\
				   test.cpp				\
				   test_wm.cpp			\
				   test_nwm.cpp			\
				   test_fatal.cpp		\
				   test_recovery.cpp	\
				   util/util.cpp		\
				   util/canvas.cpp		\
				   util/wprintf.cpp		\
				   util/memory.cpp

LINCLUDES       =  $(ROOT)/build/libraries/wm/ARM9.TWL/nwm/inc	\
				   $(ROOT)/build/libraries/wm/common.TWL/inc 

LLIBRARY_DIRS   =   
LLIBRARIES      =   

COMPONENT_NAME      =   racoon
MAKEROM_ARM7_BASE   =   ./components/racoon_wckr/bin/$(TWL_BUILDTYPE_ARM7)/racoon
MAKEROM_ARM7        =   $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

include	$(TWLSYSTEM_ROOT)/build/buildtools/commondefs

LIBRARIES_UNLINK   := libnwm$(TWL_LIBSUFFIX).a

LLIBRARY_DIRS		+= ./libraries/nwm/ARM9/lib/$(TWL_BUILDTYPE_ARM9)
LLIBRARIES			+= libnwm.WCKR$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------

# convert font

$(FONT_RES):	$(FONT_BMP) $(FONT_FILTER)
ifdef SILENT
	echo "make font:" $(notdir $<)
endif
	$(FONTCVTR_DIR)/fontcvtrc.exe									\
		-i bmp -if $< -io $(FONTCVTR_DIR)/xlor/ds_ipl.xlor -ib 1	\
		-o nitro -of $@ -oe utf16 -oa 0x23 $(FONT_OPTION)

$(OBJDIR)/$(FONT_O):	$(FONT_RES)
ifdef SILENT
	echo " b2o font:" $(notdir $<)
endif
	$(BIN2OBJ) $< $@ --begin font_NFTR_begin --end font_NFTR_end --align 32

INSTALL_TARGETS			 =   $(BINDIR)/$(TARGET_BIN)
INSTALL_DIR				 =   ./
#$(TWLSDK_INSTALL_ROOT)/bin/$(CODEGEN_PROC)-$(TWL_PLATFORM)/$(TWL_BUILD_DIR)

do-build:		$(TARGETS)
ifneq	($(TWL_NANDAPP),TRUE)
	$(REMAKE) TWL_NANDAPP=TRUE SKIP_SUBDIR=TRUE
endif

include	$(TWLSYSTEM_ROOT)/build/buildtools/modulerules
include $(TWLSDK_ROOT)/build/buildtools/twl/modulerules.add-ins.nadk

#===== End of Makefile =====
