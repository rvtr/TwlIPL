#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - save_data - DisplayVersion
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
# $Date::$
# $Rev$
# $Author$
#----------------------------------------------------------------------------


override TARGET_PLATFORM	= TWL
override TARGET_CODEGEN		= ARM
override TWL_ARCHGEN		= LIMITED



ifeq	($(ROM_NAME),)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:	do-something-build
clean:		do-something
clobber:	do-something
install:	do-something
do-build:	MY_GOAL=
clean:		MY_GOAL=clean
clobber:	MY_GOAL=clobber
install:	MY_GOAL=install

do-something-build:
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=A SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=B SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=C SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=D SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=E SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=F SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=G SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=H SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=I SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=J SKIP_SUBDIR=FALSE $(MY_GOAL)

do-something:
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=A SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=B SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=C SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=D SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=E SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=F SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=G SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=H SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=I SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL ROM_NAME=J SKIP_SUBDIR=FALSE $(MY_GOAL)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules


else

SUBDIRS		=
MACRO_FLAGS	:= 

#----------------------------------------------------------------------------
TWL_NANDAPP		:= TRUE
MASTER_NAME		:=		TitleList
APP_NAME        =       $(MASTER_NAME)_$(ROM_NAME)

SRCS		=	main.c font.c screen.c appmain.c

LLIBRARIES      =       libnam$(TWL_LIBSUFFIX).a libes$(TWL_LIBSUFFIX).a  \
                        libsea$(TWL_LIBSUFFIX).a libboc$(TWL_LIBSUFFIX).a \
			libsfs$(TWL_LIBSUFFIX).a

#-------------------------
#-- ROM A ～ Jまでのパラメータ設定

# デフォルトのロムパラメータ
PUB_SAVE	:=	16K
PRV_SAVE	:=	16K
MAKER_CODE	:=	01
SEQURE	:=	FALSE
APP_TYPE	:=	User

ifeq	($(ROM_NAME),A)
APP_TYPE	:=	System
SEQURE	:=	TRUE
endif

ifeq	($(ROM_NAME),B)
APP_TYPE	:=	System
endif

ifeq	($(ROM_NAME),C)
endif

ifeq	($(ROM_NAME),D)
PRV_SAVE	:=	0K
endif

ifeq	($(ROM_NAME),E)
PUB_SAVE	:=	0K
endif

ifeq	($(ROM_NAME),F)
PRV_SAVE	:=	0K
PUB_SAVE	:=	0K
endif

ifeq	($(ROM_NAME),G)
MAKER_CODE	:=	02
endif

ifeq	($(ROM_NAME),H)
MAKER_CODE	:=	02
PRV_SAVE	:=	0K
endif

ifeq	($(ROM_NAME),I)
MAKER_CODE	:=	02
PUB_SAVE	:=	0K
endif

ifeq	($(ROM_NAME),J)
MAKER_CODE	:=	02
PRV_SAVE	:=	0K
PUB_SAVE	:=	0K
endif



ENABLE_PUB	:=	TRUE
ENABLE_PRV	:=	TRUE

ifeq	($(ENABLE_PUB),0K)
ENABLE_PUB	:=	FALSE
endif

ifeq	($(ENABLE_PRV),0K)
ENABLE_PRV	:=	FALSE
endif


GAME_CODE	=	4K$(ROM_NAME)A
TARGET_BIN	=	$(APP_NAME).tad



#-------------------------
#-- NAND アプリではいくつかのパラメータの指定のために固有の RSF ファイルが必要です。

ROM_SPEC			=	$(MASTER_NAME).$(ROM_NAME).autogen.rsf
ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf
ROM_SPEC_PARAM		=	MakerCode=$(MAKER_CODE)					\
						TitleName=TITLE_LIST_$(ROM_NAME)	\
						GameCode=$(GAME_CODE)					\
						BannerFile=./banner/banner_$(ROM_NAME).bnr	\
						PublicSaveDataSize=$(PUB_SAVE)			\
						PrivateSaveDataSize=$(PRV_SAVE)		\
						WramMapping=MAP_TS_SCR		\
						AppType=$(APP_TYPE)			\
						Media=NAND			\
						Secure=$(SEQURE)			\



#-------------------------
#-- NAMを使うためにsecure componentを使う

