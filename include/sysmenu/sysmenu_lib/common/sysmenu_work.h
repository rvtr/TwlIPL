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
#include <sysmenu/reloc_info/common/reloc_info.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data ------------------------------------
typedef enum SYSMCloneBootMode {
    SYSM_CLONE_BOOT_MODE = 1,
    SYSM_OTHER_BOOT_MODE = 2
}
SYSMCloneBootMode;

// NAMTitleIDをHiLoに分割してアクセスする場合に使用
typedef struct TitleID_HiLo {
	u8			Lo[ 4 ];
	u32			Hi;
}TitleID_HiLo;


//----------------------------------------------------------------------
//　ROMエミュレーション情報
//----------------------------------------------------------------------
#define SYSM_ROMEMU_INFO_SIZE			0x20		// ROMエミュレーションデータサイズ
#define SYSM_ROMEMU_INFO_MAGIC_CODE		0x444c5754  // "TWLD"の文字列

// ISデバッガROMエミュレーション情報
typedef struct SYSMRomEmuInfo {
	// マジックコード（SYSM_ROMEMU_INFO_MAGIC_CODEの固定値）
	u32			magic_code;
	// フラグ類
	u32			isEnableSlot1 : 1;
	u32			isEnableSlot2 : 1;
	u32			bootSlotNo : 2;
	u32			isEnableExMainMemory : 1;
	u32			isBootMachineSettings : 1;
	u32			isBootSpecifiedNANDApp : 1;
	u32			isForceNTRMode : 1;
	u32			isForceBannerViewMode : 1;
	u32			rsv_flags : 23;
	// isBootSpecifiedNANDAppで起動するアプリのTitleID
	u64			titleID;
	// 予約
	u8			rsv[ 0x10 ];
}
SYSMRomEmuInfo;


//----------------------------------------------------------------------
//　SYSMワーク
//----------------------------------------------------------------------
// SYSM共有ワーク構造体
typedef struct SYSM_work {
	Relocate_Info		romRelocateInfo[RELOCATE_INFO_NUM];	// ROM再配置情報（arm9,arm7それぞれltdとflxで最大4つ）
	struct {
		struct {
			vu32		isFatalError :1;				// FATALエラー
			vu32		isARM9Start :1;					// ARM9スタートフラグ
			vu32		isHotStart :1;					// Hot/Coldスタート判定
			vu32		isValidLauncherParam :1;			// リセットパラメータ有効
			vu32		isValidTSD :1;					// NITRO設定データ無効フラグ
			vu32		isLogoSkip :1;					// ロゴデモスキップ
			vu32		isOnDebugger :1;				// デバッガ動作か？
			vu32		isLoadSucceeded :1;				// アプリロード完了？
			vu32		isCardBoot :1;					// カードブートか？
			vu32		isBrokenHWNormalInfo :1;		// HWノーマル情報が破損している。
			vu32		isBrokenHWSecureInfo :1;		// HWセキュア情報が破損している。
			vu32		isResetRTC :1;					// RTCリセット発生
			vu32		:0;
		}common;
        struct {
            vu16		isExistCard :1;					// 有効なNTR/TWLカードが存在するか？
			vu16		isEnableHotSW :1;				// 活線挿抜有効？
			vu16		isBusyHotSW :1;					// 活線挿抜処理中？
			vu16		isCardLoadCompleted :1;			// カードからデータロード完了？
   			vu16		isValidCardBanner :1;			// バナーデータ更新？
			vu16		is1stCardChecked :1;			// カードデータの1stチェック完了？
            vu16		:10;
            vu8			isCardStateChanged;				// カード状態更新フラグ
        }hotsw;
	}flags; // 7B

	u16					cardHeaderCrc16;				// カード検出時に算出したROMヘッダCRC16（ARM9側でコピーして使用する側）
	u16					cardHeaderCrc16_bak;			// カード検出時に算出したROMヘッダCRC16（ARM7側ライブラリでダイレクトに書き換わる側）
	
    OSLockWord			lockCardRsc ATTRIBUTE_ALIGN(8);	// カードリソース排他制御用
	OSLockWord			lockHotSW;						// カードリソース排他制御用
	u32					nCardID;						// カードID
	u32					gameCommondParam;				// NTRのゲームコマンドパラメータ(NTRのROMヘッダのゲームコマンドパラメータに上書きする)
	u8					cloneBootMode;
    
	LauncherParam		launcherParam;
	SYSMRomEmuInfo		romEmuInfo;
	
	// NTR-IPL2のレガシー　最終的には消すと思う
	BOOL				enableCardNormalOnly;
	u8					rtcStatus;
}SYSM_work;

typedef struct SYSM_work2 {
	SVCHMACSHA1Context hmac_sha1_context;
}SYSM_work2;

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
// SYSMリセットパラメータアドレスの取得（※ライブラリ向け。ARM9側はSYSM_GetLauncherParamを使用して下さい。）
#define SYSMi_GetLauncherParamAddr()			( (LauncherParam *)HW_PARAM_LAUNCH_PARAM )

// SYSM共有ワークの取得
#define SYSMi_GetWork()						( (SYSM_work *)HW_TWL_MAIN_MEM_SHARED )
#define SYSMi_GetWork2()					( (SYSM_work2 *)HW_MAIN_MEM_SHARED )

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

