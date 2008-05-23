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
#include <twl/os/common/banner.h>
#include <sysmenu/reloc_info/common/reloc_info.h>
#include <launcherParam_private.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data----------------------------------------------------------

#ifndef SDK_FINALROM
//#define SYSM_DEBUG_												// �f�o�b�O�R�[�h�p�r���h�X�C�b�`
//#define ENABLE_INITIAL_SETTINGS_
#endif // SDK_FINALROM

#define CARD_SLOT_NUM							1					// �J�[�h�X���b�g��
#define LAUNCHER_TITLE_LIST_NUM					( LCFG_TWL_FREE_SOFT_BOX_COUNT_MAX + 1 )	// �����`���[�̃^�C�g�����X�g��

#define SYSM_PAD_SHORTCUT_TP					( PAD_BUTTON_X )
#define SYSM_PAD_SHORTCUT_MACHINE_SETTINGS		( PAD_BUTTON_SELECT )
#define SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT	( PAD_BUTTON_A | PAD_BUTTON_B	\
												| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_R )
																	// �ʎY�H���Ŏg�p���鏉��N���ݒ���L�����Z�����ăJ�[�h�u�[�g����V���[�g�J�b�g�L�[

#define SYSM_MOUNT_INFO_SIZE				(0x400 - OS_MOUNT_PATH_LEN)
#define SYSM_LAUNCHER_VER					1	// �����`���[�o�[�W�����iSDK���Ń����`���[�ɗ��ޏ����̔���p�j

#define SYSM_ALIGNMENT_LOAD_MODULE			32	// ���W���[����srl����ǂݍ��ލۂ̃A���C�����g�iAES�����AES�Ŏg��DMA�̎d�l�ɂ��j

// �^�C�g�����
typedef struct TitleProperty {			// ���̏��́A�����`���[���ɂ͔F�ؒʂ��ĂȂ����ǁA�N�����ɂ͔F�ؒʂ��̂ő��v���낤�B
	NAMTitleId			titleID;		// �^�C�g��ID�iTitleID_Hi�ŋN�����f�B�A�͔���ł���H�j
	LauncherBootFlags	flags;			// �u�[�g���̃����`���[����t���O
	TWLBannerFile		*pBanner;		// �o�i�[�ւ̃|�C���^�i�Œ蒷�t�H�[�}�b�g�Ȃ�U������Ă����v���낤�B)
}TitleProperty;

// �A�v���F�،���
typedef enum AuthResult {
	AUTH_RESULT_SUCCEEDED = 0,
	AUTH_RESULT_PROCESSING = 1,
	AUTH_RESULT_TITLE_LOAD_FAILED = 2,
	AUTH_RESULT_TITLE_POINTER_ERROR = 3,
	AUTH_RESULT_AUTHENTICATE_FAILED = 4,
	AUTH_RESULT_ENTRY_ADDRESS_ERROR = 5,
	AUTH_RESULT_TITLE_BOOTTYPE_ERROR = 6,
	AUTH_RESULT_SIGN_DECRYPTION_FAILED = 7,
	AUTH_RESULT_SIGN_COMPARE_FAILED = 8,
	AUTH_RESULT_HEADER_HASH_CALC_FAILED = 9,
	AUTH_RESULT_TITLEID_COMPARE_FAILED = 10,
	AUTH_RESULT_VALID_SIGN_FLAG_OFF = 11,
	AUTH_RESULT_CHECK_TITLE_LAUNCH_RIGHTS_FAILED = 12,
	AUTH_RESULT_MODULE_HASH_CHECK_FAILED = 13,
	AUTH_RESULT_MODULE_HASH_CALC_FAILED = 14,
	AUTH_RESULT_MEDIA_CHECK_FAILED = 15,
	AUTH_RESULT_DL_MAGICCODE_CHECK_FAILED = 16,
	AUTH_RESULT_DL_SIGN_DECRYPTION_FAILED = 17,
	AUTH_RESULT_DL_HASH_CALC_FAILED = 18,
	AUTH_RESULT_DL_SIGN_COMPARE_FAILED = 19,
	AUTH_RESULT_WHITELIST_INITDB_FAILED = 20,
	AUTH_RESULT_WHITELIST_NOTFOUND = 21,
	AUTH_RESULT_DHT_PHASE1_FAILED = 22,
	AUTH_RESULT_DHT_PHASE2_FAILED = 23,
	AUTH_RESULT_LANDING_TMP_JUMP_FLAG_OFF = 24,
	AUTH_RESULT_TWL_BOOTTYPE_UNKNOWN = 25,
	AUTH_RESULT_NTR_BOOTTYPE_UNKNOWN = 26,
	AUTH_RESULT_PLATFORM_UNKNOWN = 27,
	
	AUTH_RESULT_MAX = 28
}AuthResult;


// global variable------------------------------------------------------
#ifdef SDK_ARM9
extern const char *g_strIPLSvnRevision;
extern const char *g_strSDKSvnRevision;
extern void *SYSM_Alloc( u32 size );
extern void SYSM_Free( void *ptr );
//extern void *(*SYSM_Alloc)( u32 size );			// ���C�u���������g�p
//extern void  (*SYSM_Free)( void *ptr );			// ����
#endif

// function-------------------------------------------------------------

#ifdef SDK_ARM9

