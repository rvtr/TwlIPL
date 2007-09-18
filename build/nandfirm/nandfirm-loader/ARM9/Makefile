#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - tools - nandfirm-loader
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

TARGET_BIN	=	twl_nandfirm9_loader.axf

SRCS		=	main.c

CRT0_O		=	crt0_firm.o

#SRCDIR		=	# using default
#LCFILE		=	# using default


include	$(TWLFIRM_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLFIRM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
