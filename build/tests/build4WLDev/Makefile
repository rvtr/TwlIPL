#! make -f
#----------------------------------------------------------------------------
# Project:  SystemUpdater
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

TARGET_FIRM				=	SYSTEMMENU

# force to build for debugger
export SYSMENU_DEBUGGER_BUILD		=	TRUE

# SDK configuration for export
override TWL_ARCHGEN	:= LIMITED
export TWL_ARCHGEN
export OBJDIR			= ./obj.WLAN/$(TWL_BUILDTYPE)
export DEPENDDIR		= ./depend.WLAN/$(TWL_BUILDTYPE)
export LIBDIR			= ./lib.WLAN/$(TWL_BUILDTYPE)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

export LDIRT_CLEAN = ./obj.WLAN ./depend.WLAN ./lib.WLAN

#----------------------------------------------------------------------------

LAUNCHER_DIR				=	$(SYSMENU_ROOT)/build/systemMenu_RED/Launcher
MACHINESETTINGS_DIR			=	$(SYSMENU_ROOT)/build/systemMenu_RED/MachineSettings
WLANFIRM_DIR				=	$(SYSMENU_ROOT)/build/systemMenu_RED/wlanfirm
DS_HASH_TABLE				=	$(SYSMENU_ROOT)/build/systemMenu_RED/DSHashTable
SHARED_FONT_DIR				=	$(SYSMENU_ROOT)/build/systemMenu_RED/sharedFont
NANDFIRM_DIR                =   $(SYSMENU_ROOT)/build/nandfirm/menu-launcher

SYSTEMUPDATER_DIR			=	$(SYSMENU_ROOT)/build/systemMenu_tools/SystemUpdater

SUBMAKES +=	./scripts/Makefile											\
			$(SYSMENU_ROOT)/build/tools/Makefile						\
			$(SYSMENU_ROOT)/build/libraries/Makefile					\
			$(SYSMENU_ROOT)/build/libraries_sysmenu/Makefile_autogen	\
																		\
			$(LAUNCHER_DIR)/Makefile_autogen							\
			$(MACHINESETTINGS_DIR)/Makefile_autogen						\
			$(WLANFIRM_DIR)/Makefile									\
			$(DS_HASH_TABLE)/Makefile									\
			$(SHARED_FONT_DIR)/Makefile									\
			$(NANDFIRM_DIR)/Makefile									\
			$(SYSTEMUPDATER_DIR)/data/Makefile							\
			$(SYSTEMUPDATER_DIR)/banner/Makefile						\
			$(SYSTEMUPDATER_DIR)/ARM7.TWL/Makefile						\
			$(SYSTEMUPDATER_DIR)/ARM9.TWL/Makefile

# COMPILE SWITCH for build SystemMenu

export UPDATER_RELEASE				=	TRUE

export DO_NOT_SHOW_LAUNCHER			=	TRUE
export DISABLE_WDS_SCAN				=	FALSE

#############################################
# For Wireless development
#

# disable signature check
export IGNORE_WLFIRM_SIGNCHECK		=	TRUE

export DISABLE_WLFIRM_LOAD			=	FALSE
export DISABLE_INITIAL_SETTINGS		=	TRUE
export SYSM_OUTSIDE_UI				=	TRUE

# MAKEROM SWITCH for build SystemMenu
export DISABLE_DEBUG				=	TRUE
export USE_WLANFIRM_LOCAL_PUBKEY	=	FALSE

DATE_AND_TIME	?= $(shell date +"%y%m%d %H%M")

DATE		?= $(word 1,$(DATE_AND_TIME))
TIME		?= $(word 2,$(DATE_AND_TIME))
TIME_NOZERO	 = $(shell echo $(TIME) | sed -e 's/^0*\(.\)/\1/')

INSTALL_TARGETS	 = $(SYSTEMUPDATER_DIR)/ARM9.TWL/bin/$(TWL_BUILDTYPE)/SystemUpdater.srl
INSTALL_DIR		 = .

GDIRT_INSTALLED	+= ./$(notdir $(subst .srl,_WL_$(DATE)_*.srl,$(INSTALL_TARGETS)))

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

# Override generic DO_INSTALL
define DO_INSTALL
	$(INSTALL) -d $(INSTALL_DIR) $(AND)					\
	$(foreach FILE, $(INSTALL_TARGETS),			   		\
		if [ ! -e $(INSTALL_DIR)/$(FILE) -o			\
			$(FILE) -nt $(INSTALL_DIR)/$(FILE) ];		\
		then 								\
			$(ECHO) "  install: $(FILE) -> $(subst .srl,_WL_$(DATE)_$(TIME).srl,$(FILE))" $(AND)	\
			$(INSTALL) $(FILE) $(INSTALL_DIR)/$(notdir $(subst .srl,_WL_$(DATE)_$(TIME).srl,$(FILE)));	\
		fi $(AND) ) $(TRUE)
endef

test:
	@echo $(GDIRT_INSTALLED)

#===== End of Makefile =====
