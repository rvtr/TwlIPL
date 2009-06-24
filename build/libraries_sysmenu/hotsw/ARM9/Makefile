#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL
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
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS				=

#----------------------------------------------------------------------------
TARGET_FIRM			=	SYSTEMMENU
TARGET_PLATFORM		=	TWL
TWL_ARCHGEN			=	LIMITED
TWL_PROC	    	=	ARM9

#----------------------------------------------------------------------------

SRCDIR				=	./src

SRCS				=	hotsw_ctrl.c

TARGET_LIB			=	libhotsw$(TWL_LIBSUFFIX).a

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR			=	$(SYSMENU_INSTALL_LIBDIR)

ifeq ($(HOTSW_NO_MESSAGE),TRUE)
MACRO_FLAGS		   +=	-DHOTSW_NO_MESSAGE
endif

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====

