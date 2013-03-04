#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - FileRecoveryTest
# File:     Makefile
#
# Copyright **** Nintendo.  All rights reserved.
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
SUBDIRS          = $(TWL_IPL_RED_ROOT)/build/libraries_sysmenu/util/

TARGET_FIRM		=	SYSTEMMENU
TARGET_PLATFORM	=	TWL
TWL_ARCHGEN		=	LIMITED

#----------------------------------------------------------------------------

TARGET_BIN       = main.srl

SRCS             = main.c

LLIBRARIES		+=	libsysmutil$(TWL_LIBSUFFIX).a

ROM_SPEC		=	main.rsf

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
MAKEROM		 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
