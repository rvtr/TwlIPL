#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - dummyromheader - 
#  File:     Makefile
#
#  Copyright 2007 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date:: 
#  $Rev: 
#  $Author: 
#---------------------------------------------------------------------------


#----------------------------------------------------------------------------

#----------------------------------------------------------------------------

TARGET_FIRM			=	SYSTEMMENU

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

TARGETS			= attach_dummyromheader.exe


SOURCES			= attach_dummyromheader.c

OBJECTS			= $(SOURCES:.c=.o)


LDIRT_CLEAN		= $(OBJECTS) $(TARGETS)
INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(SYSMENU_TOOLSDIR)/bin


include $(TWLSDK_ROOT)/build/buildtools/twl/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build: $(TARGETS)

$(TARGETS): $(OBJECTS)
	 $(CC_X86) $+ -o $@



#===== End of Makefile =====
