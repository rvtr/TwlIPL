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

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------
#define BACKLIGHT_BRIGHTNESS_MAX   	4

// global variable------------------------------------------------------
// function-------------------------------------------------------------

#ifdef SDK_ARM9

// バックライト
extern u32 UTL_SetBacklightBrightness( u8 brightness );						// バックライト輝度セット
extern u32 UTL_GetBacklightBrightness( u8 *pBrightness );					// バックライト輝度ゲット

// タッチパネル
extern void UTL_CaribrateTP( const LCFGTWLTPCalibData *pCalib );

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

// タイトル数取得（内部でNAMを使用するので、NAM_Initが事前に呼ばれている必要あり）
extern int  UTL_GetInstalledSoftBoxCount( void );

#endif

#ifdef __cplusplus
}
#endif

#endif  // __SYSM_UTIL_H__
