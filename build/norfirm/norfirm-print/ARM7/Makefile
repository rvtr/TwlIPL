#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - tools - norfirm-print
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

TWL_PROC      = ARM7

SUBDIRS		=

LINCLUDES	=   ../include

#----------------------------------------------------------------------------

TARGET_BIN	=	twl_norfirm7_print.tef

CRT0_O		=	crt0_firm.o

SRCS		=	\
                main.c \

#SRCDIR		=	# using default
#LCFILE		=	# using default

MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037c0000'

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
