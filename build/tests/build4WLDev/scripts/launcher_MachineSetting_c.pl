#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - 
# File:     launcher_MachineSetting_c.pl
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

my $src = "$IPLROOT/build/systemMenu_RED/MachineSettings/ARM9/src/MachineSetting.c";
my $dst = "$IPLROOT/build/systemMenu_RED/MachineSettings/ARM9/src/MachineSetting_autogen.c";

my $rn = "\x0D\x0A";

open(F, $src) or die "cant open $src\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst") or die "cant open $dst\n";
foreach my $line ( @lines )
{
	print OUT $line;
    # 従来版との見た目の区別のために、無線開発用の表示を追加。
	if( $line =~ m/^[\t ]*PrintfSJIS\(.*\"MACHINE SETTINGS  IPL:%s SDK:%s\".*\);/ )
	{
        print OUT "//\t[AUTO ADDITION] 従来版との見た目の区別のために、無線開発用の表\示を追加。", $rn;
        print OUT "\tPrintfSJIS( 0, 22*8, TXT_COLOR_BLUE, \"[ BUILT FOR WIRELESS DEVELOPMENT ]\");", $rn;
	}
}
close(OUT);

