#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools - NandFiler
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

override TARGET_PLATFORM	= TWL
override TARGET_CODEGEN		= ARM
override TWL_ARCHGEN		= LIMITED



#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL
TWL_ARCHGEN		:= LIMITED
SRCS			 =	main.cpp
TARGET_NAME		=	sd2photo
TWLNMENU_ROOT	?=	.


#-------------------------
#-- �o�i�[�f�[�^�𐶐����܂�
BANNER          = ./banner/banner.bnr
BANNERSRC      := $(wildcard ./banner/data/Cell/*.nce)
MAKEBANNER      = $(TWL_TOOLSDIR)/bin/makebanner.TWL.exe
BANNERCVTR      = $(TWL_TOOLSDIR)/bin/bannercvtr.exe


#-------------------------
#-- NAND �A�v���ł͂������̃p�����[�^�̎w��̂��߂ɌŗL�� RSF �t�@�C�����K�v�ł��B 

ROM_SPEC			=	$(TARGET_NAME)$(ROM_SPEC_NAME_PART).autogen.rsf
ROM_SPEC_TEMPLATE	=	$(ROOT)/include/twl/specfiles/ROM-TS_sys.rsf
ROM_SPEC_PARAM		=	MakerCode=01				\
						GameCode=4S01				\
						Secure=TRUE					\
						BannerFile=./banner/banner.bnr	\
						NANDAccess=TRUE				\
						SDCardAccess=TRUE	\
						TitleName=$(TARGET_NAME)

#-------------------------
#-- CARD, NAND�ɂ��I�v�V�����̎w��

TWL_NANDAPP		=	TRUE

ifeq	($(APP_TYPE),)
APP_TYPE	=	CARD
endif

ifeq	($(APP_TYPE),CARD)
MEDIA	=	GameCard
SUFFIX	=	.srl
endif

ifeq	($(APP_TYPE),NAND)
MEDIA	=	NAND
SUFFIX	=	.tad
ROM_SPEC_NAME_PART	:=	$(ROM_SPEC_NAME_PART).nand
endif

ROM_SPEC_PARAM	+=	Media=$(MEDIA)
TARGET_BIN	=	$(TARGET_NAME)$(SUFFIX)

#-------------------------
# �Ώۊg���q�̂��߂̃C���N���[�h�t�@�C��������
ifneq	($(SUFFIX_DEFINE),)
SUFFIX_FILE	=	$(SUFFIX_DEFINE)
else
SUFFIX_FILE	=	default_suffix.h
endif


include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#-------------------------
# �Z�L���A�p 
MAKEROM			:=	$(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe
MAKETAD_OPTION	:=	-s


#-------------------------
# �r���h�p�����[�^ 
INCDIR			=	$(TWLSYSTEM_ROOT)/include	\
					./include


#-------------------------
# �C���X�g�[���w��

ifneq	($(TWL_IPL_RED_ROOT),)
INSTALL_DIR	= $(TWL_IPL_RED_ROOT)/debugsoft/$(TARGET_NAME)
INSTALL_TARGETS	= $(BINDIR)/$(TARGET_BIN)
endif

#-------------------------
# �Ώۊg���q�̂��߂̃C���N���[�h�t�@�C�����r���h�f�B���N�g���ɃR�s�[
.PHONY: suffix

suffix:
	cp $(SUFFIX_FILE) ./include/suffix.h

#-------------------------
# �r���h
do-build:		suffix $(TARGETS)

#-------------------------
#-- SRL ���쐬����O�Ƀo�i�[���쐬�����悤�ɂ��܂��B

ifdef	MAKE_BANNER
$(BINDIR)/$(TARGET_NAME)$(ROM_SPEC_NAME_PART).srl: $(BANNER)
endif


#-------------------------
#-- �o�i�[�쐬�p�^�[�Q�b�g

$(BANNER): $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf)
	$(MAKEBANNER) -A $(BANNER:.bnr=.bin) $(BANNER:.bnr=.bsf) $@

$(BANNER:.bnr=.bin): $(BANNERSRC)
	$(BANNERCVTR) -o $@ $<

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====