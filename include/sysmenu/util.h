/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     SYSM_lib.h

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

#ifndef	__SYSM_UTIL_H__
#define	__SYSM_UTIL_H__

#include <twl.h>
#include <sysmenu.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------
#define BACKLIGHT_BRIGHTNESS_MAX   	4

typedef enum FatalErrorCode {
	FATAL_ERROR_UNDEFINED = 0,
	FATAL_ERROR_NAND = 1,
	FATAL_ERROR_HWINFO_NORMAL = 2,
	FATAL_ERROR_HWINFO_SECURE = 3,
	FATAL_ERROR_TWLSETTINGS = 4,
	FATAL_ERROR_SHARED_FONT = 5,
	FATAL_ERROR_WLANFIRM_AUTH = 6,
	FATAL_ERROR_WLANFIRM_LOAD = 7,
	FATAL_ERROR_MAX = 8
}FatalErrorCode;


// global variable------------------------------------------------------
// function-------------------------------------------------------------

#ifdef SDK_ARM9

// バックライト
extern u32 UTL_SetBacklightBrightness( u8 brightness );						// バックライト輝度セット
extern u32 UTL_GetBacklightBrightness( u8 *pBrightness );					// バックライト輝度ゲット

// タッチパネル
extern void UTL_CaribrateTP( const LCFGTWLTPCalibData *pCalib );			// TPキャリブレーション
extern BOOL UTL_IsValidCalibration( u16 x, u16 y, u16 correct_x, u16 correct_y );	// TPキャリブレーション後にタッチしたポイントが正確か？

// スリープ
extern void UTL_GoSleepMode( void );

// RTC関係
extern BOOL UTL_CheckRTCDate( RTCDate *pDate );								// 日付が正常かチェック
extern BOOL UTL_CheckRTCTime( RTCTime *pTime );								// 時刻が正常かチェック
extern s64  UTL_CalcRTCOffset( RTCDate *pNewDate, RTCTime *pNewTime );		// RTCオフセット計算とRTCへの日付時刻チェックを行う
extern u32  UTL_GetDayNum( u32 year, u32 month );							// 指定された年・月の日数を取得する
extern BOOL UTL_IsLeapYear100( u32 year );									// 指定された年がうるう年か調べる

// ペアレンタルコントロール問い合わせ
extern u32  UTL_CalcPCTLInquiryCode( void );								// 問い合わせコード（１０進８桁）算出
extern u32  UTL_CalcPCTLMasterKey( void );									// マスターキー　　（１０進５桁）算出（※内部でRTC_GetDateを使用します。）

// アプリROMヘッダの要EULAフラグ取得
static inline BOOL UTL_IsROMHeaderEULARequired( void )
{
	return (BOOL)SYSM_GetAppRomHeader()->exFlags.agree_EULA;
}

#endif

// FATALエラー
extern BOOL UTL_IsFatalError( void );										// FATALエラーか？
extern void UTL_SetFatalError( FatalErrorCode error );						// FATALエラーのセット
extern u32  UTL_GetFatalError( void );										// FATALエラー状態の取得（FatalErrorCodeをビットに割り当てて格納しています。）


#ifdef __cplusplus
}
#endif

#endif  // __SYSM_UTIL_H__
