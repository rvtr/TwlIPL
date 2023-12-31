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

override TARGET_PLATFORM	=	TWL
override TWL_PROC			=	ARM7
override TWL_ARCHGEN		=	LIMITED
##override TWL_PLATFORM		=	TS
TWL_NO_STD_PCHDR			=	True
override TARGET_CODEGEN		=	ARM

ifndef	TWLSDK_NOCRYPTO
#----------------------------------------------------------------------------

SRCS		=	crt0.SCFG_NOLOCK.c \
				main.c \
				kami_pxi.c \
				formatter.c \
                mcu_firm.c

TARGET_NAME		=	armadillo

TARGET_NEF		=	$(TARGET_NAME).tef
LCFILE_SPEC		=	$(TARGET_NAME).lsf
LCFILE_TEMPLATE	=	$(ROOT)/build/components/$(TARGET_NAME).TWL/$(TARGET_NAME).lcf.template
LDRES_TEMPLATE	=	$(ROOT)/build/components/$(TARGET_NAME).TWL/$(TARGET_NAME).response.template

CRT0_O			=	crt0.SCFG_NOLOCK.o

# スタック不足防止の為、インライン展開せずにコンパイルする
CCFLAGS_OPT		=	-O4 -inline off

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

MACRO_FLAGS		+=	-DSDK_ARM7COMP_LTD -DSDK_SEA -DSDK_NOINIT

MAKELCF_FLAGS	+=	-DISDBG_LIBS_TWL='$(if $(ISDBG_LIBS_TWL),$(ISDBG_LIBS_TWL),libstubsistd_sp$(TWL_LIBSUFFIX).a)'	\
					-DISDBG_LIBS_NITRO='libstubsisd_sp$(TWL_LIBSUFFIX).a'

LINCLUDES		+=	$(ROOT)/build/libraries/spi/ARM7/include \
					$(ROOT)/build/libraries/os/common/include \
					$(TWL_IPL_RED_ROOT)/include \
					../common/include \
					$(ROOT)/build/libraries/init/common/include \
					../../common/ARM7/include \
					$(ROOT)/build/libraries/fatfs/ARM7.TWL/include \
					$(ROOT)/build/libraries/fatfs/ARM7.TWL/include/fatfs \
					$(ROOT)/build/libraries/fatfs/ARM7.TWL/include/twl/fatfs/ARM7

SRCDIR		    +=  ../../common/ARM7/src

LLIBRARY_DIRS	+=	$(TWL_IPL_RED_ROOT)/lib/ARM7-TS/$(TWL_BUILD_DIR) \
					./obj/ARM7-TS.LTD/$(TWL_BUILD_DIR)

LLIBRARIES		+=	libwl_sp$(TWL_LIBSUFFIX).a			\
					libsdio_sp$(TWL_LIBSUFFIX).a		\
					libathdrv_sp$(TWL_LIBSUFFIX).a		\
					libwpa_sp$(TWL_LIBSUFFIX).a			\
					libhotsw_sp$(TWL_LIBSUFFIX).a		\
					libreloc_info_sp$(TWL_LIBSUFFIX).a

LLIBRARIES		+=	libpm_sp.SYSMENU$(TWL_LIBSUFFIX).a
GLIBRARIES		:= $(filter-out libpm_sp$(TWL_LIBSUFFIX).a,$(GLIBRARIES))

LDEPENDS_NEF	 = $(TWL_LIBS) $(LLIBRARIES)

do-build:	$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------
endif	#ifndef	TWLSDK_NOCRYPTO

#===== End of Makefile =====
