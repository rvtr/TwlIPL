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

move_data_to_tmp() {
	tmp=tmp/$1
	install -d $tmp
	for src in `find $1 -name v* -type d`
	do
		mv $src $tmp
	done
}

do_cls() {
	tmp=tmp/$1
	for target in `find $tmp -name v* -type d | sort -t "v" -g -k 2`
	do
		cp -rp $target $1
		echo "$target"
		genLic $1
	done
}

for app in `find . -maxdepth 1 -mindepth 1 -type d`
do
	name=`echo "$app" | cut -c 3-`
	move_data_to_tmp $name
	do_cls $name
done


rm -rf tmp
