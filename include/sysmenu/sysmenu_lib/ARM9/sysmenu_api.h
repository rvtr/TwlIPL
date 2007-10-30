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

// �^�C�g�����
typedef struct TitleProperty {	// ���̏��́A�����`���[���ɂ͔F�ؒʂ��ĂȂ����ǁA�N�����ɂ͔F�ؒʂ��̂ő��v���낤�B
	u64		titleID;			// �^�C�g��ID
	void	*pBanner;			// �o�i�[�ւ̃|�C���^�i�Œ蒷�t�H�[�}�b�g�Ȃ�U������Ă����v���낤�B)
	u32		rsv;
}TitleProperty;


// ���Z�b�g�p�����[�^
typedef struct ResetParam {
	u64			bootTitleID;	// �N������^�C�g�������邩�H����Ȃ炻�̃^�C�g��ID
	u32			rsv;
	BOOL		isLogoSkip;		// ���S�f�����X�L�b�v���邩�H
}ResetParam;


// �A�v���F�،���
typedef enum AuthResult {
	AUTH_PROCESSING = 0,
	AUTH_RESULT_SUCCEEDED = 1,
	AUTH_RESULT_TITLE_POINTER_ERROR = 2,
	AUTH_RESULT_AUTHENTICATE_FAILED = 3,
	AUTH_RESULT_ENTRY_ADDRESS_ERROR = 4
}AuthResult;


// global variable------------------------------------------------------

// function-------------------------------------------------------------
extern void SYSM_Init( void );
extern BOOL SYSM_IsLogoDemoSkip( void );
extern void SYSM_CaribrateTP( void );
extern int  SYSM_GetCardTitleList( TitleProperty *pTitleList_Card );
extern int  SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand );
extern AuthResult SYSM_LoadAndAuthenticateTitle( TitleProperty *pBootTitle );
extern void SYSM_Finalize( void );

extern void SYSM_PermitToBootSelectedTarget( void );
extern void SYSM_LoadSYSMData( void );
extern BOOL SYSM_BootNITRO( void );
extern void SYSM_BootPictChat( void );
extern void SYSM_BootDSDownloadPlay( void );
extern void SYSM_BootMachineSetting( void );

extern BOOL SYSM_ReadBannerFile( BannerFile *banner );
extern BOOL SYSM_IsTPReadable( void );

extern BOOL SYSM_CheckNinLogo( u16 *logo_cardp );
extern void SYSM_LoadNintendoLogo2D( u16 *ninLogoDatap, u16 *dstp, u16 color, u32 *tempBuffp );	// tempBuffp�ɂ�0x700byte�K�v�ł��B
extern void SYSM_LoadNintendoLogo1D( u16 *ninLogoDatap, u16 *dstp, u16 color, u32 *tempBuffp ); // ����B

extern void SYSM_SetBootFlag( u32 value );
extern void SYSM_ClearBootFlag( u32 value );


extern void SYSM_GoSleepMode( void );
extern void PMm_SetBackLightBrightness( void );


extern void NCD_ClearOwnerInfo( void );								// �j�b�N�l�[���E�a�����E�D���ȐF�̃N���A
extern BOOL SYSM_CheckRTCDate( RTCDate *datep );
extern BOOL SYSM_CheckRTCTime( RTCTime *timep );
extern s64  SYSM_CalcRtcOffsetAndSetDateTime( RTCDate *newDate, RTCTime *newTime );
extern u32  SYSM_GetDayNum( u32 year, u32 month );
extern BOOL SYSM_IsLeapYear100( u32 year );

// ���ȉ��̊֐��́ASYSM_Main���R�[�����ꂽ��ɐ������l���擾�ł���悤�ɂȂ�܂��B

// NITRO�J�[�h���������Ă��邩�H
static inline BOOL SYSM_IsNITROCard( void )
{
	return (SYSM_GetBootFlag() & BFLG_EXIST_NITRO_CARD) ? TRUE : FALSE;
}

// �����pNITRO�J�[�h���������Ă��邩�H
static inline BOOL SYSM_IsInspectNITROCard( void )
{
	return ( (SYSM_IsNITROCard()) && (GetRomHeaderAddr()->inspectCard) );
}


#ifdef __cplusplus
}
#endif

#endif  // __SYSM_LIB_H__
