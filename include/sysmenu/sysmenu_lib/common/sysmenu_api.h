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

#ifndef	__SYSM_LIB_H__
#define	__SYSM_LIB_H__

#include <twl.h>
#include <twl/nam.h>
#include <twl/os/common/format_rom.h>
#include <twl/os/common/banner.h>
#include <sysmenu/reloc_info/common/reloc_info.h>
#include <launcherParam_private.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

#ifndef SDK_FINALROM
//#define SYSM_DEBUG_												// デバッグコード用ビルドスイッチ
//#define ENABLE_INITIAL_SETTINGS_
#endif // SDK_FINALROM

#define CARD_SLOT_NUM							1					// カードスロット数
#define LAUNCHER_TITLE_LIST_NUM					( LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX + 1 )	// ランチャーのタイトルリスト数

#define SYSM_PAD_SHORTCUT_TP					( PAD_BUTTON_X )
#define SYSM_PAD_SHORTCUT_MACHINE_SETTINGS		( PAD_BUTTON_SELECT )
#define SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT	( PAD_BUTTON_A | PAD_BUTTON_B	\
												| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_R )
																	// 量産工程で使用する初回起動設定をキャンセルしてカードブートするショートカットキー

#define SYSM_MOUNT_INFO_SIZE				(0x400 - OS_MOUNT_PATH_LEN)
#define SYSM_LAUNCHER_VER					1	// ランチャーバージョン（SDK側でランチャーに絡む処理の判定用）

#define SYSM_ALIGNMENT_LOAD_MODULE			32	// モジュールをsrlから読み込む際のアライメント（AESおよびAESで使うDMAの仕様による）

// タイトル情報
typedef struct TitleProperty {			// この情報は、ランチャー時には認証通ってないけど、起動時には認証通すので大丈夫だろう。
	NAMTitleId			titleID;		// タイトルID（TitleID_Hiで起動メディアは判定できる？）
	LauncherBootFlags	flags;			// ブート時のランチャー動作フラグ
	TWLBannerFile		*pBanner;		// バナーへのポインタ（固定長フォーマットなら偽造されても大丈夫だろう。)
}TitleProperty;

// アプリ認証結果
typedef enum AuthResult {
	AUTH_RESULT_SUCCEEDED = 0,
	AUTH_RESULT_PROCESSING = 1,
	AUTH_RESULT_TITLE_LOAD_FAILED = 2,
	AUTH_RESULT_TITLE_POINTER_ERROR = 3,
	AUTH_RESULT_AUTHENTICATE_FAILED = 4,
	AUTH_RESULT_ENTRY_ADDRESS_ERROR = 5,
	AUTH_RESULT_TITLE_BOOTTYPE_ERROR = 6
}AuthResult;


// global variable------------------------------------------------------
#ifdef SDK_ARM9
extern const char *g_strIPLSvnRevision;
extern const char *g_strSDKSvnRevision;
extern void *SYSM_Alloc( u32 size );
extern void SYSM_Free( void *ptr );
//extern void *(*SYSM_Alloc)( u32 size );			// ライブラリ内部使用
//extern void  (*SYSM_Free)( void *ptr );			// 同上
#endif

// function-------------------------------------------------------------

#ifdef SDK_ARM9

// 初期化
extern void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) );			// 初期化。
extern void SYSM_InitPXI( void );												// PXI初期化
extern void SYSM_SetArena( void );												// システムメニューのアリーナ初期化。OS_Initの後で呼んでください。
extern void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) );	// SYSM_initで設定した場合は必要なし。
extern TitleProperty *SYSM_ReadParameters( void );								// 本体設定データ、ランチャーパラメータなどを取得

// アプリ情報取得
extern int  SYSM_GetCardTitleList( TitleProperty *pTitleList_Card );			// カードアプリタイトルリストの取得
extern BOOL SYSM_InitNandTitleList( void );										// NANDアプリタイトルリスト取得準備
extern void SYSM_FreeNandTitleList( void );										// NANDアプリタイトルリスト
extern int  SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int size );	// NAND  アプリタイトルリストの取得

