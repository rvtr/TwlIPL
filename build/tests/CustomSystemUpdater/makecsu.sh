#!/bin/sh
#----------------------------------------------------------------------------
# Project:  TwlSDK - tools
# File:     autoupload.sh
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

gen_updater(){
#ruby sumaker.rb custom $1 $2
make ROMSET=$1 REGION=$2
rm `find . -name "*autogen*"`
}


if [ $# -lt 1 ];
then
	echo "Usage: makecsu vername"
	exit
fi


gen_updater $1 JPN
gen_updater $1 USA
gen_updater $1 AUS
gen_updater $1 EUR
