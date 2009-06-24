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

override TARGET_PLATFORM	= TWL
override TARGET_CODEGEN		= THUMB
override TWL_ARCHGEN		= LIMITED


TARGET_FIRM				=	SYSTEMMENU

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

#.PHONY: no_rom_set
#no_rom_set:
#	echo "Usage: make ROMSET=dirname"
#
#ifeq	($(ROMSET),)
#	no_rom_set
#endif
#

# リージョンが指定されてなかったら全部やる
ifeq	($(REGION),)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:	do-something
clean:		do-something
clobber:	do-something
install:	do-something
do-build:	MY_GOAL=
clean:		MY_GOAL=clean
clobber:	MY_GOAL=clobber
install:	MY_GOAL=install

do-something:
	-$(REMAKE) TWLSDK_PLATFORM=TWL REGION=JPN ROMSET=$(ROMSET) DISABLE_ROMCHECK=$(DISABLE_ROMCHECK) SKIP_SUBDIR=FALSE $(MY_GOAL)
	-rm `find . -name "*autogen*"`
	-$(REMAKE) TWLSDK_PLATFORM=TWL REGION=AUS ROMSET=$(ROMSET) DISABLE_ROMCHECK=$(DISABLE_ROMCHECK) SKIP_SUBDIR=FALSE $(MY_GOAL)
	-rm `find . -name "*autogen*"`
	-$(REMAKE) TWLSDK_PLATFORM=TWL REGION=USA ROMSET=$(ROMSET) DISABLE_ROMCHECK=$(DISABLE_ROMCHECK) SKIP_SUBDIR=FALSE $(MY_GOAL)
	-rm `find . -name "*autogen*"`
	-$(REMAKE) TWLSDK_PLATFORM=TWL REGION=EUR ROMSET=$(ROMSET) DISABLE_ROMCHECK=$(DISABLE_ROMCHECK) SKIP_SUBDIR=FALSE $(MY_GOAL)
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

else

LAUNCHER_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/Launcher
MACHINESETTINGS_DIR			=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/MachineSettings
WLANFIRM_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/wlanfirm
DS_HASH_TABLE				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/DSHashTable
SHARED_FONT_DIR				=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/sharedFont
NANDFIRM_DIR                =   $(TWL_IPL_RED_ROOT)/build/nandfirm/menu-launcher



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

# CustomSystemUpdater
export SYSM_REGION						=	$(REGION)
export ROMSET	:=	$(ROMSET)
export DISABLE_ROMCHECK	:=	$(DISABLE_ROMCHECK)

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

endif

#===== End of Makefile =====