// アプリ起動
extern void SYSM_StartLoadTitle( TitleProperty *pBootTitle );					// 指定したTitlePropertyを別スレッドでロード開始
extern BOOL SYSM_IsLoadTitleFinished( void );									// SYSM_StartLoadTitleで起動したスレッドが終了したかどうかを確認
extern void SYSM_StartAuthenticateTitle( TitleProperty *pBootTitle );			// 指定したTitlePropertyを別スレッドで検証開始
extern BOOL SYSM_IsAuthenticateTitleFinished( void );							// SYSM_StartAuthenticateTitleで起動したスレッドが終了したかどうかを確認
extern AuthResult SYSM_TryToBootTitle( TitleProperty *pBootTitle );				// pBootTitleで指定したタイトルをブート。成功時は、never return.

// AES領域デクリプト
extern void SYSM_StartDecryptAESRegion( ROM_Header_Short *hs );					// 起動するROMのAES暗号化領域のデクリプト開始
extern BOOL SYSM_InitDecryptAESRegion_W( ROM_Header_Short *hs );				// WRAM経由ファイル読み込みのコールバックで使うAESデクリプト処理の初期化
extern void SYSM_StartDecryptAESRegion_W( const void *wram_addr, const void *orig_addr, u32 size );
																				// WRAM経由ファイル読み込みのコールバックで使うAESデクリプト処理関数

// デバイス制御
extern void SYSM_CaribrateTP( void );											// タッチパネルキャリブレーション
extern void SYSM_SetBackLightBrightness( u8 brightness );						// バックライトを制御（本体設定データへの値セーブも行う）
extern u8   SYSM_GetBackLightBlightness( void );								// バックライト輝度を取得（X2以前とX3以降で挙動に違い）

// Nintendoロゴ制御
extern BOOL SYSM_CheckNintendoLogo( u16 *pLogoData );							// Nintendoロゴデータのチェック
extern void SYSM_LoadNintendoLogo2D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // NintendoロゴデータをOBJ_2D形式でロード（pTempBufferには0x700bytes必要)
extern void SYSM_LoadNintendoLogo1D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // NintendoロゴデータをOBJ_1D形式でロード（同上）

// RTC制御
extern BOOL SYSM_CheckRTCDate( RTCDate *pDate );								// 日付が正常かチェック
extern BOOL SYSM_CheckRTCTime( RTCTime *pTime );								// 時刻が正常かチェック
extern s64  SYSM_CalcRTCOffset( RTCDate *pNewDate, RTCTime *pNewTime );			// RTCオフセット計算とRTCへの日付時刻チェックを行う
extern u32  SYSM_GetDayNum( u32 year, u32 month );								// 指定された年・月の日数を取得する
extern BOOL SYSM_IsLeapYear100( u32 year );										// 指定された年がうるう年か調べる

// スリープモード
extern void SYSM_GoSleepMode( void );                                           // スリープモードへの遷移

// ペアレンタルコントロール問い合わせ
u32 SYSM_CalcPCTLInquiryCode( void );											// 問い合わせコード（１０進８桁）算出
u32 SYSM_CalcPCTLMasterKey( void );												// マスターキー　　（１０進５桁）算出（※内部でRTC_GetDateを使用します。）

#endif

// 状態チェック
extern BOOL SYSM_IsExistCard( void );											// TWL/NTRカードが差さっているか？（アプリは未認証状態）
extern BOOL SYSM_IsInspectCard( void );											// 検査カードが差さっているか？
extern BOOL SYSM_IsHotStart( void );											// ホットスタートか？
extern BOOL SYSM_IsFatalError( void );											// FATALエラーか？
extern void SYSM_SetFatalError( BOOL isFatalError );							// FATALエラーのセット
extern BOOL SYSM_IsLogoDemoSkip( void );										// ロゴデモ飛ばし状態か？
extern void SYSM_SetLogoDemoSkip( BOOL skip );									// ロゴデモ飛ばし状態フラグを設定する。
extern BOOL SYSM_IsValidTSD( void );											// TWL設定データは有効か？
extern void SYSM_SetValidTSD( BOOL valid );										// TWL設定データの有効／無効フラグを設定する。
extern const LauncherParamBody *SYSM_GetLauncherParamBody( void );				// ランチャーパラメータの取得
extern BOOL SYSM_IsRunOnDebugger( void );										// ISデバッガ上で動作しているか？

extern BOOL SYSM_IsLauncherHidden( void );										// ランチャーの画面を表示しないバージョンか？

// AES領域デクリプト
extern void SYSM_InitDecryptAESPXICallback( void );								// AES領域デクリプト用のPXIコールバック設定

#ifdef __cplusplus
}
#endif

#endif  // __SYSM_LIB_H__
