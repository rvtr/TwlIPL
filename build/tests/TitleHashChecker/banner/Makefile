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
# $Date:: 2008-07-01#$
# $Rev: 1750 $
# $Author: nakasima $
#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

ICON_DIR			= ./icon

BANNER_ICON			= $(ICON_DIR)/gameIcon.bmp
BANNER_SPEC			= banner_v3.bsf

TARGETS				= banner.bnr
INSTALL_DIR			= ./
INSTALL_TARGETS		= $(TARGETS)

BANNER_ICON_NAME	= $(basename $(BANNER_ICON))
BANNER_ICON_MIDDLE	= $(addprefix $(BANNER_ICON_NAME), .nbfs .nbfc .nbfp)

LDIRT_CLEAN			= $(TARGETS) \
					  $(BANNER_ICON_MIDDLE) \
					  $(TARGETS:.bnr=.srl)

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(BANNER_SPEC) $(BANNER_ICON) $(BANNER_ICON_MIDDLE)
				$(MAKEBANNER) -N $(BANNER_ICON_NAME) $(BANNER_SPEC) $(TARGETS)

#
