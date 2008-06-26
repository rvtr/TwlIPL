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
#include <twl/os/common/msJump.h>
#include <sysmenu/reloc_info/common/reloc_info.h>
#include <application_jump_private.h>

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

#define SYSM_PAD_SHORTCUT_TP_CALIBRATION		( PAD_BUTTON_L | PAD_BUTTON_R | PAD_BUTTON_START )
#define SYSM_PAD_SHORTCUT_MACHINE_SETTINGS		( PAD_BUTTON_SELECT )
#define SYSM_PAD_PRODUCTION_SHORTCUT_CARD_BOOT	( PAD_BUTTON_A | PAD_BUTTON_B	\
												| PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_R )
																	// �ʎY�H���Ŏg�p���鏉��N���ݒ���L�����Z�����ăJ�[�h�u�[�g����V���[�g�J�b�g�L�[

#define SYSM_MOUNT_INFO_SIZE				(0x400 - OS_MOUNT_PATH_LEN)
#define SYSM_LAUNCHER_VER					1	// �����`���[�o�[�W�����iSDK���Ń����`���[�ɗ��ޏ����̔���p�j

#define SYSM_ALIGNMENT_LOAD_MODULE			32	// ���W���[����srl����ǂݍ��ލۂ̃A���C�����g�iAES�����AES�Ŏg��DMA�̎d�l�ɂ��j

typedef struct TitleInfoSub {
	RomExpansionFlags	exFlags;
	char				platform_code;
	u8					parental_control_rating_info[0x10];
	u32					card_region_bitmap;
	u8					agree_EULA_version;
}TitleInfoSub;

// �^�C�g�����
typedef struct TitleProperty {			// ���̏��́A�����`���[���ɂ͔F�ؒʂ��ĂȂ����ǁA�N�����ɂ͔F�ؒʂ��̂ő��v���낤�B
	NAMTitleId			titleID;		// �^�C�g��ID�iTitleID_Hi�ŋN�����f�B�A�͔���ł���H�j
	LauncherBootFlags	flags;			// �u�[�g���̃����`���[����t���O
	TWLBannerFile		*pBanner;		// �o�i�[�ւ̃|�C���^�i�Œ蒷�t�H�[�}�b�g�Ȃ�U������Ă����v���낤�B)
	TitleInfoSub		sub_info;
}TitleProperty;

// �^�C�g�����X�g�쐬�p���\����
typedef struct TitleListMakerInfo {
	char				makerCode[MAKER_CODE_MAX];
	u32					public_save_data_size;
	u32					private_save_data_size;
	BOOL				permit_landing_normal_jump;
	TitleInfoSub		sub_info;
}TitleListMakerInfo;


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
extern void SYSM_GetNandTitleListMakerInfo( void );								// �A�v�������n���^�C�g�����X�g�쐬�p���̎擾�i�_�C���N�g�u�[�g�p�j

// �A�v���N��
extern void SYSM_StartLoadTitle( TitleProperty *pBootTitle );					// �w�肵��TitleProperty��ʃX���b�h�Ń��[�h�J�n
extern BOOL SYSM_IsLoadTitleFinished( void );									// SYSM_StartLoadTitle�ŋN�������X���b�h���I���������ǂ������m�F
extern void SYSM_StartAuthenticateTitle( TitleProperty *pBootTitle );			// �w�肵��TitleProperty��ʃX���b�h�Ō��؊J�n
extern BOOL SYSM_IsAuthenticateTitleFinished( void );							// SYSM_StartAuthenticateTitle�ŋN�������X���b�h���I���������ǂ������m�F
extern void SYSM_TryToBootTitle( TitleProperty *pBootTitle );					// pBootTitle�Ŏw�肵���^�C�g�����u�[�g�Bnever return.

// AES�̈�f�N���v�g
extern void SYSM_StartDecryptAESRegion( ROM_Header_Short *hs );					// �N������ROM��AES�Í����̈�̃f�N���v�g�J�n
extern BOOL SYSM_InitDecryptAESRegion_W( ROM_Header_Short *hs );				// WRAM�o�R�t�@�C���ǂݍ��݂̃R�[���o�b�N�Ŏg��AES�f�N���v�g�����̏�����
extern void SYSM_StartDecryptAESRegion_W( const void *wram_addr, const void *orig_addr, u32 size );
																				// WRAM�o�R�t�@�C���ǂݍ��݂̃R�[���o�b�N�Ŏg��AES�f�N���v�g�����֐�
// Nintendo���S����
extern BOOL SYSM_CheckNintendoLogo( u16 *pLogoData );							// Nintendo���S�f�[�^�̃`�F�b�N
extern void SYSM_LoadNintendoLogo2D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // Nintendo���S�f�[�^��OBJ_2D�`���Ń��[�h�ipTempBuffer�ɂ�0x700bytes�K�v)
extern void SYSM_LoadNintendoLogo1D( u16 *pLogoData, u16 *pDst, int paletteColorIndex ); // Nintendo���S�f�[�^��OBJ_1D�`���Ń��[�h�i����j

extern s32 SYSMi_getCheckTitleLaunchRightsResult( void );						// CheckTitleLaunchRights�̌��ʂ�Ԃ��i�f�o�O�p�j

extern BOOL SYSM_IsLoadTitlePaused(void);										// ���[�f�B���O�X���b�h���ꎞ��~���Ă��邩�H
extern void SYSM_ResumeLoadingThread( BOOL force );								// ���[�f�B���O�X���b�h���ꎞ��~���Ă�����ĊJ

extern BOOL SYSM_MakeTitleListMakerInfoFromHeader( TitleListMakerInfo *info, ROM_Header_Short *hs);
																				// �A�v�������n���^�C�g�����X�g�쐬�p�����w�b�_��񂩂�쐬

#endif

// ��ԃ`�F�b�N
extern BOOL SYSM_IsExistCard( void );											// TWL/NTR�J�[�h���������Ă��邩�H�i�A�v���͖��F�؏�ԁj
extern BOOL SYSM_IsInspectCard( void );											// �����J�[�h���������Ă��邩�H
extern BOOL SYSM_IsHotStart( void );											// �z�b�g�X�^�[�g���H
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
