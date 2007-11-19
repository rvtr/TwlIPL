/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     main.c

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

#include <twl.h>
#include "launcher.h"
#include "misc.h"
#include "logoDemo.h"

// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------

// function's prototype-------------------------------------------------------
static TitleProperty *CheckShortcutBoot( TitleProperty *pTitleList );
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------

// ���C��
void TwlMain( void )
{
	enum {
		START = 0,
		LOGODEMO = 1,
		LAUNCHER_INIT = 2,
		LAUNCHER = 3,
		LOAD_START = 4,
		LOADING = 5,
		AUTHENTICATE = 6,
		BOOT = 7,
		STOP = 8
	};
	u32 state = START;
	TitleProperty *pBootTitle = NULL;
	TitleProperty pTitleList[ LAUNCHER_TITLE_LIST_NUM ];
	
	// �V�X�e�����j���[������----------
	SYSM_Init( Alloc, Free );											// OS_Init�̑O�ŃR�[���B
	
	// OS������------------------------
    OS_Init();
    PM_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
	FS_Init( FS_DMA_NOT_USE );
    GX_Init();
	PM_Init();
	TP_Init();
	RTC_Init();
    
	// ���荞�݋���--------------------
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, INTR_VBlank);
	(void)OS_EnableIrqMask(OS_IE_V_BLANK);
	(void)GX_VBlankIntr(TRUE);
	
	// �V�X�e���̏�����----------------
	InitAllocator();											// ��SYSM_Init�ȊO��SYSM���C�u�����֐����ĂԑO��
																//   Alloc, Free�œo�^�����������A���P�[�^�����������Ă��������B
	// �e��p�����[�^�̎擾------------
	SYSM_ReadParameters();										// �{�̐ݒ�f�[�^���̃��[�h
	(void)SYSM_GetNandTitleList( pTitleList, LAUNCHER_TITLE_LIST_NUM );	// NAND�A�v�����X�g�̎擾�i�����A�v����pTitleList[1]����i�[�����j
	(void)SYSM_GetCardTitleList( pTitleList );					// �J�[�h�A�v�����X�g�̎擾�i�J�[�h�A�v����pTitleList[0]�Ɋi�[�����j
	
	// ���Z�b�g�p�����[�^���V���[�g�J�b�g�`�F�b�N----------
	if( SYSM_GetResetParamBody()->v1.bootTitleID ) {			// �A�v�����ڋN���̎w�肪�������烍�S�f�����΂��Ďw��A�v���N��
		pBootTitle = (TitleProperty *)&SYSM_GetResetParamBody()->v1;
	}else {
		pBootTitle = CheckShortcutBoot( pTitleList );
	}
	
	// �_�C���N�g�u�[�g�Ń��S�f���X�L�b�v�łȂ����A�e�탊�\�[�X�̃��[�h------------
	if( !( pBootTitle && !pBootTitle->flags.isLogoSkip ) ) {
//		FS_ReadContentFile( ContentID );						// �^�C�g�������\�[�X�t�@�C���̃��[�h
//		FS_ReadSharedContentFile( ContentID );					// ���L�R���e���g�t�@�C���̃��[�h
	}
	
	// �J�n�X�e�[�g�̔���--------------
	if( pBootTitle ) {
		// �_�C���N�g�N���^�C�g���̎w�肪����Ȃ�A���S�A�����`���[���΂��ċN��
		if( pBootTitle->flags.isAppLoadCompleted ) {
			// ���[�h�ςݏ�ԂȂ�A���ڔF�؂�
			state = AUTHENTICATE;
		}else {
			// �����Ȃ��΁A���[�h�J�n
			state = LOAD_START;
		}
	}else if( SYSM_IsLogoDemoSkip() ) {
		// ���Z�b�g�p�����[�^�Ń��S�f���X�L�b�v���w�肳��Ă�����A�����`���[�N��
		state = LAUNCHER_INIT;
	}else {
		// �����Ȃ��Ȃ�A���S�f���N��
		state = START;
	}
	
	// ���C�����[�v--------------------
	while( 1 ) {
		OS_WaitIrq(1, OS_IE_V_BLANK);							// V�u�����N���荞�ݑ҂�
		
		ReadKeyPad();											// �L�[���͂̎擾
		ReadTP();												// TP���͂̎擾
		
		switch( state ) {
		case START:
			state = LOGODEMO;
			break;
		case LOGODEMO:
			if( LogoMain() ) {
				state = LAUNCHER_INIT;
			}
			break;
		case LAUNCHER_INIT:
			InitBG();										// BG������
			LauncherInit( pTitleList );
			state = LAUNCHER;
			break;
		case LAUNCHER:
			pBootTitle = LauncherMain( pTitleList );
			if( pBootTitle ) {
				state = LOAD_START;
			}
			break;
		case LOAD_START:
			SYSM_StartLoadTitle( pBootTitle );
			state = LOADING;
			break;
		case LOADING:
			LauncherLoading( pTitleList );
			if( SYSM_IsLoadTitleFinished() )
			{
				GX_DispOff();
				GXS_DispOff();
				state = AUTHENTICATE;
			}
			break;
		case AUTHENTICATE:
			switch ( SYSM_AuthenticateTitle( pBootTitle ) ) {	// �A�v���F�؁��u�[�g	�������Fnever return
			case AUTH_PROCESSING:
				break;
			case AUTH_RESULT_TITLE_POINTER_ERROR:
			case AUTH_RESULT_AUTHENTICATE_FAILED:
			case AUTH_RESULT_ENTRY_ADDRESS_ERROR:
				state = STOP;
				break;
			}
			break;
		case STOP:												// ��~
			break;
		}
		
		// �J�[�h�A�v�����X�g�̎擾�i�X���b�h�Ő����J�[�h�}����ʒm�������̂����C�����[�v�Ŏ擾�j
		(void)SYSM_GetCardTitleList( pTitleList );
	}
}


