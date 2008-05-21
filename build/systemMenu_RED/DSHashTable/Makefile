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

TARGET_FIRM			=	SYSTEMMENU

override TARGET_PLATFORM	:= TWL
override TARGET_CODEGEN		:= ARM
override TWL_ARCHGEN		:= LIMITED

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MY_TITLE		=	HNHA

MY_DATA			=	DSHashTable.bin

#MY_TITLE_NAME		=	$(MY_TITLE)
MY_TITLE_NAME		=	DSHashTable


#----------------------------------------------------------------------------
MY_TAD			=	$(MY_TITLE).tad

MY_TITLE_ID_HI		=	0003000F
MY_TITLE_ID_LO		=	$(shell perl -e 'printf "%02X%02X%02X%02X", unpack("C4", "'$(MY_TITLE)'")')
MY_TITLE_ID		=	$(MY_TITLE_ID_HI)$(MY_TITLE_ID_LO)
MY_GROUP_ID		=	3031

MY_VERSION		=	$(shell LANG=C svn info $(MY_DATA) | grep 'Revision' | gawk '{print $$2}')

MY_MAJOR_VERSION	=	$(shell expr $(MY_VERSION) / 256)
MY_MINOR_VERSION	=	$(shell expr $(MY_VERSION) % 256)


INSTALL_TARGETS		=	$(MY_TAD)
INSTALL_DIR		=	$(ROOT)/build/tools/TwlNMenu/data

MAKETAD_FLAGS		=	-d $(MY_TITLE_ID) $(MY_GROUP_ID) $(MY_MAJOR_VERSION) $(MY_TITLE_NAME) \
				-v $(MY_MINOR_VERSION) -p

LDIRT_CLEAN		=	$(MY_TAD)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build : $(MY_TAD)

$(MY_TAD): $(MY_DATA)
	$(MAKETAD) $(call empath,$<) $(MAKETAD_FLAGS) -o $@

#===== End of Makefile =====
