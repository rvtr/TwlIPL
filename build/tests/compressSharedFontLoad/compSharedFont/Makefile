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

SUBDIRS			=	compBLZ_modified \
					ntrcomp

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

FONT_TIMESTAMP	=	08061300
FONT_DIR		=	WW
FONTS			=	TBF1_l.NFTR \
					TBF1_m.NFTR \
					TBF1_s.NFTR
FONT_RSC		=	$(addprefix $(FONT_DIR)/, $(FONTS))

FONT_ORG_DIR	=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/sharedFont/WW
FONT_ORG_RSC	=	$(addprefix $(FONT_ORG_DIR)/, $(FONTS))

ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
FONT_TABLE		=	TWLFontTable.dat
endif

GEN_FONT_TABLE	=	./genFontTable.plx

#----------------------------------------------------------------------------
INSTALL_TARGETS	=	$(FONT_TABLE)
INSTALL_DIR		=	$(TWL_IPL_RED_ROOT)/build/systemMenu_tools/NandInitializerRed/data

LDIRT_CLEAN		=	$(FONT_TABLE) $(FONT_RSC)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build : $(FONT_TABLE)

# step1 : copy sharedFonts into local directory for work
$(FONT_RSC):
	cp $(FONT_ORG_RSC) $(FONT_DIR)

# step2 : compress sharedFonts and them in local directory
$(FONT_TABLE): $(FONT_RSC)
	$(GEN_FONT_TABLE) $(FONT_TIMESTAMP) $(FONT_RSC)

#===== End of Makefile =====
