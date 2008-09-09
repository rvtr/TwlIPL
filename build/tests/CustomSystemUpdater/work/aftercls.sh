#!/bin/sh
#! bash -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools
# File:     aftercls.sh
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

AQUA=//10.116.1.5/
CLSED_PATH=${AQUA}/TWL_debug/sysmenu/rom/debug_rom/CLS_processed
DAILY_CLSED_DIR=`date +'%Y%m%d'`_cls_test


if [ $# -lt 1 ];
then
	echo "input target dir name"
	exit
fi

echo "tocsu.sh"
./tocsu.sh $1

echo "toaqua.rb"
./toaqua.rb $1

# AquaÇ÷ÉRÉsÅ[
echo "Copying to Aqua"
cp -r $1_foraqua ${CLSED_PATH}/${DAILY_CLSED_DIR}

