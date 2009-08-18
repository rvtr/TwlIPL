#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - tools - mastering.TWL
#  File:     Makefile.TWL
#
#  Copyright 2007 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: #$
#  $Rev$
#  $Author$
#---------------------------------------------------------------------------
TARGET_PLATFORM		= TWL

include	$(TWLSDK_ROOT)/build/buildtools/commondefs



TARGETS			=	$(PREFIX)$(SUFFIX).exe

SOURCES_C		=	main.c keys.c

SOURCES			=	$(SOURCES_C)

OBJECTS			=	$(SOURCES:.c=.o) 

INCDIR			=	$(TWLSDK_ROOT)/build/tools/makerom.TWL \
					$(TWL_IPL_RED_ROOT)/include

REVISION_H		=	revision.h

#INSTALL_DIR		=	$(FIRM_INSTALL_TOOLSDIR)/bin
#INSTALL_TARGETS	=	$(TARGETS)

MACROS			+=	$(addprefix -I,$(INCDIR)) -DPLATFORM_ENDIAN_LITTLE

LIBACSIGN			=	$(ROOT)/build/tools/makerom.TWL/acsign/lib/X86/libacsign_x86.a
LIBAES				=	$(ROOT)/build/tools/makerom.TWL/aes/lib/X86/libaes_x86.a
LIBDIGEST			=	$(ROOT)/build/tools/makerom.TWL/digest/lib/X86/libdigest_x86.a
#LIBDGT				=	$(ROOT)/build/tools/libdgt.TWL/lib/libdgt.TWL.a

ifeq ($(KEY_LAUNCHER),TRUE)
MACROS			+= -DKEY_LAUNCHER
SUFFIX			=  .launcher
else
ifeq ($(KEY_SECURE),TRUE)
MACROS			+= -DKEY_SECURE
SUFFIX			=  .secure
else
ifeq ($(KEY_SYSTEM),TRUE)
MACROS			+= -DKEY_SYSTEM
SUFFIX			=  .sys
else
MACROS			+= -DKEY_USER
SUFFIX			=  .user
endif
endif
endif

ifeq ($(FOR_RED),TRUE)
MACROS			+= -DFOR_RED
PREFIX			= FakeRomHeader
else
PREFIX			= RomVersionChanger
endif

LDIRT_CLEAN		=	$(OBJECTS) $(TARGETS) *.BAK $(REVISION_H)

include	$(TWLSDK_ROOT)/build/buildtools/twl/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build: $(TARGETS)

$(TARGETS):		$(REVISION_H) $(OBJECTS) $(LIBACSIGN) $(LIBAES) $(LIBDIGEST)
			$(CC_X86) $+ -o $@ -lnetapi32 -lwsock32

main.o:	main.c

#main.h:	

#  avoid to warning message
#misc.o:WARNING	+=	-Wno-format-y2k

#

$(REVISION_H) ::
	@if test -e $(TWLSDK_ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(TWLSDK_ROOT)) | grep 'Revision' | sed "s|Revision: \(.*\)|#define  SDK_REVISION  \"\1\"|" > $(REVISION_H); \
	else \
		echo "#define  SDK_REVISION  \"-\"" > $(REVISION_H); \
	fi
	@if test -e $(TWL_IPL_RED_ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(TWL_IPL_RED_ROOT)) | grep 'Revision' | sed "s|Revision: \(.*\)|#define  IPL_REVISION  \"\1\"|" >> $(REVISION_H); \
	else \
		echo "#define  IPL_REVISION  \"-\"" >> $(REVISION_H); \
	fi

#test:			path.c misc.c
#			$(CC_X86) -DTEST $+ -o $@
