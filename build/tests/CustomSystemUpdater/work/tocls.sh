#!/bin/sh
#! bash -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools
# File:     conv2svnDir.sh
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


if [ $# -lt 1 ];
then
	echo "input target dir name"
	exit
fi

target=$1_forcls
mkdir $target
cd $target
find ../$1 -name "*.tad" -exec ../mkcls.py {} \; 
cp ../cls.sh ./

# nandファームと、フォントを一応リージョン毎にコピー
mkdir jp
mkdir aus
mkdir usa
mkdir euro

cp `find ../$1/jp -name "*.nand"` jp
cp `find ../$1/jp -name "*.nand"` aus
cp `find ../$1/jp -name "*.nand"` usa
cp `find ../$1/jp -name "*.nand"` euro
cp `find ../$1/jp -name "*.dat"` jp
cp `find ../$1/jp -name "*.dat"` aus
cp `find ../$1/jp -name "*.dat"` usa
cp `find ../$1/jp -name "*.dat"` euro


