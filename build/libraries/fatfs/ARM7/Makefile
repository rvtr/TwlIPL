#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - libraries - fatfs
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
# $Rev:$
# $Author:$
#----------------------------------------------------------------------------

SUBDIRS		=
SUBMAKES	=


#----------------------------------------------------------------------------

# build ARM & THUMB libraries
FIRM_CODEGEN_ALL 	?= TRUE

# Codegen for sub processer
FIRM_PROC		= ARM7

INCDIR			= rom_sdmc \
			  $(TWLSDK_ROOT)/build/libraries/fatfs/ARM7.TWL/include \
			  $(TWLSDK_ROOT)/build/libraries/fatfs/ARM7.TWL/include/twl/fatfs/ARM7

SRCDIR			= src rom_sdmc

SRCS			= \
				fatfs_loader.c 	\
				fatfs_firm.c 	\
#				sdmc.c		\
#				sdmc_nandinit.c	\
#				drsdmc.c	\


TARGET_LIB		= libfatfs_sp$(FIRM_LIBSUFFIX).a

#----------------------------------------------------------------------------

# DEBUG版ビルドの場合、RELEASE版でビルドして
# DEBUG版のライブラリを装います。

ifdef	NITRO_DEBUG
NITRO_BUILD_TYPE	= RELEASE
endif

include	$(TWLFIRM_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR		= $(FIRM_INSTALL_LIBDIR)


#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLFIRM_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
