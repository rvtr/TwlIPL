/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     util.c

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
#include <twl/nam.h>
#include <sysmenu.h>

// define data------------------------------------------
#define TP_CL_CONFIRM_MARGIN		4				// TPキャリブレーションの座標マージン（キャリブレーション後の座標での値）

// extern data------------------------------------------
// function's prototype declaration---------------------
static s64 UTLi_CalcRTCSecOffset( RTCDate *datep, RTCTime *timep );

// global variable -------------------------------------
// static variable -------------------------------------
// const data  -----------------------------------------
// function's description-------------------------------


//======================================================================
//  バックライト
//======================================================================

// バックライト輝度セット
u32 UTL_SetBacklightBrightness( u8 brightness )
{
	brightness %= ( BACKLIGHT_BRIGHTNESS_MAX + 1 );
	return PM_SendUtilityCommand( PMi_UTIL_SET_BACKLIGHT_BRIGHTNESS, (u16)brightness, NULL );
}


// バックライト輝度
u32 UTL_GetBacklightBrightness( u8 *pBrightness )
{
	u16 status;
	u32 result = PM_SendUtilityCommand( PM_UTIL_GET_STATUS, PMi_UTIL_GET_BACKLIGHT_BRIGHTNESS, &status);

	if ( result == PM_RESULT_SUCCESS )
	{
		if (pBrightness)
		{
			*pBrightness = (u8)status;
		}
	}
	return result;
}


//======================================================================
//  タッチパネル
//======================================================================

// タッチパネルキャリブレーション
void UTL_CaribrateTP( const LCFGTWLTPCalibData *pCalib )
{
	TPCalibrateParam calibParam;
	
	// TPキャリブレーション
	( void )TP_CalcCalibrateParam( &calibParam,							// タッチパネル初期化
			pCalib->data.raw_x1, pCalib->data.raw_y1, (u16)pCalib->data.dx1, (u16)pCalib->data.dy1,
			pCalib->data.raw_x2, pCalib->data.raw_y2, (u16)pCalib->data.dx2, (u16)pCalib->data.dy2 );
	TP_SetCalibrateParam( &calibParam );
	OS_TPrintf("TP_calib: %4d %4d %4d %4d %4d %4d\n",
			pCalib->data.raw_x1, pCalib->data.raw_y1, (u16)pCalib->data.dx1, (u16)pCalib->data.dy1,
			pCalib->data.raw_x2, pCalib->data.raw_y2, (u16)pCalib->data.dx2, (u16)pCalib->data.dy2 );
}


// キャリブレーションが正常に行われたかチェック
BOOL UTL_IsValidCalibration( u16 x, u16 y, u16 correct_x, u16 correct_y )
{
	return !( x < correct_x - TP_CL_CONFIRM_MARGIN ||
			  x > correct_x + TP_CL_CONFIRM_MARGIN ||
			  y < correct_y - TP_CL_CONFIRM_MARGIN ||
			  y > correct_y + TP_CL_CONFIRM_MARGIN );
}


//======================================================================
//  スリープ
//======================================================================

// スリープモードへの遷移
void UTL_GoSleepMode( void )
{
    // 蓋閉じ判定
    if ( ! PAD_DetectFold() )
    {
        return;
    }

    // デバッガ接続中だけはスリープに入らない（蓋閉じでもデバッガが起動するように）
    if ( !SYSM_IsRunOnDebugger() || (OSi_DetectDebugger() & OS_CONSOLE_TWLDEBUGGER) )
    {
        // カード抜け無検出設定
        //   TWLではゲームカードの再ロードが可能なため
        //   スリープ時のカード抜け検出を無効化
        //   （DS-IPLではゲームカードが起動できなくなるので
        //     レジューム時のROM-IDチェックでエラーになると
        //     シャットダウンしていた）
        OSIntrMode enable = OS_DisableInterrupts();
        reg_MI_MCCNT0 &= ~REG_MI_MCCNT0_I_MASK;
        OS_ResetRequestIrqMask( OS_IE_CARD_IREQ );
        OS_RestoreInterrupts( enable );

        // スリープ遷移
    	PM_GoSleepMode( PM_TRIGGER_COVER_OPEN |
	    				PM_TRIGGER_RTC_ALARM,
		    			0,
			    		0 );
    }
}


//======================================================================
//  RTCオフセット制御
//======================================================================

