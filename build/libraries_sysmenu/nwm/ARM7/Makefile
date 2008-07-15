#! make -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - WM - libraries
# File:     Makefile
#
# Copyright 2003 Nintendo.  All rights reserved.
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

TARGET_FIRM			=	SYSTEMMENU

SUBDIRS			=

#----------------------------------------------------------------------------
override TARGET_PLATFORM = TWL

# Codegen for sub processer
TWL_PROC		 = ARM7

# build ARM & THUMB libraries
TWL_CODEGEN		?= ALL

MACRO_FLAGS  = -DSDK_SYSMENU_BUILD

NWM_SRCDIR		=	$(ROOT)/build/libraries/wm/ARM7.TWL/nwm

#----------------------------------------------------------------------------

# INCDIR 注意点：
INCDIR			= $(NWM_SRCDIR)/inc $(NWM_SRCDIR)/../../inc 				\
				  $(NWM_SRCDIR)/../../ARM7/inc $(NWM_SRCDIR)/../../common.TWL/inc \
				  $(NWM_SRCDIR)/../common/inc								\
				  $(NWM_SRCDIR)/../../../wpa/include						\
				  $(TWLWIRELESS_ROOT)/include								\
				  $(TWLWIRELESS_ROOT)/host/include							\
				  $(TWLWIRELESS_ROOT)/host/wlan/include						\
				  $(TWLWIRELESS_ROOT)/host/os/twl/include					\
				  $(TWLWIRELESS_ROOT)/host/os/twl/library/athioctl/include	\
				  $(TWLWIRELESS_ROOT)/host/os/nitro/include					\
				  $(TWLWIRELESS_ROOT)/host/os/nitro/library/athioctl/include	\
				  $(ROOT)/build/libraries/spi/ARM7/include

SRCDIR_			= ../../ARM7/src src ../../common.TWL/src

SRCDIR			= $(addprefix $(NWM_SRCDIR)/, $(SRCDIR_))

SRCS_ARM7		= nwmsp_system.c		\
				  nwmsp_cmd.c			\
				  nwmsp_event.c			\
				  nwmsp_wlan_handler.c	\
				  nwmsp_event_handler.c	\
				  nwmsp_beaconinfo.c	

SRCS_COMMON		= nwm_loaddev.c			\
				  nwm_unloaddev.c		\
				  nwm_open.c			\
				  nwm_close.c			\
				  nwm_startscan.c		\
				  nwm_connect.c			\
				  nwm_disconnect.c		\
				  nwm_sendframe.c		\
				  nwm_recvframe.c		\
				  nwm_setwepkey.c		\
				  nwm_setpowersave.c	\
				  nwm_wpa.c				\
				  nwm_qos.c				\
				  nwm_reset.c			\
				  nwm_installfirm.c		

SRCS			= $(addprefix $(NWM_SRCDIR)/src/, $(SRCS_ARM7))	\
				  $(addprefix $(NWM_SRCDIR)/../../common.TWL/src/, $(SRCS_COMMON))

TARGET_LIB		= libnwm_sp.SYSMENU$(TWL_LIBSUFFIX).a

#----------------------------------------------------------------------------

# DEBUG版ビルドの場合、RELEASE版でビルドして
# DEBUG版のライブラリを装います。

ifdef	TARGET_DEBUG
TWL_BUILD_TYPE	 = RELEASE
else
ifdef	TWL_DEBUG
TWL_BUILD_TYPE	 = RELEASE
CCFLAGS 		+= -DNWM_DEBUG
else
ifdef	NITRO_DEBUG
TWL_BUILD_TYPE	 = RELEASE
endif
endif
endif

#----------------------------------------------------------------------------
# Default では LIMITED のライブラリを作成します
TWL_ARCHGEN		?=	LIMITED

#----------------------------------------------------------------------------

# 新無線用のデバッグフラグなど

ifdef	NWM_DEBUG
CCFLAGS +=	-DNWM_DEBUG
endif

include	$(TWL_IPL_RED_ROOT)/build/buildtools/commondefs

INSTALL_TARGETS		= $(TARGETS)
INSTALL_DIR     	= $(SYSMENU_INSTALL_LIBDIR)
GDIRT_INSTALLED		= $(addprefix $(INSTALL_DIR)/,$(notdir $(INSTALL_TARGETS)))

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====

