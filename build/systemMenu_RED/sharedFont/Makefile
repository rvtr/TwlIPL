#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL
# File:     Makefile -
#
# Copyright 2007 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 
# $Rev: 
# $Author: 
#----------------------------------------------------------------------------

TARGET_FIRM			=	SYSTEMMENU

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

FONT_DIR		=	WW
FONTS			=	TWL_BF00_l.NFTR \
					TWL_BF00_m.NFTR \
					TWL_BF00_s.NFTR
FONT_RSC		=	$(addprefix $(FONT_DIR)/, $(FONTS))

FONT_TABLE		=	TWLFontTable.dat
GEN_FONT_TABLE	=	$(SYSMENU_TOOLSDIR)/bin/genFontTable.plx

#----------------------------------------------------------------------------
#INSTALL_TARGETS	=	$(FONT_TABLE)
#INSTALL_DIR		=	$(FONT_TABLE)/build/tools/TwlNMenu/data

LDIRT_CLEAN		=	$(FONT_TABLE)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build : $(FONT_TABLE)

$(FONT_TABLE): $(FONT_RSC)
	$(GEN_FONT_TABLE) $(FONT_RSC)
	cp $(FONT_RSC) ./

#===== End of Makefile =====
