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

#ifndef __SYSM_UTIL_H__
#define __SYSM_UTIL_H__

#include <twl.h>
#include <twl/os/common/format_rom.h>
#ifdef SYSM_BUILD_FOR_DEBUGGER
#include <sysmenu/sysmenu_lib/common/sysmenu_work.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------
#define BACKLIGHT_BRIGHTNESS_MAX    4

typedef enum FatalErrorCode {
    FATAL_ERROR_UNDEFINED = 0,
    FATAL_ERROR_NAND = 1,                                   // NANDデバイスのエラー
    FATAL_ERROR_HWINFO_NORMAL = 2,                          // HWノーマル情報のリードエラー
    FATAL_ERROR_HWINFO_SECURE = 3,                          // HWセキュア情報のリードエラー
    FATAL_ERROR_TWLSETTINGS = 4,                            // 本体設定データのリードエラー
    FATAL_ERROR_SHARED_FONT = 5,                            // 共有フォントのリードエラー
    FATAL_ERROR_WLANFIRM_AUTH = 6,                          // 無線ファームの認証エラー
    FATAL_ERROR_WLANFIRM_LOAD = 7,                          // 無線ファームのロードエラー
    FATAL_ERROR_TITLE_LOAD_FAILED = 8,                      // アプリケーションのロードエラー
    FATAL_ERROR_TITLE_POINTER_ERROR = 9,                    // ブート要求されたが、アプリが指定されていない
    FATAL_ERROR_AUTHENTICATE_FAILED = 10,                   // アプリ認証失敗
    FATAL_ERROR_ENTRY_ADDRESS_ERROR = 11,                   // アプリの起動アドレスが不正
    FATAL_ERROR_TITLE_BOOTTYPE_ERROR = 12,                  // アプリブートタイプが不正（NANDブート、カードブート、MBブート以外の値）
    FATAL_ERROR_SIGN_DECRYPTION_FAILED = 13,                // アプリ署名デクリプト失敗
    FATAL_ERROR_SIGN_COMPARE_FAILED = 14,                   // アプリ署名検証失敗
    FATAL_ERROR_HEADER_HASH_CALC_FAILED = 15,               // アプリハッシュ計算用メモリ確保失敗
    FATAL_ERROR_TITLEID_COMPARE_FAILED = 16,                // ブート要求されたTWLアプリと実際にロードしたアプリのTitleIDが不一致
    FATAL_ERROR_VALID_SIGN_FLAG_OFF = 17,                   // アプリROMヘッダの署名有効フラグが立っていない
    FATAL_ERROR_CHECK_TITLE_LAUNCH_RIGHTS_FAILED = 18,      // アプリ起動認証失敗
    FATAL_ERROR_MODULE_HASH_CHECK_FAILED = 19,              // アプリハッシュ不一致
    FATAL_ERROR_MODULE_HASH_CALC_FAILED = 20,               // アプリハッシュ計算用メモリ確保失敗
    FATAL_ERROR_MEDIA_CHECK_FAILED = 21,                    // カードアプリをNAND起動 or NANDアプリをカード起動しようとした（デバッガ起動を除く）
    FATAL_ERROR_DL_MAGICCODE_CHECK_FAILED = 22,             // DSダウンロードプレイアプリ署名のマジックコードが不正（TEMPブートアプリブート時）
    FATAL_ERROR_DL_SIGN_DECRYPTION_FAILED = 23,             // DSダウンロードプレイアプリ署名のデクリプト失敗（TEMPブートアプリブート時）
    FATAL_ERROR_DL_HASH_CALC_FAILED = 24,                   // DSダウンロードプレイアプリハッシュ計算用メモリ確保失敗（TEMPブートアプリブート時）
    FATAL_ERROR_DL_SIGN_COMPARE_FAILED = 25,                // DSダウンロードプレイアプリハッシュ不一致（TEMPブートアプリブート時）
    FATAL_ERROR_WHITELIST_INITDB_FAILED = 26,               // NTRホワイトリスト自身の認証失敗
    FATAL_ERROR_WHITELIST_NOTFOUND = 27,                    // 起動NTRアプリのイニシャルコードがNTRホワイトリストに見つからなかった
    FATAL_ERROR_DHT_PHASE1_FAILED = 28,                     // アプリのNTRホワイトリスト認証失敗（フェーズ１）
    FATAL_ERROR_DHT_PHASE2_FAILED = 29,                     // アプリのNTRホワイトリスト認証失敗（フェーズ２）
    FATAL_ERROR_LANDING_TMP_JUMP_FLAG_OFF = 30,             // TMPブートアプリのROMヘッダにTMPジャンプ許可ビットが立っていない
    FATAL_ERROR_TWL_BOOTTYPE_UNKNOWN = 31,                  // TWLアプリブートタイプ不明
    FATAL_ERROR_NTR_BOOTTYPE_UNKNOWN = 32,                  // NTRアプリブートタイプ不明
    FATAL_ERROR_PLATFORM_UNKNOWN = 33,                      // ROMヘッダのプラットホームコード不明
    FATAL_ERROR_LOAD_UNFINISHED = 34,                       // アプリロードが完了していないのに、認証フェーズに進んだ
    FATAL_ERROR_LOAD_OPENFILE_FAILED = 35,                  // NANDアプリのファイルオープン失敗
    FATAL_ERROR_LOAD_MEMALLOC_FAILED = 36,                  // アプリハッシュ計算用メモリ確保失敗
    FATAL_ERROR_LOAD_SEEKFILE_FAILED = 37,                  // NANDアプリのファイルシーク失敗
    FATAL_ERROR_LOAD_READHEADER_FAILED = 38,                // アプリROMヘッダロード失敗
    FATAL_ERROR_LOAD_LOGOCRC_ERROR = 39,                    // アプリROMヘッダNintendoロゴCRC不正
    FATAL_ERROR_LOAD_READDLSIGN_FAILED = 40,                // TMPブートアプリのDSダウンロードプレイ署名リード失敗
    FATAL_ERROR_LOAD_RELOCATEINFO_FAILED = 41,              // アプリ再配置情報生成失敗
    FATAL_ERROR_LOAD_READMODULE_FAILED = 42,                // アプリロード失敗
    FATAL_ERROR_NINTENDO_LOGO_CHECK_FAILED = 43,            // アプリROMヘッダNintendoロゴデータ不正
    FATAL_ERROR_SYSMENU_VERSION = 44,                       //
    FATAL_ERROR_DHT_PHASE1_CALC_FAILED = 45,                // NTRアプリホワイトリストハッシュ計算用メモリ確保失敗
    FATAL_ERROR_LOAD_UNKNOWN_BOOTTYPE = 46,                 // アプリブートタイプが不正
    FATAL_ERROR_LOAD_AUTH_HEADER_FAILED = 47,               // アプリROMヘッダ認証失敗
    FATAL_ERROR_LOAD_NEVER_STARTED = 48,                    // ロードが開始されていないのに、認証が開始された
    FATAL_ERROR_EJECT_CARD_AFTER_LOAD_START = 49,           // カードが抜かれているのに、カードアプリのロードが開始された
    FATAL_ERROR_TITLEID_COMPARE_FAILED_NTR = 50,            // ブート要求されたNTRアプリと実際にロードしたアプリのTitleIDが不一致
    FATAL_ERROR_DHT_PHASE3_FAILED = 51,                     // アプリのNTRホワイトリスト認証失敗（フェーズ３）
    FATAL_ERROR_DHT_PHASE4_FAILED = 52,                     // アプリのNTRホワイトリスト認証失敗（フェーズ４）
    FATAL_ERROR_BACKUP_DATA_CHECK_FAILED = 53,              // バックアップデータの検証失敗

    FATAL_ERROR_MAX = 53
}FatalErrorCode;


