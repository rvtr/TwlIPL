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
				kami_font.c	\
				graphics.c	\
				font_data.c	

LINCLUDES		+=	$(MISC_DIR)/include 

ROM_SPEC	=	main.rsf

TARGET_BIN		=	menu.srl

LLIBRARIES		+=	liberrorlog$(TWL_LIBSUFFIX).a

MAKETAD_OPTION	+= -s

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEROM			=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe
DEFAULT_MAKEROM_ARM7_BASE	= $(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
DEFAULT_MAKEROM_ARM7		= $(DEFAULT_MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

#----------------------------------------------------------------------------
do-build:		$(TARGETS)

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules



#===== End of Makefile =====
