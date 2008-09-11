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

ifndef MODE
#------------------------------------------------------------------------
# APP_TYPE �̎w�肪�Ȃ��Ȃ�S�Ă��r���h����

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
	+$(REMAKE) TWLSDK_PLATFORM=TWL MODE=SEQURE SKIP_SUBDIR=FALSE $(MY_GOAL)
	rm `find -name *.lcf`
	rm -rf depend
	rm -rf obj
	+$(REMAKE) TWLSDK_PLATFORM=TWL MODE=USER SKIP_SUBDIR=FALSE $(MY_GOAL)

do-something:
	+$(REMAKE) TWLSDK_PLATFORM=TWL MODE=SEQURE SKIP_SUBDIR=FALSE $(MY_GOAL)
	+$(REMAKE) TWLSDK_PLATFORM=TWL MODE=USER SKIP_SUBDIR=FALSE $(MY_GOAL)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

else



SUBDIRS		=


#----------------------------------------------------------------------------
TWL_NANDAPP		:= TRUE
MASTER_NAME		:=		SaveDataTest
APP_NAME        =       $(MASTER_NAME)_$(MODE)

SRCS		=	main.c font.c screen.c menu.c dataver.c

LLIBRARIES      =       libnam$(TWL_LIBSUFFIX).a libes$(TWL_LIBSUFFIX).a  \
                        libsea$(TWL_LIBSUFFIX).a libboc$(TWL_LIBSUFFIX).a \
			libsfs$(TWL_LIBSUFFIX).a

#-------------------------
#-- �Z�L���A�A�v���A���[�U�A�v���̐؂�ւ�

ifeq	($(MODE),USER)
MACRO_FLAGS	+=	-DUSER_MODE
APP_TYPE	:=	User
SEQURE	:=	FALSE
GAME_CODE	:=	4S5A
endif

ifeq	($(MODE),SEQURE)
APP_TYPE	:=	System
SEQURE	:=	TRUE
GAME_CODE	:=	4S4A
endif


TARGET_BIN	=	$(APP_NAME).tad


#-------------------------
#-- NAND �A�v���ł͂������̃p�����[�^�̎w��̂��߂ɌŗL�� RSF �t�@�C�����K�v�ł��B

ROM_SPEC			=	dispver.$(MODE).autogen.rsf
ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf
ROM_SPEC_PARAM		=	MakerCode=01					\
						TitleName=VER_CHECKER	\
						GameCode=$(GAME_CODE)					\
						BannerFile=./banner/banner_$(MODE).bnr	\
						PublicSaveDataSize=16K			\
						PrivateSaveDataSize=16K		\
						WramMapping=MAP_TS_SCR		\
						AppType=$(APP_TYPE)			\
						Media=NAND			\
						Secure=$(SEQURE)			\



#-------------------------
#-- NAM���g�����߂�secure component���g��

COMPONENT_NAME      =   armadillo
MAKEROM_ARM7_BASE   =   $(TWL_COMPONENTSDIR)/$(COMPONENT_NAME)/$(TWL_BUILDTYPE_ARM7)/$(COMPONENT_NAME)
MAKEROM_ARM7        =   $(MAKEROM_ARM7_BASE).$(TWL_ELF_EXT)

#-------------------------
# �C���X�g�[���w��

ifneq	($(TWL_IPL_RED_ROOT),)
INSTALL_DIR	= $(TWL_IPL_RED_ROOT)/debugsoft/$(MASTER_NAME)
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif


#-------------------------
#-- �J�[�h�A�v���Ɠ��l�� ROM-FS �Ƀt�@�C�����������܂��B

#MAKEROM_ROMROOT	 = ./rom_data
#MAKEROM_ROMFILES = *.*

#-------------------------
#-- �o�i�[�f�[�^�𐶐����܂�
BANNER          = ./banner/banner.bnr
BANNERSRC      := $(wildcard ./banner/data/Cell/*.nce)
MAKEBANNER      = $(TWL_TOOLSDIR)/bin/makebanner.TWL.exe
BANNERCVTR      = $(TWL_TOOLSDIR)/bin/bannercvtr.exe

#-------------------------
# rom ���̃t�@�C���͑S�Ď��������Ȃ̂� clean �ō폜�����悤�ɂ��܂�
LDIRT_CLEAN		 =	$(MAKEROM_ROMROOT)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

ifeq	($(MODE),USER)
MAKEROM        :=   $(TWL_TOOLSDIR)/bin/makerom.TWL.exe   # commondefs �� include ������łȂ���΂Ȃ�Ȃ�
endif

ifeq	($(MODE),SEQURE)
MAKEROM        :=   $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe   # commondefs �� include ������łȂ���΂Ȃ�Ȃ�
MAKETAD_OPTION :=   -s                                           # modulerules �� include ����O�łȂ���΂Ȃ�Ȃ�
endif

#----------------------------------------------------------------------------

#.PHONY:	build_time

do-build:		$(TARGETS)

#-------------------------
#-- SRL ���쐬����O�Ƀo�i�[���쐬�����悤�ɂ��܂��B

$(BINDIR)/$(TARGET_BIN_BASENAME).$(TWL_ELF_EXT):	make_dir
#$(BINDIR)/$(TARGET_BIN_BASENAME).$(TWL_ELF_EXT):	make_dir $(BANNER)
#build_time

#-------------------------
#-- rom_data/build_time.txt ���쐬���܂��B

make_dir:
	$(INSTALL) -d $(MAKEROM_ROMROOT)
#build_time:
#	$(INSTALL) -d $(MAKEROM_ROMROOT)
#	echo -n "build at `date \"+%Y/%m/%d %H:%M:%S\"` on \"`hostname`\"" > $(MAKEROM_ROMROOT)/build_time.txt

#-------------------------
#-- �o�i�[�쐬�p�^�[�Q�b�g

$(BANNER): $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf)
	$(MAKEBANNER) -A $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf) $@

$(BANNER:.bnr=.bin): $(BANNERSRC)
	$(BANNERCVTR) -o $@ $<


include	$(TWLSDK_ROOT)/build/buildtools/modulerules

endif

#===== End of Makefile =====