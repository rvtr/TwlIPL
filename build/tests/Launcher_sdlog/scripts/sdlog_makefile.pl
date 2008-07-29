#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - 
# File:     sdlog_makefile.pl
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

my $src = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/Makefile";
my $dst = "$IPLROOT/build/systemMenu_RED/Launcher/ARM9/Makefile_sdlog";

open(F, $src) or die "cant open $src\n";
my @lines = <F>;
close(F);


open(OUT, ">$dst") or die "cant open $dst\n";
foreach my $line ( @lines )
{
	if( $line =~ m/main\.c/ )
	{
        $line =~ s/main\.c/main_sdlog\.c/;
	}

    if( $line =~ m/^COMPONENT_DIR/ )
	{
        $line =~ s/\/build\/components/\/build\/tests\/Launcher_sdlog/;
        print OUT $line;
        print OUT "override COMPONENTSDIR\t=\t\$(call eupath, \$(COMPONENT_DIR)/../components)", $rn;
        next;
	}

    if( $line =~ m/^WRAM_MAP_FILE[\t ]*=[\t ]*\$\(COMPONENT_DIR\)\/wram_regs\/wram_regs.rbin/ )
    {
        $line =~ s/\$\(COMPONENT_DIR\)/\$\(TWL_IPL_RED_ROOT\)\/build\/components\/\$\(DEFAULT_COMP_ARM7\)\.TWL/;
    }

    if( $line =~ m/^INSTALL_DIR/ )
    {
        $line =~ s/\$\(SDK_NMENU_DATADIR\)/\$\(TWL_IPL_RED_ROOT\)\/build\/tests\/Launcher_sdlog/;
    }

    if( $line =~ m/^TARGET_BIN[\t ]*=[\t ]*\$\(TITLEID_LO\)\.tad/ )
    {
        print OUT <<__SDLOG_DESCRIPTIONS__;
\# --------------------------------------------------------------
\# SDLOG
ifeq	(\$(TWL_FINALROM),) \# SDLOG is enabled only on Release and Debug

ifeq (\$(SDLOG), TRUE)
override SDLOG		 = ARM9
endif

ifneq (\$(filter ARM7,\$(SDLOG)),)
\$(error "Invalid SDLOG option : should be SDLOG=[ARM9/ALL]")
endif

SDLOG_LIST	 = \$(if \$(filter ALL,\$(SDLOG)),ARM7 ARM9,\$(SDLOG))

ifneq	(\$(filter ARM9,\$(SDLOG_LIST)),)
MACRO_FLAGS				+=	-DUSE_SDLOG
ELIBRARIES				+= libsdlogRed\$(TWL_LIBSUFFIX).a
ELIBRARY_DIRS			+= \$(SYSMENU_ROOT)/build/tests/Launcher_sdlog/lib/\$(TWL_LIBTYPE)
EINCLUDES				+= \$(SYSMENU_ROOT)/build/tests/Launcher_sdlog/sdlog/ARM9.TWL/include \\
						   \$(ROOT)/build/tests/sdlog/sdlog/include
ifneq	(\$(filter ARM7,\$(SDLOG_LIST)),)
MACRO_FLAGS				+=	-DUSE_SDLOG_SUBP
endif \# SDLOG [ARM7]
endif \# SDLOG [ARM9]

endif \# FINALROM
__SDLOG_DESCRIPTIONS__
    }

	print OUT $line;
}
close(OUT);

