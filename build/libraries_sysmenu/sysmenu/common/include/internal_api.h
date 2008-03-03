/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     internal_api.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef SYSM_INTERNAL_API_H_
#define SYSM_INTERNAL_API_H_

#include <twl.h>
#include <sysmenu/sysmenu_lib/common/pxi.h>

#ifdef __cplusplus
extern "C" {
#endif

//#define SYSM_LIB_NO_MESSAGE					// Printf抑制スイッチ

#ifdef	SYSM_LIB_NO_MESSAGE
#define OS_Printf( ... )					((void)0)
#define OS_TPrintf( ... )					((void)0)
#define OS_PutString( ... )					((void)0)
#endif

#ifdef SDK_ARM9
//-------------------------------------------------------
// マウント情報セット
//-------------------------------------------------------

// ランチャー用
void SYSMi_SetLauncherMountInfo( void );

// 起動アプリ用
void SYSMi_SetBootAppMountInfo( TitleProperty *pBootTitle );


//-------------------------------------------------------
// デバイス
//-------------------------------------------------------

// RTC補正
void SYSMi_WriteAdjustRTC( void );

// RTCチェック
void SYSMi_CheckRTC( void );


//-------------------------------------------------------
// バナー
//-------------------------------------------------------

// カードバナーリード（※NTR-IPL2仕様）
BOOL SYSMi_ReadCardBannerFile( u32 bannerOffset, TWLBannerFile *pBanner );

// NANDアプリバナーリード
BOOL SYSMi_ReadBanner_NAND( NAMTitleId titleID, TWLBannerFile *pDst );

//-------------------------------------------------------
// 活線挿抜
//-------------------------------------------------------
void SYSMi_EnableHotSW( BOOL enable );


#endif


//=======================================================
//
// ARM9/ARM7共通API
//
//=======================================================
BOOL SYSMi_IsDebuggerBannerViewMode( void );
BOOL SYSMi_CheckEntryAddress( void );
BOOL SYSMi_CopyCardRomHeader( void );
BOOL SYSMi_CopyCardBanner( void );


#ifdef __cplusplus
}
#endif

#endif  // SYSM_INTERNAL_API_H_
