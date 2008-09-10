#! make -f
#---------------------------------------------------------------------------
#  Project:  NitroSDK - tools
#  File:     Makefile
#
#  Copyright 2003,2004 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Log: Makefile,v $
#  Revision 1.1  2004/11/01 08:25:59  ooe
#  (none)
#
#  $NoKeywords: $
#---------------------------------------------------------------------------

include $(TWLSDK_ROOT)/build/buildtools/commondefs

BANNER_ICON		= 000A_WLCK.bmp
BANNER_SPEC		= 000A_WLCK.bsf

TARGETS			= $(BANNER_SPEC:.bsf=.bnr)

BANNER_ICON_NAME	= $(basename $(BANNER_ICON))

LDIRT_CLEAN		= $(TARGETS) \
			  $(BANNER_ICON_NAME).nbfs \
			  $(BANNER_ICON_NAME).nbfc \
			  $(BANNER_ICON_NAME).nbfp \
			  $(TARGETS:.bnr=.srl)

include $(TWLSDK_ROOT)/build/buildtools/modulerules


#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(BANNER_SPEC) $(BANNER_ICON_NAME).nbfc $(BANNER_ICON_NAME).nbfp
			$(MAKEBANNER) -N $(BANNER_ICON_NAME) $(BANNER_SPEC) $(TARGETS)

#
