#!/bin/perl
#----------------------------------------------------------------------------
# Project:  TwlIPL - tests - 
# File:     make_nwm_fw_wrack.pl
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

my $src      = "$SDKROOT/build/libraries/wm/common.TWL/src/nwm_installfirm.c";
my $dst_dir  = "../src";
my $dst_file = "nwm_installfirm_autogen.c";

open(F, $src) or die "cant open $src\n";
my @lines = <F>;
close(F);

mkdir $dst_dir unless -e $dst_dir;

my $in_instfirm_count = 0;

open(OUT, ">$dst_dir/$dst_file") or die "cant open $dst_dir/$dst_file\n";
foreach my $line ( @lines )
{
    if ( $line =~ m/^NWMRetCode NWMi_InstallFirmware\(NWMCallbackFunc callback, void\* addr, u32 size, BOOL isColdstart\)/ )
    {
        print OUT <<__WRACK_FIRMWARE_API__;

\#ifdef NWM_WRACK_FIRMWARE
NWMRetCode NWMi_WrackFirmware(NWMCallbackFunc callback);

NWMRetCode NWMi_WrackFirmware(NWMCallbackFunc callback)
{
    NWMArm9Buf *sys = NWMi_GetSystemWork();
    NWMRetCode result;
    
    // コールバック関数を登録
    NWMi_SetCallbackTable(NWM_APIID_INSTALL_FIRMWARE, callback);

    // ARM7にFIFOで通知
    result = NWMi_SendCommand(NWM_APIID_INSTALL_FIRMWARE, 5,
                              (u32)(sys->NWM7), (u32)(sys->status), (u32)(sys->fifo7to9), NULL, FALSE);
    
    return NWM_RETCODE_OPERATING;
}
\#endif // NWM_WRACK_FIRMWARE

__WRACK_FIRMWARE_API__
    }
     
    if ( $line =~ m/MCU_SetWifiLedBlinkingStatus\( FALSE \);/ )
    {
        print OUT "#ifndef NWM_WRACK_FIRMWARE", $rn;
    }
    
    if( $line =~ m/^installfirm_err:/ )
    {
        print OUT <<__WRACK_FIRMWARE__;

\#else // NWM_WRACK_FIRMWARE
\#pragma unused (err)
    // set WiFi_RST to disable WiFi
    NWMSPi_SetWiFiReset(FALSE);
    OS_Sleep(1); // wait 1msec
    // set WiFi_RST to enable WiFi
    NWMSPi_SetWiFiReset(TRUE);
    {
        NWMCallback *cb;

        NWMSP_ClearAPIActiveFlag(NWM_APIID_INSTALL_FIRMWARE);
        // 正常終了
        cb = (NWMCallback *)NWMSP_GetBuffer4Callback2Nwm9();
        cb->apiid = NWM_APIID_INSTALL_FIRMWARE;
        cb->retcode = NWM_RETCODE_FATAL_ERROR;
        NWMSP_ReturnResult2Nwm9((void *)cb);
        return;
    }
\#endif // NWM_WRACK_FIRMWARE
__WRACK_FIRMWARE__
	}
	print OUT $line;


}
close(OUT);

