#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - norfirm - norfirm-print
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

TARGET_FIRM_BIN		=	norfirm_print-$(TWL_BUILD_TYPE)$(CODEGEN_ARCH).nor
BINDIR			=	.
MAKEFIRM_ARM9		=	ARM9/bin/$(TWL_BUILDTYPE_ARM9)/twl_norfirm9_print.tef
MAKEFIRM_ARM7		=	ARM7/bin/$(TWL_BUILDTYPE_ARM7)/twl_norfirm7_print.tef
MAKEFIRM_RSA_PRVKEY	=	$(TWL_KEYSDIR)/rsa/private_nor.der
LDEPENDS_BIN		+=	wram_regs/wram_regs.rbin

MAKEFIRM_FLAGS		+=	-p
FIRM_SPEC		=	norfirm-BB.norsf

LDIRT_CLEAN		+=	$(wildcard *.nor) \
				rsa_public.sbin \

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:	$(TARGET_BIN)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
