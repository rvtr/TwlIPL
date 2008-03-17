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
# $Date::            $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS		=


#----------------------------------------------------------------------------

#============================================================================
# NandInitializerから一部の機能を削除するために定義を追加します。
MACRO_FLAGS				+=	-DNAND_FORMATTER_MODE
#============================================================================

override TARGET_PLATFORM	= TWL
override TWL_PROC           = ARM7
TWL_ARCHGEN                 = LIMITED
TWL_NO_STD_PCHDR			= True
#TWL_CODEGEN   =       THUMB

TARGET_TEF      =   main.tef
LCFILE_SPEC     = 	../../NandInitializer/ARM7.TWL/main.lsf
LCFILE_TEMPLATE	=	../../NandInitializer/ARM7.TWL/racoon.lcf.template
LDRES_TEMPLATE	=	../../NandInitializer/ARM7.TWL/racoon.response.template

SRCS		=	main.c \
				kami_pxi.c \
				formatter.c \
				nvram_misc.c \
				initScfg.c \

LINCLUDES	=	../../NandInitializer/common/include \
				../../NandInitializer/ARM7.TWL/include \
                ../common/include $(ROOT)/build/libraries/spi/ARM7/include


SRCDIR		=	../../NandInitializer/ARM7.TWL/src
#LCFILE		=	# using default


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

ifdef TWLSDK_NOCRYPTO
CCFLAGS			+=	-DSDK_NOCRYPTO
endif

ifndef TWLSDK_NOCRYPTO
MAKELCF_FLAGS	+=	-DUSE_CRYPTO_LIBS='libsea_sp$(TWL_LIBSUFFIX).a libcrypto_sp$(TWL_LIBSUFFIX).a'
MACRO_FLAGS		+=	-DSDK_SEA
endif

ifeq ($(TWL_PLATFORM),BB)
MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037e0000'
else
MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037c0000'
endif

MAKELCF_FLAGS	+=	-DISDBG_LIBS_TWL='$(if $(ISDBG_LIBS_TWL),$(ISDBG_LIBS_TWL),libstubsistd_sp$(TWL_LIBSUFFIX).a)'	\
					-DISDBG_LIBS_NITRO='libstubsisd_sp$(TWL_LIBSUFFIX).a'

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
