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
# $Date::            $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

BNRNUM				= k

ICON_DIR			= ./icon

BANNER_SPEC			= test-utf16_$(BNRNUM)_v3.TWL.bsf

MAKEBANNER	=	$(TWL_TOOLSDIR)/bin/makebanner.TWL.exe

TARGETS				= banner_$(BNRNUM).bnr
INSTALL_DIR			= ./
INSTALL_TARGETS		= $(TARGETS)

BANNER_ICON_NAME	= $(basename $(BANNER_ICON))

LDIRT_CLEAN			= $(TARGETS) \
					  $(TARGETS:.bnr=.srl)

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(BANNER_SPEC)
				$(MAKEBANNER) -p TWL $(BANNER_SPEC) $(TARGETS)

#
