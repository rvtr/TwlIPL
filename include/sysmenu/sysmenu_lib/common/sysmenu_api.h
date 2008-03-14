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
#define LAUNCHER_TITLE_LIST_NUM					40					// ランチャーのタイトルリスト数

#define TITLE_ID_LAUNCHER						( 0x000300074c4e4352LLU )	// ランチャーのタイトルID
#define TITLE_ID_MACHINE_SETTINGS				( 0x000300054d534554LLU )	// 本体設定のタイトルID

#define SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT	( PAD_BUTTON_A | PAD_BUTTON_B	\
												| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_R )
																	// 量産工程で使用する初回起動設定をキャンセルしてカードブートするショートカットキー

typedef enum PlatformCode {
	PLATFORM_NTR = 0,
	PLATFORM_TWL = 1
}PlatformCode;


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
extern TitleProperty *SYSM_ReadParameters( void );								// 本体設定データ、リセットパラメータなどを取得

// アプリ情報取得
extern int  SYSM_GetCardTitleList( TitleProperty *pTitleList_Card );			// カードアプリタイトルリストの取得
extern int  SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int size );	// NAND  アプリタイトルリストの取得

// アプリ起動
extern void SYSM_StartLoadTitle( TitleProperty *pBootTitle );					// 指定したTitlePropertyを別スレッドでロード開始
extern BOOL SYSM_IsLoadTitleFinished( void );									// SYSM_StartLoadTitleで起動したスレッドが終了したかどうかを確認
extern void SYSM_StartAuthenticateTitle( TitleProperty *pBootTitle );			// 指定したTitlePropertyを別スレッドで検証開始
extern BOOL SYSM_IsAuthenticateTitleFinished( void );							// SYSM_StartAuthenticateTitleで起動したスレッドが終了したかどうかを確認
extern AuthResult SYSM_TryToBootTitle( TitleProperty *pBootTitle );				// 指定したTitlePropertyをブート
																				// 成功時は、never return.
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

#endif

// 状態チェック
extern BOOL SYSM_IsExistCard( void );											// TWL/NTRカードが差さっているか？（アプリは未認証状態）
extern BOOL SYSM_IsInspectCard( void );											// 検査カードが差さっているか？
extern BOOL SYSM_IsTPReadable( void );											// TPリード可能か？
extern BOOL SYSM_IsLogoDemoSkip( void );										// ロゴデモ飛ばし状態か？
extern void SYSM_SetLogoDemoSkip( BOOL skip );									// ロゴデモ飛ばし状態フラグを設定する。
extern BOOL SYSM_IsValidTSD( void );											// TWL設定データは有効か？
extern void SYSM_SetValidTSD( BOOL valid );										// TWL設定データの有効／無効フラグを設定する。
extern const LauncherParamBody *SYSM_GetLauncherParamBody( void );				// リセットパラメータの取得
extern BOOL SYSM_IsRunOnDebugger( void );										// ISデバッガ上で動作しているか？

extern BOOL SYSM_IsLauncherHidden( void );										// ランチャーの画面を表示しないバージョンか？

#ifdef __cplusplus
}
#endif

#endif  // __SYSM_LIB_H__
