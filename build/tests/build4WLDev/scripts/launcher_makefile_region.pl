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

my $src1 = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/Makefile";
my $dst1 = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/Makefile_autogen_HNAJ";

open(F, $src1) or die "cant open $src1\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst1") or die "cant open $dst1\n";
foreach my $line ( @lines )
{
	if( $line =~ m/loadWlanFirm\.c/ )
	{
        $line =~ s/loadWlanFirm\.c/loadWlanFirm_autogen\.c/;
	}

    if( $line =~ m/^TITLEID_LO/ )
	{
        $line =~ s/HNAA/HNAJ/;
	}

    if( $line =~ m/^CARD_REGION/ )
	{
        $line =~ s/ALL/Japan/;
        print OUT $line;
        print OUT "BINDIR					= ./bin.HNAJ/\$(TWL_BUILDTYPE)", $rn;
        next;
	}

    if( $line =~ m/^INSTALL_DIR/ )
	{
        print OUT "INSTALL_DIR				=	\$(UPDATER_HOST_ROOT_DIR)/debugger/japan", $rn;
        print OUT "MAKETAD			:=	\$(TWL_IPL_RED_PRIVATE_TOOLSDIR)/bin/maketad.updater.exe", $rn;
        next;
	}

	print OUT $line;
}
close(OUT);


my $src2 = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/Makefile";
my $dst2 = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/Makefile_autogen_HNAE";

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

    if( $line =~ m/^TITLEID_LO/ )
	{
        $line =~ s/HNAA/HNAE/;
	}

    if( $line =~ m/^CARD_REGION/ )
	{
        $line =~ s/ALL/America/;
        print OUT $line;
        print OUT "BINDIR					= ./bin.HNAE/\$(TWL_BUILDTYPE)", $rn;
        next;
	}

    if( $line =~ m/^INSTALL_DIR/ )
	{
        print OUT "INSTALL_DIR				=	\$(UPDATER_HOST_ROOT_DIR)/debugger/america", $rn;
        print OUT "MAKETAD			:=	\$(TWL_IPL_RED_PRIVATE_TOOLSDIR)/bin/maketad.updater.exe", $rn;
        next;
	}

	print OUT $line;
}
close(OUT);

