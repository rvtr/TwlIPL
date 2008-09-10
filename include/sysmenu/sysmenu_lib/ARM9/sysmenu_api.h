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
#include <sysmenu/banner.h>
#include <sysmenu/settings.h>
#include <sysmenu/sysmenu_lib/common/sysmenu_work.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

#ifndef SDK_FINALROM
//#define SYSM_DEBUG_												// �f�o�b�O�R�[�h�p�r���h�X�C�b�`
//#define ENABLE_INITIAL_SETTINGS_
#endif // SDK_FINALROM

#define CARD_SLOT_NUM							1					// �J�[�h�X���b�g��
#define LAUNCHER_TITLE_LIST_NUM					40					// �����`���[�̃^�C�g�����X�g��

#define TITLE_ID_MACHINE_SETTINGS				0x000100015445534d	// �{�̐ݒ�̃^�C�g��ID

#define PAD_PRODUCTION_SHORTCUT_CARD_BOOT		( PAD_BUTTON_A | PAD_BUTTON_B	\
												| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_R )
																	// �ʎY�H���Ŏg�p���鏉��N���ݒ���L�����Z�����ăJ�[�h�u�[�g����V���[�g�J�b�g�L�[

typedef enum PlatformCode {
	PLATFORM_NTR = 0,
	PLATFORM_TWL = 1
}PlatformCode;


// �^�C�g�����
typedef struct TitleProperty {	// ���̏��́A�����`���[���ɂ͔F�ؒʂ��ĂȂ����ǁA�N�����ɂ͔F�ؒʂ��̂ő��v���낤�B
	NAMTitleId		titleID;		// �^�C�g��ID�iTitleID_Hi�ŋN�����f�B�A�͔���ł���H�j
	BootFlags		flags;			// �u�[�g���̃����`���[����t���O
	TWLBannerFile	*pBanner;		// �o�i�[�ւ̃|�C���^�i�Œ蒷�t�H�[�}�b�g�Ȃ�U������Ă����v���낤�B)
}TitleProperty;

// �A�v���F�،���
typedef enum AuthResult {
	AUTH_RESULT_SUCCEEDED = 0,
	AUTH_RESULT_PROCESSING = 1,
	AUTH_RESULT_TITLE_LOAD_FAILED = 2,
	AUTH_RESULT_TITLE_POINTER_ERROR = 3,
	AUTH_RESULT_AUTHENTICATE_FAILED = 4,
	AUTH_RESULT_ENTRY_ADDRESS_ERROR = 5
}AuthResult;


// global variable------------------------------------------------------
extern void *(*SYSM_Alloc)( u32 size );			// ���C�u���������g�p
extern void  (*SYSM_Free)( void *ptr );			// ����

// function-------------------------------------------------------------

// ������
extern void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) );			// �������BOS_Init�̑O�̂ւ�ŃR�[�����Ă��������B
extern void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) );	// SYSM_init�Őݒ肵���ꍇ�͕K�v�Ȃ��B
extern TitleProperty *SYSM_ReadParameters( void );								// �{�̐ݒ�f�[�^�A���Z�b�g�p�����[�^�Ȃǂ��擾

// �A�v�����擾
extern int  SYSM_GetCardTitleList( TitleProperty *pTitleList_Card );			// �J�[�h�A�v���^�C�g�����X�g�̎擾
extern int  SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int size );	// NAND  �A�v���^�C�g�����X�g�̎擾

// �A�v���N��
extern void SYSM_StartLoadTitle( TitleProperty *pBootTitle );					// �w�肵��TitleProperty��ʃX���b�h�Ń��[�h�J�n
extern BOOL SYSM_IsLoadTitleFinished( TitleProperty *pBootTitle );				// SYSM_StartLoadTitle�ŋN�������X���b�h���I���������ǂ������m�F
extern AuthResult SYSM_AuthenticateTitle( TitleProperty *pBootTitle );			// �w�肵��TitleProperty��F�؂��ău�[�g
																				// �������́Anever return.
// �f�o�C�X����
extern void SYSM_CaribrateTP( void );											// �^�b�`�p�l���L�����u���[�V����
extern void SYSM_SetBackLightBrightness( u8 brightness );						// �o�b�N���C�g�𐧌�i�{�̐ݒ�f�[�^�ւ̒l�Z�[�u���s���j

// ��ԃ`�F�b�N
extern BOOL SYSM_IsExistCard( void );											// TWL/NTR�J�[�h���������Ă��邩�H�i�A�v���͖��F�؏�ԁj
extern BOOL SYSM_IsInspectCard( void );											// �����J�[�h���������Ă��邩�H
extern BOOL SYSM_IsTPReadable( void );											// TP���[�h�\���H
extern BOOL SYSM_IsLogoDemoSkip( void );										// ���S�f����΂���Ԃ��H
extern void SYSM_SetLogoDemoSkip( BOOL skip );									// ���S�f����΂���ԃt���O��ݒ肷��B
extern BOOL SYSM_IsValidTSD( void );											// TWL�ݒ�f�[�^�͗L�����H
extern void SYSM_SetValidTSD( BOOL valid );										// TWL�ݒ�f�[�^�̗L���^�����t���O��ݒ肷��B
extern const ResetParamBody *SYSM_GetResetParamBody( void );					// ���Z�b�g�p�����[�^�̎擾

// �{�̐ݒ�f�[�^�A�N�Z�X
extern BOOL SYSM_ReadTWLSettingsFile( void );									// TWL�ݒ�f�[�^�̃��[�h
extern BOOL SYSM_WriteTWLSettingsFile( void );									// TWL�ݒ�f�[�^�̃��C�g
extern void SYSM_VerifyAndRecoveryNTRSettings( void );

// Nintendo���S����
extern BOOL SYSM_CheckNintendoLogo( u16 *pLogoData );							// Nintendo���S�f�[�^�̃`�F�b�N
extern void SYSM_LoadNintendoLogo2D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // Nintendo���S�f�[�^��OBJ_2D�`���Ń��[�h�ipTempBuffer�ɂ�0x700bytes�K�v)
extern void SYSM_LoadNintendoLogo1D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // Nintendo���S�f�[�^��OBJ_1D�`���Ń��[�h�i����j

// RTC����
extern BOOL SYSM_CheckRTCDate( RTCDate *pDate );								// ���t�����킩�`�F�b�N
extern BOOL SYSM_CheckRTCTime( RTCTime *pTime );								// ���������킩�`�F�b�N
extern s64  SYSM_CalcRTCOffset( RTCDate *pNewDate, RTCTime *pNewTime );			// RTC�I�t�Z�b�g�v�Z��RTC�ւ̓��t�����`�F�b�N���s��
extern u32  SYSM_GetDayNum( u32 year, u32 month );								// �w�肳�ꂽ�N�E���̓������擾����
extern BOOL SYSM_IsLeapYear100( u32 year );										// �w�肳�ꂽ�N�����邤�N�����ׂ�


#ifdef __cplusplus
}
#endif

#endif  // __SYSM_LIB_H__