COMPONENT_NAME      =   armadillo
MAKEROM_ARM7_BASE   =   $(TWL_COMPONENTSDIR)/$(COMPONENT_NAME)/$(TWL_BUILDTYPE_ARM7)/$(COMPONENT_NAME)
MAKEROM_ARM7        =   $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

#-------------------------
# インストール指定

ifneq	($(TWL_IPL_RED_ROOT),)
INSTALL_DIR	= $(TWL_IPL_RED_ROOT)/debugsoft/$(MASTER_NAME)
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif

#-------------------------
#-- カードアプリと同様に ROM-FS にファイルを持たせます。

#MAKEROM_ROMROOT	 = ./rom_data
#MAKEROM_ROMFILES = *.*

#-------------------------
#-- バナーデータを生成します
BANNER          = ./banner/banner_$(ROM_NAME).bnr
BANNERSRC      := ./banner/data/Cell/Cell_$(ROM_NAME).nce
#BANNERSRC      := $(wildcard ./banner/data/Cell/*.nce)
MAKEBANNER      = $(TWL_TOOLSDIR)/bin/makebanner.TWL.exe
BANNERCVTR      = $(TWL_TOOLSDIR)/bin/bannercvtr.exe

#-------------------------
# rom 内のファイルは全て自動生成なので clean で削除されるようにします
LDIRT_CLEAN		 =	$(MAKEROM_ROMROOT)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

ifeq	($(SEQURE),TRUE)
MAKEROM        :=   $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe   # commondefs を include した後でなければならない
MAKETAD_OPTION :=   -s                                           # modulerules を include する前でなければならない
else
ifeq	($(APP_TYPE),System)
MAKEROM        :=   $(TWL_TOOLSDIR)/bin/makerom.TWL.sys.exe   # commondefs を include した後でなければならない
MAKETAD_OPTION :=   -s                                           # modulerules を include する前でなければならない
else
MAKEROM        :=   $(TWL_TOOLSDIR)/bin/makerom.TWL.exe   # commondefs を include した後でなければならない
endif
endif
#----------------------------------------------------------------------------

#MACRO_FLAGS	+=	-DMYCODE=\"$(GAME_CODE)\"
#MACRO_FLAGS	+=	-DMYNAME=\"$(ROM_NAME)\"
#MACRO_FLAGS	+=	-DENABLEPUB=$(ENABLE_PUB)
#MACRO_FLAGS	+=	-DENABLEPRV=$(ENABLE_PRV)


do-build:		rmlcf mycode $(TARGETS)
#do-build:		rmlcf $(TARGETS)

.PHONY:	mycode rmlcf

mycode:
	echo -e "#ifndef MYCODE_\n#define MYCODE_\n#define MYCODE \"$(GAME_CODE)\"\n#define MYNAME \"$(ROM_NAME)\"\n#define ENABLEPUB $(ENABLE_PUB)\n#define ENABLEPRV $(ENABLE_PRV)\n\n#endif" > ./include/mycode.h

#mycode:
#	echo -e "#ifndef MYCODE_\n#define MYCODE_\n#define MYCODE \"$(GAME_CODE)\"\n#define MYNAME \"$(ROM_NAME)\"\n#define ENABLEPUB $(ENABLE_PUB)\n#define ENABLEPRV $(ENABLE_PRV)\n\n#endif" > ./include/mycode.h


rmlcf:
	rm -rf $(BINDIR)/*.lcf

#-------------------------
#-- SRL を作成する前にバナーが作成されるようにします。

$(BINDIR)/$(TARGET_BIN_BASENAME).$(TWL_ELF_EXT):	make_dir
#$(BINDIR)/$(TARGET_BIN_BASENAME).$(TWL_ELF_EXT):	$(BANNER) make_dir

#-------------------------
#-- rom_data/build_time.txt を作成します。

make_dir:
	$(INSTALL) -d $(MAKEROM_ROMROOT)
#build_time:
#	$(INSTALL) -d $(MAKEROM_ROMROOT)
#	echo -n "build at `date \"+%Y/%m/%d %H:%M:%S\"` on \"`hostname`\"" > $(MAKEROM_ROMROOT)/build_time.txt

#-------------------------
#-- バナー作成用ターゲット

$(BANNER): $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf)
	$(MAKEBANNER) -A $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf) $@

$(BANNER:.bnr=.bin): $(BANNERSRC)
	$(BANNERCVTR) -o $@ $<


include	$(TWLSDK_ROOT)/build/buildtools/modulerules

endif

#===== End of Makefile =====
