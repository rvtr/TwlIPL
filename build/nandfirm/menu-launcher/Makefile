#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - nandfirm - menu-launcher
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
# $Rev$
# $Author$
#----------------------------------------------------------------------------


#----------------------------------------------------------------------------

SUBDIRS =	\
            wram_regs \
            ARM7 \
            ARM9 \

TARGET_FIRM_BIN		=	menu_launcher-$(TWL_BUILD_TYPE)$(CODEGEN_ARCH)-$(IPL_REVISION)-$(SDK_REVISION).$(FIRM_MODE).nand
BINDIR			=	.
MAKEFIRM_ARM9		=	ARM9/bin/$(TWL_BUILDTYPE_ARM9)/menu_launcher9.tef
MAKEFIRM_ARM7		=	ARM7/bin/$(TWL_BUILDTYPE_ARM7)/menu_launcher7.tef
MAKEFIRM_RSA_PRVKEY	=	$(TWL_KEYSDIR)/rsa/private_nand.der
LDEPENDS_BIN		+=	wram_regs/wram_regs.rbin

#MAKEFIRM_FLAGS		+=	-p
FIRM_SPEC		=	nandfirm.nandsf

MY_APPEND			=	revision.bin

LDIRT_CLEAN		+=	$(wildcard *.nand) \
				rsa_public.sbin \

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

ifeq ($(FIRM_USE_PRODUCT_KEYS),TRUE)
FIRM_MODE			=	prod
else
FIRM_MODE			=	dev
endif

IPL_REVISION 		:=	$(shell LANG=C svn info $(call empath,$(FIRM_ROOT)) | grep 'Revision' | sed s/[^0-9]//g)
SDK_REVISION		:=	$(shell LANG=C svn info $(call empath,$(ROOT)) | grep 'Revision' | sed s/[^0-9]//g)

ifeq ($(IPL_REVISION),)
IPL_REVISION		:=	$(shell date +%Y%m%d)
endif
ifeq ($(SDK_REVISION),)
SDK_REVISION		:=	$(shell date +%H%M%S)
endif

#----------------------------------------------------------------------------

do-build:	$(TARGET_BIN) $(MY_APPEND)
	cat $(MY_APPEND) >> $(TARGET_BIN)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

$(MY_APPEND)::
	echo $(IPL_REVISION) > $(MY_APPEND)
	echo $(SDK_REVISION) >> $(MY_APPEND)

test::
	echo IPL_REVISION=$(IPL_REVISION)


#===== End of Makefile =====
