#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlWLAN - components - mongoose_sdlog.TWL
# File:     make_lsf.pl
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

my $ROOT;

my $COMPONENT = $ARGV[0];
print $COMPONENT;
# search TWL_IPL_RED_ROOT
foreach ( sort keys ( %ENV ) ){
    if ($_ =~ m/TWL_IPL_RED_ROOT/s) {
        $ROOT = $ENV{$_};
    }
}

my $src = sprintf("$ROOT/build/components/%s.TWL/%s.lsf", $COMPONENT, $COMPONENT);
my $dst = sprintf("%s_sdlog.lsf", $COMPONENT);


my $rn = "\x0D\x0A";

open(F, $src) or die "cant open $src\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst") or die "cant open $dst\n";
foreach my $line ( @lines )
{
	if( $line =~ m/libos_sp/ )
	{
        my $tmp = $line;
        $tmp =~ s/libos_sp/libossdlog_sp/;
        
        print OUT $tmp;
        next;
	}

	print OUT $line;
}
close(OUT);
