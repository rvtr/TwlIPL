#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - components - racoon.TWL
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

# Definition to override for racoon_wckr
#

LLIBRARIES				+=	libnwm_sp.WCKR$(TWL_LIBSUFFIX).a
LLIBRARY_DIRS			+=	../../libraries/nwm/ARM7/lib/$(TWL_BUILDTYPE)

COMPONENT_DIR			+=	$(ROOT)/build/components/racoon.TWL
COMPONENT_SRC_DIR			+=	$(ROOT)/build/components/mongoose.TWL/src

override SRCS			 =	$(COMPONENT_SRC_DIR)/main.c $(COMPONENT_SRC_DIR)/initScfg.c

override TARGET_NAME	 =	racoon

# Generate my own lsf from original lsf of mongoose 
LSF_ORG					=	$(COMPONENT_DIR)/$(TARGET_NAME).lsf
MAKE_LSF				=	make_lsf.pl
AUTOGEN_LSF				=	$(TARGET_NAME)_wckr.lsf

LDIRT_CLEAN				=	$(AUTOGEN_LSF)

override TARGET_NEF		 =	$(TARGET_NAME).tef
override LCFILE_SPEC	 =	$(AUTOGEN_LSF)
override LCFILE_TEMPLATE =	$(COMPONENT_DIR)/$(TARGET_NAME).lcf.template
override LDRES_TEMPLATE	 =	$(COMPONENT_DIR)/$(TARGET_NAME).response.template

override TWL_INSTALL_COMPONENTSDIR= 
override INSTALL_TARGETS	 =
override INSTALL_DIR		 =

# 

include $(TWLSDK_ROOT)/build/components/racoon.TWL/Makefile

$(AUTOGEN_LSF): $(LSF_ORG) $(MAKE_LSF)
	@echo "     make:" $@
	@perl $(MAKE_LSF)  $(TARGET_NAME)

#===== End of Makefile =====
