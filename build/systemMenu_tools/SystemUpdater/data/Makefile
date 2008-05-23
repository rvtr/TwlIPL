#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL
# File:     Makefile -
#
# Copyright 2007 Nintendo.  All rights reserved.
#
# These coded instructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date:: 
# $Rev: 
# $Author: 
#----------------------------------------------------------------------------

TARGET_FIRM				=	SYSTEMMENU

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs


#----------------------------------------------------------------------------

LDIRT_CLEAN		=	HNAA.tad \
					HNBA.tad \
					HNCA.tad \
					HNHA.tad \
					menu_launcher.nand

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build :	HNAA.tad \
			HNBA.tad \
			HNCA.tad \
			HNHA.tad \
			menu_launcher.nand

HNAA.tad: ../../../systemMenu_RED/Launcher/ARM9/bin/ARM9-TS.LTD/$(TWL_BUILD_DIR)/HNAA.tad
	cp $< ./$@

HNBA.tad: ../../../systemMenu_RED/MachineSettings/ARM9/bin/ARM9-TS.LTD/$(TWL_BUILD_DIR)/HNBA.tad
	cp $< ./$@

HNCA.tad: ../../../systemMenu_RED/wlanfirm/HNCA.tad
	cp $< ./$@

HNHA.tad: ../../../systemMenu_RED/DSHashTable/HNHA.tad
	cp $< ./$@

menu_launcher.nand: ../../../nandfirm/menu-launcher/menu_launcher-$(TWL_BUILD_TYPE)$(CODEGEN_ARCH).nand
	cp $< ./$@

#===== End of Makefile =====
