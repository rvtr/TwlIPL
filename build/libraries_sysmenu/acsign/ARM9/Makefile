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

#----------------------------------------------------------------------------

SUBDIRS				=
SUBMAKES			=	Makefile.encrypt


#----------------------------------------------------------------------------
TARGET_FIRM			=	SYSTEMMENU
TARGET_PLATFORM		=	TWL
TWL_ARCHGEN			=	LIMITED
TWL_PROC	    	=	ARM9

INCDIR				= 	include \
						$(ROOT)/build/libraries/mb/common/include

SRCS				=	acsign.c acmemory.c acsign_util.c \

TARGET_LIB			=	libacsign$(TWL_LIBSUFFIX).a


include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR			=	$(SYSMENU_INSTALL_LIBDIR)

CCFLAGS				+=	-DSMALL_CODE_SIZE	\
						-DSTANDALONE		\
						-DOPT_32_BIT		\
						-DNO_SPLIT			\
						-DNO_FP_API			\
						-DNO_R_DIAG			\
						-DNO_STDIO_H		\
						-DNO_STDLIB_H

do-build:	$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
