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

include	$(TWLIPL_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------
export TWL_ARCHGEN = ALL

SUBDIRS =	\
			$(TWLSDK_ROOT)/add-ins/NitroWireless \
			$(TWLSDK_ROOT)/build/libraries/ese \
			$(TWLSDK_ROOT)/build/libraries/nam \
			$(TWLSDK_ROOT)/build/libraries/reboot \
			$(TWLSDK_ROOT)/build/libraries/aes \
			$(TWLSDK_ROOT)/build/libraries/spi \
			$(TWLSDK_ROOT)/build/components/racoon.TWL \
			$(TWLSYSTEM_ROOT) \
			build

#----------------------------------------------------------------------------

include	$(TWLIPL_ROOT)/build/buildtools/modulerules


#===== End of Makefile =====
