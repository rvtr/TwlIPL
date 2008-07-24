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

# Main memory にコードを退避させたathdrvを使用。(for test)
TMP_WL_MAIN ?= FALSE

TARGET_FIRM					=	SYSTEMMENU
override TARGET_PLATFORM	=	TWL
override TWL_PROC			=	ARM7
override TWL_ARCHGEN		=	LIMITED
TWL_NO_STD_PCHDR			=	True
TWL_CODEGEN					?=	ARM

FIRM_LIBSUFFIX	=	.firm$(ARCHGEN_TYPE)$(CODEGEN_ARCH)

#----------------------------------------------------------------------------

SUBDIRS			=	$(SYSMENU_ROOT)/build/libraries/aes \
					wram_regs

SRCS			=	crt0.LTD.c \
					main.c \
					initScfg.c

TARGET_NAME		=	hyena

TARGET_NEF		=	$(TARGET_NAME).tef

ifeq ($(TMP_WL_MAIN), TRUE)
LCFILE_SPEC		=	$(TARGET_NAME)_wlmain.lsf
else
LCFILE_SPEC		=	$(TARGET_NAME).lsf
endif

LCFILE_TEMPLATE	=	$(TARGET_NAME).lcf.template
LDRES_TEMPLATE	=	$(ROOT)/build/components/racoon.TWL/racoon.response.template

# crt0.oは、ランチャー向けに特殊ビルドしたものを使用
CRT0_O			=	$(OBJDIR)/crt0.LTD.o

# スタック不足防止の為、インライン展開せずにコンパイルする
CCFLAGS_OPT		=	-O4 -inline off

#----------------------------------------------------------------------------
include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

# crt0.o 、 libpm_sp および libnwm_sp は、ローカルでビルドしたものを使用するので、除外する。
ifeq ($(TMP_WL_MAIN), TRUE)
GLIBRARIES		:= $(filter-out $(CRT0_O) libpm_sp$(TWL_LIBSUFFIX).a libnwm_sp$(TWL_LIBSUFFIX).a libathdrv_sp$(TWL_LIBSUFFIX).a,$(GLIBRARIES))
else
GLIBRARIES		:= $(filter-out $(CRT0_O) libpm_sp$(TWL_LIBSUFFIX).a libnwm_sp$(TWL_LIBSUFFIX).a,$(GLIBRARIES))
endif

ifdef TWLSDK_NOCRYPTO
CCFLAGS			+=	-DSDK_NOCRYPTO
endif

ifndef TWLSDK_NOCRYPTO
MAKELCF_FLAGS	+=	-DUSE_CRYPTO_LIBS='libsea_sp$(TWL_LIBSUFFIX).a libcrypto_sp$(TWL_LIBSUFFIX).a'
MACRO_FLAGS		+=	-DSDK_SEA
endif

MACRO_FLAGS		+=	-DSDK_NOINIT \
					-DSDK_SYSMENU_BUILD
#MACRO_FLAGS		+=	-DSDK_ARM7COMP_LTD

ifeq ($(DEFAULT_COMP_ARM7),hyena)
MACRO_FLAGS		+=	-DUSE_HYENA_COMPONENT
endif

MAKELCF_FLAGS	+=	-DADDRESS_LTDWRAM='0x037a0000' \
					-DADDRESS_FLXMAIN='0x02280200' \
					-DADDRESS_BOOTCORE='0x0380f000' \
					-DCRT0_O='$(CRT0_O)' \
					-DFIRM_LIBSUFFIX='$(FIRM_LIBSUFFIX)'

MAKELCF_FLAGS	+=	-DISDBG_LIBS_TWL='$(if $(ISDBG_LIBS_TWL),$(ISDBG_LIBS_TWL),libstubsistd_sp$(TWL_LIBSUFFIX).a)'	\
					-DISDBG_LIBS_NITRO='libstubsisd_sp$(TWL_LIBSUFFIX).a'


#--------------------------------
#   install target
#--------------------------------
INSTALL_TARGETS	=	$(BINDIR)/$(TARGET_NAME).tef				\
					$(BINDIR)/$(TARGET_NAME).TWL.FLX.sbin		\
					$(BINDIR)/$(TARGET_NAME)_defs.TWL.FLX.sbin	\
					$(BINDIR)/$(TARGET_NAME).TWL.LTD.sbin		\
					$(BINDIR)/$(TARGET_NAME)_defs.TWL.LTD.sbin

INSTALL_DIR		=	$(TWL_INSTALL_COMPONENTSDIR)/$(TARGET_NAME)/$(TWL_BUILDTYPE)

LINCLUDES		= 	$(ROOT)/build/libraries/spi/ARM7/include \
					$(ROOT)/build/libraries/spi/ARM7/pm/include \
					$(ROOT)/build/libraries/init/common/include \
					$(ROOT)/build/libraries/reboot/common/include \
					$(SYSMENU_ROOT)/build/libraries_sysmenu/sysmenu/common/include

SYSMENU_LIBS	=	\
					libsysmenu_sp$(TWL_LIBSUFFIX).a \
					libsysmmcu_sp$(TWL_LIBSUFFIX).a \
					libboot_sp$(TWL_LIBSUFFIX).a \
					libds_sp$(TWL_LIBSUFFIX).a \
					libhotsw_sp$(TWL_LIBSUFFIX).a \
					libdht_sp$(TWL_LIBSUFFIX).a \
					libpm_sp.SYSMENU$(TWL_LIBSUFFIX).a

ifeq ($(TMP_WL_MAIN), TRUE)
SYSMENU_LIBS	+=	\
					libathdrv_sp.SYSMENU$(TWL_LIBSUFFIX).a 
endif

SDK_APPEND_LIBS	=	\
					libwl_sp$(TWL_LIBSUFFIX).a \
					libaes_sp$(FIRM_LIBSUFFIX).a \
					libsea_sp$(TWL_LIBSUFFIX).a \
					libreboot_sp$(TWL_LIBSUFFIX).a

LLIBRARIES		+=	$(SYSMENU_LIBS) $(SDK_APPEND_LIBS)

LLIBRARY_DIRS	+=	./

ifeq ($(TMP_WL_MAIN), TRUE)
LLIBRARY_DIRS	+=	$(SYSMENU_ROOT)/lib/TwlWireless/$(TWL_LIBTYPE)
endif

LDEPENDS_NEF	= $(TWL_LIBS) $(LLIBRARIES)


do-build:	$(TARGETS)

#----------------------------------------------------------------------------
include	$(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
