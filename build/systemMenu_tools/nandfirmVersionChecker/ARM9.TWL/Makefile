#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - systemMenu_tools - nandfirmVersionChecker
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

SYSM_DISABLE_DEBUG	= TRUE

TARGET_FIRM			= SYSTEMMENU
TARGET_PLATFORM		= TWL
TWL_ARCHGEN    		= LIMITED

#TARGET_CODEGEN   	= THUMB

TITLEID_LO			= 0NVA
#TARGET_TAD			=

TARGET_BIN	= nandfirmVersionChecker.srl

LCFILE_SPEC	= ../../NandInitializerRed/ARM9.TWL/ARM9-TS.lsf
ROM_SPEC	= ./main.rsf

MAKEROM_ARM7_BASE	=	../ARM7.TWL/bin/$(TWL_BUILDTYPE_ARM7)/armadillo
MAKEROM_ARM7		=	$(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

SRCS		=	main.c \
				kami_pxi.c \
				font_data.c \
				graphics.c \
				keypad.c \
				kami_font.c

LINCLUDES	=	../../NandInitializerRed/common/include \
				../../NandInitializerRed/ARM9.TWL/include

SRCDIR		=	./src \
                ../../NandInitializerRed/ARM9.TWL/src

#LCFILE		=	# using default

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DCARD_REGION='$(CARD_REGION)' \
							-DDISABLE_DEBUG='$(SYSM_DISABLE_DEBUG)'

MAKETAD_FLAGS			+=	-s

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

#include	$(TWLSDK_ROOT)/build/buildtools/modulerules
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
