#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK
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
override TARGET_PLATFORM = TWL

TWL_CODEGEN		?= ARM
TWL_ARCHGEN		?= LIMITED
TWL_PROC		 = ARM7

OS_LIBRARY_SRC_DIR_BASE = $(ROOT)/build/libraries/os
OS_LIBSRC_ARM7 = $(OS_LIBRARY_SRC_DIR_BASE)/ARM7/src
OS_LIBSRC_ARM7TWL = $(OS_LIBRARY_SRC_DIR_BASE)/ARM7.TWL/src
OS_LIBSRC_COMMON = $(OS_LIBRARY_SRC_DIR_BASE)/common/src

SRCDIR			 = src ../common/src ../ARM7/src
INCDIR			 = $(OS_LIBSRC_ARM7TWL)/../../common/include

SRCS			 = $(OS_LIBSRC_ARM7TWL)/os_irqHandler.c		\
				   $(OS_LIBSRC_COMMON)/os_irqTable.c		\
				   $(OS_LIBSRC_COMMON)/os_interrupt.c		\
				   $(OS_LIBSRC_COMMON)/os_pxi.c				\
				   $(OS_LIBSRC_COMMON)/os_spinLock.c		\
				   os_printf.c								\
				   $(OS_LIBSRC_COMMON)/os_thread.c			\
				   $(OS_LIBSRC_COMMON)/os_context.c			\
				   $(OS_LIBSRC_COMMON)/os_emulator.c		\
				   $(OS_LIBSRC_COMMON)/os_message.c			\
				   $(OS_LIBSRC_COMMON)/os_mutex.c			\
				   $(OS_LIBSRC_COMMON)/os_init.c			\
				   $(OS_LIBSRC_COMMON)/os_arena.c			\
				   $(OS_LIBSRC_COMMON)/os_alloc.c			\
				   $(OS_LIBSRC_COMMON)/os_exception.c		\
				   $(OS_LIBSRC_COMMON)/os_timer.c			\
				   $(OS_LIBSRC_COMMON)/os_tick.c			\
				   $(OS_LIBSRC_COMMON)/os_alarm.c			\
				   $(OS_LIBSRC_COMMON)/os_valarm.c			\
				   $(OS_LIBSRC_COMMON)/os_system.c			\
				   $(OS_LIBSRC_COMMON)/os_systemWork.c		\
				   $(OS_LIBSRC_COMMON)/os_profile.c			\
				   $(OS_LIBSRC_COMMON)/os_reset.c			\
				   $(OS_LIBSRC_COMMON)/os_ownerInfo.c		\
				   $(OS_LIBSRC_COMMON)/os_ownerInfoEx.c		\
				   $(OS_LIBSRC_COMMON)/os_entropy.c			\
				   $(OS_LIBSRC_ARM7)/os_terminate_sp.c		\
				   $(OS_LIBSRC_COMMON)/os_event.c			\
				   $(OS_LIBSRC_COMMON)/os_application_jump.c

TARGET_LIB		= libossdlog_sp$(TWL_LIBSUFFIX).a

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

#----------------------------------------------------------------------------
# HYBRID / LIMITED 両方のライブラリを作成します
TWL_ARCHGEN		?=	ALL

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS	 = $(TARGETS)
INSTALL_DIR		= $(TWL_IPL_RED_ROOT)/build/tests/Launcher_sdlog/lib/$(TWL_LIBTYPE)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
