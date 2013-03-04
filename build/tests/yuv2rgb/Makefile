#! make -f
#---------------------------------------------------------------------------
#  Project:  TwlSDK - YUV2RGB
#  File:     Makefile
#
#  Copyright 2008 Nintendo.  All rights reserved.
#
#  These coded instructions, statements, and computer programs contain
#  proprietary information of Nintendo of America Inc. and/or Nintendo
#  Company Ltd., and are protected by Federal copyright law.  They may
#  not be disclosed to third parties or copied or duplicated in any form,
#  in whole or in part, without the prior written consent of Nintendo.
#
#  $Date::            $
#  $Rev$
#  $Author$
#---------------------------------------------------------------------------

ifdef TWL_IPL_RED_ROOT
#コンポーネントを切り替えます。
#コメントアウトするとracoonを、有効化するとjackalを使います。
#下の行を変更したらmake cleanしてください。
TWL_IPL_USE_RED_IPL			:= TRUE
endif

ifdef TWL_IPL_USE_RED_IPL
TARGET_FIRM			=	SYSTEMMENU
REBOOT_LIB			=	libreboot$(TWL_LIBSUFFIX).a
LDEPENDS_NEF			+=	$(REBOOT_LIB)
ADDRESS_DTCM			=	0x0e000000
DEFAULT_COMP_ARM7		=	jackal
MAKEROM_FLAGS			+=	-F
LCFILE_SPEC			=	$(SYSMENU_ROOT)/build/systemMenu_RED/Launcher/ARM9/ARM9-TS.lsf
LCFILE_TEMPLATE			=	$(SYSMENU_ROOT)/build/systemMenu_RED/Launcher/ARM9/ARM9-TS.lcf.template
MACRO_FLAGS += -DTWL_IPL_USE_RED_IPL
MAKELCF_FLAGS			+=	-DREBOOT_LIB='$(REBOOT_LIB)'
endif


TARGET_PLATFORM    := TWL
TWL_PROC           := ARM9
TWL_ARCHGEN        := LIMITED

TARGET_BIN          = main.srl
ROM_SPEC_TEMPLATE   = yuv2rgb.rsf.template
ROM_SPEC            = yuv2rgb.autogen.rsf
SRCS                = snd_data.c mainy.cpp new.cpp cpu_yuv2rgb.c menuIrqHandler.c menuCameraManager.cpp #dsp_yuv2rgb.c 
DATA_DIR            = ./data
EXT_OBJS           += $(patsubst $(DATA_DIR)/%.dat,$(OBJDIR)/%.o,$(wildcard $(DATA_DIR)/*.dat))

# DSPファームウェアを定数として静的リンクする。
#DSP_FIRMWARE_DIR    = $(ROOT)/build/dspcomponents/build/tests/yuv2rgb/bin
#DATA_DIR            = .
#EXT_OBJS           += $(patsubst $(DSP_FIRMWARE_DIR)/%.a,$(OBJDIR)/%.o,$(wildcard $(DSP_FIRMWARE_DIR)/*.a))

MACRO_FLAGS += -DTWL_IPL_MENU_CAMERA_MANAGER_FOR_LOCAL_PROJECT

#----------------------------------------------------------------------------

ifdef TWL_IPL_USE_RED_IPL
include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include	$(TWLSDK_ROOT)/build/libraries/reboot/commondefs.reboot
MAKEROM := $(TWL_IPL_RED_ROOT)/tools/bin/makerom.TWL.launcher.exe
else
include $(TWLSDK_ROOT)/build/buildtools/commondefs
endif
include	$(TWLSDK_ROOT)/build/buildtools/commondefs.gx.demolib


LLIBRARIES  +=   libcpu_jpegEncoder$(TWL_LIBSUFFIX).a


#----------------------------------------------------------------------------

#do-build:		$(ROM_SPEC) prepare $(TARGETS)
do-build:		$(ROM_SPEC) $(TARGETS)

#SUBDIRS		=	../../../dspcomponents/build/tests/yuv2rgb

include	$(TWLSDK_ROOT)/build/buildtools/modulerules

# UIGランチャーのソースをそのままコピーします。
.PHONY: prepare
prepare:
	cp -p $(TWL_IPL_ROOT)/menu/scene/src/menuCameraManager.cpp src
	cp -p $(TWL_IPL_ROOT)/menu/scene/include/menuCameraManager.h include

ifdef TWL_IPL_USE_RED_IPL
# for $(SYSMENU_LIBS)
include redipl_defs_autogen_by_uig
LLIBRARIES		+=	$(SYSMENU_LIBS)

$(ROM_SPEC): $(ROM_SPEC_TEMPLATE) Makefile
	grep -v MAKEROM_WRAM_MAPPING $(ROM_SPEC_TEMPLATE) | grep -v NTRJ | grep -v User > $(ROM_SPEC)

else
$(ROM_SPEC): $(ROM_SPEC_TEMPLATE) Makefile
	grep -v WramMappingDirect $(ROM_SPEC_TEMPLATE) | grep -v HNAJ | grep -v Secure | grep -v System | grep -v HwAESSlot > $(ROM_SPEC)

endif

LDIRT_CLEAN += $(ROM_SPEC)

#----------------------------------------------------------------------------

$(OBJDIR)/%.o : $(DATA_DIR)/%.dat
	$(BIN2OBJ) --compatible $< $@ >/dev/null

$(DATA_DIR)/%.dat : $(call eupath,$(DSP_FIRMWARE_DIR)/%.a)
	cp -T $< $@


ZIPDIR=$(notdir $(shell pwd))
ZIPFILE=$(HOME)/$(ZIPDIR)-$(shell date +%y%m%d_%H%M).zip
.PHONY: zip
zip:
	@cd .. && find $(ZIPDIR) ! -type d | grep -v -e \~ -e .tmp -e .svn -e .obj -e /bin/ -e /depend/ -e /obj/ | zip $(ZIPFILE) -@
	@ls -l $(ZIPFILE)


#===== End of Makefile =====
