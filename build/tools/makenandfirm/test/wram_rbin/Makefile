#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - tools - nandfirm-print
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

SUBDIRS		=

LINCLUDES	=   ../include

#----------------------------------------------------------------------------

TARGET_BIN	=	wram_regs.rbin

SRCS		=	\
                wram_regs.c \

#SRCDIR		=	# using default
#LCFILE		=	# using default


include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_DIR	= .
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
