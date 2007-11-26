/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     sysmenu_work.c

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

#ifndef	__SYSMENU_WORK_H__
#define	__SYSMENU_WORK_H__

#include <twl.h>
#include <twl/nam.h>

#include <sysmenu/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------
#define SYSM_RESET_PARAM_MAGIC_CODE			"TRST"
#define SYSM_RESET_PARAM_MAGIC_CODE_LEN		4

#define CLONE_BOOT_MODE						1
#define OTHER_BOOT_MODE						2


// NAMTitleIDをHiLoに分割してアクセスする場合に使用
typedef struct TitleID_HiLo {
	u8			Lo[ 4 ];
	u32			Hi;
}TitleID_HiLo;


// BootFlagsで使用するmedia情報
typedef enum TitleMedia {
	TITLE_MEDIA_NAND = 0,
	TITLE_MEDIA_CARD = 1,
	TITLE_MEDIA_MAX  = 2
}TitleMedia;


// タイトル＆リセットパラメータ　フラグ
typedef struct BootFlags {
	u16			isValid : 1;				// TRUE:valid, FALSE:invalid
	u16			media : 3;					// 0:nand, 1:card, 2-7:rsv;
	u16			isLogoSkip : 1;				// ロゴデモスキップ要求
	u16			isInitialShortcutSkip : 1;	// 初回起動シーケンススキップ要求
	u16			isAppLoadCompleted : 1;		// アプリロード済みを示す
	u16			isAppRelocate : 1;			// アプリ再配置要求
	u16			rsv : 9;
}BootFlags;


// リセットパラメータ　ヘッダ
typedef struct ResetParameterHeader {
	u32			magicCode;				// SYSM_RESET_PARAM_MAGIC_CODEが入る
	u8			type;					// タイプによってBodyを判別する。
	u8			bodyLength;				// bodyの長さ
	u16			crc16;					// bodyのCRC16
}ResetParamHeader;


// リセットパラメータ　ボディ
typedef union ResetParamBody {
	struct {							// ※とりあえず最初はTitlePropertyとフォーマットを合わせておく
		NAMTitleId	bootTitleID;		// リセット後にダイレクト起動するタイトルID
		BootFlags	flags;				// リセット時のランチャー動作フラグ
		u8			rsv[ 4 ];			// 予約
	}v1;
}ResetParamBody;


// リセットパラメータ
typedef struct ResetParam {
	ResetParamHeader	header;
	ResetParamBody		body;
}ResetParam;


//----------------------------------------------------------------------
//　データ型定義
//----------------------------------------------------------------------

// SYSM共有ワーク構造体
typedef struct SYSM_work {
	vu16			isARM9Start :1;					// ARM9スタートフラグ
	vu16			isHotStart :1;					// Hot/Coldスタート判定
	vu16			isValidResetParam :1;			// リセットパラメータ有効
	vu16			isValidTSD :1;					// NITRO設定データ無効フラグ
	vu16			isLogoSkip :1;					// ロゴデモスキップ
	vu16			isOnDebugger :1;				// デバッガ動作か？
	vu16			isExistCard :1;					// 有効なNTR/TWLカードが存在するか？
	vu16			isCardStateChanged :1;			// カード状態更新フラグ
	vu16			isLoadSucceeded :1;
#ifdef DEBUG_USED_CARD_SLOT_B_
	vu16			isValidCardBanner :1;
	vu16			is1stCardChecked :1;
	vu16			rsv :5;
#else
	vu16			rsv :7;
#endif
	
	u16				cardHeaderCrc16;				// カード検出時に算出したROMヘッダCRC16（ARM9側でコピーして使用する側）
	u16				cardHeaderCrc16_bak;			// カード検出時に算出したROMヘッダCRC16（ARM7側ライブラリでダイレクトに書き換わる側）
	OSLockWord		lockCardRsc;					// カードリソース排他制御用
	int				cloneBootMode;
	u32				nCardID;						// カードID
	
	ResetParam		resetParam;
	
	// NTR-IPL2のレガシー　最終的には消すと思う
	BOOL			enableCardNormalOnly;
	u8				rtcStatus;
}SYSM_work;

// NTRにおける仕様を継承する必要のあるワーク
typedef struct SDKBootCheckInfo{
	u32 nCardID;					// NORMALカードID				// SDKではここだけ見ているっぽい　※最終的にはランチャーでここにカードIDをセットする
	u32 sCardID;					// SECUREカードID
	u16 cardHeaderCrc16;			// カードヘッダCRC16
	u16 cardSecureCrc16;			// カードSECURE領域CRC16
	s16 cardHeaderError;			// カードヘッダエラー
	s16 disableEncryptedCardData;	// カードSECURE領域暗号化データ無効
	
	u16 sysromCrc16;				// システムROMのCRC16
	s16 enableCardNormalOnly;		// カードNORMALモードのみ有効
	s16 isOnDebugger;				// デバッガ上で動作中か
	s8  rtcError;					// RTCエラー
	u8  rtcStatus1;					// RTCステータス1
	
}SDKBootCheckInfo;

//----------------------------------------------------------------------
//　SYSM共有ワーク領域のアドレス獲得
//----------------------------------------------------------------------
// SYSMリセットパラメータアドレスの取得（※ライブラリ向け。ARM9側はSYSM_GetResetParamを使用して下さい。）
#define SYSMi_GetResetParamAddr()			( (ResetParam *)0x02000100 )

#if 0
// SYSM共有ワークの取得
#define SYSMi_GetWork()						( (SYSM_work *)HW_RED_RESERVED )
#else
#define SYSMi_GetWork()						( (SYSM_work *)( HW_RED_RESERVED + 0x10 ) )
#endif

// SDKブートチェック（アプリ起動時にカードIDをセットする必要がある。）
#define SYSMi_GetSDKBootCheckInfo()			( (SDKBootCheckInfo *)HW_BOOT_CHECK_INFO_BUF )
#define SYSMi_GetSDKBootCheckInfoForNTR()	( (SDKBootCheckInfo *)0x027ffc00 )

// NANDファームがロードしてくれているマイコンフリーレジスタ値の取得
#define SYSMi_GetMCUFreeRegisterValue()		( *(vu8 *)HW_RESET_PARAMETER_BUF )

// カードROMヘッダワークの取得
#define SYSM_GetCardRomHeader()				( (ROM_Header_Short *)SYSM_CARD_ROM_HEADER_BUF )


#ifdef __cplusplus
}
#endif

#endif // __SYSMENU_WORK_H__

