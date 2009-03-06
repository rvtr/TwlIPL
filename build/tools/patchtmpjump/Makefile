#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - patchtmpjump
#  File:     Makefile
#
#  Copyright 2009 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date::            $
#  $Rev$
#  $Author$
#---------------------------------------------------------------------------

TWLSDK_PLATFORM	= NITRO
TARGET_PLATFORM	= NITRO

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

TARGETS			= patchtmpjump.exe

SOURCES			= patchtmpjump.c
OBJECTS			= patchtmpjump.o

#INSTALL_DIR		= $(TWL_INSTALL_TOOLSDIR)/bin
#INSTALL_TARGETS	= $(TARGETS)

LDIRT_CLEAN		= $(OBJECTS) $(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules.x86


#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJECTS)
			$(CC_X86) $+ -o $@

patchtmpjump.o:		patchtmpjump.c
#
