#/*---------------------------------------------------------------------------*
#  Project:  TwlIPL - tests - FatalErrorMaker
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

SUBDIRS          =

MISC_DIR	=	../../../systemMenu_RED/misc

SRCS			=	main.c	fatalErrorMaker.c \
				$(MISC_DIR)/src/misc.c	\
				$(MISC_DIR)/src/cmn.c

LINCLUDES		+=	$(MISC_DIR)/include 

ROM_SPEC	=	main.rsf

TARGET_BIN		=	FatalErrorMaker.srl

SYSMENU_LIBS			=	\
							liblcfg$(TWL_LIBSUFFIX).a \
							libsysmenu$(TWL_LIBSUFFIX).a \
							libsysmmcu$(TWL_LIBSUFFIX).a \
							libsysmutil$(TWL_LIBSUFFIX).a \
							libboot$(TWL_LIBSUFFIX).a \
							libds$(TWL_LIBSUFFIX).a \
							libhotsw$(TWL_LIBSUFFIX).a \
							libdht$(TWL_LIBSUFFIX).a \
							libreloc_info$(TWL_LIBSUFFIX).a \
							libnamut$(TWL_LIBSUFFIX).a \
							libsharedfont_launcher$(TWL_LIBSUFFIX).a \
							WDS$(TWL_LIBSUFFIX).a

SDK_APPEND_LIBS		=		\
							libes$(TWL_LIBSUFFIX).a		\
							libboc$(TWL_LIBSUFFIX).a	\
							libsfs$(TWL_LIBSUFFIX).a	\
							libnam$(TWL_LIBSUFFIX).a	\
							libsea$(TWL_LIBSUFFIX).a	\
							libreboot$(TWL_LIBSUFFIX).a	\
							libna$(TWL_LIBSUFFIX).a

LLIBRARIES	+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)


include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEROM			=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe
DEFAULT_MAKEROM_ARM7_BASE	= $(TWL_COMPONENTSDIR)/armadillo/$(TWL_BUILDTYPE_ARM7)/armadillo
DEFAULT_MAKEROM_ARM7		= $(DEFAULT_MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)



#----------------------------------------------------------------------------
do-build:		$(TARGETS)

#----------------------------------------------------------------------------

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
