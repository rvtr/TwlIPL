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

SUBDIRS				=

#----------------------------------------------------------------------------
TARGET_FIRM			=	SYSTEMMENU
TARGET_PLATFORM		=	TWL
TWL_ARCHGEN			=	LIMITED
TWL_PROC	    	=	ARM7
override TWL_CODEGEN 	= ALL

#----------------------------------------------------------------------------

SRCDIR				=	./src

INCDIR				=	../common/include \
						$(ROOT)/build/libraries/fatfs/ARM7.TWL/include \
						$(ROOT)/build/libraries/fatfs/ARM7.TWL/include/twl/fatfs/ARM7 \

SRCS				=	bootAPI.c \

TARGET_LIB			=	libboot_sp$(TWL_LIBSUFFIX).a


include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/libraries/reboot/commondefs.reboot

LINCLUDES		+=	$(SYSMENU_ROOT)/build/libraries_sysmenu/sysmenu/common/include

INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR			=	$(SYSMENU_INSTALL_LIBDIR)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====

