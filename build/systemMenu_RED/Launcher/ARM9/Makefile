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
							../../../libraries_sysmenu/boot \
							../../../components/hyena.TWL

#----------------------------------------------------------------------------

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	LNCR
TARGET_WAD				=	$(BINDIR)/$(TITLEID_LO).wad

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

DEFAULT_COMP_ARM7			=	hyena
DEFAULT_MAKEROM_ARM7_BASE	=	$(TWLIPL_ROOT)/components/$(DEFAULT_COMP_ARM7)/$(TWL_BUILDTYPE_ARM7)/$(DEFAULT_COMP_ARM7)

MAKEROM_FLAGS			+=	-F -DTITLEID_LO='$(TITLEID_LO)'

include $(TWLIPL_ROOT)/build/buildtools/commondefs

MAKELCF_FLAGS			+=	-DADDRESS_BOOTCORE='0x02e7fc00'

LDIRT_CLEAN				=	$(TARGET_WAD)
INSTALL_TARGETS			=	$(TARGET_WAD)
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(FONT_OBJS) $(TARGETS)


include $(TWLIPL_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
