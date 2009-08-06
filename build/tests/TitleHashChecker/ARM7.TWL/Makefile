#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - components - racoon.TWL
# File:     Makefile
#
# Copyright 2007-2008 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 2008-09-18#$
# $Rev: 8573 $
# $Author: okubata_ryoma $
#----------------------------------------------------------------------------

override TARGET_PLATFORM	 =	TWL
override TWL_PROC			 =	ARM7
override TWL_ARCHGEN		 =	LIMITED
TWL_NO_STD_PCHDR			 =	True
TWL_CODEGEN					?=	ALL

#----------------------------------------------------------------------------

SRCDIR			+=	./src

SRCS			 =	main.c initScfg.c kami_pxi.c

TARGET_NAME		 =	racoon

TARGET_NEF		 =	$(TARGET_NAME).tef
LCFILE_SPEC		 =	$(TARGET_NAME).lsf
LCFILE_TEMPLATE	 =	$(ROOT)/build/components/$(TARGET_NAME).TWL/$(TARGET_NAME).lcf.template
LDRES_TEMPLATE	 =	$(ROOT)/build/components/$(TARGET_NAME).TWL/$(TARGET_NAME).response.template

CRT0_O			 =	crt0.LTD.TWL.o

# スタック不足防止の為、インライン展開せずにコンパイルする
CCFLAGS_OPT		=	-O4 -inline off

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#MACRO_FLAGS	+=	-DSDK_ARM7COMP_LTD

ifeq ($(TWL_PLATFORM),BB)
MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037e0000'
else
MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037c0000'
endif

MAKELCF_FLAGS	+=	-DISDBG_LIBS_TWL='$(if $(ISDBG_LIBS_TWL),$(ISDBG_LIBS_TWL),libstubsistd_sp$(TWL_LIBSUFFIX).a)'	\
					-DISDBG_LIBS_NITRO='libstubsisd_sp$(TWL_LIBSUFFIX).a'

LLIBRARY_DIRS	+=	$(TWL_IPL_RED_ROOT)/lib/ARM7-TS/$(TWL_BUILD_DIR) \
					./obj/ARM7-TS.LTD/$(TWL_BUILD_DIR)

LINCLUDES		+=	$(ROOT)/build/libraries/spi/ARM7/include \
					$(ROOT)/build/libraries/os/common/include \
					$(TWL_IPL_RED_ROOT)/include \
					$(ROOT)/build/libraries/init/common/include \
					$(ROOT)/build/libraries/fatfs/ARM7.TWL/include \
					$(ROOT)/build/libraries/fatfs/ARM7.TWL/include/fatfs \
					$(ROOT)/build/libraries/fatfs/ARM7.TWL/include/twl/fatfs/ARM7 \
					../common/include

LLIBRARIES		+= libwl_sp.TWL.LTD.a

LDEPENDS_NEF	 = $(TWL_LIBS) $(LLIBRARIES)

do-build:	$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
