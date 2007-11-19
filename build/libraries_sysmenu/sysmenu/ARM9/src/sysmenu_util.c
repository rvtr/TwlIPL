/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_util.c

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

// define data------------------------------------------

// extern data------------------------------------------

// function's prototype declaration---------------------
static s64 SYSMi_CalcRTCSecOffset( RTCDate *datep, RTCTime *timep );

// global variable -------------------------------------

// static variable -------------------------------------

// const data  -----------------------------------------

// function's description-------------------------------

//======================================================================
//  RTCオフセット制御
//======================================================================

// RTCに新しい設定値をセットして、その値をもとにrtcOffset値を算出する。
s64 SYSM_CalcRTCOffset( RTCDate *newDatep, RTCTime *newTimep )
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
	if( ( oldDate.year < TSD_GetRTCLastSetYear() ) && ( TSD_IsSetDateTime() ) ) {
		oldDate.year += 100;										// 前回の設定～今回の設定の間にRTCが一周してしまったら、yearは100を加算してoffsetを計算する。
	}
	TSD_SetRTCLastSetYear( (u8)newDatep->year );
	
	offset0	= SYSMi_CalcRTCSecOffset( &oldDate, &oldTime );			// 設定直前のRTC値のオフセットを算出
	offset1	= SYSMi_CalcRTCSecOffset(  newDatep, newTimep );		// 新しくセットされたRTC値のオフセットを算出
	offset	= TSD_GetRTCOffset() + offset1 - offset0;				// 新RTC_ofs と 現在のRTC_ofs の差分の値を加算してリターン。
	
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
static s64 SYSMi_CalcRTCSecOffset( RTCDate *datep, RTCTime *timep )
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
		dayNum += SYSM_GetDayNum( datep->year, i );
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
u32 SYSM_GetDayNum( u32 year, u32 month )
{
	u32 dayNum = 31;
	if( month == 2 ) {
		if( SYSM_IsLeapYear100( year ) ) {
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
BOOL SYSM_IsLeapYear100( u32 year )
{
	if( ( year & 0x03 ) || ( year == 100 ) ) {						// うるう年は、「4で割り切れ　かつ　100で割り切れない年」または「400で割り切れる年」
		return FALSE;
	}else {
		return TRUE;
	}
}


// RTCの日付が正しいかチェック
BOOL SYSM_CheckRTCDate( RTCDate *datep )
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
BOOL SYSM_CheckRTCTime( RTCTime *timep )
{
	if(  ( timep->hour   > 23 )
	  || ( timep->minute > 59 )
	  || ( timep->second > 59 ) ) {
		return FALSE;
	}
	return TRUE;
}

