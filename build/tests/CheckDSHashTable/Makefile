#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - CheckDSHashTable
# File:     Makefile
#
# Copyright 2008 Nintendo.  All rights reserved.
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

SUBDIRS		=

#----------------------------------------------------------------------------

TARGET_PLATFORM		:= TWL
TARGET_FIRM		:= SYSTEMMENU
override TWL_ARCHGEN	:= LIMITED

SRCS			= main.c
TARGET_BIN		= CheckDSHashTable.srl
ROM_SPEC		= CheckDSHashTable.rsf

DS_HASH_TABLE_SRL_ROOT	?= D:/SRL

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

LLIBRARIES			+=	libdht$(TWL_LIBSUFFIX).a


MAKEROM_FLAGS		+= -DMAKEROM_SRL_ROOT=$(DS_HASH_TABLE_SRL_ROOT)


#----------------------------------------------------------------------------

do-build:	$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
