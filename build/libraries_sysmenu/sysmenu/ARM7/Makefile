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
# $Date::            $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS			=	

#----------------------------------------------------------------------------
TARGET_FIRM			=	SYSTEMMENU
TARGET_PLATFORM		=	TWL
TWL_ARCHGEN			=	LIMITED
TWL_PROC	   		=	ARM7
override TWL_CODEGEN 	= ALL

SRCS_COMMON			=	status.c \
						pxi.c \
						mountInfo.c \
						decryptAES.c

SRCS				=	setAESKey.c \
						$(addprefix ../common/src/, $(SRCS_COMMON))

TARGET_LIB			=	libsysmenu_sp$(TWL_LIBSUFFIX).a

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR			=	$(SYSMENU_INSTALL_LIBDIR)

LINCLUDES			=	$(SYSMENU_ROOT)/build/libraries_sysmenu/sysmenu/common/include \
						$(ROOT)/build/libraries/spi/ARM7/pm/include \

#----------------------------------------------------------------------------

do-build: $(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
