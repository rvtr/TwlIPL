#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlFirm - tools - makegcdfirm
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

TARGET_PLATFORM		=	TWL

SUPPORT_ECC			=	0

ifeq ($(SUPPORT_ECC),1)
ECC_SRCDIR			=	../../libraries/acsign_ecc/common \
						../../libraries/acsign_ecc/common/algae/common/ecc \
						../../libraries/acsign_ecc/common/algae/cmp \
						../../libraries/acsign_ecc/common/algae/ecsource \

ECC_INCDIR			=	../../libraries/acsign_ecc/include \
						../../libraries/acsign_ecc/common/algae/include \
						../../libraries/acsign_ecc/common/algae/common/include \

ECC_SRCS			=	acsign_ecc.c acsign_cryptoc.c \
						\
						cmparith.c cmpbits.c cmpcnv.c cmpdiv.c cmpmem.c \
						cmpmod.c cmpmuldv.c cmpspprt.c cmpsqr.c cmpvectr.c \
						computem.c ecfpatbl.c ecfpsmul.c \
						spcprime.c secfpcom.c \
						\
						p224v1.c p224v1a.c \

ECC_DEFS 			=	-DRSA_PROTOTYPES=RSA_ENABLED \
						-DRCOM_BUILD=RSA_ENABLED -DRSA_FAST_INVERSE=RSA_ENABLED \
						-DRSA_STD_MEM_FUNCS=RSA_ENABLED -DRSA_STD_ALLOC_FUNCS=RSA_ENABLED \
else
ECC_SRCDIR			=
ECC_INCDIR			=
ECC_SRCS			=
ECC_DEFS			=
endif

SRCDIR				+=	../acsign $(ECC_SRCDIR)
INCDIR				+=	../acsign/include $(ECC_INCDIR) $(ECC_SRCDIR)


include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

TARGETS				=	makegcdfirm.exe

SOURCES_C			=	makegcdfirm.c \
						out_gcdfirm.c \
						misc.c \
						path.c \
						defval.c \
						compress.c \
						wram_regs.c \
						acsign.c \
						acsign_gcd.c \
						aes2.c \
						$(ECC_SRCS)

SOURCES				=	$(SORUCES_C)

OBJECTS				=	$(SOURCES_C:.c=.o) 

HEADERS				=	format_nlist.h \
						makegcdfirm.h \
						path.h \
						format_rom.h \
						misc.h \
						defval.h \
						compress.h \

MACROS				+=	-DSMALL_CODE_SIZE \
						-DSTANDALONE \
						-DOPT_32_BIT \
						-DNO_SPLIT \
						-DNO_FP_API \
						-DNO_R_DIAG \
						$(ECC_DEFS)

INSTALL_DIR			=	$(FIRM_INSTALL_TOOLSDIR)/bin
INSTALL_TARGETS		=	$(TARGETS)

LDIRT_CLEAN			=	$(OBJECTS) $(TARGETS) version.h


VPATH				=	$(SRCDIR)
TWL_INCDIR			:=	$(TWL_INCDIR) $(addprefix -I,$(INCDIR)) $(addprefix -I,$(FIRM_INCDIR))

include	$(TWLSDK_ROOT)/build/buildtools/twl/modulerules.x86

#----------------------------------------------------------------------------
#  build
#----------------------------------------------------------------------------
do-build:		$(TARGETS)

$(TARGETS):		$(OBJECTS)
			$(CC_X86) $+ -o $@

makegcdfirm.o:	makegcdfirm.c makegcdfirm.h format_rom.h path.h version.h
out_gcdfirm.o:	out_gcdfirm.c misc.h format_rom.h format_nlist.h format_sign.h elf.h compress.h \
				$(FIRM_INCDIR)/firm/format/sign.h \
				$(FIRM_INCDIR)/firm/format/wram_regs.h \
				$(FIRM_INCDIR)/firm/format/gcdfirm.h \

misc.o:			misc.c misc.h
path.o:			path.c path.h
compress.o:		compress.c compress.h
wram_regs.o:	wram_regs.c
acsign.o:		acsign.c ../acsign/include/acsign.h
acsign_gcd.o:	acsign_gcd.c format_sign.h \
				$(FIRM_INCDIR)/firm/format/sign.h \
				$(FIRM_INCDIR)/firm/format/wram_regs.h \
				$(FIRM_INCDIR)/firm/format/gcdfirm.h \

aes2.o:			aes2.c aes2.h

$(FIRM_INCDIR)/firm/format/sign.h:
$(FIRM_INCDIR)/firm/format/wram_regs.h:
$(FIRM_INCDIR)/firm/format/gcdfirm.h:
format_nlist.h:
format_rom.h:
makegcdfirm.h:
acsign.h:
acsign_gcd.h:
path.h:

#  avoid to warning message
misc.o:WARNING		+= -Wno-format-y2k

#

version.h:		$(SOURCES) $(HEADERS)
			@for i in $^ ;							\
			do								\
				date -r $$i +'#define SDK_DATE_OF_LATEST_FILE %Y%m%dUL';	\
			done | sort | tail -1 > $@

test:			path.c misc.c
			$(CC_X86) -DTEST $+ -o $@
