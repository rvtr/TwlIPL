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
# $Date:: 2007-10-09#$
# $Rev: 1486 $
# $Author: terui $
#----------------------------------------------------------------------------

TARGET_PLATFORM		=	TWL
TWL_ARCHGEN        	=	LIMITED
TWL_PROC			=	ARM7
TWL_NO_STD_PCHDR	=	TRUE

#----------------------------------------------------------------------------

SRCS			=	main.c		\
					dummy.FLXMAIN.c	\
					dummy.FLXWRAM.c	\
					dummy.LTDMAIN.c	\
					dummy.LTDWRAM.c

TARGET_NAME		=	formatter_sub

TARGET_NEF		=	$(TARGET_NAME).tef
LCFILE_SPEC		=	$(TARGET_NAME).lsf
LCFILE_TEMPLATE	=	$(TARGET_NAME).lcf.template
LDRES_TEMPLATE	=	$(TARGET_NAME).response.template

CRT0_O			=	crt0.LTD.TWL.o

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

MACRO_FLAGS		+=	-DSDK_ARM7COMP_LTD

MAKELCF_FLAGS	+=	-DISTD_LIBS='$(ISTD_LIBS)'

do-build:	$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
