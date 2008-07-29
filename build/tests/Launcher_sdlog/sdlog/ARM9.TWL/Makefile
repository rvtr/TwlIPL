#! make -f
#----------------------------------------------------------------------------
# Project:  TWL WLAN - libraries - os
# File:     Makefile
#
# Copyright 2003 - 2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
#----------------------------------------------------------------------------

TARGET_PLATFORM		:= TWL

TWL_PROC		=  ARM9

TWL_CODEGEN		?= ARM
TWL_ARCHGEN		?= LIMITED

SRCDIR		=	./src
INCDIR		=	./include $(ROOT)/build/tests/sdlog/sdlog/include \
				$(ROOT)/build/tests/sdlog/sdlog/src

SRCS			= sd_log_wrapper.c 

SRCS			+= $(ROOT)/build/tests/sdlog/sdlog/src/os_log.c

#----------------------------------------------------------------------------

TARGET_LIB		= libsdlogRed$(TWL_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(TWL_IPL_RED_ROOT)/build/tests/Launcher_sdlog/lib/$(TWL_LIBTYPE)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
