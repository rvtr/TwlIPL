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

# バージョンファイルとTADを生成する
# TWL_IPL_RED_PRIVATE_ROOTが設定されていない場合は
# 既存のバージョンファイルからTADのみ生成する

# 現在、製品版の証明書を含んだものは生成しない


override TARGET_PLATFORM := TWL

#SUBMAKES				=	 Makefile.J Makefile.E Makefile.P Makefile.U Makefile.C Makefile.K Makefile.J.kst
SUBMAKES				=	 Makefile.A

include $(TWL_IPL_RED_ROOT)/build/buildtools/commondefs
include $(TWL_IPL_RED_ROOT)/build/buildtools/modulerules

#===== End of Makefile =====
