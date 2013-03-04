#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - components - mongoose.TWL
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

# Definition to override for jackal with sdlog
#

LLIBRARIES				+=	libossdlog_sp$(TWL_LIBSUFFIX).a
LLIBRARY_DIRS			+=	$(call eupath, $(TWL_IPL_RED_ROOT)/build/tests/Launcher_sdlog/lib/$(TWL_LIBTYPE))

COMPONENT_DIR			+=	$(SYSMENU_ROOT)/build/components/jackal.TWL
COMPONENT_SRC_DIR		+=	$(COMPONENT_DIR)/../hyena.TWL/src

override SUBDIRS		=	$(SYSMENU_ROOT)/build/libraries/aes \
							$(SYSMENU_ROOT)/build/components/jackal.TWL/wram_regs

override SRCS			 =$(COMPONENT_SRC_DIR)/crt0.LTD.c $(COMPONENT_SRC_DIR)/main.c $(COMPONENT_SRC_DIR)/initScfg.c

override TARGET_NAME	 =	jackal

# Generate my own lsf from original lsf of mongoose 
LSF_ORG					=	$(COMPONENT_DIR)/$(TARGET_NAME).lsf
MAKE_LSF				=	make_lsf.pl
AUTOGEN_LSF				=	$(TARGET_NAME)_sdlog.lsf

LDIRT_CLEAN				=	$(AUTOGEN_LSF)

override TARGET_NEF		 =	$(TARGET_NAME).tef
override LCFILE_SPEC	 =	$(AUTOGEN_LSF)
override LCFILE_TEMPLATE =	$(COMPONENT_DIR)/$(TARGET_NAME).lcf.template

override TWL_INSTALL_COMPONENTSDIR= $(call eupath, $(TWL_IPL_RED_ROOT)/build/tests/Launcher_sdlog/components)
override INSTALL_DIR		 =	$(TWL_INSTALL_COMPONENTSDIR)/$(TARGET_NAME)/$(TWL_BUILDTYPE)

#

include $(TWL_IPL_RED_ROOT)/build/components/jackal.TWL/Makefile

$(AUTOGEN_LSF): $(LSF_ORG) $(MAKE_LSF)
	@echo "     make:" $@
	@perl $(MAKE_LSF)  $(TARGET_NAME)

#===== End of Makefile =====

