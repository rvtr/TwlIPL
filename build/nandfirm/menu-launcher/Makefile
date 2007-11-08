#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - nandfirm - menu-launcher
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
# $Rev:$
# $Author:$
#----------------------------------------------------------------------------


FIRM_TARGET    = FIRM

# if you have valid keys, set environment value like below
#export TWL_KEYSDIR='$(FIRM_ROOT)/../twl_firmware/bootrom/build/keys'


#----------------------------------------------------------------------------

SUBDIRS =	\
            wram_regs \
            ARM7 \
            ARM9 \

TARGET_FIRM_BIN		=	menu_launcher-$(FIRM_BUILD_TYPE).nand
BINDIR			=	.
MAKEFIRM_ARM9		=	ARM9/bin/ARM9-$(FIRM_PLATFORM)$(CODEGEN_ARCH)/$(FIRM_BUILD_DIR)/menu_launcher9.tef
MAKEFIRM_ARM7		=	ARM7/bin/ARM7-$(FIRM_PLATFORM)$(CODEGEN_ARCH)/$(FIRM_BUILD_DIR)/menu_launcher7.tef
MAKEFIRM_RSA_PRVKEY	=	$(TWL_KEYSDIR)/rsa/private_nand.der
LDEPENDS_BIN		+=	wram_regs/wram_regs.rbin

MAKEFIRM_FLAGS		+=	-p
FIRM_SPEC		=	nandfirm.nandsf

LDIRT_CLEAN		+=	$(TARGETS) \
				rsa_public.sbin \

include	$(TWLIPL_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:	$(TARGET_BIN)

include	$(TWLIPL_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