// ������
extern void SYSM_Init( void *(*pAlloc)(u32), void (*pFree)(void*) );			// �������B
extern void SYSM_InitPXI( void );												// PXI������
extern void SYSM_SetArena( void );												// �V�X�e�����j���[�̃A���[�i�������BOS_Init�̌�ŌĂ�ł��������B
extern void SYSM_SetAllocFunc( void *(*pAlloc)(u32), void (*pFree)(void*) );	// SYSM_init�Őݒ肵���ꍇ�͕K�v�Ȃ��B
extern TitleProperty *SYSM_ReadParameters( void );								// �{�̐ݒ�f�[�^�A�����`���[�p�����[�^�Ȃǂ��擾
extern void SYSM_DeleteTmpDirectory( TitleProperty *pBootTitle );              // "nand:/tmp"�t�H���_�̃N���[��

// �A�v�����擾
extern int  SYSM_GetCardTitleList( TitleProperty *pTitleList_Card );			// �J�[�h�A�v���^�C�g�����X�g�̎擾
extern BOOL SYSM_InitNandTitleList( void );										// NAND�A�v���^�C�g�����X�g�擾����
extern void SYSM_FreeNandTitleList( void );										// NAND�A�v���^�C�g�����X�g
extern int  SYSM_GetNandTitleList( TitleProperty *pTitleList_Nand, int size );	// NAND  �A�v���^�C�g�����X�g�̎擾

// �A�v���N��
extern void SYSM_StartLoadTitle( TitleProperty *pBootTitle );					// �w�肵��TitleProperty��ʃX���b�h�Ń��[�h�J�n
extern BOOL SYSM_IsLoadTitleFinished( void );									// SYSM_StartLoadTitle�ŋN�������X���b�h���I���������ǂ������m�F
extern void SYSM_StartAuthenticateTitle( TitleProperty *pBootTitle );			// �w�肵��TitleProperty��ʃX���b�h�Ō��؊J�n
extern BOOL SYSM_IsAuthenticateTitleFinished( void );							// SYSM_StartAuthenticateTitle�ŋN�������X���b�h���I���������ǂ������m�F
extern AuthResult SYSM_TryToBootTitle( TitleProperty *pBootTitle );				// pBootTitle�Ŏw�肵���^�C�g�����u�[�g�B�������́Anever return.

// AES�̈�f�N���v�g
extern void SYSM_StartDecryptAESRegion( ROM_Header_Short *hs );					// �N������ROM��AES�Í����̈�̃f�N���v�g�J�n
extern BOOL SYSM_InitDecryptAESRegion_W( ROM_Header_Short *hs );				// WRAM�o�R�t�@�C���ǂݍ��݂̃R�[���o�b�N�Ŏg��AES�f�N���v�g�����̏�����
extern void SYSM_StartDecryptAESRegion_W( const void *wram_addr, const void *orig_addr, u32 size );
																				// WRAM�o�R�t�@�C���ǂݍ��݂̃R�[���o�b�N�Ŏg��AES�f�N���v�g�����֐�
// Nintendo���S����
extern BOOL SYSM_CheckNintendoLogo( u16 *pLogoData );							// Nintendo���S�f�[�^�̃`�F�b�N
extern void SYSM_LoadNintendoLogo2D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // Nintendo���S�f�[�^��OBJ_2D�`���Ń��[�h�ipTempBuffer�ɂ�0x700bytes�K�v)
extern void SYSM_LoadNintendoLogo1D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // Nintendo���S�f�[�^��OBJ_1D�`���Ń��[�h�i����j

#endif

// ��ԃ`�F�b�N
extern BOOL SYSM_IsExistCard( void );											// TWL/NTR�J�[�h���������Ă��邩�H�i�A�v���͖��F�؏�ԁj
extern BOOL SYSM_IsInspectCard( void );											// �����J�[�h���������Ă��邩�H
extern BOOL SYSM_IsHotStart( void );											// �z�b�g�X�^�[�g���H
extern BOOL SYSM_IsFatalError( void );											// FATAL�G���[���H
extern void SYSM_SetFatalError( BOOL isFatalError );							// FATAL�G���[�̃Z�b�g
extern BOOL SYSM_IsLogoDemoSkip( void );										// ���S�f����΂���Ԃ��H
extern void SYSM_SetLogoDemoSkip( BOOL skip );									// ���S�f����΂���ԃt���O��ݒ肷��B
extern BOOL SYSM_IsValidTSD( void );											// TWL�ݒ�f�[�^�͗L�����H
extern void SYSM_SetValidTSD( BOOL valid );										// TWL�ݒ�f�[�^�̗L���^�����t���O��ݒ肷��B
extern const LauncherParamBody *SYSM_GetLauncherParamBody( void );				// �����`���[�p�����[�^�̎擾
extern BOOL SYSM_IsRunOnDebugger( void );										// IS�f�o�b�K��œ��삵�Ă��邩�H

extern BOOL SYSM_IsLauncherHidden( void );										// �����`���[�̉�ʂ�\�����Ȃ��o�[�W�������H

// AES�̈�f�N���v�g
extern void SYSM_InitDecryptAESPXICallback( void );								// AES�̈�f�N���v�g�p��PXI�R�[���o�b�N�ݒ�

#ifdef __cplusplus
}
#endif

#endif  // __SYSM_LIB_H__
