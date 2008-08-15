#! make -f
#----------------------------------------------------------------------------
# Project:  TWL WLAN - twl - 
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
#----------------------------------------------------------------------------

TARGET_FIRM			=	SYSTEMMENU

override TARGET_PLATFORM	:= TWL
override TWL_CODEGEN		:= ARM
override TWL_ARCHGEN		:= LIMITED
export TWL_CODEGEN
export TWL_ARCHGEN
TWL_PROC					:= ARM7

# always build only ARM codes under sub processes
SUBDIR_FLAGS				= TWL_CODEGEN=$(TWL_CODEGEN)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

export TWL_WLAN_ROOT	= $(subst $(SPACE),\ ,$(subst \,/,$(TWL_IPL_RED_ROOT)))
export TWLWLAN_DRV_ROOT	= $(subst $(SPACE),\ ,$(subst \,/,$(TWLWIRELESS_ROOT)))

#----------------------------------------------------------------------------

# set enviromental variables for TianShan
ATH_OS_TYPE := TWL
ATH_WLAN_DRV_PATH := $(TWLWLAN_DRV_ROOT)
NIN_PACKAGE_PATH := 

# COM_TYPE = [SERIAL/SPIUSB]
COM_TYPE ?= SPIUSB

# env. variables for TianShan tree
export ATH_OS_TYPE
export NIN_PACKAGE_PATH
export ATH_WLAN_DRV_PATH
export COM_TYPE

export TARGET_FIRM
export LIBDIR			 = ./lib.SYSMENU/$(TWL_BUILDTYPE)
export OBJDIR			 = ./obj.SYSMENU/$(TWL_BUILDTYPE)
export DEPENDDIR		 = ./depend.SYSMENU/$(TWL_BUILDTYPE)
export MACRO_FLAGS  = -DSDK_SYSMENU_BUILD

export TWLSDK_INSTALL_ROOT	=	$(SYSMENU_INSTALL_LIBDIR)/../../..

export LDIRT_CLEAN = ./obj.SYSMENU ./depend.SYSMENU ./lib.SYSMENU

# Install 
INSTALL_DIR     =  $(call eupath,$(TWLSDK_INSTALL_ROOT)/lib/TwlWireless/$(TWL_LIBTYPE))
INSTALL_TARGETS	=	$(INSTALL_DIR)/libsdio_sp.SYSMENU$(TWL_LIBSUFFIX).a \
					$(INSTALL_DIR)/libathdrv_sp.SYSMENU$(TWL_LIBSUFFIX).a

SUBDIRS			=	$(TWLWIRELESS_ROOT)

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

# Override generic DO_INSTALL
define DO_INSTALL
	$(INSTALL) -d $(INSTALL_DIR) $(AND)					\
	$(foreach FILE, $(INSTALL_TARGETS),			   		\
		if [ ! -e $(INSTALL_DIR)/$(FILE) -o			\
			$(FILE) -nt $(INSTALL_DIR)/$(FILE) ];		\
		then 								\
			$(ECHO) "  install: $(subst .SYSMENU,,$(FILE)) -> $(FILE)" $(AND)	\
			$(INSTALL) $(subst .SYSMENU,,$(FILE)) $(INSTALL_DIR)/$(notdir $(FILE)) $(AND)	\
			$(RM) $(INSTALL_DIR)/$(notdir $(subst .SYSMENU,,$(FILE)));	\
		fi $(AND) ) $(TRUE)
endef


#===== End of Makefile =====

