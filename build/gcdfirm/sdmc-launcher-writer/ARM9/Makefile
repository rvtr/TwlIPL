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
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS		=

#LINCLUDES	=   ../include

#----------------------------------------------------------------------------

TARGET_BIN	=	sdmc_launcher_writer9.srl

SRCS		=	main.c \
                screen.c \
                font.c

CRT0_O		=	crt0_firm.o

ADDRESS_STATIC	=	0x037c0000

MAKEROM_ARM7		=	../ARM7/bin/$(TWL_BUILDTYPE_ARM7)/sdmc_launcher_writer7.tef
MAKEROM_ARM7_BASE       =       $(basename $(MAKEROM_ARM7))

#LCFILE_TEMPLATE =	$(FIRM_SPECDIR)/$(TWL_PROC)-$(TWL_PLATFORM)-PARTNER.lcf.template

#SRCDIR		=	# using default
#LCFILE		=	# using default

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
