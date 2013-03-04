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

#DO_MASTERING			=	TRUE

TARGET_FIRM				=	SYSTEMMENU

TARGET_PLATFORM			=	TWL
TWL_ARCHGEN				=	LIMITED
TWL_NANDAPP				=	TRUE

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include $(TWL_IPL_RED_ROOT)/build/systemMenu_RED/sysmenuVersion/commondefs.sysmenuVersion

ifeq ($(TWL_IPL_RED_PRIVATE_ROOT),)
override DO_MASTERING	=
endif

ifeq ($(DO_MASTERING),TRUE)
OUT_DIR			=	prod
MASTER			=	.master
MAKETAD_OPTION	+=	-p
else
OUT_DIR			=	dev
MASTER			=	
endif

NAND_INITIALIZER_NAME	=	NandInitializerProduction
SYSMENU_DIR		=	../systemMenu_RED
SRL_DIR			=	srl
NEWDIRS			+=	$(OUT_DIR)/$(SRL_DIR)

TARGETS			=	$(OUT_DIR)/HNAA \
					$(OUT_DIR)/HNBA \
					$(OUT_DIR)/HNCA \
					$(OUT_DIR)/HNHA \
					$(OUT_DIR)/HNLA \
					$(OUT_DIR)/HNLJ \
					$(OUT_DIR)/HNLE \
					$(OUT_DIR)/HNLP \
					$(OUT_DIR)/HNLU \
					$(OUT_DIR)/TWLFontTable.dat \
					$(OUT_DIR)/menu_launcher-$(TWL_BUILD_TYPE).nand \
					$(OUT_DIR)/$(NAND_INITIALIZER_NAME)-$(TWL_BUILD_TYPE)$(MASTER).srl

MAKEROM_FLAGS	+=	-p

#----------------------------------------------------------------------------

LDIRT_CLEAN		=	$(TARGETS) \
					$(OUT_DIR)

#----------------------------------------------------------------------------

include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

do-build :	$(TARGETS)

#MASTERING =	$(TWL_IPL_RED_PRIVATE_ROOT)/tools/bin/mastering.TWL.exe
MASTERING =	$(TWL_IPL_RED_PRIVATE_ROOT)/build/tools/mastering.TWL/mastering.TWL.exe

MASTERING_FLAGS	=	-t

#---------------------------
#ランチャー
#---------------------------
$(OUT_DIR)/HNAA : $(SYSMENU_DIR)/Launcher/ARM9/bin/$(TWL_BUILDTYPE)/HNAA.nand.srl
ifneq ($(DO_MASTERING),)
	$(MASTERING) $(MASTERING_FLAGS) $< $(OUT_DIR)/$(SRL_DIR)/$(notdir $<)
else
	cp $< $(OUT_DIR)/$(SRL_DIR)
endif
ifeq ($(DO_MASTERING),TRUE)
	$(SYSMENU_TOOLSDIR)/bin/makeTitleIdDir.plx $< $(OUT_DIR) | \
	xargs -iarg $(MAKETAD) -s $(OUT_DIR)/$(SRL_DIR)/$(notdir $<) $(MAKETAD_FLAGS) -o arg/$(notdir $(basename $<))-$(TWL_BUILD_TYPE)$(MASTER).tad $(MAKETAD_OPTION)
	ls $@ | xargs -iarg mv $@/arg/properties $@
else
	$(MAKETAD) -s $(OUT_DIR)/$(SRL_DIR)/$(notdir $<) $(MAKETAD_FLAGS) -o $(OUT_DIR)/$(notdir $(basename $<))-$(TWL_BUILD_TYPE)$(MASTER).tad $(MAKETAD_OPTION)
endif


#---------------------------
#本体設定
#---------------------------
$(OUT_DIR)/HNBA : $(SYSMENU_DIR)/MachineSettings/ARM9/bin/$(TWL_BUILDTYPE)/HNBA.nand.srl
ifneq ($(DO_MASTERING),)
	$(MASTERING) $(MASTERING_FLAGS) $< $(OUT_DIR)/$(SRL_DIR)/$(notdir $<)
else
	cp $< $(OUT_DIR)/$(SRL_DIR)
endif
ifeq ($(DO_MASTERING),TRUE)
	$(SYSMENU_TOOLSDIR)/bin/makeTitleIdDir.plx $< $(OUT_DIR) | \
	xargs -iarg $(MAKETAD) -s $(OUT_DIR)/$(SRL_DIR)/$(notdir $<) $(MAKETAD_FLAGS) -o arg/$(notdir $(basename $<))-$(TWL_BUILD_TYPE)$(MASTER).tad $(MAKETAD_OPTION)
	ls $@ | xargs -iarg mv $@/arg/properties $@
else
	$(MAKETAD) -s $(OUT_DIR)/$(SRL_DIR)/$(notdir $<) $(MAKETAD_FLAGS) -o $(OUT_DIR)/$(notdir $(basename $<))-$(TWL_BUILD_TYPE)$(MASTER).tad $(MAKETAD_OPTION)
endif


