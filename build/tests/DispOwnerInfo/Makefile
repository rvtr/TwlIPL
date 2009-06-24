#! make -f
#----------------------------------------------------------------------------
# Project:  debugIPL2
# File:     Makefile
#
# Copyright 2003,2004 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: Makefile,v $
# Revision 1.10.2.1  2006/02/01 05:10:42  yosiokat
# ビルドソースにIPLType.cを追加。
#
# Revision 1.10  2005/04/14 05:53:29  yosiokat
# NITRO設定データ関係を強制的に中国版にするスイッチFORCE_CHINAを追加。
#
# Revision 1.9  2005/04/07 02:23:25  yosiokat
# リリースタイム更新用のルール追加。
#
# Revision 1.8  2005/04/06 02:19:03  yosiokat
# ソースのcheckFontData.cをcheckIPL2Font.cに変更。
#
# Revision 1.7  2005/04/05 08:05:14  yosiokat
# BUILD_MAINTEスイッチによって、共通ソースからDebugToolForIPL2とMaintenanceToolForIPL2
# をビルドできるよう変更。
#
# Revision 1.6  2005/03/31 05:45:12  yosiokat
# rev.1.5は間違いNitroConfigData.cに戻す。
#
# Revision 1.5  2005/03/31 05:44:17  yosiokat
# NitroConfigData.cをnitroConfigData.cに戻す。
#
# Revision 1.4  2005/03/28 09:34:12  yosiokat
# バナー追加。
#
# Revision 1.3  2005/03/09 04:44:11  yosiokat
# 機能追加。
#
# Revision 1.2  2004/09/23 11:27:45  yosiokat
# small fix.
#
# Revision 1.1.1.1  2004/08/31 06:20:24  Yosiokat
# no message
#
#
# $NoKeywords: $
#----------------------------------------------------------------------------

SUBDIRS				=	banner

#----------------------------------------------------------------------------

TARGET_NEF			=	DispOwnerInfo.nef
TARGET_BIN			=	DispOwnerInfo.srl

SRCS				=	main.c initFunc.c font.c data.c dispNCD.c myFunc.c myChar.c

FONT_DIR			=	font
FONTS				=	f08han.dat f08zen.dat f10han.dat f10zen.dat f12han.dat f12zen.dat f16han.dat f16zen.dat
FONT_DATAS			=	$(addprefix $(FONT_DIR)/, $(FONTS))
FONT_OBJS			=	$(FONT_DATAS:.dat=.o)

ROM_SPEC			=	main.rsf

#LCFILE_SPEC		=	# using default
#SRCDIR				=	# using default

# makerom settings
MAKEROM_ROMROOT		=	./data
MAKEROM_ROMFILES	=	*.bin

LINCLUDES			+=	include
LLIBRARY_DIRS   	+=	mlib/$(NITRO_LIBTYPE) $(FONT_DIR)
LLIBRARIES 			+=	libfnt.a $(FONTS:.dat=.o)

LDIRT_CLEAN			=	$(FONT_DIR)/*.o

include	$(NITROSDK_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

do-build:	$(FONT_OBJS)  $(TARGETS)

include	$(NITROSDK_ROOT)/build/buildtools/modulerules

$(TARGETS) : $(MAKEROM_ARM7)

$(FONT_DIR)/%.o : $(FONT_DIR)/%.dat
	$(CW_BINDIR)/BinToElf.exe $< -aligndata 4 -endian little -output $@


#===== End of Makefile =====
