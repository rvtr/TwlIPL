#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - libraries - wm
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

override TWL_CODEGEN		:= ARM
override TWL_ARCHGEN		:= LIMITED

NWM_ORG					=	$(ROOT)/build/libraries/wm/common.TWL/src/nwm_installfirm.c
MAKE_NWM				=	make_nwm_fw_wrack.pl
AUTOGEN_NWM				=	src/nwm_installfirm_autogen.c
LDIRT_CLEAN				+=	$(AUTOGEN_NWM)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#----------------------------------------------------------------------------

TARGETS		=	$(AUTOGEN_NWM)

#----------------------------------------------------------------------------
do-build:	$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

$(AUTOGEN_NWM): $(NWM_ORG) $(MAKE_NWM)
	@echo "     make:" $@
	@perl $(MAKE_NWM)

#===== End of Makefile =====
