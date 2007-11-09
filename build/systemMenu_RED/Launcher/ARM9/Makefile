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

SUBDIRS					=	../../../libraries_sysmenu/sysmenu \
							../../../libraries_sysmenu/boot \
							../../../components/hyena.TWL

#----------------------------------------------------------------------------

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TARGET_BIN				=	main.srl

LCFILE_SPEC				=	ARM9-TS.lsf
ROM_SPEC				=	main.rsf

LOGO_DIR				=	Logo

SRCS_LOGO				=	logoDemo.c logoData.c
SRCS					=	main.c launcher.c misc.c $(addprefix $(LOGO_DIR)/, $(SRCS_LOGO))
#SRCS_OVERLAY			=	

LINCLUDES				=	$(SRCDIR)/$(LOGO_DIR)
#LLIBRARY_DIRS			=	
#LLIBRARIES				=	
#LDEPENDS_NEF			=	

LLIBRARIES		+=	libnam$(TWL_LIBSUFFIX).a\
				libese$(TWL_LIBSUFFIX).a\
				libboot$(TWL_LIBSUFFIX).a

ADDRESS_DTCM	= 0x0e000000

DEFAULT_COMP_ARM7			=	hyena
DEFAULT_MAKEROM_ARM7_BASE	=	$(TWLIPL_ROOT)/components/$(DEFAULT_COMP_ARM7)/$(TWL_BUILDTYPE_ARM7)/$(DEFAULT_COMP_ARM7)

include $(TWLIPL_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build : $(FONT_OBJS) $(TARGETS)


include $(TWLIPL_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