#---------------------------
#無線ファーム
#---------------------------
# [TODO] 無線ファームからバージョンを取得して、バージョン番号でディレクトリを作成する。
$(OUT_DIR)/HNCA : $(call eupath,$(shell find $(SYSMENU_DIR)/wlanfirm -name 'HNCA*.tad' -type f))
ifeq ($(DO_MASTERING),TRUE)
	mkdir -p $(OUT_DIR)/$(notdir $(basename $@))
	mkdir -p $(OUT_DIR)/$(notdir $(basename $@))/V0
	cp $< ./$@/V0/
	cp $(dir $<)/properties ./$@
else
	cp $< $(OUT_DIR)
endif

#---------------------------
#ホワイトリスト
#---------------------------
# [TODO] TADからバージョンを取得して、バージョン番号でディレクトリを作成する。
$(OUT_DIR)/HNHA : $(call eupath,$(shell find $(SYSMENU_DIR)/DSHashTable/ -name 'HNHA*.tad' -type f))
ifeq ($(DO_MASTERING),TRUE)
	mkdir -p $(OUT_DIR)/$(notdir $(basename $@))
	mkdir -p $(OUT_DIR)/$(notdir $(basename $@))/V0
	cp $< ./$@/V0/
	cp $(dir $<)/properties ./$@
else
	cp $< $(OUT_DIR)
endif

#---------------------------
#システムメニューバージョン
#---------------------------
# [TODO] TADからバージョンを取得して、バージョン番号でディレクトリを作成する。
# FIRM_USE_PRODUCT_KEYS = TRUE の時のみPROD用のシステムメニューバージョンを取得します。
$(OUT_DIR)/HNLA : $(call eupath,$(shell find $(SYSMENU_DIR)/sysmenuVersion/$(OUT_DIR).out/HNLA -name 'HNL*.tad' -type f))
ifeq ($(DO_MASTERING),TRUE)
	mkdir -p $@
	mkdir -p $@/V0
	cp $< ./$@/V0/
	cp $(dir $<)/properties ./$@
else
	cp $< $(OUT_DIR)
endif

$(OUT_DIR)/HNLJ : $(call eupath,$(shell find $(SYSMENU_DIR)/sysmenuVersion/$(OUT_DIR).out/HNLJ -name 'HNL*.tad' -type f))
ifeq ($(DO_MASTERING),TRUE)
	mkdir -p $@
	mkdir -p $@/V0
	cp $< ./$@/V0/
	cp $(dir $<)/properties ./$@
else
	cp $< $(OUT_DIR)
endif

$(OUT_DIR)/HNLE : $(call eupath,$(shell find $(SYSMENU_DIR)/sysmenuVersion/$(OUT_DIR).out/HNLE -name 'HNL*.tad' -type f))
ifeq ($(DO_MASTERING),TRUE)
	mkdir -p $@
	mkdir -p $@/V0
	cp $< ./$@/V0/
	cp $(dir $<)/properties ./$@
else
	cp $< $(OUT_DIR)
endif

$(OUT_DIR)/HNLP : $(call eupath,$(shell find $(SYSMENU_DIR)/sysmenuVersion/$(OUT_DIR).out/HNLP -name 'HNL*.tad' -type f))
ifeq ($(DO_MASTERING),TRUE)
	mkdir -p $@
	mkdir -p $@/V0
	cp $< ./$@/V0/
	cp $(dir $<)/properties ./$@
else
	cp $< $(OUT_DIR)
endif

$(OUT_DIR)/HNLU : $(call eupath,$(shell find $(SYSMENU_DIR)/sysmenuVersion/$(OUT_DIR).out/HNLU -name 'HNL*.tad' -type f))
ifeq ($(DO_MASTERING),TRUE)
	mkdir -p $@
	mkdir -p $@/V0
	cp $< ./$@/V0/
	cp $(dir $<)/properties ./$@
else
	cp $< $(OUT_DIR)
endif

#---------------------------
#共有フォント
#---------------------------
$(OUT_DIR)/TWLFontTable.dat: $(call eupath,$(shell find $(SYSMENU_DIR)/sharedFont -name 'TWLFontTable_*.dat' -type f))
	cp $< ./$(OUT_DIR)/


#---------------------------
#NANDファーム
#---------------------------
$(OUT_DIR)/menu_launcher-$(TWL_BUILD_TYPE).nand: $(call eupath,$(shell find ../nandfirm/menu-launcher -name 'menu_launcher-$(TWL_BUILD_TYPE)*.nand' -type f))
	cp $< ./$(OUT_DIR)/


#---------------------------
#NandInitializerProduction
#---------------------------
$(OUT_DIR)/$(NAND_INITIALIZER_NAME)-$(TWL_BUILD_TYPE)$(MASTER).srl: ../systemMenu_tools/$(NAND_INITIALIZER_NAME)/ARM9.TWL/bin/$(TWL_BUILDTYPE)/$(NAND_INITIALIZER_NAME).srl
ifneq ($(DO_MASTERING),)
	$(MASTERING) -s $(MASTERING_FLAGS) $< $@
else
	cp $< $@
endif


#===== End of Makefile =====
