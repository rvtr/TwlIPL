#! make -f
#----------------------------------------------------------------------------
# Project:  NitroWiFi - demos - ps-ping
# File:     Makefile
#
# Copyright 2005 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Log: Makefile,v $
#
# $NoKeywords: $
#----------------------------------------------------------------------------

# Generate my own lsf from original lsf of mongoose 
RSF_ORG					=	$(ROOT)/include/twl/specfiles/ROM-TS.rsf
MAKE_RSF				=	make_rsf.pl

AUTOGEN_RSF				=	wireless_checker.rsf

LDIRT_CLEAN				=	$(AUTOGEN_RSF)

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

do-build:	$(AUTOGEN_RSF)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

$(AUTOGEN_RSF): $(RSF_ORG) $(MAKE_RSF)
	@echo "     make:" $@
	perl $(MAKE_RSF) $(AUTOGEN_RSF)

#===== End of Makefile =====
