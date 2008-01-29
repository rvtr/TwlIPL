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
#include <spi.h>
#include "internal_api.h"

// define data-----------------------------------------------------------------
// extern data-----------------------------------------------------------------
// function's prototype-------------------------------------------------------
#ifdef SDK_FINALROM
u32 PM_SendUtilityCommandAsync(u32 number, u16 parameter, u16* retValue, PMCallback callback, void *arg);
u32 PM_SendUtilityCommand(u32 number, u16 parameter, u16* retValue);
u32 PMi_WriteRegister(u16 registerAddr, u16 data);
u32 PMi_WriteRegisterAsync(u16 registerAddr, u16 data, PMCallback callback, void *arg);
#endif // SDK_FINALROM

// global variable-------------------------------------------------------------
// static variable-------------------------------------------------------------
// const data------------------------------------------------------------------

// ============================================================================
//
// デバイス制御
//
// ============================================================================

// バックライト輝度調整
void SYSM_SetBackLightBrightness( u8 brightness )
{
	if( brightness > LCFG_TWL_BACKLIGHT_LEVEL_MAX ) {
		OS_Panic( "Backlight brightness over : %d\n", brightness );
	}
	( void )PMi_WriteRegister( 0x20, (u16)brightness );
	LCFG_TSD_SetBacklightBrightness( brightness );
	
	// [TODO:] バックライト輝度は毎回セーブせずに、アプリ起動やリセット、電源OFF時に値が変わっていたらセーブするようにする。
	LCFG_WriteTWLSettings();
}


// タッチパネルキャリブレーション
void SYSM_CaribrateTP( void )
{
	LCFGTWLTPCalibData store;
	TPCalibrateParam calibParam;
	
	// 本体設定データからキャリブレーション情報を取得
	LCFG_TSD_GetTPCalibration( &store );
	
	// TPキャリブレーション
	( void )TP_CalcCalibrateParam( &calibParam,							// タッチパネル初期化
			store.data.raw_x1, store.data.raw_y1, (u16)store.data.dx1, (u16)store.data.dy1,
			store.data.raw_x2, store.data.raw_y2, (u16)store.data.dx2, (u16)store.data.dy2 );
	TP_SetCalibrateParam( &calibParam );
	OS_TPrintf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			store.data.raw_x1, store.data.raw_y1, (u16)store.data.dx1, (u16)store.data.dy1,
			store.data.raw_x2, store.data.raw_y2, (u16)store.data.dx2, (u16)store.data.dy2 );
}


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
	if( !SYSM_CheckRTCDate( &date ) ||
	    !SYSM_CheckRTCTime( &time )
#ifndef __IS_DEBUGGER_BUILD											// 青デバッガではRTCの電池がないので、毎回ここにひっかかって設定データが片方クリアされてしまう。これを防ぐスイッチ。
		||
		SYSMi_GetWork()->flags.common.isResetRTC
#endif
		) {							// RTCの異常を検出したら、rtc入力フラグ＆rtcOffsetを0にしてNVRAMに書き込み。
		OS_TPrintf("\"RTC reset\" or \"Illegal RTC data\" detect!\n");
		LCFG_TSD_SetFlagDateTime( FALSE );
		LCFG_TSD_SetRTCOffset( 0 );
		LCFG_TSD_SetRTCLastSetYear( 0 );
		LCFG_WriteTWLSettings();
	}
}


#ifdef SDK_FINALROM
/*---------------------------------------------------------------------------*
  Name:         PMi_WriteRegisterAsync

  Description:  send write register command to ARM7

  Arguments:    registerAddr : PMIC register number (0-3)
                data         : data written to PMIC register
                callback     : callback function
                arg          : callback argument

  Returns:      result of issueing command
                PM_RESULT_BUSY    : busy
                PM_RESULT_SUCCESS : success
 *---------------------------------------------------------------------------*/
u32 PMi_WriteRegisterAsync(u16 registerAddr, u16 data, PMCallback callback, void *arg)
{
	return PM_SendUtilityCommandAsync(PMi_UTIL_WRITEREG, (u16)((registerAddr<<16) | (data&0xff)), NULL, callback, arg);
}

u32 PMi_WriteRegister(u16 registerAddr, u16 data)
{
	return PM_SendUtilityCommand(PMi_UTIL_WRITEREG, (u16)((registerAddr<<16) | (data&0xff)), NULL);
}
#endif // SDK_FINALROM
