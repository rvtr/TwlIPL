#!/bin/sh
#! bash -f
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools
# File:     tocsu.sh
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


target=$1_forcsu
tmp=$1_tmp
mkdir $target
cd $target

cp -rf ../$1/jp .
cp -rf ../$1/aus .
cp -rf ../$1/usa .
cp -rf ../$1/euro .

mkdir $tmp

find ../$1 -name "*.out" -exec cp {} $tmp \;

find $tmp -name "*[jaJA]-v0*.out" | gawk -F/ '{printf("cp %s jp/%s\n",$0, $2)}' | sed -e 's/\.out$//g' > jp.sh
find $tmp -name "*[uaUA]-v0*.out" | gawk -F/ '{printf("cp %s aus/%s\n",$0, $2)}' | sed -e 's/\.out$//g' > aus.sh
find $tmp -name "*[paPA]-v0*.out" | gawk -F/ '{printf("cp %s euro/%s\n",$0, $2)}' | sed -e 's/\.out$//g' > euro.sh
find $tmp -name "*[eaEA]-v0*.out" | gawk -F/ '{printf("cp %s usa/%s\n",$0, $2)}' | sed -e 's/\.out$//g' > usa.sh

./jp.sh
./aus.sh
./euro.sh
./usa.sh

mv $tmp ../
rm -rf *.sh
