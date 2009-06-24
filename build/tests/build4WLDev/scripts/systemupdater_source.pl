#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - 
# File:     systemupdater_source.pl
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

my $src1 = "$IPLROOT/build/systemMenu_tools/SystemUpdaterRegionSelect/ARM9.TWL/src/process_check_console.c";
my $dst1 = "$IPLROOT/build/systemMenu_tools/SystemUpdaterRegionSelect/ARM9.TWL/src/process_check_console_autogen.c";

my $in_check_console = 0;

open(F, $src1) or die "cant open $src1\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst1") or die "cant open $dst1\n";
foreach my $line ( @lines )
{
	if( $line =~ m/^void ProcessCheckConsole\(void\)/ )
	{
        $in_check_console = 1;
	}

    if ($in_check_console == 1)
    {
        if( $line =~ m/^\/\*/ )
        {
            print OUT "\tkamiFontPrintfConsole( FONT_COLOR_GREEN, \"This updater is for Wireless development.\\n\");", $rn;
            print OUT "//\t[AUTO COMMENTOUT] 無線開発用SystemUpdaterの場合、コメントアウトをはずします。", $rn;
            # add comment out at the top of line
            $line  =~ s/^/\/\//;
        }
        if( $line =~ m/^\*\// )
        {
            print OUT "//\t[AUTO COMMENTOUT] 無線開発用SystemUpdaterの場合、コメントアウトをはずします。", $rn;
            # add comment out at the top of line
            $line  =~ s/^/\/\//;
        }
        if( $line =~ m/^}/ )
        {
            $in_check_console = 0;
        }
	}

	print OUT $line;
}
close(OUT);

my $src2 = "$IPLROOT/build/systemMenu_tools/SystemUpdaterRegionSelect/ARM9.TWL/src/process_select_region.c";
my $dst2 = "$IPLROOT/build/systemMenu_tools/SystemUpdaterRegionSelect/ARM9.TWL/src/process_select_region_autogen.c";

open(F, $src2) or die "cant open $src2\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst2") or die "cant open $dst2\n";
foreach my $line ( @lines )
{
    # WLANのLoad時に限り、FatalErrorのセット無効にする。
	if( $line =~ m/OS_TWL_REGION_AUSTRALIA/ )
	{
        print OUT "//\t[AUTO COMMENTOUT] 無線開発用SystemUpdaterの場合、RegionはJapanとAmericaのみ。", $rn;
        $line  =~ s/OS_TWL_REGION_AUSTRALIA/OS_TWL_REGION_AMERICA/;
	}

	print OUT $line;
}
close(OUT);

my $src3 = "$IPLROOT/build/systemMenu_tools/SystemUpdaterRegionSelect/ARM9.TWL/Makefile";
my $dst3 = "$IPLROOT/build/systemMenu_tools/SystemUpdaterRegionSelect/ARM9.TWL/Makefile_autogen";

open(F, $src3) or die "cant open $src3\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst3") or die "cant open $dst3\n";
foreach my $line ( @lines )
{
	if( $line =~ m/UPDATER_HOST_ROOT_DIR =/ )
	{
        $line  =~ s/UPDATER_HOST_ROOT_DIR =/UPDATER_HOST_ROOT_DIR ?=/;
	}
    
	if( $line =~ m/process_select_region\.c/ )
	{
        $line  =~ s/process_select_region\.c/process_select_region_autogen\.c/;
	}

	if( $line =~ m/process_check_console\.c/ )
	{
        $line  =~ s/process_check_console\.c/process_check_console_autogen\.c/;
	}

	print OUT $line;
}
close(OUT);
