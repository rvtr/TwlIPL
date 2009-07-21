#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - systemMenu_RED - NandInitializerRed
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
# $Date:: 2009-03-11#$
# $Rev: 2763 $
# $Author: akabane_jumpei $
#----------------------------------------------------------------------------

#----------------------------------------------------------------------------
SYSM_DISABLE_DEBUG	= FALSE

#TARGET_FIRM			= SYSTEMMENU
TARGET_PLATFORM		= TWL
TWL_ARCHGEN    		= LIMITED

#TARGET_CODEGEN   	= THUMB

TITLEID_LO			= 0NRA
#TARGET_TAD			=

TARGET_BIN			= NandFirmWriter.srl
LCFILE_SPEC			= ARM9-TS.lsf
ROM_SPEC			= main.rsf

MAKEROM_ARM7_BASE	= ../ARM7.TWL/bin/$(TWL_BUILDTYPE_ARM7)/racoon
MAKEROM_ARM7		= $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

SRCS				=	main.c \
						common.c \
						font.c \
						screen.c \
						kami_pxi.c \
						kami_write_nandfirm.c \

LINCLUDES			=	../common/include \
						$(ROOT)/build/libraries/lcfg/ARM9.TWL/include \
						../../common/ARM9/include

SRCDIR				=	src \
						../../common/ARM9/src

#LCFILE				=	# using default

#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/commondefs

MAKEROM_FLAGS		+=	-DTITLEID_LO='$(TITLEID_LO)' \
						-DDISABLE_DEBUG='$(SYSM_DISABLE_DEBUG)'

MAKETAD_FLAGS		+=	-s

SDK_APPEND_LIBS		=	\
						libes$(TWL_LIBSUFFIX).a \
						libboc$(TWL_LIBSUFFIX).a \
						libsfs$(TWL_LIBSUFFIX).a \
						libnam$(TWL_LIBSUFFIX).a \
						libsea$(TWL_LIBSUFFIX).a \
						liblcfg$(TWL_LIBSUFFIX).a \
						libna$(TWL_LIBSUFFIX).a 

LLIBRARIES			+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)

MAKEROM			 	= $(TWL_TOOLSDIR)/bin/makerom.TWL.exe

#LDIRT_CLEAN		=
#INSTALL_TARGETS	=
INSTALL_DIR			=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
