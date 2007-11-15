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

#ifdef __cplusplus
extern "C" {
#endif

// compile switch ---------------------------------
//#define SYSM_RESET_PARAM_READY_

// define data ------------------------------------
#define CLONE_BOOT_MODE						1
#define OTHER_BOOT_MODE						2


// タイトル情報フラグ
typedef struct TitleFlags {
	u16			platform : 4;
	u16			media    : 4;
	u16			isLogoSkip : 1;
	u16			rsv : 7;
}TitleFlags;


// リセットパラメータ
typedef struct ResetParam {
	NAMTitleId	bootTitleID;	// 起動するタイトルがあるか？あるならそのタイトルID
	u32			rsv_A;
	TitleFlags	flags;
	u8			rsv_B[ 2 ];
}ResetParam;


//----------------------------------------------------------------------
//　データ型定義
//----------------------------------------------------------------------

// SYSM共有ワーク構造体
typedef struct SYSM_work {
	volatile BOOL	isARM9Start;					// ARM9スタートフラグ
	BOOL			isHotStart;						// Hot/Coldスタート判定
	BOOL			isValidTSD;						// NITRO設定データ無効フラグ
	BOOL			isOnDebugger;					// デバッガ動作か？
	BOOL			isExistCard;					// 有効なNTR/TWLカードが存在するか？
	u16				cardHeaderCrc16;				// システムメニューで計算したROMヘッダCRC16
	int				cloneBootMode;
	ResetParam		resetParam;
	
	// NTR-IPL2のレガシー　最終的には消すと思う
	u32				nCardID;
	BOOL			enableCardNormalOnly;
	u8				rtcStatus;
}SYSM_work;


//----------------------------------------------------------------------
//　SYSM共有ワーク領域のアドレス獲得
//----------------------------------------------------------------------
#ifdef SYSM_RESET_PARAM_READY_
// SYSMリセットパラメータの取得（※ライブラリ向け。ARM9側はSYSM_GetResetParamを使用して下さい。）
#define SYSMi_GetResetParam()		( (ResetParam *)0x02000100 )
// SYSM共有ワークの取得
#define SYSMi_GetWork()				( (SYSM_work *)HW_RED_RESERVED )
#else	// SYSM_RESET_PARAM_READY_
#define SYSMi_GetResetParam()		( (ResetParam *)HW_RED_RESERVED )
#define SYSMi_GetWork()				( (SYSM_work *)( HW_RED_RESERVED + 0x40 ) )
#endif	// SYSM_RESET_PARAM_READY_

// カードROMヘッダワークの取得
#define SYSM_GetCardRomHeader()		( (ROM_Header_Short *)HW_TWL_ROM_HEADER_BUF )

#ifdef __cplusplus
}
#endif

#endif // __SYSMENU_WORK_H__

