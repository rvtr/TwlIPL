#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - camera-test
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
# $Date:: 2007-12-18#$
# $Rev: 3150 $
# $Author: yutaka $
#----------------------------------------------------------------------------

SUBDIRS		=


#----------------------------------------------------------------------------
TARGET_PLATFORM		:= TWL
#TWL_CODEGEN   =       THUMB
TWL_PROC      =       ARM7
TWL_ARCHGEN    =	   LIMITED

TARGET_TEF = main.tef

SRCS		=	main.c kami_pxi.c formatter.c nvram_misc.c

LINCLUDES	=	../common/include $(TWLSDK_ROOT)/build/libraries/spi/ARM7/include


#SRCDIR		=	# using default
#LCFILE		=	# using default

LCFILE_SPEC     = 	main.lsf
LCFILE_TEMPLATE	 =	racoon.lcf.template
LDRES_TEMPLATE	 =	racoon.response.template

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

ifdef TWLSDK_NOCRYPTO
CCFLAGS			+=	-DSDK_NOCRYPTO
endif

ifndef TWLSDK_NOCRYPTO
MAKELCF_FLAGS	+=	-DUSE_CRYPTO_LIBS='libaes_sp$(TWL_LIBSUFFIX).a'
endif


TWL_LIBS_EX		= 	libcamera_sp$(TWL_LIBSUFFIX).a	\
					libfatfs_sp$(TWL_LIBSUFFIX).a   \
					libi2c_sp$(TWL_LIBSUFFIX).a	\
        			libcamera_sp$(TWL_LIBSUFFIX).a	\
					libcdc_sp$(TWL_LIBSUFFIX).a	\
					libtpex_sp$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWLSDK_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
