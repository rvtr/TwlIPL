#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tests - NotLaunch
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
# $Rev:$
# $Author:$
#----------------------------------------------------------------------------
override TARGET_PLATFORM	=	TWL
override TWL_ARCHGEN		=	LIMITED

ifeq ($(APP_TYPE),)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:	do-something
clean:		do-something
clobber:	do-something
install:	do-something
do-build:	MY_GOAL=default
clean:		MY_GOAL=clean
clobber:	MY_GOAL=clobber
install:	MY_GOAL=install

do-something:
	+$(REMAKE) -B APP_TYPE=SYSTEM CARD_REGION=ALL $(MY_GOAL)
	+$(REMAKE) -B APP_TYPE=SYSTEM CARD_REGION=Japan $(MY_GOAL)
	+$(REMAKE) -B APP_TYPE=SYSTEM CARD_REGION=America $(MY_GOAL)
	+$(REMAKE) -B APP_TYPE=SYSTEM CARD_REGION=Europe $(MY_GOAL)
	+$(REMAKE) -B APP_TYPE=SYSTEM CARD_REGION=Australia $(MY_GOAL)
#	+$(REMAKE) -B APP_TYPE=USER CARD_REGION=ALL $(MY_GOAL)
#	+$(REMAKE) -B APP_TYPE=USER CARD_REGION=Japan $(MY_GOAL)
#	+$(REMAKE) -B APP_TYPE=USER CARD_REGION=America $(MY_GOAL)
#	+$(REMAKE) -B APP_TYPE=USER CARD_REGION=Europe $(MY_GOAL)
#	+$(REMAKE) -B APP_TYPE=USER CARD_REGION=Australia $(MY_GOAL)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

else

TWL_NANDAPP		=	TRUE
TARGET_BIN		=	NotLaunch_$(APP_TYPE)_$(CARD_REGION).tad

SRCDIR			=	./src

SRCS			=	main.c

ROM_SPEC			=	NotLaunch.autogen.rsf

ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf

#----------------------------------------------------------------------------
# decide gamecode (region)

ifeq ($(CARD_REGION),ALL)
GAME_CODE_R		=	A
endif

ifeq ($(CARD_REGION),Japan)
GAME_CODE_R		=	J
endif

ifeq ($(CARD_REGION),America)
GAME_CODE_R		=	E
endif

ifeq ($(CARD_REGION),Europe)
GAME_CODE_R		=	P
endif

ifeq ($(CARD_REGION),Australia)
GAME_CODE_R		=	U
endif

ifeq ($(CARD_REGION),China)
GAME_CODE_R		=	C
endif

ifeq ($(CARD_REGION),Korea)
GAME_CODE_R		=	K
endif
#----------------------------------------------------------------------------

ROM_SPEC_PARAM		=	MakerCode=01					\
						GameCode=457$(GAME_CODE_R)		\
						Media=NAND						\
						Launch=FALSE					\
						CardRegion=$(CARD_REGION)

ifeq ($(APP_TYPE),USER)
ROM_SPEC_PARAM		+=	AppType=User
endif

endif	# ifeq ($(APP_TYPE),)

#----------------------------------------------------------------------------
include	$(TWLSDK_ROOT)/build/buildtools/commondefs

ifeq ($(APP_TYPE),SYSTEM)
MAKEROM        :=   $(TWL_TOOLSDIR)/bin/makerom.TWL.sys.exe
MAKETAD_OPTION :=   -s
endif

do-build:	$(TARGETS)

INSTALL_TARGETS		=	$(BINDIR)/$(TARGET_BIN)
INSTALL_DIR			=	./roms

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====


