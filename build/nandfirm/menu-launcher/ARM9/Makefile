#! make -f
#----------------------------------------------------------------------------
# Project:  TwlFirm - nandfirm - menu-launcher
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

TWL_PROC       = ARM9

SUBDIRS		=

LINCLUDES	=

#----------------------------------------------------------------------------

TARGET_BIN	=	menu_launcher9.srl

REVISION_SRC	=	revision.c

SRCS		=	main.c $(REVISION_SRC)

CRT0_O		=	crt0_firm.o

ADDRESS_STATIC	=	0x037c0000

MAKEROM_ARM7		=	../ARM7/bin/$(TWL_BUILDTYPE_ARM7)/menu_launcher7.tef
MAKEROM_ARM7_BASE       =       $(basename $(MAKEROM_ARM7))

#LCFILE_TEMPLATE =	$(FIRM_SPECDIR)/$(TWL_PROC)-$(TWL_PLATFORM)-PARTNER.lcf.template

#SRCDIR		=	# using default
#LCFILE		=	# using default


include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

# no thumb version, no final rom version
LLIBRARIES	+=	libes.TWL$(ARCHGEN_TYPE).a libboc.TWL$(ARCHGEN_TYPE).a \
				libsfs$(TWL_LIBSUFFIX).a	\
				liblcfg$(TWL_LIBSUFFIX).a

LDIRT_CLEAN		=	$(REVISION_SRC)

#----------------------------------------------------------------------------

do-build:		$(TARGETS)


include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#SVNƒŠƒrƒWƒ‡ƒ“–„‚ßž‚Ý
$(REVISION_SRC) ::
	@if test -e $(TWL_IPL_RED_ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(TWL_IPL_RED_ROOT)) | grep 'Revision' | sed "s|Revision: \(.*\)|const char *g_strIPLSvnRevision = \"\1\";|" > $(REVISION_SRC); \
	else \
		echo "const char *g_strIPLSvnRevision = \"-\";" > $(REVISION_SRC); \
	fi
	@if test -e $(ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(ROOT)) | grep 'Revision' | sed "s|Revision: \(.*\)|const char *g_strSDKSvnRevision = \"\1\";|" >> $(REVISION_SRC); \
	else \
		echo "const char *g_strSDKSvnRevision = \"-\";" >> $(REVISION_SRC); \
	fi



#===== End of Makefile =====
