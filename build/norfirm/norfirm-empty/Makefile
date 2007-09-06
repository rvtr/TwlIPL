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


FIRM_TARGET    = NORFIRM


include	$(TWLFIRM_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

SUBDIRS =	\

TARGET_BIN		=	norfirm_empty.nor
BINDIR			=	.
MAKEFIRM_RSA_PRVKEY	= $(TWL_KEYSDIR)/rsa/private_nor.der

#※TwlSDKにはまだちゃんとcomponentsがないので、NitroSDKのものを使用。
MAKEROM_ARM9	 	= $(NITROSDK_ROOT)/components/printserver/ARM9-TS/$(FIRM_BUILD_DIR)/main.nef
MAKEROM_ARM7	 	= $(NITROSDK_ROOT)/components/mongoose/ARM7-TS/$(FIRM_BUILD_DIR)/mongoose_sub.nef


NITRO_MAKEROM	=	TRUE
MAKEFIRM_FLAGS	+=	-p
FIRM_SPEC		=	norfirm-BB.norsf

#----------------------------------------------------------------------------

do-build:	$(TARGET_BIN)

include	$(TWLFIRM_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