// �V���[�g�J�b�g�N���̃`�F�b�N
static TitleProperty *CheckShortcutBoot( TitleProperty *pTitleList )
{
#if 0	// ��������
	TitleProperty *pTgt;
	
	ReadKeyPad();													// �L�[���͂̎擾
	
	//-----------------------------------------------------
	// TWL�ݒ�f�[�^�����͎��̏���N���V�[�P���X�N��
	//-----------------------------------------------------
#ifdef ENABLE_INITIAL_SETTINGS_
	if( !TSD_IsSetTP() ||
		!TSD_IsSetLanguage() ||
		!TSD_IsSetDateTime() ||
		!TSD_IsSetUserColor() ||
		!TSD_IsSetNickname() ) {
		return SYSM_GetTitleProperty( TITLE_ID_MACHINE_SETTINGS, pTitleList );	// ��������
	}
#endif // ENABLE_INITIAL_SETTINGS_
	
	//-----------------------------------------------------
	// �ʎY�H���p�V���[�g�J�b�g�L�[ or
	// �����J�[�h�N��
	//-----------------------------------------------------
	if( ( SYSM_IsExistCard() &&
		  ( ( pad.cont & PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) == PAD_PRODUCTION_SHORTCUT_CARD_BOOT ) ) ||
		SYSM_IsInspectCard() ) {
		pTgt = SYSM_GetTitleProperty();	// ��������
		if( pTgt ) {
			pTgt->flags.isLogoSkip = TRUE;							// ���S�f�����΂�
			pTgt->flags.isInitialShortcutSkip = TRUE;				// ����N���V�[�P���X���΂�
		}
		return pTgt;
	}
#endif	// 0
	return NULL;													// �u�u�[�g���e����v�Ń��^�[��
}


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// V�u�����N�����`�F�b�N�̃Z�b�g
}

