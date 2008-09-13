#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - CalcHMAC_forBroadOn
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
# $Date:: #$
# $Rev$
# $Author$
#----------------------------------------------------------------------------
TARGET_PLATFORM	:= TWL

SRCS            =   CalcHMAC_forBroadOn.c

TARGET_BIN      =   CalcHMAC_forBroadOn.srl

TITLEID_LO		=	0BRA

ROM_SPEC		=	main.rsf

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

LINCLUDES       = 	$(ROOT)/build/libraries/lcfg/ARM9.TWL/include
LLIBRARIES		=	liblcfg$(TWL_LIBSUFFIX).a

ifneq ($(TWL_IPL_RED_PRIVATE_ROOT),)
MAKEROM_FLAGS	+=	-DHWINFO_PRIVKEY='private_HWInfo.der'
endif

MAKEROM_FLAGS	+=	-DTITLEID_LO='$(TITLEID_LO)' \
					-DPRIVKEY_PATH='$(TWL_IPL_RED_PRIVATE_ROOT)/keys/rsa' \
					-DCARD_REGION='$(CARD_REGION)'

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
