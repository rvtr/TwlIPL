#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - nandfirm - menu-launcher
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
# $Rev:$
# $Author:$
#----------------------------------------------------------------------------

FIRM_TARGET    = FIRM

SUBDIRS		=

LINCLUDES	=   ../include

#----------------------------------------------------------------------------

TARGET_BIN	=	menu_launcher9.srl

SRCS		=	main.c

CRT0_O		=	crt0_firm.o

MAKEROM_ARM9		=	ARM9/bin/ARM9-$(FIRM_PLATFORM)$(CODEGEN_ARCH)/$(FIRM_BUILD_DIR)/menu_launcher9.tef
MAKEROM_ARM7		=	../ARM7/bin/ARM7-$(FIRM_PLATFORM)$(CODEGEN_ARCH)/$(FIRM_BUILD_DIR)/menu_launcher7.tef
MAKEROM_ARM7_BASE       =       $(basename $(MAKEROM_ARM7))
MAKEROM_FLAGS		+=	-F

#LCFILE_TEMPLATE =	$(FIRM_SPECDIR)/$(FIRM_PROC)-$(FIRM_PLATFORM)-PARTNER.lcf.template

#SRCDIR		=	# using default
#LCFILE		=	# using default


include	$(TWLIPL_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLIPL_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
