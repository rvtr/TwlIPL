#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - nandfirm - menu-launcher2
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

SUBDIRS		=

LINCLUDES	=   ../include

#----------------------------------------------------------------------------

TARGET_BIN	=	menu_launcher2_9.srl

SRCS		=	main.c

CRT0_O		=	crt0_firm.o

MAKEROM_ARM7		=	../ARM7/bin/$(TWL_BUILDTYPE_ARM7)/menu_launcher2_7.tef
MAKEROM_ARM7_BASE       =       $(basename $(MAKEROM_ARM7))

#LCFILE_TEMPLATE =	$(FIRM_SPECDIR)/$(TWL_PROC)-$(TWL_PLATFORM)-PARTNER.lcf.template

#SRCDIR		=	# using default
#LCFILE		=	# using default


include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

# no thumb version, no final rom version
ifeq ($(TARGET_FINALROM),)
LLIBRARIES	+=	libes.TWL$(ARCHGEN_TYPE).a libboc.TWL$(ARCHGEN_TYPE).a
else
LLIBRARIES	+=	../Release/libes.TWL$(ARCHGEN_TYPE).a ../Release/libboc.TWL$(ARCHGEN_TYPE).a
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
