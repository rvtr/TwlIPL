#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - 
# File:     launcher_loadWlanFirm_c.pl
#
# Copyright 2007 Nintendo.  All rights reserved.
#
# These coded insructions, statements, and computer programs contain
# proprietary information of Nintendo of America Inc. and/or Nintendo
# Company Ltd., and are protected by Federal copyright law.  They may
# not be disclosed to third parties or copied or duplicated in any form,
# in whole or in part, without the prior written consent of Nintendo.
#
# $Date::            $
# $Rev$
# $Author$
#----------------------------------------------------------------------------

use strict;

my $IPLROOT;

# search TWL_IPL_RED_ROOT
foreach ( sort keys ( %ENV ) ){
    if ($_ =~ m/TWL_IPL_RED_ROOT/s) {
        $IPLROOT = $ENV{$_};
    }
}

my $src = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/src/loadWlanFirm.c";
my $dst = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/src/loadWlanFirm_autogen.c";


my $rn = "\x0D\x0A";

open(F, $src) or die "cant open $src\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst") or die "cant open $dst\n";
foreach my $line ( @lines )
{
    # WLANのLoad時に限り、FatalErrorのセット無効にする。
	if( $line =~ m/^[\t ]*UTL_SetFatalError\([\t ]*FATAL_ERROR_WLANFIRM_(LOAD|AUTH)[\t ]*\)/ )
	{
        print OUT "//\t[AUTO COMMENTOUT] 無線開発用ランチャーの場合、無線関連のFATALエラー発生は無視してブートするようにします。", $rn;
        # add comment out at the top of line
        $line  =~ s/^/\/\//;
	}

	print OUT $line;
}
close(OUT);

