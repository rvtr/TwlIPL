#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - 
# File:     sdlog_main_c.pl
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

my $src = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/src/main.c";
my $dst = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/src/main_sdlog.c";


my $rn = "\x0D\x0A";

open(F, $src) or die "cant open $src\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst") or die "cant open $dst\n";
foreach my $line ( @lines )
{
	print OUT $line;
    
    if( $line =~ m/^\#include \"scanWDS\.h\"/ )
    {
        print OUT "//\t[AUTO COMMENT] SDLOGのヘッダをインクルードし、ログ用バッファを定義します。", $rn;
        print OUT <<__SDLOG_DEFINE__;
\#ifdef USE_SDLOG
\#include "sd_log_wrapper.h"
\#define BUFFER_SIZE 0x800
char logBuffer[ (BUFFER_SIZE + 1) ];
\#endif
__SDLOG_DEFINE__
    }
	if( $line =~ m/^[\t ]*RTC_Init\(\);/ )
	{
        print OUT "//\t[AUTO COMMENT] SDカードにOS_Printfのログを記録するために、SDLOGを初期化します。", $rn;
        print OUT <<__SDLOG_INITEX__;
\#ifdef USE_SDLOG
    {
        RTCDate rDate;
        RTCTime rTime;
        char filename[FS_ENTRY_LONGNAME_MAX];

        (void)RTC_GetDateTime(&rDate, &rTime);
        OS_SPrintf(filename, "Launcher_%02d%02d%02d_%02d%02d.txt",
                   rDate.year, rDate.month, rDate.day,
                   rTime.hour, rTime.minute);

        SDLOG_InitEx("sdmc:/log", filename, logBuffer,
                   BUFFER_SIZE, 0x80, SDLOG_WRITE_TYPE_CONTINUANCE);
    }
\#endif
__SDLOG_INITEX__
	}

    if( $line =~ m/^[\t ]*OS_SetIrqCheckFlag\(OS_IE_V_BLANK\);/ )
    {
        print OUT <<__SDLOG_PRINTARM7__;
\#ifdef USE_SDLOG
\#ifdef USE_SDLOG_SUBP
    SDLOG_PrintServer();
\#endif
\#endif
__SDLOG_PRINTARM7__
    }

}
close(OUT);

