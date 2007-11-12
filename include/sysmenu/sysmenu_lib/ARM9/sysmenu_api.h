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
#include <sysmenu/settings.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

#define LAUNCHER_TITLE_LIST_NUM		40			// ランチャーのタイトルリスト数

// タイトル情報
typedef struct TitleProperty {	// この情報は、ランチャー時には認証通ってないけど、起動時には認証通すので大丈夫だろう。
	u64		titleID;			// タイトルID
	void	*pBanner;			// バナーへのポインタ（固定長フォーマットなら偽造されても大丈夫だろう。)
	u32		rsv;
}TitleProperty;


// リセットパラメータ
typedef struct ResetParam {
	u64			bootTitleID;	// 起動するタイトルがあるか？あるならそのタイトルID
	u32			rsv;
	BOOL		isLogoSkip;		// ロゴデモをスキップするか？
}ResetParam;


// アプリ認証結果
typedef enum AuthResult {
	AUTH_PROCESSING = 0,
	AUTH_RESULT_SUCCEEDED = 1,
	AUTH_RESULT_TITLE_POINTER_ERROR = 2,
	AUTH_RESULT_AUTHENTICATE_FAILED = 3,
	AUTH_RESULT_ENTRY_ADDRESS_ERROR = 4
}AuthResult;


// global variable------------------------------------------------------
extern void *(*SYSM_Alloc)( u32 size );
extern void  (*SYSM_Free)( void *ptr );

// function-------------------------------------------------------------
extern void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) );
//extern void SYSM_Init( void );
extern void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) );

extern BOOL SYSM_IsLogoDemoSkip( void );
extern void SYSM_CaribrateTP( void );
extern int  SYSM_GetCardTitleList( TitleProperty *pTitleList_Card );
extern int  SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int size );
extern AuthResult SYSM_LoadAndAuthenticateTitle( TitleProperty *pBootTitle );
extern void SYSM_Finalize( void );

extern BOOL SYSM_ReadTWLSettingsFile( void );
extern BOOL SYSM_WriteTWLSettingsFile( void );
extern void SYSM_VerifyAndRecoveryNTRSettings( void );
extern void SYSM_SetBackLightBrightness( void );


extern void SYSM_PermitToBootSelectedTarget( void );
extern void SYSM_LoadSYSMData( void );
extern BOOL SYSM_BootNITRO( void );
extern void SYSM_BootPictChat( void );
extern void SYSM_BootDSDownloadPlay( void );
extern void SYSM_BootMachineSetting( void );

extern BOOL SYSM_ReadBannerFile( BannerFile *banner );
extern BOOL SYSM_IsTPReadable( void );

extern BOOL SYSM_CheckNintendoLogo( u16 *logo_cardp );
extern void SYSM_LoadNintendoLogo2D( u16 *ninLogoDatap, u16 *dstp, u16 color, u32 *tempBuffp );	// tempBuffpには0x700byte必要です。
extern void SYSM_LoadNintendoLogo1D( u16 *ninLogoDatap, u16 *dstp, u16 color, u32 *tempBuffp ); // 同上。

extern void SYSM_SetBootFlag( u32 value );
extern void SYSM_ClearBootFlag( u32 value );


extern void SYSM_GoSleepMode( void );


extern void NCD_ClearOwnerInfo( void );								// ニックネーム・誕生日・好きな色のクリア
extern BOOL SYSM_CheckRTCDate( RTCDate *datep );
extern BOOL SYSM_CheckRTCTime( RTCTime *timep );
extern s64  SYSM_CalcRTCOffsetAndSetDateTime( RTCDate *newDate, RTCTime *newTime );
extern u32  SYSM_GetDayNum( u32 year, u32 month );
extern BOOL SYSM_IsLeapYear100( u32 year );

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