// RTCに新しい設定値をセットして、その値をもとにrtcOffset値を算出する。
s64 UTL_CalcRTCOffset( RTCDate *newDatep, RTCTime *newTimep )
{
	RTCDate oldDate;
	RTCTime oldTime;
	s64		offset0;
	s64		offset1;
	s64		offset;
	
	// RTCへの新しい値の設定
	(void)RTC_GetDateTime( &oldDate, &oldTime );					// ライト直前に現在のRTC値を取得する。
	oldTime.second = 0;
	
	// RTC設定時は、今回の設定でどれだけRTC値が変化したか（秒オフセット単位）を算出。
	if( ( oldDate.year < LCFG_TSD_GetRTCLastSetYear() ) && ( LCFG_TSD_IsFinishedInitialSetting() ) ) {
		oldDate.year += 100;										// 前回の設定～今回の設定の間にRTCが一周してしまったら、yearは100を加算してoffsetを計算する。
	}
	LCFG_TSD_SetRTCLastSetYear( (u8)newDatep->year );
	
	offset0	= UTLi_CalcRTCSecOffset( &oldDate, &oldTime );			// 設定直前のRTC値のオフセットを算出
	offset1	= UTLi_CalcRTCSecOffset(  newDatep, newTimep );		// 新しくセットされたRTC値のオフセットを算出
	offset	= LCFG_TSD_GetRTCOffset() + offset1 - offset0;			// 新RTC_ofs と 現在のRTC_ofs の差分の値を加算してリターン。
	
	OS_Printf ("Now    Date = year:%3d month:%3d date:%3d  hour:%3d minute:%3d second:%3d\n",
			   oldDate.year, oldDate.month,  oldDate.day,
			   oldTime.hour, oldTime.minute, oldTime.second);
	OS_Printf ("Set    Date = year:%3d month:%3d date:%3d  hour:%3d minute:%3d second:%3d\n",
				newDatep->year, newDatep->month,  newDatep->day,
				newTimep->hour, newTimep->minute, newTimep->second);
	OS_Printf ("offset[0] = %x\n", offset0 );
	OS_Printf ("offset[1] = %x\n", offset1 );
	OS_Printf ("rtcOffset = %x\n", offset );
	
	return offset;
}


// RTCオフセット値の算出
#define SECOND_OFFSET
static s64 UTLi_CalcRTCSecOffset( RTCDate *datep, RTCTime *timep )
{
	u32 i;
	int uruu   = 0;
	int dayNum = 0;
	s64 offset;
	
	// 時、分、秒を　秒 or 分オフセットに
#ifdef SECOND_OFFSET
	offset = ( timep->hour * 60 + timep->minute ) * 60 + timep->second;	// ※キャスト部分にバグあり
#else
	offset =   timep->hour * 60 + timep->minute;
#endif
	
	// 月、日を　日数に換算してから、　秒 or 分オフセットに
	dayNum = (int)datep->day - 1;
	for( i = 1; i < datep->month; i++ ) {
		dayNum += UTL_GetDayNum( datep->year, i );
	}
	
	// 年を　日数に換算
	if( datep->year > 0 ) {
		uruu = ( ( (int)datep->year - 1 ) >> 2 ) + 1;					// 指定年-1までのうるう年の個数を算出して、その日数を加算。
	}
	dayNum += uruu + (u32)( datep->year * 365 );
	
	// 年・月・日を日数に換算した値を　秒 or 分オフセットに
#ifdef SECOND_OFFSET
	offset += (s64)( dayNum * 24 * 3600 );	// ※キャスト部分にバグあり
#else
	offset += (s64)( dayNum * 24 * 60 );
#endif
	
	return offset;
}


// 指定された年・月の日数を返す。
u32 UTL_GetDayNum( u32 year, u32 month )
{
	u32 dayNum = 31;
	if( month == 2 ) {
		if( UTL_IsLeapYear100( year ) ) {
			dayNum -= 2;
		}else {
			dayNum -= 3;
		}
	}else if( ( month == 4 ) || ( month == 6 ) || ( month == 9 ) || ( month == 11 ) ) {
		dayNum--;
	}
	return dayNum;
}


// 簡易うるう年の判定 (うるう年：1、通常の年：0）※RTCのとりうる範2000～2100年に限定する。
BOOL UTL_IsLeapYear100( u32 year )
{
	if( ( year & 0x03 ) || ( year == 100 ) ) {						// うるう年は、「4で割り切れ　かつ　100で割り切れない年」または「400で割り切れる年」
		return FALSE;
	}else {
		return TRUE;
	}
}


// RTCの日付が正しいかチェック
BOOL UTL_CheckRTCDate( RTCDate *datep )
{
	if(	 ( datep->year >= 100 )
	  || ( datep->month < 1 ) || ( datep->month > 12 )
	  || ( datep->day   < 1 ) || ( datep->day   > 31 )
	  || ( datep->week >= RTC_WEEK_MAX ) ) {
		return FALSE;
	}
	return TRUE;
}


// RTCの時刻が正しいかチェック
BOOL UTL_CheckRTCTime( RTCTime *timep )
{
	if(  ( timep->hour   > 23 )
	  || ( timep->minute > 59 )
	  || ( timep->second > 59 ) ) {
		return FALSE;
	}
	return TRUE;
}

