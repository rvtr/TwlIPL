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



if [ $# -lt 1 ];
then
	echo "input target dir name"
	exit
fi

./tocsu.sh $1
ruby toaqua.rb $1