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

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

# SystemUpdaterでインポートするTADは専用のmaketadでTAD化します。
SUBDIR_FLAGS += MAKETAD=$(SYSMENU_TOOLSDIR)/bin/maketad.updater.exe

#----------------------------------------------------------------------------

LAUNCHER_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/Launcher
MACHINESETTINGS_DIR			=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/MachineSettings
WLANFIRM_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/wlanfirm
DS_HASH_TABLE				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/DSHashTable
SHARED_FONT_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/sharedFont
NANDFIRM_DIR                =   $(TWL_IPL_RED_ROOT)/build/nandfirm/menu-launcher

SUBDIRS =	$(LAUNCHER_DIR) \
			$(MACHINESETTINGS_DIR) \
			$(WLANFIRM_DIR) \
			$(DS_HASH_TABLE) \
			$(SHARED_FONT_DIR) \
			$(NANDFIRM_DIR) \
			data \
			banner \
			ARM7.TWL \
			ARM9.TWL

# COMPILE SWITCH for build SystemMenu

export UPDATER_RELEASE		=	TRUE

export LAUNCHER_DEBUG_MODE	=	FALSE
export DO_NOT_SHOW_LAUNCHER	=	TRUE
export DISABLE_WDS_SCAN		=	TRUE
export IGNORE_WLFIRM_SIGNCHECK	=	FALSE
export DISABLE_WLFIRM_LOAD	=	FALSE
export DISABLE_SLEEP		=	FASLE
export SYSM_OUTSIDE_UI		=	TRUE
export SYSM_BROADON_UI		=	FALSE
export AUTH_NTR_CARD_PRODUCT_ONLY =	TRUE

# MAKEROM SWITCH for build SystemMenu
export DISABLE_DEBUG		=	TRUE
export ENABLE_AES			=	TRUE


#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
