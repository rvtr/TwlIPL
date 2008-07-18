#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - 
# File:     launcher_fake_macaddress.pl
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

my $rn = "\x0D\x0A";

my $SDKROOT;

# search TWLSDK_ROOT
foreach ( sort keys ( %ENV ) ){
    if ($_ =~ m/TWLSDK_ROOT/s) {
        $SDKROOT = $ENV{$_};
    }
}

my $src = "$SDKROOT/build/libraries/wm/ARM7.TWL/nwm/src/nwmsp_event_handler.c";
my $dst = "$SDKROOT/build/libraries/wm/ARM7.TWL/nwm/src/nwmsp_event_handler_autogen.c";

open(F, $src) or die "cant open $src\n";
my @lines = <F>;
close(F);

open(OUT, ">$dst") or die "cant open $dst\n";
foreach my $line ( @lines )
{
	if( $line =~ m/^ *MI_CpuCopy8\(ev->macaddr, sys->status->macAddress, NWM_SIZE_MACADDR\);/ )
	{
        print OUT "//\t\t[AUTO COMMENT] fake MAC address of TWL wireless, copy MAC address from that in NOR flash.", $rn;
        # add comment out at the top of line
        $line  =~ s/ev->macaddr,/wmMac,/;
	}

	print OUT $line;
}
close(OUT);

my $IPLROOT;

# search TWL_IPL_RED_ROOT
foreach ( sort keys ( %ENV ) ){
    if ($_ =~ m/TWL_IPL_RED_ROOT/s) {
        $IPLROOT = $ENV{$_};
    }
}

my $src_make1 = "$IPLROOT/build/libraries_sysmenu/Makefile";
my $dst_make1 = "$IPLROOT/build/libraries_sysmenu/Makefile_autogen";

open(F, $src_make1) or die "cant open $src_make1\n";
my @lines = <F>;
close(F);

open(OUT, ">$dst_make1") or die "cant open $dst_make1\n";
foreach my $line ( @lines )
{
    if( $line =~ m/^SUBDIRS/ )
    {
        print OUT "SUBMAKES\t\t=\t\tnwm/Makefile_autogen", $rn, $rn;
    }
    
	if( $line =~ m/nwm/ )
	{
        $line  =~ s/^/\#/;
	}

	print OUT $line;
}
close(OUT);

my $src_make2 = "$IPLROOT/build/libraries_sysmenu/nwm/Makefile";
my $dst_make2 = "$IPLROOT/build/libraries_sysmenu/nwm/Makefile_autogen";

open(F, $src_make2) or die "cant open $src_make2\n";
my @lines = <F>;
close(F);

open(OUT, ">$dst_make2") or die "cant open $dst_make2\n";
foreach my $line ( @lines )
{
	if( $line =~ m/^SUBDIRS_P *\+= *ARM7/ )
	{
        print OUT "SUBMAKES_P   +=  ARM7/Makefile_autogen", $rn;
        next
	}

	print OUT $line;
}
close(OUT);

my $src_make3 = "$IPLROOT/build/libraries_sysmenu/nwm/ARM7/Makefile";
my $dst_make3 = "$IPLROOT/build/libraries_sysmenu/nwm/ARM7/Makefile_autogen";

open(F, $src_make3) or die "cant open $src_make3\n";
my @lines = <F>;
close(F);

open(OUT, ">$dst_make3") or die "cant open $dst_make3\n";
foreach my $line ( @lines )
{
	if( $line =~ m/^[\t ]*nwmsp_event_handler\.c/ )
	{
        $line  =~ s/nwmsp_event_handler\.c/nwmsp_event_handler_autogen\.c/;
	}

	print OUT $line;
}
close(OUT);
