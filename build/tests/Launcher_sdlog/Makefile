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

export SDLOG ?= ARM9

TARGET_FIRM				=	SYSTEMMENU

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

LAUNCHER_DIR				=	$(SYSMENU_ROOT)/build/systemMenu_RED/Launcher

SUBMAKES		=	sdlog/Makefile scripts/Makefile	\
					$(LAUNCHER_DIR)/banner/Makefile $(LAUNCHER_DIR)/ARM9/Makefile_sdlog

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
