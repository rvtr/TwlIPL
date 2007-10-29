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
#include <sysmenu/rom_header.h>
#include <sysmenu/banner.h>
#include <sysmenu/sysmenu_work.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

#define	SYSM_CTRDG_DMA_NO		0		// カートリッジ周辺機器情報の読み込みに使用するDMA番号です。
										// 同時に使用するDMA番号の中で最も優先が高い番号にして下さい。
										// 他のDMAへ割り込まれると、読み込みに失敗する可能性があります。


// global variable------------------------------------------------------

// function-------------------------------------------------------------
extern void SYSM_Init( void );
extern BOOL SYSM_WaitARM7Init( void );
extern s32  SYSM_Main( void );

extern void SYSM_PermitToBootSelectedTarget( void );
extern void SYSM_LoadSYSMData( void );
extern BOOL SYSM_BootNITRO( void );
extern void SYSM_BootPictChat( void );
extern void SYSM_BootDSDownloadPlay( void );
extern void SYSM_BootMachineSetting( void );

extern BOOL SYSM_ReadBannerFile( BannerFile *banner );
extern BOOL SYSM_IsTPReadable( void );

extern BOOL SYSM_CheckNinLogo( u16 *logo_cardp );
extern void SYSM_LoadNintendoLogo2D( u16 *ninLogoDatap, u16 *dstp, u16 color, u32 *tempBuffp );	// tempBuffpには0x700byte必要です。
extern void SYSM_LoadNintendoLogo1D( u16 *ninLogoDatap, u16 *dstp, u16 color, u32 *tempBuffp ); // 同上。

extern void SYSM_SetBootFlag( u32 value );
extern void SYSM_ClearBootFlag( u32 value );


extern void SYSM_GoSleepMode( void );
extern void PMm_SetBackLightBrightness( void );


extern void NCD_ClearOwnerInfo( void );								// ニックネーム・誕生日・好きな色のクリア
extern BOOL SYSM_CheckRTCDate( RTCDate *datep );
extern BOOL SYSM_CheckRTCTime( RTCTime *timep );
extern s64  SYSM_CalcRtcOffsetAndSetDateTime( RTCDate *newDate, RTCTime *newTime );
extern u32  SYSM_GetDayNum( u32 year, u32 month );
extern BOOL SYSM_IsLeapYear100( u32 year );

extern BOOL SYSM_IsDebuggerBannerViewMode( void );

// ※以下の関数は、SYSM_Mainがコールされた後に正しい値が取得できるようになります。

// NITROカードが差さっているか？
static inline BOOL SYSM_IsNITROCard( void )
{
	return (SYSM_GetBootFlag() & BFLG_EXIST_NITRO_CARD) ? TRUE : FALSE;
}

// 検査用NITROカードが差さっているか？
static inline BOOL SYSM_IsInspectNITROCard( void )
{
	return ( (SYSM_IsNITROCard()) && (GetRomHeaderAddr()->inspectCard) );
}


#ifdef __cplusplus
}
#endif

#endif  // __SYSM_LIB_H__
