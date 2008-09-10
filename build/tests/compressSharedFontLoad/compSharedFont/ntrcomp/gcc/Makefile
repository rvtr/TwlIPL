#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - build - tools - ntrcomp - gcc
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
TARGET_PLATFORM	 =	NITRO

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

OBJECTS			 = ntcompress_main.o ntcompress_test.o nitroCompLib.o multipleCompLib.o rangeCoder.o

WIN32 ?= 1

ifneq ($(WIN32),0)
TARGETS			 = ntrcomp.exe
CC_X86			:= gcc -mno-cygwin
else
TARGETS			 = ntrcomp
CC_X86			:= gcc
endif

LDIRT_CLEAN		 = $(OBJECTS) $(TARGETS)

#INSTALL_DIR      = $(NITRO_INSTALL_TOOLSDIR)/bin
#INSTALL_TARGETS	 = $(TARGETS)

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJECTS)
			$(CC_X86) $+ -o $@

ntcompress_main.o:	../src/ntcompress_main.c ../src/nitroCompLib.h
	$(CC_X86) -c $< -o $@
ntcompress_test.o:	../src/ntcompress_test.c ../src/nitroCompLib.h
	$(CC_X86) -c $< -o $@
nitroCompLib.o:		../src/nitroCompLib.c ../src/nitroCompLib.h
	$(CC_X86) -c $< -o $@
multipleCompLib.o:	../src/multipleCompLib.c ../src/multipleCompLib.h
	$(CC_X86) -c $< -o $@
rangeCoder.o:	../src/rangeCoder.c ../src/rangeCoder.h
	$(CC_X86) -c $< -o $@

include	$(TWLSDK_ROOT)/build/buildtools/modulerules.x86

