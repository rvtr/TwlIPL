#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - build
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


#----------------------------------------------------------------------------

SUBDIRS =	\
            ARM7 \
            ARM9 \
            wram_regs \

TARGET_FIRM_BIN	=	norfirm_print.nor norfirm_print.srl
BINDIR			=	.
MAKEFIRM_ARM9	=	ARM9/bin/ARM9-$(FIRM_PLATFORM)$(CODEGEN_ARCH)/$(FIRM_BUILD_DIR)/twl_norfirm9_print.tef
MAKEFIRM_ARM7	=	ARM7/bin/ARM7-$(FIRM_PLATFORM)$(CODEGEN_ARCH)/$(FIRM_BUILD_DIR)/twl_norfirm7_print.tef
MAKEFIRM_RSA_PRVKEY	= $(TWL_KEYSDIR)/rsa/private_nor.der
LDEPENDS_BIN	+=	wram_regs/wram_regs.rbin

MAKEROM_ARM7_BASE       =       $(basename $(MAKEFIRM_ARM7))

NITRO_MAKEROM	=	TRUE
MAKEFIRM_FLAGS	+=	-p
FIRM_SPEC		=	norfirm-BB.norsf

LDIRT_CLEAN		+=	$(TARGETS) \
				$(basename $(firstword $(TARGETS))).tlf \
				rsa_public.sbin \

include	$(TWLFIRM_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:	$(TARGET_BIN)

include	$(TWLFIRM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
