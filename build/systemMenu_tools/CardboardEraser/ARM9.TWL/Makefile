#/*---------------------------------------------------------------------------*
#  Project:  TwlIPL - tests - FatalErrorChecker
#  File:     makefile
#
#  Copyright **** Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date::            $
#  $Rev$
#  $Author$
# *---------------------------------------------------------------------------*/
TARGET_FIRM	= SYSTEMMENU
TARGET_PLATFORM	=	TWL
TWL_ARCHGEN   = LIMITED
TWL_NANDAPP	=	TRUE

SUBDIRS          =

SRCS			=	main.c	\
				keypad.c	\
				font.c		\
				screen.c	\
				../../../NandInitializerRed/ARM9.TWL/src/kami_pxi.c

TITLEID_LO		=	0J0A

#ROM_HEADER_TEMPLATE		=	$(SYSMENU_ROM_HEADER_DIR)/$(TITLEID_LO)/rom_header_$(call toLower,$(TITLEID_LO)).template.sbin
#LIBSYSCALL				=	$(SYSMENU_ROM_HEADER_DIR)/$(TITLEID_LO)/libsyscall.a

MAKEROM_FLAGS			+=	-F -DTITLEID_LO='$(TITLEID_LO)'
#							-DROM_HEADER_TEMPLATE='$(call empath,$(ROM_HEADER_TEMPLATE))'

LCFILE_SPEC	=	ARM9-TS.lsf

LINCLUDES		+=	$(MISC_DIR)/include \
					../../NandInitializerRed/ARM9.TWL/include \
					../../NandInitializerRed/common/include \
					./include

ROM_SPEC	=	main.rsf

TARGET_BIN		=	CardboardEraser.srl

LLIBRARIES		+=	liblcfg$(TWL_LIBSUFFIX).a \
					libes$(TWL_LIBSUFFIX).a		\
					libboc$(TWL_LIBSUFFIX).a	\
					libsfs$(TWL_LIBSUFFIX).a	\
					libnam$(TWL_LIBSUFFIX).a	\
					libsea$(TWL_LIBSUFFIX).a	\
					libreboot$(TWL_LIBSUFFIX).a



include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

ifeq ($(FIRM_USE_PRODUCT_KEYS),TRUE)
	MAKELCF_FLAGS	+=	-DNANDFIRM_OBJ=./nandfirm/nandfirm.prod.o
else
	MAKELCF_FLAGS	+=	-DNANDFIRM_OBJ=./nandfirm/nandfirm.dev.o
endif

MAKEROM			=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe
DEFAULT_MAKEROM_ARM7_BASE	= ../../NandInitializerRed/ARM7.TWL/bin/$(TWL_BUILDTYPE_ARM7)/armadillo
DEFAULT_MAKEROM_ARM7		= $(DEFAULT_MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules



#===== End of Makefile =====
