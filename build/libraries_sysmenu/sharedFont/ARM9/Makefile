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
# $Date::            $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS			=	

#----------------------------------------------------------------------------
TARGET_FIRM		=	SYSTEMMENU
TARGET_PLATFORM	=	TWL
TWL_ARCHGEN		=	LIMITED
TWL_PROC	   	=	ARM9

SRCDIR			=	src
SRCS			=	sharedFont.c

TARGET_LIB		=	libsharedfont$(TWL_LIBSUFFIX).a

LINCLUDES		+=	$(SYSMENU_ROOT)/build/libraries_sysmenu/sysmenu/ARM9/include

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	=	$(TARGETS)
INSTALL_DIR		=	$(SYSMENU_INSTALL_LIBDIR)

LDIRT_CLEAN		=	$(REVISION_SRC)

#----------------------------------------------------------------------------

do-build: $(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
