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

TARGET_FIRM		=	SYSTEMMENU

SUBDIRS			=	

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

# WW,CN,KR����I��
FONT_REGION		=	WW
FONT_DIR		=	$(FONT_REGION)

ifeq ($(FONT_REGION),WW)
FONT_TIMESTAMP	=	08073100
FONT_REGION_CODE =  0
FONTS			=	$(FONT_DIR)/TBF1_l.NFTR \
					$(FONT_DIR)/TBF1_m.NFTR \
					$(FONT_DIR)/TBF1_s.NFTR
endif # REGION=WW

ifeq ($(FONT_REGION),CN)
FONT_TIMESTAMP	=	09052700
FONT_REGION_CODE =  4
FONTS			=	NULL \
					NULL \
					NULL \
					$(FONT_DIR)/TBF1-cn_l.NFTR \
					$(FONT_DIR)/TBF1-cn_m.NFTR \
					$(FONT_DIR)/TBF1-cn_s.NFTR \
					NULL \
					NULL \
					NULL
endif # REGION=CN

ifeq ($(FONT_REGION),KR)
FONT_TIMESTAMP	=	09052700
FONT_REGION_CODE =  5
FONTS			=	NULL \
					NULL \
					NULL \
					NULL \
					NULL \
					NULL \
					$(FONT_DIR)/TBF1-kr_l.NFTR \
					$(FONT_DIR)/TBF1-kr_m.NFTR \
					$(FONT_DIR)/TBF1-kr_s.NFTR
endif # REGION=KR

FONT_RSC		=	$(FONTS)

FONT_ORG_DIR	=	$(TWL_IPL_RED_ROOT)/build/systemMenu_RED/sharedFont/WW
FONT_ORG_RSC	=	$(addprefix $(FONT_ORG_DIR)/, $(FONTS))

ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
FONT_TABLE		=	TWLFontTable.dat
FONT_TABLE_WITH_TIMESTAMP = TWLFontTable_$(FONT_REGION)_$(FONT_TIMESTAMP).dat
endif

GEN_FONT_TABLE	=	$(SYSMENU_TOOLSDIR)/bin/genFontTable.plx

#----------------------------------------------------------------------------

LDIRT_CLEAN		=	$(FONT_TABLE_WITH_TIMESTAMP)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build : $(FONT_TABLE) $(FONT_TABLE_WITH_TIMESTAMP)

# compress sharedFonts and them in local directory
$(FONT_TABLE): $(FONT_RSC) $(GEN_FONT_TABLE) ./Makefile
	$(GEN_FONT_TABLE) $(FONT_TIMESTAMP) $(FONT_REGION_CODE) $(FONT_RSC)

$(FONT_TABLE_WITH_TIMESTAMP): $(FONT_TABLE)
	cp $< $@

NULL:

#===== End of Makefile =====
