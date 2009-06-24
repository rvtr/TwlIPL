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

LAUNCHER_DIR				=	$(SYSMENU_ROOT)/build/systemMenu_RED/Launcher

SYSMENU_LIBRARIES_DIR		=	$(SYSMENU_ROOT)/build/libraries_sysmenu

SYSTEMUPDATER_DIR			=	$(SYSMENU_ROOT)/build/systemMenu_tools/SystemUpdaterRegionSelect

SCRIPTS_DIR					=	./

MAKEFILES_ORG			=	$(LAUNCHER_DIR)/ARM9/Makefile
MAKE_MAKEFILES			=	$(SCRIPTS_DIR)/launcher_makefile_region.pl
AUTOGEN_MAKEFILES		=	$(LAUNCHER_DIR)/ARM9/Makefile_autogen_HNAJ $(LAUNCHER_DIR)/ARM9/Makefile_autogen_HNAE

LDIRT_CLEAN				+=	$(AUTOGEN_MAKEFILES)

LOADWLAN_ORG			=	$(LAUNCHER_DIR)/ARM9/src/loadWlanFirm.c
MAKE_LOADWLAN			=	$(SCRIPTS_DIR)/launcher_loadWlanFirm_c.pl
AUTOGEN_LOADWLAN		=	$(LAUNCHER_DIR)/ARM9/src/loadWlanFirm_autogen.c
LDIRT_CLEAN				+=	$(AUTOGEN_LOADWLAN)

SYSUP_ORG				=	$(SYSTEMUPDATER_DIR)/ARM9.TWL/src/process_check_console.c \
							$(SYSTEMUPDATER_DIR)/ARM9.TWL/src/process_select_region.c \
							$(SYSTEMUPDATER_DIR)/ARM9.TWL/Makefile
MAKE_SYSUP				=	$(SCRIPTS_DIR)/systemupdater_source.pl
AUTOGEN_SYSUP			=	$(SYSTEMUPDATER_DIR)/ARM9.TWL/src/process_check_console_autogen.c \
							$(SYSTEMUPDATER_DIR)/ARM9.TWL/src/process_select_region_autogen.c \
							$(SYSTEMUPDATER_DIR)/ARM9.TWL/Makefile_autogen
LDIRT_CLEAN				+=	$(AUTOGEN_SYSUP)


TARGETS		=	$(AUTOGEN_MAKEFILES) $(AUTOGEN_LOADWLAN) $(AUTOGEN_SYSUP)

#----------------------------------------------------------------------------
do-build:	$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

$(AUTOGEN_MAKEFILES): $(MAKEFILES_ORG) $(MAKE_MAKEFILES)
	@echo "     make:" $@
	@perl $(MAKE_MAKEFILES)

$(AUTOGEN_LOADWLAN): $(LOADWLAN_ORG) $(MAKE_LOADWLAN)
	@echo "     make:" $@
	@perl $(MAKE_LOADWLAN)

$(AUTOGEN_SYSUP): $(SYSUP_ORG) $(MAKE_SYSUP)
	@echo "     make:" $@
	@perl $(MAKE_SYSUP)


#===== End of Makefile =====
