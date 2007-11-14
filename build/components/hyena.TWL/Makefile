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
# $Rev: 1499 $
# $Author: hatamoto_minoru $
#----------------------------------------------------------------------------

TARGET_FIRM					=	SYSTEMMENU
override TARGET_PLATFORM	=	TWL
override TWL_PROC			=	ARM7
override TWL_ARCHGEN		=	LIMITED
TWL_NO_STD_PCHDR			=	True
TWL_CODEGEN					?=	ARM

#----------------------------------------------------------------------------

SRCS			=	main.c

TARGET_NAME		=	hyena

TARGET_NEF		=	$(TARGET_NAME).tef
LCFILE_SPEC		=	$(TARGET_NAME).lsf
LCFILE_TEMPLATE	=	$(TARGET_NAME).lcf.template
LDRES_TEMPLATE	=	$(TWLSDK_ROOT)/build/components/racoon.TWL/racoon.response.template

CRT0_O			=	crt0.LTD.TWL.o

#----------------------------------------------------------------------------
include	$(TWLIPL_ROOT)/build/buildtools/commondefs

#MACRO_FLAGS		+=	-DSDK_ARM7COMP_LTD

MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037c0000' \
					-DADDRESS_FLXMAIN='0x02280000' \
					-DADDRESS_BOOTCORE='0x0380f000' \
					-DCRT0_O='$(CRT0_O)'
MAKELCF_FLAGS	+=	-DISDBG_LIBS='$(ISDBG_LIBS)'

#--------------------------------
#   install target
#--------------------------------
INSTALL_TARGETS	=	$(BINDIR)/$(TARGET_NAME).tef				\
					$(BINDIR)/$(TARGET_NAME).FLX.TWL.sbin		\
					$(BINDIR)/$(TARGET_NAME)_defs.FLX.TWL.sbin	\
					$(BINDIR)/$(TARGET_NAME).LTD.TWL.sbin		\
					$(BINDIR)/$(TARGET_NAME)_defs.LTD.TWL.sbin

INSTALL_DIR		=	$(TWL_INSTALL_COMPONENTSDIR)/$(TARGET_NAME)/$(TWL_BUILDTYPE)

LINCLUDES		= $(ROOT)/build/libraries/spi/ARM7/include
LDEPENDS_NEF	= $(TWL_LIBS) $(LLIBRARIES)

do-build:	$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWLIPL_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
