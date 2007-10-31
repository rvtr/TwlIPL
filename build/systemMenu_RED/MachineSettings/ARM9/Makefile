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

TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TARGET_BIN				=	main.srl

#LCFILE_SPEC			=	
ROM_SPEC				=	main.rsf

MISC_DIR				=	../../Launcher/ARM9/src

SRCS					=	main.c $(MISC_DIR)/misc.c \
							MachineSetting.c setRTC.c selectLanguage.c calibrationTP.c
							#setOwnerInfo.c

#SRCS_OVERLAY			=	

LINCLUDES				=	$(MISC_DIR)
#LLIBRARY_DIRS			=	
#LLIBRARIES				=	
#LDEPENDS_NEF			=	

include $(TWLIPL_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build : $(FONT_OBJS) $(TARGETS)


include $(TWLIPL_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
