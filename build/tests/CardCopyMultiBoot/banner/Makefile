#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - nandApp - demos - card_backup
# File:     Makefile
#
# Copyright 2008 Nintendo.  All rights reserved.
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
TARGET_PLATFORM		= TWL

include $(TWLSDK_ROOT)/build/buildtools/commondefs
MAKEBANNER			= $(TWL_TOOLSDIR)/bin/makebanner.TWL.exe

ICON_DIR			= ../data

BANNER_ICON			= $(ICON_DIR)/myGameIcon.bmp
BANNER_SPEC			= banner_v3.bsf

TARGETS				= banner.bnr
INSTALL_DIR			= .
INSTALL_TARGETS		= $(TARGETS)

BANNER_ICON_NAME	= $(basename $(BANNER_ICON))
BANNER_ICON_MIDDLE	= $(addprefix $(BANNER_ICON_NAME), .nbfs .nbfc .nbfp)

LDIRT_CLEAN			= $(TARGETS) \
					  $(BANNER_ICON_MIDDLE) \
					  $(TARGETS:.bnr=.srl)

include $(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(BANNER_SPEC) $(BANNER_ICON) $(BANNER_ICON_MIDDLE)
				$(MAKEBANNER) -N $(BANNER_ICON_NAME) $(BANNER_SPEC) $(TARGETS)

#
