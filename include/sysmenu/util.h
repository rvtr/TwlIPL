/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     util.h

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
#include <twl/os/common/format_rom.h>

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
	FATAL_ERROR_TITLE_LOAD_FAILED = 8,
	FATAL_ERROR_TITLE_POINTER_ERROR = 9,
	FATAL_ERROR_AUTHENTICATE_FAILED = 10,
	FATAL_ERROR_ENTRY_ADDRESS_ERROR = 11,
	FATAL_ERROR_TITLE_BOOTTYPE_ERROR = 12,
	FATAL_ERROR_SIGN_DECRYPTION_FAILED = 13,
	FATAL_ERROR_SIGN_COMPARE_FAILED = 14,
	FATAL_ERROR_HEADER_HASH_CALC_FAILED = 15,
	FATAL_ERROR_TITLEID_COMPARE_FAILED = 16,
	FATAL_ERROR_VALID_SIGN_FLAG_OFF = 17,
	FATAL_ERROR_CHECK_TITLE_LAUNCH_RIGHTS_FAILED = 18,
	FATAL_ERROR_MODULE_HASH_CHECK_FAILED = 19,
	FATAL_ERROR_MODULE_HASH_CALC_FAILED = 20,
	FATAL_ERROR_MEDIA_CHECK_FAILED = 21,
	FATAL_ERROR_DL_MAGICCODE_CHECK_FAILED = 22,
	FATAL_ERROR_DL_SIGN_DECRYPTION_FAILED = 23,
	FATAL_ERROR_DL_HASH_CALC_FAILED = 24,
	FATAL_ERROR_DL_SIGN_COMPARE_FAILED = 25,
	FATAL_ERROR_WHITELIST_INITDB_FAILED = 26,
	FATAL_ERROR_WHITELIST_NOTFOUND = 27,
	FATAL_ERROR_DHT_PHASE1_FAILED = 28,
	FATAL_ERROR_DHT_PHASE2_FAILED = 29,
	FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF = 30,
	FATAL_ERROR_TWL_BOOTTYPE_UNKNOWN = 31,
	FATAL_ERROR_NTR_BOOTTYPE_UNKNOWN = 32,
	FATAL_ERROR_PLATFORM_UNKNOWN = 33,
	FATAL_ERROR_LOAD_UNFINISHED = 34,
	FATAL_ERROR_LOAD_OPENFILE_FAILED = 35,
	FATAL_ERROR_LOAD_MEMALLOC_FAILED = 36,
	FATAL_ERROR_LOAD_SEEKFILE_FAILED = 37,
	FATAL_ERROR_LOAD_READHEADER_FAILED = 38,
	FATAL_ERROR_LOAD_LOGOCRC_ERROR = 39,
	FATAL_ERROR_LOAD_READDLSIGN_FAILED = 40,
	FATAL_ERROR_LOAD_RELOCATEINFO_FAILED = 41,
	FATAL_ERROR_LOAD_READMODULE_FAILED = 42,
    FATAL_ERROR_NINTENDO_LOGO_CHECK_FAILED = 43,
    FATAL_ERROR_SYSMENU_VERSION = 44,

	FATAL_ERROR_MAX = 45
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
extern BOOL UTL_IsROMHeaderEULARequired( void );

// アプリROMヘッダのnintendoロゴの正当性チェック
extern BOOL UTL_CheckNintendoLogoData( ROM_Header_Short *rh );

#endif

// FATALエラー
extern BOOL UTL_IsFatalError( void );										// FATALエラーか？
extern void UTL_SetFatalError( FatalErrorCode error );						// FATALエラーのセット
extern u64  UTL_GetFatalError( void );										// FATALエラー状態の取得（FatalErrorCodeをビットに割り当てて格納しています。）


// リージョンチェック
static inline BOOL UTL_CheckAppRegion( u32 card_region_bitmap )
{
	return ( card_region_bitmap & ( 0x00000001 << OS_GetRegion() ) ) ? TRUE : FALSE;
}

// CRCチェック
static BOOL UTL_CheckAppCRC16( ROM_Header_Short *pROMH )
{
	u16 calc_crc = SVC_GetCRC16( 65535, pROMH, 0x015e );
	if( ( calc_crc != pROMH->header_crc16 ) ||
	    ( 0xcf56   != pROMH->nintendo_logo_crc16 ) ){
		return FALSE;
	}
	return TRUE;
}


#ifdef __cplusplus
}
#endif

#endif  // __SYSM_UTIL_H__
