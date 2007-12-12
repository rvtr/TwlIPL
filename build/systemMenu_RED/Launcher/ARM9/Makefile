#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - simpleShoot-1
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
# $Date:: $
# $Rev:  $
# $Author:  $
#----------------------------------------------------------------------------

SUBDIRS					=	\
							../../../libraries_sysmenu/sysmenu \
							../../../libraries_sysmenu/settings \
							../../../libraries_sysmenu/boot \
							../../../libraries_sysmenu/hotsw \
							../../../components/hyena.TWL

#----------------------------------------------------------------------------

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	LNCR
TARGET_TAD				=	$(BINDIR)/$(TITLEID_LO).tad

TARGET_BIN				=	main.srl

LCFILE_SPEC				=	ARM9-TS.lsf
ROM_SPEC				=	main.rsf

LOGO_DIR				=	Logo
MISC_DIR				=	../../misc

BG_DIR					=	../../data

SRCS_LOGO				=	logoDemo.c logoData.c
SRCS					=	main.c launcher.c \
							$(addprefix $(LOGO_DIR)/, $(SRCS_LOGO)) \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c \
							$(BG_DIR)/BGData_Launcher.c

LINCLUDES				=	$(MISC_DIR)/include \
							$(SRCDIR)/$(LOGO_DIR)

LLIBRARIES				+=	libnam$(TWL_LIBSUFFIX).a \
							libese$(TWL_LIBSUFFIX).a

ADDRESS_DTCM			=	0x0e000000

DEFAULT_COMP_ARM7		=	hyena

MAKEROM_FLAGS			+=	-F -DTITLEID_LO='$(TITLEID_LO)'

include	$(TWLSDK_ROOT)/add-ins/es/commondefs.es
include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/libraries/reboot/commondefs.reboot

LDIRT_CLEAN				=	$(TARGET_TAD)
INSTALL_TARGETS			=	$(TARGET_TAD)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
