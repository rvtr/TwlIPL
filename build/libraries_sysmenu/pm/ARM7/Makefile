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

SUBDIRS			 =

#----------------------------------------------------------------------------
TARGET_FIRM		=	SYSTEMMENU
TARGET_PLATFORM	=	TWL
TWL_CODEGEN		?=	ALL
TWL_ARCHGEN		=	LIMITED
TWL_PROC		=	ARM7

SRCDIR			=	$(ROOT)/build/libraries/spi/ARM7/pm/src \
					$(ROOT)/build/libraries/spi/ARM7.TWL/pm/src
INCDIR			=	$(ROOT)/build/libraries/spi/ARM7/include \
					$(ROOT)/build/libraries/spi/ARM7/pm/include \
					$(ROOT)/build/libraries/spi/ARM7.TWL/pm/include

SRCS			=  pm_sp.c \
				   pm_send.c \
				   pm_pmic.c \
				   pm_utility.c \
				   pm_sleep.c \
				   pm_selfBlink.c \
				   pm_shutdown.c 


TARGET_LIB		=	libpm_mset_sp$(TWL_LIBSUFFIX).a

MACRO_FLAGS		+=	-DSDK_SYSMENU_BUILD

#----------------------------------------------------------------------------

# DEBUG版ビルドの場合、RELEASE版でビルドして
# DEBUG版のライブラリを装います。

ifdef	TARGET_DEBUG
TWL_BUILD_TYPE	= RELEASE
else
ifdef	TWL_DEBUG
TWL_BUILD_TYPE	= RELEASE
else
ifdef	NITRO_DEBUG
TWL_BUILD_TYPE	= RELEASE
endif
endif
endif


include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	 =	$(TARGETS)
INSTALL_DIR		=	$(SYSMENU_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules
