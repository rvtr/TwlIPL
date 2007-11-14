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
#include <twl/os/common/format_rom.h>
#include <sysmenu/banner.h>
#include <sysmenu/sysmenu_work.h>
#include <sysmenu/settings.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

#define CARD_SLOT_NUM				1			// �J�[�h�X���b�g��
#define LAUNCHER_TITLE_LIST_NUM		40			// �����`���[�̃^�C�g�����X�g��

typedef enum PlatformCode {
	PLATFORM_NTR = 0,
	PLATFORM_TWL = 1
}PlatformCode;


// �^�C�g�����t���O
typedef struct TitleFlags {
	u16			platform : 4;
	u16			media    : 4;
	u16			isLogoSkip : 1;
	u16			rsv : 7;
}TitleFlags;


// �^�C�g�����
typedef struct TitleProperty {	// ���̏��́A�����`���[���ɂ͔F�ؒʂ��ĂȂ����ǁA�N�����ɂ͔F�ؒʂ��̂ő��v���낤�B
	u64			titleID;						// �^�C�g��ID�iTitleID_Hi�ŋN�����f�B�A�͔���ł���H�j
	void	*pBanner;			// �o�i�[�ւ̃|�C���^�i�Œ蒷�t�H�[�}�b�g�Ȃ�U������Ă����v���낤�B)
	TitleFlags	flags;
	u8			rsv[ 2 ];
}TitleProperty;


// ���Z�b�g�p�����[�^
typedef struct ResetParam {
	u64			bootTitleID;	// �N������^�C�g�������邩�H����Ȃ炻�̃^�C�g��ID
	u32			rsv_A;
	TitleFlags	flags;
	u8			rsv_B[ 2 ];
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
extern void *(*SYSM_Alloc)( u32 size );
extern void  (*SYSM_Free)( void *ptr );

// function-------------------------------------------------------------

// ������
extern void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) );			// �������BOS_Init�̑O�̂ւ�ŃR�[�����Ă��������B
extern void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) );	// SYSM_init�Őݒ肵���ꍇ�͕K�v�Ȃ��B
extern void SYSM_ReadParameters( void );										// �{�̐ݒ�f�[�^�A���Z�b�g�p�����[�^�Ȃǂ��擾

// �A�v�����擾
extern int  SYSM_GetCardTitleList( TitleProperty *pTitleList_Card );			// �J�[�h�A�v���^�C�g�����X�g�̎擾
extern int  SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int size );	// NAND  �A�v���^�C�g�����X�g�̎擾

// �A�v���N��
extern AuthResult SYSM_LoadAndAuthenticateTitle( TitleProperty *pBootTitle );	// �w�肵��TitleProperty�����[�h���F�؂��ău�[�g
																				// �������́Anever return.
// �f�o�C�X����
extern void SYSM_CaribrateTP( void );											// �^�b�`�p�l���L�����u���[�V����
extern void SYSM_SetBackLightBrightness( void );						// �o�b�N���C�g�𐧌�i�{�̐ݒ�f�[�^�ւ̒l�Z�[�u���s���j

// ��ԃ`�F�b�N
extern BOOL SYSM_IsExistCard( void );											// TWL/NTR�J�[�h���������Ă��邩�H�i�A�v���͖��F�؏�ԁj
extern BOOL SYSM_IsInspectCard( void );											// �����J�[�h���������Ă��邩�H
extern BOOL SYSM_IsTPReadable( void );											// TP���[�h�\���H
extern BOOL SYSM_IsLogoDemoSkip( void );										// ���S�f����΂���Ԃ��H
extern void SYSM_SetLogoDemoSkip( BOOL skip );									// ���S�f����΂���Ԃ�ݒ肷��B

// �{�̐ݒ�f�[�^�A�N�Z�X
extern BOOL SYSM_ReadTWLSettingsFile( void );									// TWL�ݒ�f�[�^�̃��[�h
extern BOOL SYSM_WriteTWLSettingsFile( void );									// TWL�ݒ�f�[�^�̃��C�g
extern void SYSM_VerifyAndRecoveryNTRSettings( void );

// Nintendo���S����
extern BOOL SYSM_CheckNintendoLogo( u16 *pLogoData );							// Nintendo���S�f�[�^�̃`�F�b�N
extern void SYSM_LoadNintendoLogo2D( u16 *pLogoData, u16 *pDst, u16 color, u32 *pTempBuffer ); // Nintendo���S�f�[�^��OBJ_2D�`���Ń��[�h�ipTempBuffer�ɂ�0x700bytes�K�v)
extern void SYSM_LoadNintendoLogo1D( u16 *pLogoData, u16 *pDst, u16 color, u32 *pTempBuffer ); // Nintendo���S�f�[�^��OBJ_1D�`���Ń��[�h�i����j

// RTC����
extern BOOL SYSM_CheckRTCDate( RTCDate *pDate );								// ���t�����킩�`�F�b�N
extern BOOL SYSM_CheckRTCTime( RTCTime *pTime );								// ���������킩�`�F�b�N
extern s64  SYSM_CalcRTCOffsetAndSetDateTime( RTCDate *pNewDate, RTCTime *pNewTime );	// RTC�I�t�Z�b�g�v�Z��RTC�ւ̓��t�����`�F�b�N���s��
extern u32  SYSM_GetDayNum( u32 year, u32 month );								// �w�肳�ꂽ�N�E���̓������擾����
extern BOOL SYSM_IsLeapYear100( u32 year );										// �w�肳�ꂽ�N�����邤�N�����ׂ�


void SYSM_GoSleepMode( void );
inline BOOL SYSM_IsNITROCard( void )
{
	return TRUE;
}

#ifdef __cplusplus
}
#endif

#endif  // __SYSM_LIB_H__
