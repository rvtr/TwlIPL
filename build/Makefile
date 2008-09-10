#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - build
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

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

ifneq	($(TWL_KEYSDIR),)
SUBDIRS =	tools
endif

SUBDIRS +=	libraries_sysmenu \
			components \
			systemMenu_RED


ifneq	($(TWL_KEYSDIR),)
SUBDIRS +=	libraries \
			nandfirm \
			gcdfirm \
			systemMenu_tools
endif

ifdef TWL_IPL_WITH_TESTS
SUBDIRS +=	tests
endif

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
