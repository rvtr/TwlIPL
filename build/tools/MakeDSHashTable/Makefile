#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlIPL - tools - MakeDSHashTalbe
#  File:     Makefile
#
#  Copyright 2008 Nintendo.  All rights reserved.
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

TARGET_PLATFORM	=	TWL
TARGET_FIRM	=	SYSTEMMENU

SUBDIRS		=

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

TARGETS		= 	MakeDSHashTable.exe

SOURCES		=	MakeDSHashTable.c
OBJECTS		=	$(SOURCES:.c=.o)

LIBDIR		=	$(ROOT)/build/tools/makerom.TWL

LIBACSIGN	=	$(LIBDIR)/acsign/lib/X86/libacsign_x86.a
LIBDIGEST	=	$(LIBDIR)/digest/lib/X86/libdigest_x86.a

MACROS		=	-I$(LIBDIR)			\
			-I$(ROOT)/include		\
			-I$(SYSMENU_ROOT)/include

INSTALL_DIR	=	$(SYSMENU_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS	=	$(TARGETS)

LDIRT_CLEAN	=	depend $(OBJECTS)

include	$(TWLSDK_ROOT)/build/buildtools/twl/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJECTS) $(LIBACSIGN) $(LIBDIGEST)
			$(CC_X86) $+ -o $@

depend: $(SOURCES)
	-@ $(RM) depend
	-@ for i in $(SOURCES); do\
		$(CC_X86) $(MACROS) -MM -w -DSDK_WIN32 $(WARNING) -I. -c -I$(TWL_INCDIR) $$i >> depend;\
	done

-include depend
