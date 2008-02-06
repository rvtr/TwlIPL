#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - components - racoon.TWL
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
# $Date:: 2007-11-30#$
# $Rev: 2778 $
# $Author: terui $
#----------------------------------------------------------------------------

override TARGET_PLATFORM	 =	TWL
override TWL_PROC			 =	ARM7
override TWL_ARCHGEN		 =	LIMITED
TWL_NO_STD_PCHDR			 =	True
TWL_CODEGEN					?=	ALL

#----------------------------------------------------------------------------

SRCDIR			 =	$(ROOT)/build/components/mongoose.TWL/src

SRCS			 =	main.c

TARGET_NAME		 =	my_racoon

TARGET_NEF		 =	$(TARGET_NAME).tef
LCFILE_SPEC		 =	$(TARGET_NAME).lsf
LCFILE_TEMPLATE	 =	$(TARGET_NAME).lcf.template
LDRES_TEMPLATE	 =	$(TARGET_NAME).response.template

CRT0_O			 =	crt0.LTD.TWL.o

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

ifdef TWLSDK_NOCRYPTO
CCFLAGS			+=	-DSDK_NOCRYPTO
endif

ifndef TWLSDK_NOCRYPTO
MAKELCF_FLAGS	+=	-DUSE_CRYPTO_LIBS='libaes_sp$(TWL_LIBSUFFIX).a'
endif

#MACRO_FLAGS	+=	-DSDK_ARM7COMP_LTD

ifeq ($(TWL_PLATFORM),BB)
MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037e0000'
else
MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037c0000'
endif

MAKELCF_FLAGS	+=	-DISDBG_LIBS_TWL='$(if $(ISDBG_LIBS_TWL),$(ISDBG_LIBS_TWL),libstubsistd_sp$(TWL_LIBSUFFIX).a)'	\
					-DISDBG_LIBS_NITRO='libstubsisd_sp$(TWL_LIBSUFFIX).a'


#--------------------------------
#   install target
#--------------------------------
INSTALL_TARGETS	 =	$(BINDIR)/$(TARGET_NAME).tef				\
					$(BINDIR)/$(TARGET_NAME).TWL.FLX.sbin		\
					$(BINDIR)/$(TARGET_NAME)_defs.TWL.FLX.sbin	\
					$(BINDIR)/$(TARGET_NAME).TWL.LTD.sbin		\
					$(BINDIR)/$(TARGET_NAME)_defs.TWL.LTD.sbin

INSTALL_DIR		 =	$(TWL_INSTALL_COMPONENTSDIR)/$(TARGET_NAME)/$(TWL_BUILDTYPE)

LINCLUDES		 = $(ROOT)/build/libraries/spi/ARM7/include
#LLIBRARIES		+= libwl_sp.TWL.LTD.a libsdio_hcd_twl$(TWL_LIBSUFFIX).a libsdio_busdriver$(TWL_LIBSUFFIX).a libsdio_lib$(TWL_LIBSUFFIX).a libARM7athdrv$(TWL_LIBSUFFIX).a
LLIBRARIES		+= libwl_sp.TWL.LTD.a
#LLIBRARIES		+=	libwl_sp$(TWL_LIBSUFFIX).a			\
#					libcdc_sp$(TWL_LIBSUFFIX).a			\
#					libtpex_sp$(TWL_LIBSUFFIX).a		\
#					libmicex_sp$(TWL_LIBSUFFIX).a		\
#					libsdio_hcd_twl$(TWL_LIBSUFFIX).a	\
#					libsdio_busdriver$(TWL_LIBSUFFIX).a	\
#					libsdio_lib$(TWL_LIBSUFFIX).a		\
#					librompatch_sp$(TWL_LIBSUFFIX).a	\
#					libARM7athdrv$(TWL_LIBSUFFIX).a

LDEPENDS_NEF	 = $(TWL_LIBS) $(LLIBRARIES)

do-build:	$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
