#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - debugsoft - ApplicationJump
# File:     Makefile
#
# Copyright 2008 Nintendo.  All rights reserved.
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
override TARGET_PLATFORM	=	TWL
override TARGET_CODEGEN		=	ARM
override TWL_ARCHGEN		=	LIMITED

TWL_NANDAPP		=	TRUE
TARGET_BIN		=	appJumpChecker.tad

INCDIR			=	./include $(ROOT)/build/libraries/os/common/include
SRCDIR			=	./src

SRCS			=	main.c screen.c font.c common.c

LLIBRARIES		+=	libes$(TWL_LIBSUFFIX).a		\
					libboc$(TWL_LIBSUFFIX).a	\
					libsfs$(TWL_LIBSUFFIX).a
					
#----------------------------------
# sea を使用

LLIBRARIES		+= libsea$(TWL_LIBSUFFIX).a

#----------------------------------
# nam を使用

LLIBRARIES		+=	libnam$(TWL_LIBSUFFIX).a


ROM_SPEC			=	appJumpChecker.autogen.rsf
ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf

ROM_SPEC_PARAM		=	MakerCode=01					\
						NANDAccess=TRUE					\
						Media=NAND						\
						Secure=TRUE						\
						BannerFile=appjumpchecker.bnr	\
						PermitLandingNormalJump=TRUE	\
						WramMapping=MAP_TS_SCR
						
#						GameCode=456A					\

COMPONENT_NAME		=	armadillo
MAKEROM_ARM7_BASE   =   $(TWL_COMPONENTSDIR)/$(COMPONENT_NAME)/$(TWL_BUILDTYPE_ARM7)/$(COMPONENT_NAME)
MAKEROM_ARM7        =   $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)


#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

MAKEROM			:=		$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe
MAKETAD_OPTION	:=	-s

do-build:	$(TARGETS)

INSTALL_TARGETS		=	$(BINDIR)/$(TARGET_BIN)
INSTALL_DIR			=	./roms

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====


