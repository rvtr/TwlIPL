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
#include <twl/os/common/msJump.h>
#include <sysmenu/reloc_info/common/reloc_info.h>
#include <application_jump_private.h>

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

#define SYSM_PAD_SHORTCUT_TP_CALIBRATION		( PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START )
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
	u8					agree_EULA:1;
	u8					availableSubBannerFile:1;
	u8					WiFiConnectionIcon:1;
	u8					DSWirelessIcon:1;
	u8					rsv:4;
	char				platform_code;
	u8					parental_control_rating_info[0x10];
	u32					card_region_bitmap;
}TitleProperty;

// タイトルリスト作成用情報構造体
typedef struct TitleListMakerInfo {
	char				makerCode[MAKER_CODE_MAX];
	u32					public_save_data_size;
	u32					private_save_data_size;
	BOOL				permit_landing_normal_jump;
	u8					agree_EULA:1;
	u8					availableSubBannerFile:1;
	u8					WiFiConnectionIcon:1;
	u8					DSWirelessIcon:1;
	u8					rsv:4;
	char				platform_code;
	u8					parental_control_rating_info[0x10];
	u32					card_region_bitmap;
}TitleListMakerInfo;

// アプリ認証結果
typedef enum AuthResult {
	AUTH_RESULT_SUCCEEDED = 0,
	AUTH_RESULT_PROCESSING = 1,
	AUTH_RESULT_TITLE_LOAD_FAILED = 2,
	AUTH_RESULT_TITLE_POINTER_ERROR = 3,
	AUTH_RESULT_AUTHENTICATE_FAILED = 4,
	AUTH_RESULT_ENTRY_ADDRESS_ERROR = 5,
	AUTH_RESULT_TITLE_BOOTTYPE_ERROR = 6,
	AUTH_RESULT_SIGN_DECRYPTION_FAILED = 7,
	AUTH_RESULT_SIGN_COMPARE_FAILED = 8,
	AUTH_RESULT_HEADER_HASH_CALC_FAILED = 9,
	AUTH_RESULT_TITLEID_COMPARE_FAILED = 10,
	AUTH_RESULT_VALID_SIGN_FLAG_OFF = 11,
	AUTH_RESULT_CHECK_TITLE_LAUNCH_RIGHTS_FAILED = 12,
	AUTH_RESULT_MODULE_HASH_CHECK_FAILED = 13,
	AUTH_RESULT_MODULE_HASH_CALC_FAILED = 14,
	AUTH_RESULT_MEDIA_CHECK_FAILED = 15,
	AUTH_RESULT_DL_MAGICCODE_CHECK_FAILED = 16,
	AUTH_RESULT_DL_SIGN_DECRYPTION_FAILED = 17,
	AUTH_RESULT_DL_HASH_CALC_FAILED = 18,
	AUTH_RESULT_DL_SIGN_COMPARE_FAILED = 19,
	AUTH_RESULT_WHITELIST_INITDB_FAILED = 20,
	AUTH_RESULT_WHITELIST_NOTFOUND = 21,
	AUTH_RESULT_DHT_PHASE1_FAILED = 22,
	AUTH_RESULT_DHT_PHASE2_FAILED = 23,
	AUTH_RESULT_LANDING_TMP_JUMP_FLAG_OFF = 24,
	AUTH_RESULT_TWL_BOOTTYPE_UNKNOWN = 25,
	AUTH_RESULT_NTR_BOOTTYPE_UNKNOWN = 26,
	AUTH_RESULT_PLATFORM_UNKNOWN = 27,
	
	AUTH_RESULT_MAX = 28
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
extern void SYSM_DeleteTmpDirectory( TitleProperty *pBootTitle );              // "nand:/tmp"フォルダのクリーン

// アプリ情報取得
extern int  SYSM_GetCardTitleList( TitleProperty *pTitleList_Card );			// カードアプリタイトルリストの取得
extern BOOL SYSM_InitNandTitleList( void );										// NANDアプリタイトルリスト取得準備
extern void SYSM_FreeNandTitleList( void );										// NANDアプリタイトルリスト
extern int  SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int size );	// NAND  アプリタイトルリストの取得
extern void SYSM_GetNandTitleListMakerInfo( void );								// アプリ引き渡しタイトルリスト作成用情報の取得（ダイレクトブート用）

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
// Nintendoロゴ制御
extern BOOL SYSM_CheckNintendoLogo( u16 *pLogoData );							// Nintendoロゴデータのチェック
extern void SYSM_LoadNintendoLogo2D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // NintendoロゴデータをOBJ_2D形式でロード（pTempBufferには0x700bytes必要)
extern void SYSM_LoadNintendoLogo1D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // NintendoロゴデータをOBJ_1D形式でロード（同上）

extern s32 SYSMi_getCheckTitleLaunchRightsResult( void );						// CheckTitleLaunchRightsの結果を返す（デバグ用）

extern BOOL SYSM_IsLoadTitlePaused(void);										// ローディングスレッドが一時停止しているか？
extern void SYSM_ResumeLoadingThread( BOOL force );								// ローディングスレッドが一時停止していたら再開

extern BOOL SYSM_MakeTitleListMakerInfoFromHeader( TitleListMakerInfo *info, ROM_Header_Short *hs);
																				// アプリ引き渡しタイトルリスト作成用情報をヘッダ情報から作成

#endif

// 状態チェック
extern BOOL SYSM_IsExistCard( void );											// TWL/NTRカードが差さっているか？（アプリは未認証状態）
extern BOOL SYSM_IsInspectCard( void );											// 検査カードが差さっているか？
extern BOOL SYSM_IsHotStart( void );											// ホットスタートか？
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
