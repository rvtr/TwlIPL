#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlWLAN - demos - wifi
# File:     make_rsf.pl
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

# search TWLSDK_ROOT
foreach ( sort keys ( %ENV ) ){
    if ($_ =~ m/TWLSDK_ROOT/s) {
        $ROOT = $ENV{$_};
    }
}

my $src1 = "$ROOT/include/twl/specfiles/ROM-TS.rsf";
my $dst1 = "wireless_checker.rsf";


my $rn = "\x0D\x0A";

open(F, $src1) or die "cant open $src1\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst1") or die "cant open $dst1\n";
foreach my $line ( @lines )
{
    if( $line =~ m/TitleName\t+/ )
    {
        $line =~ s/\t\#?TitleName.*\".*\"/\tTitleName\t\t\"WLAN CHECKER\"/;
    }
    
    if( $line =~ m/^\tBannerFile/ )
	{
        print OUT "\tBannerFile\t\t\$(BANNER_FILE)", $rn;
        print OUT $rn, "\t#", $rn;
        print OUT "\t# Permit LandingNormalJump: for TWL \"ApplicationJump\" function [TRUE/FALSE]", $rn;
        print OUT "\t#", $rn;
        print OUT "\tPermitLandingNormalJump\tTRUE", $rn;
        next;
	}

	print OUT $line;
}
close(OUT);


my $src2 = "$ROOT/include/twl/specfiles/ROM-TS_nand.rsf";
my $dst2 = "wireless_checker_nand.rsf";

open(F, $src2) or die "cant open $src2\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst2") or die "cant open $dst2\n";
foreach my $line ( @lines )
{
    if( $line =~ m/^\tBannerFile/ )
	{
        print OUT "\tBannerFile\t\t\$(BANNER_FILE)", $rn;
        next;
	}

    if( $line =~ m/GameCode\t+/ )
    {
        $line =~ s/\t\#?GameCode.*[a-zA-Z0-9]{4}/\tGameCode\t\t\t\t000A/;
    }

    if( $line =~ m/TitleName\t+/ )
    {
        $line =~ s/\t\#?TitleName.*\".*\"/\tTitleName\t\t\"WLAN CHECKER\"/;
    }
    
    if( $line =~ m/PermitLandingNormalJump/ )
    {
        $line =~ s/\t\#?.*(TRUE|FALSE)/\tPermitLandingNormalJump\tTRUE/;
    }

	print OUT $line;
}