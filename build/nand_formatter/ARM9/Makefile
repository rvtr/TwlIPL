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
# $Date:: 2007-09-18 #$
# $Rev: 972 $
# $Author: hatamoto_minoru $
#----------------------------------------------------------------------------

TARGET_PLATFORM     = TWL

TWL_PROC            = ARM9
#TWL_ARCHGEN         = LIMITED
TARGET_BIN          = formatter.srl

MAKEROM_ARM7_BASE   = ../ARM7/bin/$(TWL_BUILDTYPE_ARM7)/formatter_sub
MAKEROM_ARM7        = $(MAKEROM_ARM7_BASE).tef

SRCS                = main.c

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

SUBDIRS		= ../ARM7

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