// global variable------------------------------------------------------
// function-------------------------------------------------------------

#ifdef SDK_ARM9

// バックライト
extern u32 UTL_SetBacklightBrightness( u8 brightness );                     // バックライト輝度セット
extern u32 UTL_GetBacklightBrightness( u8 *pBrightness );                   // バックライト輝度ゲット

// タッチパネル
extern void UTL_CaribrateTP( const LCFGTWLTPCalibData *pCalib );            // TPキャリブレーション
extern BOOL UTL_IsValidCalibration( u16 x, u16 y, u16 correct_x, u16 correct_y );   // TPキャリブレーション後にタッチしたポイントが正確か？

// スリープ
extern void UTL_GoSleepMode( void );

// RTC関係
extern BOOL UTL_CheckRTCDate( RTCDate *pDate );                             // 日付が正常かチェック
extern BOOL UTL_CheckRTCTime( RTCTime *pTime );                             // 時刻が正常かチェック
extern s64  UTL_CalcRTCOffset( RTCDate *pNewDate, RTCTime *pNewTime );      // RTCオフセット計算とRTCへの日付時刻チェックを行う
extern u32  UTL_GetDayNum( u32 year, u32 month );                           // 指定された年・月の日数を取得する
extern BOOL UTL_IsLeapYear100( u32 year );                                  // 指定された年がうるう年か調べる

// ペアレンタルコントロール問い合わせ
extern u32  UTL_CalcPCTLInquiryCode( void );                                // 問い合わせコード（１０進８桁）算出
extern u32  UTL_CalcPCTLMasterKey( void );                                  // マスターキー　　（１０進５桁）算出（※内部でRTC_GetDateを使用します。）

// アプリROMヘッダの要EULAフラグ取得
extern BOOL UTL_IsROMHeaderEULARequired( void );

// アプリROMヘッダのnintendoロゴの正当性チェック
extern BOOL UTL_CheckNintendoLogoData( ROM_Header_Short *rh );

#endif

// FATALエラー
extern BOOL UTL_IsFatalError( void );                                       // FATALエラーか？
extern void UTL_SetFatalError( FatalErrorCode error );                      // FATALエラーのセット
extern u64  UTL_GetFatalError( void );                                      // FATALエラー状態の取得（FatalErrorCodeをビットに割り当てて格納しています。）


// リージョンチェック
static inline BOOL UTL_CheckAppRegion( u32 card_region_bitmap )
{
#ifdef SYSM_BUILD_FOR_DEBUGGER
#pragma unused(card_region_bitmap)
	// デバッガ動作時のみ、リージョンチェックを無効にする。
	if( SYSM_IsRunOnDebugger() ) {
		return TRUE;
	}
#endif
    return ( card_region_bitmap & ( 0x00000001 << OS_GetRegion() ) ) ? TRUE : FALSE;
}

// CRCチェック
static inline BOOL UTL_CheckAppCRC16( ROM_Header_Short *pROMH )
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
