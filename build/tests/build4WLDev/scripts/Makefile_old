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
MACHINESETTINGS_DIR			=	$(SYSMENU_ROOT)/build/systemMenu_RED/MachineSettings

SYSMENU_LIBRARIES_DIR		=	$(SYSMENU_ROOT)/build/libraries_sysmenu

SYSTEMUPDATER_DIR			=	$(SYSMENU_ROOT)/build/systemMenu_tools/SystemUpdater

SCRIPTS_DIR					=	./

MAKEFILES_ORG			=	$(LAUNCHER_DIR)/Makefile $(LAUNCHER_DIR)/ARM9/Makefile \
							$(MACHINESETTINGS_DIR)/Makefile $(MACHINESETTINGS_DIR)/ARM9/Makefile
MAKE_MAKEFILES			=	$(SCRIPTS_DIR)/launcher_makefile.pl
AUTOGEN_MAKEFILES		=	$(LAUNCHER_DIR)/Makefile_autogen $(LAUNCHER_DIR)/ARM9/Makefile_autogen \
							$(MACHINESETTINGS_DIR)/Makefile_autogen $(MACHINESETTINGS_DIR)/ARM9/Makefile_autogen

LDIRT_CLEAN				+=	$(AUTOGEN_MAKEFILES)

LOADWLAN_ORG			=	$(LAUNCHER_DIR)/ARM9/src/loadWlanFirm.c
MAKE_LOADWLAN			=	$(SCRIPTS_DIR)/launcher_loadWlanFirm_c.pl
AUTOGEN_LOADWLAN		=	$(LAUNCHER_DIR)/ARM9/src/loadWlanFirm_autogen.c
LDIRT_CLEAN				+=	$(AUTOGEN_LOADWLAN)

MACHSETTING_ORG			=	$(MACHINESETTINGS_DIR)/ARM9/src/MachineSetting.c
MAKE_MACHSETTING		=	$(SCRIPTS_DIR)/launcher_MachineSetting_c.pl
AUTOGEN_MACHSETTING		=	$(MACHINESETTINGS_DIR)/ARM9/src/MachineSetting_autogen.c
LDIRT_CLEAN				+=	$(AUTOGEN_MACHSETTING)

NWM_ORG					=	$(SYSMENU_LIBRARIES_DIR)/Makefile	\
							$(SYSMENU_LIBRARIES_DIR)/nwm/Makefile	\
							$(SYSMENU_LIBRARIES_DIR)/nwm/ARM7/Makefile	\
							$(ROOT)/build/libraries/wm/ARM7.TWL/nwm/src/nwmsp_event_handler.c
MAKE_NWM				=	$(SCRIPTS_DIR)/launcher_fake_macaddress.pl
AUTOGEN_NWM				=	$(SYSMENU_LIBRARIES_DIR)/Makefile_autogen	\
							$(SYSMENU_LIBRARIES_DIR)/nwm/Makefile_autogen	\
							$(SYSMENU_LIBRARIES_DIR)/nwm/ARM7/Makefile_autogen	\
							$(ROOT)/build/libraries/wm/ARM7.TWL/nwm/src/nwmsp_event_handler_autogen.c
LDIRT_CLEAN				+=	$(AUTOGEN_NWM)


TARGETS		=	$(AUTOGEN_MAKEFILES) $(AUTOGEN_LOADWLAN) $(AUTOGEN_MACHSETTING) $(AUTOGEN_NWM)

#----------------------------------------------------------------------------
do-build:	$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

$(AUTOGEN_MAKEFILES): $(MAKEFILES_ORG) $(MAKE_MAKEFILES)
	@echo "     make:" $@
	@perl $(MAKE_MAKEFILES)

$(AUTOGEN_LOADWLAN): $(LOADWLAN_ORG) $(MAKE_LOADWLAN)
	@echo "     make:" $@
	@perl $(MAKE_LOADWLAN)

$(AUTOGEN_MACHSETTING): $(MACHSETTING_ORG) $(MAKE_MACHSETTING)
	@echo "     make:" $@
	@perl $(MAKE_MACHSETTING)

$(AUTOGEN_NWM): $(NWM_ORG) $(MAKE_NWM)
	@echo "     make:" $@
	@perl $(MAKE_NWM)

#===== End of Makefile =====
