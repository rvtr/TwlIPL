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
#include <twl/rtc.h>

#ifdef __cplusplus
extern "C" {
#endif

// compile switch ---------------------------------
#ifndef SDK_FINALROM

//#define __SYSM_DEBUG

#endif // SDK_FINALROM

//#define __DEBUG_SECURITY_CODE										// PassMeのセキュリティコード確認用スイッチ


// define data ------------------------------------
#define SYSMENU_VER							0x071113				// SystemMenuバージョン

#define PAD_PRODUCTION_SKIP_INITIAL_SHORTCUT	( PAD_BUTTON_A | PAD_BUTTON_B	\
												| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_R )
																	// 量産工程で使用する初回起動設定をキャンセルするショートカットキー


	// bootFlagの値
#define BFLG_EXIST_CARD						0x00000001


#define cloneBootMode						1
#define OTHER_BOOT_MODE						2


//----------------------------------------------------------------------
//　データ型定義
//----------------------------------------------------------------------

// ロック情報
typedef struct LockVariable{
	OSLockWord			lock;
	vu32				value;
}LockVariable;

// RTC日付時刻構造体
typedef struct RtcDateTime {
	RTCDate				Date;
	RTCTime				Time;
}RtcDateTime;

// SYSM共有ワーク構造体
typedef struct SYSM_work {
	BOOL				isValidTSD;						// NITRO設定データ無効フラグ
	BOOL				isOnDebugger;					// デバッガ動作か？
	BOOL				isExistCard;					// 有効なNTR/TWLカードが存在するか？
	u16					cardHeaderCrc16;				// システムメニューで計算したROMヘッダCRC16
	u8					CloneBootMode;
	
	// NTR-IPL2のレガシー　最終的には消すと思う
	u32					nCardID;
	BOOL				enableCardNormalOnly;
	u8					rtcStatus;
}SYSM_work;


//----------------------------------------------------------------------
//　SYSM共有ワーク領域のアドレス獲得
//----------------------------------------------------------------------
#if 1
#define SYSM_GetResetParam()		( (ResetParam *)HW_RED_RESERVED )

#define GetSYSMWork()				( (SYSM_work *)( HW_RED_RESERVED + sizeof(ResetParam) ) )
#else
// SYSMリセットパラメータの取得
#define SYSM_GetResetParam()		( (ResetParam *)0x02000100 )

// SYSM共有ワークの取得
#define SYSM_GetWork()				( (SYSM_work *)HW_RED_RESERVED )
#endif

// カードROMヘッダワークの取得
#define SYSM_GetCardRomHeader()		( (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF )



inline void SYSM_SetBootFlag( u32 flag )
{
#pragma unused( flag )
}

// カードROMヘッダワークの取得
#define SYSM_GetCardRomHeader()		( (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF )

#ifdef __cplusplus
}
#endif

#endif		// __SYSMENU_WORK_H__

