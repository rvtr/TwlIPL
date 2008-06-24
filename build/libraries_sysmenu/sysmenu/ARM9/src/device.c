/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <sysmenu.h>
#include <sysmenu/mcu.h>
#include <spi.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
// const data------------------------------------------------------------------

// ============================================================================
//
// デバイス制御
//
// ============================================================================

// RTCクロック補正値をセット
void SYSMi_WriteAdjustRTC( void )
{
	RTCRawAdjust raw;
	raw.adjust = LCFG_THW_GetRTCAdjust();
	( void )RTCi_SetRegAdjust( &raw );
}


// 起動時のRTCチェック
void SYSMi_CheckRTC( void )
{
	RTCDate date;
	RTCTime	time;
	
	// RTCのリセット or おかしい値を検出した場合は初回起動シーケンスへ。
	( void )RTC_GetDateTime( &date, &time );
	if( !UTL_CheckRTCDate( &date ) ||
	    !UTL_CheckRTCTime( &time ) ||
		SYSMi_GetWork()->flags.common.isResetRTC
		) {							// RTCの異常を検出したら、rtc入力フラグ＆rtcOffsetを0にしてNVRAMに書き込み。
		OS_TPrintf("\"RTC reset\" or \"Illegal RTC data\" detect!\n");
		LCFG_TSD_SetRTCOffset( 0 );
		LCFG_TSD_SetRTCLastSetYear( 0 );
		{
			u8 *pBuffer = SYSM_Alloc( LCFG_WRITE_TEMP );
			if( pBuffer != NULL ) {
				LCFG_WriteTWLSettings( (u8 (*)[ LCFG_WRITE_TEMP ] )pBuffer );
				SYSM_Free( pBuffer );
			}
		}
	}
}

