#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - gcdfirm - memory-launcher-writer
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


#----------------------------------------------------------------------------

SUBDIRS =	\
            wram_regs \
            ARM7 \
            ARM9 \

ifneq	($(TWL_IPL_RED_PRIVATE_ROOT),)

TARGET_FIRM_BIN		=	memory_launcher_writer-$(TWL_BUILD_TYPE)$(CODEGEN_ARCH).gcd

NANDFIRM_BIN		=	../../nandfirm/memory-launcher/memory_launcher-$(TWL_BUILD_TYPE)$(CODEGEN_ARCH).nand

APPEND_SRL		=	$(TWLSDK_ROOT)/bin/ARM9-TS/Release/nand2sd.srl
#APPEND_SRL		=	../../systemMenu_tools/NandInitializerRed/ARM9.TWL/bin/ARM9-TS.LTD.thumb/Release/NandInitializerRed.srl

BINDIR			=	.
MAKEFIRM_ARM9		=	ARM9/bin/$(TWL_BUILDTYPE_ARM9)/memory_launcher_writer9.tef
MAKEFIRM_ARM7		=	ARM7/bin/$(TWL_BUILDTYPE_ARM7)/memory_launcher_writer7.tef
MAKEFIRM_RSA_PRVKEY	=	$(TWL_KEYSDIR)/rsa/private_gcd.der
LDEPENDS_BIN		+=	wram_regs/wram_regs.rbin

#MAKEFIRM_FLAGS		+=	-p
FIRM_SPEC		=	gcdfirm.gcdsf

MAKEFIRM_DEFS		+= 	-DNANDFIRM_BIN=$(NANDFIRM_BIN) -DAPPEND_SRL=$(APPEND_SRL)

LDIRT_CLEAN		+=	$(wildcard *.gcd) \
				rsa_public.sbin \

LDEPENDS_BIN		+=	$(NANDFIRM_BIN)

endif

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:	$(TARGET_BIN)

$(NANDFIRM_BIN):
	$(REMAKE) -C $(dir $(NANDFIRM_BIN))

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
