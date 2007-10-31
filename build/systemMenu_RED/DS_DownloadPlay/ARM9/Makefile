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

TARGET_BIN				=	dlplay.srl

LCFILE_SPEC				=	ARM9-TS.lsf
ROM_SPEC				=	main.rsf

SRCS					=	main.c mainFunc.c DS_DownloadPlay.c

LINCLUDES				=	$(TWLSDK_ROOT)/build/libraries/spi/arm9/include		\
							$(TWLSDK_ROOT)/build/libraries/mb/common/include	\

LLIBRARY_DIRS			=
LLIBRARIES				=
LDEPENDS_NEF			=

include $(TWLIPL_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWLIPL_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
