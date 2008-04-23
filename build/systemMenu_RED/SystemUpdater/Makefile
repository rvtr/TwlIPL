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

include	$(TWLSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

LAUNCHER_DIR				=	../Launcher
MACHINESETTINGS_DIR			=	../MachineSettings
WLANFIRM_DIR				=	../wlanfirm
NANDFIRM_DIR                =   ../../nandfirm/menu-launcher

SUBDIRS =	$(LAUNCHER_DIR) \
			$(MACHINESETTINGS_DIR) \
			$(WLANFIRM_DIR) \
			$(NANDFIRM_DIR) \
			data \
			ARM7.TWL \
			ARM9.TWL

# COMPILE SWITCH for build SystemMenu

export UPDATER_RELEASE		=	TRUE

export LAUNCHER_DEBUG_MODE	=	FALSE
export DO_NOT_SHOW_LAUNCHER	=	TRUE
export DISABLE_WDS_SCAN		=	TRUE
export IGNORE_WLFIRM_SIGNCHECK	=	FALSE
export DISABLE_WLFIRM_LOAD	=	FALSE
export DISABLE_SLEEP		=	TRUE
export LOAD_APP_VIA_WRAM	=	FALSE
export SYSM_OUTSIDE_UI		=	TRUE
export SYSM_BROADON_UI		=	FALSE

# MAKEROM SWITCH for build SystemMenu
export DISABLE_DEBUG		=	TRUE
export ENABLE_AES			=	TRUE

#----------------------------------------------------------------------------

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
