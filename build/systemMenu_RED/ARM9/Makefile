#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - demos - simpleShoot-1
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
# $Date:: $
# $Rev:  $
# $Author:  $
#----------------------------------------------------------------------------

SUBDIRS					=	

#----------------------------------------------------------------------------

TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TARGET_BIN				=	main.srl

LCFILE_SPEC				=	ARM9-TS.lsf
ROM_SPEC				=	main.rsf

LOGO_DIR				=	Logo
SETTING_DIR				=	DS_Setting
MBOOT_DIR				=	DS_DownloadPlay
CHAT_DIR				=	DS_Chat

FONT_DIR				=	font
FONTS					=	f12han.dat
FONT_DATAS				=	$(addprefix $(FONT_DIR)/, $(FONTS))
FONT_OBJS				=	$(FONT_DATAS:.dat=.o)

SRCS_IPL2				=	main.c mainFunc.c launcher.c $(SETTING_DIR)/font.c $(SETTING_DIR)/myChar.c
SRCS_LOGO				=	logoDemo.c logoData.c
SRCS_SETTING			=	DS_Setting.c settingMenu.c unicode.c misc.c \
							rtcSet.c langSelect.c tpCalib.c ownerInfo.c AgbLcdSel.c autoBoot.c
SRCS_MBOOT				=	DS_DownloadPlay.c
SRCS_DSCHAT				=	DS_Chat.c
SRCS					=	$(SRCS_IPL2) $(addprefix $(LOGO_DIR)/, $(SRCS_LOGO))
SRCS_OVERLAY			=	$(addprefix $(SETTING_DIR)/,$(SRCS_SETTING))	\
							$(addprefix $(MBOOT_DIR)/,$(SRCS_MBOOT))	\
							$(addprefix $(CHAT_DIR)/,$(SRCS_DSCHAT))	\

OBJS_OVERLAY			+=	$(FONT_OBJS)

LINCLUDES				=	$(TWLSDK_ROOT)/build/libraries/spi/arm9/include		\
							$(TWLSDK_ROOT)/build/libraries/mb/common/include	\
							$(SRCDIR)/$(LOGO_DIR)								\
							$(SRCDIR)/$(SETTING_DIR)							\
							$(SRCDIR)/$(CHAT_DIR)								\
							$(SRCDIR)/$(MBOOT_DIR)								\

LLIBRARY_DIRS			=
LLIBRARIES				=
LDEPENDS_NEF			=

include $(TWLIPL_ROOT)/build/buildtools/commondefs.sysmenu

#----------------------------------------------------------------------------

do-build : $(FONT_OBJS) $(TARGETS)


include $(TWLIPL_ROOT)/build/buildtools/modulerules.sysmenu

#----------------------------------------------------------------------------
CW_ROOT		:= $(subst $(SPACE),\ ,$(subst \,/,$(CWFOLDER_NITRO)))

$(FONT_DIR)/%.o : $(FONT_DIR)/%.dat
	$(CW_ROOT)/ARM_Tools/Command_Line_Tools/BinToElf.exe $< -aligndata 4 -endian little -output $@


#===== End of Makefile =====
