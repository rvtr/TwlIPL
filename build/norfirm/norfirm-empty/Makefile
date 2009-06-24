#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - norfirm - norfirm-empty
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

override TARGET_PLATFORM	:= TWL
override TARGET_CODEGEN		:= ARM
override TWL_ARCHGEN		:= LIMITED

override TARGET_FINALROM	:= TRUE
override TARGET_RELEASE		:=
override TARGET_DEBUG		:=

#----------------------------------------------------------------------------

SUBDIRS =	\

TARGET_FIRM_BIN		=	norfirm_empty.nor
BINDIR			=	.
MAKEFIRM_ARM9	 	=
MAKEFIRM_ARM7	 	=
MAKEFIRM_RSA_PRVKEY	=	$(TWL_KEYSDIR)/rsa/private_nor.der
#MAKEFIRM_FLAGS		+=	-p
FIRM_SPEC		=	norfirm-BB.norsf

LDIRT_CLEAN		+=	$(TARGETS) \
				rsa_public.sbin \

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

do-build:	$(TARGET_BIN)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
