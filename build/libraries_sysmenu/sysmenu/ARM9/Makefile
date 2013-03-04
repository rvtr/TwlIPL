#! make -f
#----------------------------------------------------------------------------
# Project:  TwlIPL
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

SUBDIRS			=	

#----------------------------------------------------------------------------
TARGET_FIRM		=	SYSTEMMENU
TARGET_PLATFORM	=	TWL
TWL_ARCHGEN		=	LIMITED
TWL_PROC	   	=	ARM9

REVISION_SRC	=	src/revision.c

SRCDIR			=	src
SRCS			=	sysmenu_lib.c \
					ninLogoFunc.c \
					device.c \
					keys.c \
					title.c \
					fs_wram.c \
					../common/src/status.c \
					../common/src/pxi.c \
					../common/src/mountInfo.c \
					../common/src/decryptAES.c \
					$(REVISION_SRC)


TARGET_LIB		=	libsysmenu$(TWL_LIBSUFFIX).a

ifeq ($(SYSM_DO_NOT_SHOW_LAUNCHER),TRUE)
MACRO_FLAGS		+=	-DSYSM_DO_NOT_SHOW_LAUNCHER
endif

ifeq ($(SYSM_DISABLE_INITIAL_SETTINGS),TRUE)
MACRO_FLAGS		+=	-DSYSM_DISABLE_INITIAL_SETTINGS
endif

ifeq ($(DEFAULT_COMP_ARM7),hyena)
MACRO_FLAGS		+=	-DUSE_HYENA_COMPONENT
endif

ifeq ($(SYSM_BUILD_FOR_PRODUCTION_TEST),TRUE)
MACRO_FLAGS		+=	-DSYSM_BUILD_FOR_PRODUCTION_TEST
endif

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

ifeq ($(SYSM_DEV_WHITELIST_CHECK_SKIP),TRUE)
MACRO_FLAGS 	+=	-DDEV_WHITELIST_CHECK_SKIP
endif

INSTALL_TARGETS	=	$(TARGETS)
INSTALL_DIR		=	$(SYSMENU_INSTALL_LIBDIR)

LINCLUDES		=	$(ROOT)/build/libraries/mb/common/include	\
					$(ROOT)/build/libraries/spi/ARM9/include	\
					$(SYSMENU_ROOT)/build/libraries_sysmenu/sysmenu/common/include

LDIRT_CLEAN		=	$(REVISION_SRC)

#----------------------------------------------------------------------------

do-build: $(REVISION_SRC) $(TARGETS)

include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules


#SVNƒŠƒrƒWƒ‡ƒ“–„‚ßž‚Ý
$(REVISION_SRC) ::
	@if test -e $(SYSMENU_ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(SYSMENU_ROOT)) | grep 'Revision' | sed "s|Revision: \(.*\)|const char *g_strIPLSvnRevision = \"\1\";|" > $(REVISION_SRC); \
	else \
		echo "const char *g_strIPLSvnRevision = \"-\";" > $(REVISION_SRC); \
	fi
	@if test -e $(ROOT)/.svn; then \
		LANG=C svn info $(call empath,$(ROOT)) | grep 'Revision' | sed "s|Revision: \(.*\)|const char *g_strSDKSvnRevision = \"\1\";|" >> $(REVISION_SRC); \
	else \
		echo "const char *g_strSDKSvnRevision = \"-\";" >> $(REVISION_SRC); \
	fi


#===== End of Makefile =====
