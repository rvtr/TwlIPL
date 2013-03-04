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

SYSTEMUPDATER_DIR			=	$(SYSMENU_ROOT)/build/systemMenu_tools/SystemUpdater

SCRIPTS_DIR					=	.

MAKEFILES_ORG			=	$(LAUNCHER_DIR)/ARM9/Makefile
MAKE_MAKEFILES			=	$(SCRIPTS_DIR)/sdlog_makefile.pl
AUTOGEN_MAKEFILES		=	$(LAUNCHER_DIR)/ARM9/Makefile_sdlog
LDIRT_CLEAN				+=	$(AUTOGEN_MAKEFILES)

MAIN_ORG			=	$(LAUNCHER_DIR)/ARM9/src/main.c
MAKE_MAIN			=	$(SCRIPTS_DIR)/sdlog_main_c.pl
AUTOGEN_MAIN		=	$(LAUNCHER_DIR)/ARM9/src/main_sdlog.c
LDIRT_CLEAN				+=	$(AUTOGEN_MAIN)



TARGETS		=	$(AUTOGEN_MAKEFILES) $(AUTOGEN_MAIN)

#----------------------------------------------------------------------------
do-build:	$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

$(AUTOGEN_MAKEFILES): $(MAKEFILES_ORG) $(MAKE_MAKEFILES)
	@echo "     make:" $@
	@perl $(MAKE_MAKEFILES)

$(AUTOGEN_MAIN): $(MAIN_ORG) $(MAKE_MAIN)
	@echo "     make:" $@
	@perl $(MAKE_MAIN)

#===== End of Makefile =====
