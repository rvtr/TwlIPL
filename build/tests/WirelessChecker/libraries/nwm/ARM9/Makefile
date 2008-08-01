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
#SUBDIRS		 = wm_lib

override TARGET_PLATFORM = TWL

# Codegen for sub processer
TWL_PROC		 =  ARM9
TWL_CODEGEN		?= ALL

MACRO_FLAGS  = -DNWM_WRACK_FIRMWARE

NWM_SRCDIR		=	$(ROOT)/build/libraries/wm/ARM9.TWL/nwm

# HYBRID / LIMITED 両方のライブラリを作成します
TWL_ARCHGEN		?= ALL

# LoadDeviceEx で転送するファームウェアのディレクトリ
TWLWIRELESS_FW_DIR = $(ROOT)/add-ins/TwlWireless/$(TWLWIRELESS_PACKAGE)/binfile

SRCDIR			 = ./src ../../common.TWL/src
INCDIR			 = $(NWM_SRCDIR)/inc $(NWM_SRCDIR)/../../inc $(NWM_SRCDIR)/../../ARM9/inc	\
				   $(NWM_SRCDIR)/../../common.TWL/inc $(NWM_SRCDIR)/../common/inc

SRCS_ARM9		 = 	nwm_init.c			\
					nwm_system.c		\
					nwm_end.c			\
					nwm_cmd.c			\
					nwm_passphrase.c	

SRCS_COMMON		 =	nwm_loaddev.c		\
					nwm_unloaddev.c		\
					nwm_open.c			\
					nwm_close.c			\
					nwm_startscan.c		\
					nwm_connect.c		\
					nwm_disconnect.c	\
					nwm_setwepkey.c		\
					nwm_setpowersave.c	\
					nwm_sendframe.c		\
					nwm_recvframe.c		\
					nwm_wpa.c			\
					nwm_qos.c			\
					nwm_reset.c			

SRCS			= $(addprefix $(NWM_SRCDIR)/src/, $(SRCS_ARM9))						\
				  $(addprefix $(NWM_SRCDIR)/../../common.TWL/src/, $(SRCS_COMMON))	\
				  ../src/nwm_installfirm_autogen.c

EXT_OBJS         = $(OBJDIR)/nwm_firm.o

TARGET_LIB		 = libnwm.WCKR$(TWL_LIBSUFFIX).a

include	$(TWLSDK_ROOT)/build/buildtools/commondefs

#INSTALL_TARGETS	 = $(TARGETS)

#INSTALL_DIR		 = $(TWL_INSTALL_LIBDIR)

GDIRT_INSTALLED	 = $(addprefix $(INSTALL_DIR)/,$(notdir $(INSTALL_TARGETS)))

#----------------------------------------------------------------------------
# 新無線用のデバッグフラグなど

ifdef	NWM_DEBUG
CCFLAGS +=	-DNWM_DEBUG
endif

CCFLAGS += -DENABLE_RXACK

#----------------------------------------------------------------------------

do-build:		$(TARGETS)

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

$(OBJDIR)/nwm_firm.o:	 $(TWLWIRELESS_FW_DIR)/nwm_firm.bin
			$(BIN2OBJ) $(call empath, $<) $@

#===== End of Makefile =====
