#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - components - armadillo.TWL
# File:     Makefile
#
# Copyright 2008 Nintendo.  All rights reserved.
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

TARGET_FIRM					=	SYSTEMMENU
override TARGET_PLATFORM	=	TWL
override TWL_PROC			=	ARM7
override TWL_ARCHGEN		=	LIMITED
override TWL_PLATFORM		=	TS
TWL_NO_STD_PCHDR			=	True
TWL_CODEGEN					?=	ALL

#----------------------------------------------------------------------------

COMPONENT_DIR	=	$(ROOT)/build/components/armadillo.TWL
SRCDIR			=	$(COMPONENT_DIR)/src
SRCS			=	main.c overwriteRtc.c

TARGET_NAME		=	armadillo2

TARGET_NEF		=	$(TARGET_NAME).tef
LCFILE_SPEC		=	armadillo2.lsf
LCFILE_TEMPLATE	=	$(COMPONENT_DIR)/armadillo.lcf.template
LDRES_TEMPLATE	=	$(COMPONENT_DIR)/armadillo.response.template

CRT0_O			=	crt0.SCR.TWL.o

# スタック不足防止の為、インライン展開せずにコンパイルする
CCFLAGS_OPT		=	-O4 -inline off

#----------------------------------------------------------------------------
include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MACRO_FLAGS		+=	-DSDK_ARM7COMP_LTD -DSDK_SEA

MAKELCF_FLAGS	+=	-DISDBG_LIBS_TWL='$(if $(ISDBG_LIBS_TWL),$(ISDBG_LIBS_TWL),libstubsistd_sp$(TWL_LIBSUFFIX).a)'	\
					-DISDBG_LIBS_NITRO='libstubsisd_sp$(TWL_LIBSUFFIX).a'

#--------------------------------
#   install target
#--------------------------------
INSTALL_TARGETS	=	$(BINDIR)/$(TARGET_NAME).tef				\
					$(BINDIR)/$(TARGET_NAME).TWL.FLX.sbin		\
					$(BINDIR)/$(TARGET_NAME)_defs.TWL.FLX.sbin	\
					$(BINDIR)/$(TARGET_NAME).TWL.LTD.sbin		\
					$(BINDIR)/$(TARGET_NAME)_defs.TWL.LTD.sbin

INSTALL_DIR		=	$(TWL_INSTALL_COMPONENTSDIR)/$(TARGET_NAME)/$(TWL_BUILDTYPE)

LINCLUDES		=	$(ROOT)/build/libraries/spi/ARM7/include

LLIBRARIES		+=	libwl_sp$(TWL_LIBSUFFIX).a			\
					libsdio_sp$(TWL_LIBSUFFIX).a		\
					libathdrv_sp$(TWL_LIBSUFFIX).a		\
					libwpa_sp$(TWL_LIBSUFFIX).a			\
					libsea_sp$(TWL_LIBSUFFIX).a			\

LLIBRARIES		+=	libpm_mset_sp$(TWL_LIBSUFFIX).a
GLIBRARIES		:= $(filter-out libpm_sp$(TWL_LIBSUFFIX).a,$(GLIBRARIES))

LDEPENDS_NEF	 = $(TWL_LIBS) $(LLIBRARIES)

do-build:	$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====
