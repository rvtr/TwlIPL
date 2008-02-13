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

MYSUBDIRS		=	./

#----------------------------------------------------------------------------
TARGET_FIRM		=	SYSTEMMENU
TARGET_PLATFORM	=	TWL
TWL_ARCHGEN		=	LIMITED
TWL_PROC	   	=	ARM9

SRCS			=	sysmenu_lib.c \
					util.c \
					ninLogoFunc.c \
					mountInfo.c \
					device.c \
					keys.c \
					title.c \
					banner.c \
					../common/src/status.c \
					../common/src/pxi.c


TARGET_LIB		=	libsysmenu$(TWL_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/add-ins/es/commondefs.es
include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		=	$(TARGETS)
INSTALL_DIR			=	$(SYSMENU_INSTALL_LIBDIR)

LINCLUDES		=	$(ROOT)/build/libraries/mb/common/include	\
					$(ROOT)/build/libraries/spi/ARM9/include	\
					$(SYSMENU_ROOT)/build/libraries_sysmenu/sysmenu/common/include

#----------------------------------------------------------------------------

do-build: $(MYSUBDIRS) $(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


$(MYSUBDIRS)::
#	$(MAKE) -C $@ -f MakeCrt0

#===== End of Makefile =====
