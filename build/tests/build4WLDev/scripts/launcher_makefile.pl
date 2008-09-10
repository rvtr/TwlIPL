#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - 
# File:     launcher_makefile.pl
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

my $rn = "\x0D\x0A";

my $src1 = "$IPLROOT/build/systemMenu_RED/Launcher/Makefile";
my $dst1 = "$IPLROOT/build/systemMenu_RED/Launcher/Makefile_autogen";

open(F, $src1) or die "cant open $src1\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst1") or die "cant open $dst1\n";
foreach my $line ( @lines )
{
	if( $line =~ m/^SUBDIRS\t*=\t*banner ARM9/ )
	{
        print OUT "SUBMAKES\t\t=\tbanner/Makefile ARM9/Makefile_autogen", $rn;
        next;
	}

	print OUT $line;
}
close(OUT);

my $src2 = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/Makefile";
my $dst2 = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/Makefile_autogen";

open(F, $src2) or die "cant open $src2\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst2") or die "cant open $dst2\n";
foreach my $line ( @lines )
{
	if( $line =~ m/loadWlanFirm\.c/ )
	{
        $line =~ s/loadWlanFirm\.c/loadWlanFirm_autogen\.c/;
	}

	print OUT $line;
}
close(OUT);

my $src3 = "$IPLROOT/build/systemMenu_RED/MachineSettings/Makefile";
my $dst3 = "$IPLROOT/build/systemMenu_RED/MachineSettings/Makefile_autogen";

open(F, $src3) or die "cant open $src3\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst3") or die "cant open $dst3\n";
foreach my $line ( @lines )
{
	if( $line =~ m/^SUBDIRS\t*=\t*banner ARM9/ )
	{
        print OUT "SUBMAKES\t\t=\tbanner/Makefile ARM9/Makefile_autogen", $rn;
        next;
	}

	print OUT $line;
}
close(OUT);

my $src4 = "$IPLROOT/build/systemMenu_RED/MachineSettings/ARM9/Makefile";
my $dst4 = "$IPLROOT/build/systemMenu_RED/MachineSettings/ARM9/Makefile_autogen";

open(F, $src4) or die "cant open $src4\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst4") or die "cant open $dst4\n";
foreach my $line ( @lines )
{
	if( $line =~ m/ +MachineSetting\.c/ )
	{
        $line =~ s/MachineSetting\.c/MachineSetting_autogen\.c/;
	}

	print OUT $line;
}
close(OUT);
