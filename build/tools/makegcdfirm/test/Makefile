#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlFirm - tools - makenorfirm
#  File:     Makefile
#
#  Copyright 2007 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
# $Date::            $
# $Rev:$
# $Author:$
#---------------------------------------------------------------------------

SUBDIRS =	wram_rbin \

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

MAKEGCDFIRM		=	../makegcdfirm.exe

MAKEFIRM_ARM9		=	./twl_gcdfirm9_print.axf
MAKEFIRM_ARM7		=	./twl_gcdfirm7_print.axf
SDEPENDS_BIN		+=	$(MAKEFIRM_ARM9) $(MAKEFIRM_ARM7)
MAKEFIRM_FLAGS	+=	-d
MAKEFIRM_DEFS	+= -DFIRM_ROOT='$(FIRM_ROOT)' \
				-DMAKEFIRM_ARM9='$(basename $(MAKEFIRM_ARM9))' \
				-DMAKEFIRM_ARM7='$(basename $(MAKEFIRM_ARM7))' \
				-DMAKEFIRM_RSA_PRVKEY='$(MAKEFIRM_RSA_PRVKEY)' \

TARGET		=	test.srl

%.srl:			%.gcdsf $(MAKEGCDFIRM)
			$(MAKEGCDFIRM) $(MAKEFIRM_FLAGS) $(MAKEFIRM_DEFS) $< $@

.PHONY:			build install do-autotest clean clobber

define ECHO_CURDIR
			echo "==== $(CURDIR)";
endef

build:			
			@$(ECHO_CURDIR)
			@$(MAKE) $(TARGET)

install do-autotest:
			@$(ECHO_CURDIR)

clean clobber super-clobber:
			@$(ECHO_CURDIR)
			-rm -f $(TARGET) *~

test-utf16.bsf:		icon.nbfc icon.nbfp
