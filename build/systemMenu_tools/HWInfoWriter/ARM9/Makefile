#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - SystemMenuRED - HWInfoWriter
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
# $Date:: $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

SUBDIRS					=	\

#----------------------------------------------------------------------------

#============================================================================
#�����_�Ő��i�����g�����������݂��s���Ă̓_���Ȃ̂ŁA�f�t�H���gOFF�ɂ��Ă����Ă��������B
#USE_PRODUCT_KEY			=	TRUE
#============================================================================


TARGET_FIRM				=	SYSTEMMENU
TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED

TITLEID_LO				=	0HWA
#TARGET_TAD				=

TARGET_BIN				=	main.srl

ROM_SPEC				=	main.rsf

MISC_DIR				=	../../../systemMenu_RED/misc

SRCS					=	main.c HWInfoWriter.c hwi.c \
							$(MISC_DIR)/src/misc.c $(MISC_DIR)/src/cmn.c

LINCLUDES				=	$(MISC_DIR)/include \
							$(ROOT)/build/libraries/lcfg/ARM9.TWL/include
LLIBRARIES				=	libacsign$(TWL_LIBSUFFIX).a \
							libacsign_enc$(TWL_LIBSUFFIX).a

ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
ifdef USE_PRODUCT_KEY
MACRO_FLAGS				+=	-DUSE_PRODUCT_KEY
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo.der'
else
MAKEROM_FLAGS			+=	-DHWINFO_PRIVKEY='private_HWInfo_dev.der'
endif
endif

MAKEROM_FLAGS			+=	-DTITLEID_LO='$(TITLEID_LO)' \
							-DPRIVKEY_PATH='$(TWL_KEYSDIR)/rsa'

MAKETAD_FLAGS			+=	-s


include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEROM			 = $(TWL_TOOLSDIR)/bin/makerom.TWL.secure.exe

GLIBRARIES		:= $(filter-out libcrypto$(TWL_LIBSUFFIX).a,$(GLIBRARIES))

#LDIRT_CLEAN				=
#INSTALL_TARGETS			=
INSTALL_DIR				=	$(SDK_NMENU_DATADIR)

#----------------------------------------------------------------------------

do-build : $(TARGETS)


include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#----------------------------------------------------------------------------

#===== End of Makefile =====