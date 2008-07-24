#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - init - ARM9.TWL
# File:     Makefile.FLX
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

override TARGET_PLATFORM    =   TWL

# Codegen for main processer
override TWL_PROC           =   ARM9

# Avoid to build THUMB version object
override TWL_CODEGEN    =   ARM

# Avoid to build LIMITED version object
override TWL_ARCHGEN    =   HYBRID

# Avoid to use pre-compile-header of "nitro.h"
TWL_NO_STD_PCHDR            =   True

# build ARM & THUMB libraries
TWL_CODEGEN                 ?=  ALL

SRCDIR          =   src
INCDIR          =   ../common/include

SRCS            =   crt0_hct.FLX.c
TARGET_OBJ      =   crt0_hct.FLX.TWL.o

include $(TWLSDK_ROOT)/build/buildtools/commondefs
#----------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/modulerules
#----------------------------------------------------------------------------

do-build:	$(TARGETS)

$(OBJDIR)/$(TARGET_OBJ):	$(OBJDIR)/$(SRCS:.c=.o)
	$(CP) $< $@


#===== End of Makefile =====
