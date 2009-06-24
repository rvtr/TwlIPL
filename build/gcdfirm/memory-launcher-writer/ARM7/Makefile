#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - tools - memory-launcher
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

TWL_PROC       = ARM7

SUBDIRS		=

LINCLUDES	=   ../include

#----------------------------------------------------------------------------

TARGET_BIN	=	memory_launcher_writer7.tef

SRCS		=	main.c

CRT0_O		=	crt0_firm.o

ADDRESS_STATIC	=	0x037b8000

#LCFILE_TEMPLATE =	$(FIRM_SPECDIR)/$(TWL_PROC)-$(TWL_PLATFORM)-PARTNER.lcf.template

#SRCDIR		=	# using default
#LCFILE		=	# using default

LINCLUDES	=	$(ROOT)/build/libraries/fatfs/ARM7.TWL/include	\
				$(ROOT)/build/libraries/fatfs/ARM7.TWL/include/fatfs	\
				$(ROOT)/build/libraries/fatfs/ARM7.TWL/include/twl/fatfs/ARM7

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037c0000'

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
