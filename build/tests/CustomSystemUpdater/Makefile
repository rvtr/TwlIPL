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

#----------------------------------------------------------------------------

LAUNCHER_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/Launcher
MACHINESETTINGS_DIR			=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/MachineSettings
WLANFIRM_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/wlanfirm
DS_HASH_TABLE				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/DSHashTable
SHARED_FONT_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/sharedFont
NANDFIRM_DIR                =   $(TWL_IPL_RED_ROOT)/build/nandfirm/menu-launcher

# リージョンが指定されてなかったら日本
ifeq	($(REGION),)
REGION	=	JPN
endif


# 既存データを使用する場合はTRUEを指定
SKIP_CREATE_DATA = TRUE

ifeq ($(SKIP_CREATE_DATA),FALSE)
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
else
SUBDIRS =	banner \
			ARM7.TWL \
			ARM9.TWL
endif

# CustomRegion
export SYSM_REGION						=	$(REGION)

# COMPILE SWITCH for build SystemMenu

export SYSM_UPDATER_RELEASE				=	TRUE

export SYSM_DO_NOT_SHOW_LAUNCHER		=	TRUE
export SYSM_DISABLE_WDS_SCAN			=	FALSE
export SYSM_IGNORE_WLFIRM_SIGNCHECK		=	FALSE
export SYSM_DISABLE_WLFIRM_LOAD			=	FALSE
export SYSM_DISABLE_INITIAL_SETTINGS	=	TRUE
export SYSM_OUTSIDE_UI					=	TRUE

export SYSM_BUILD_FOR_PRODUCTION_TEST	=	FALSE

# MAKEROM SWITCH for build SystemMenu
export SYSM_DISABLE_DEBUG				=	FALSE
export SYSM_USE_WLANFIRM_LOCAL_PUBKEY	=	FALSE


#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
