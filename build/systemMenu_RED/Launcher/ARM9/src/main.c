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
static BOOL CheckBootStatus( void );
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
		LOADING = 4,
		AUTHENTICATE = 5,
		BOOT = 6,
		STOP = 7
	};
	u32 state = START;
	TitleProperty *pBootTitle = NULL;
	TitleProperty pTitleList[ LAUNCHER_TITLE_LIST_NUM ];
	OSThread *thread;
	
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
	InitAllocator();
	
	// �e��p�����[�^�̎擾--------
	SYSM_ReadParameters();
	if( SYSM_GetResetParam()->flags.isLogoSkip ) {
		if( SYSM_GetResetParam()->bootTitleID ) {							// �A�v�����ڋN���̎w�肪�������烍�S�f�����΂��Ďw��A�v���N��
			pBootTitle = (TitleProperty *)SYSM_GetResetParam();
			state = AUTHENTICATE;
		}else {																// ����ȊO�̏ꍇ�́A���S�f�����΂��ă����`���[�N��
			state = LAUNCHER_INIT;
		}
	}
	
	// �R���e���g�i���\�[�X�j�t�@�C���̃��[�h
//	FS_ReadContentFile( ContentID );
	
	// ���L�R���e���g�t�@�C���̃��[�h
//	FS_ReadSharedContentFile( ContentID );
	
	// NAND�A�v�����X�g�̎擾----------
	(void)SYSM_GetNandTitleList( pTitleList, LAUNCHER_TITLE_LIST_NUM );
	
	while( 1 ) {
		OS_WaitIrq(1, OS_IE_V_BLANK);							// V�u�����N���荞�ݑ҂�
		
		ReadKeyPad();											// �L�[���͂̎擾
		ReadTP();												// TP���͂̎擾
		
		(void)SYSM_GetCardTitleList( pTitleList );				// �J�[�h�A�v�����X�g�̎擾�i�X���b�h�Ő����J�[�h�}����ʒm�������̂����C�����[�v�Ŏ擾�j
		
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
				thread = SYSM_LoadTitle( pBootTitle );
				state = LOADING;
			}
			break;
		case LOADING:
			LauncherLoading( pTitleList );
			if(OS_IsThreadTerminated( thread ))
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
	}
}


// �u�[�g��Ԃ��m�F���A���S�\���L���𔻒f����-------
static BOOL CheckBootStatus(void)
{
#if 0
	BOOL boot_decision		= FALSE;								// �u�u�[�g���e����v��
	BOOL other_shortcut_off	= FALSE;
	
	//-----------------------------------------------------
	// �f�o�b�O�p�R���p�C���X�C�b�`�ɂ�鋓��
	//-----------------------------------------------------
	{
		
#ifdef __LOGO_SKIP													// ���f�o�b�O�p���S�X�L�b�v
		SetLogoEnable( FALSE );										// ���S�\���X�L�b�v
#endif /* __LOGO_SKIP */
	}
	
	
	//-----------------------------------------------------
	// NITRO�ݒ�f�[�^�����͎��̐ݒ胁�j���[�V���[�g�J�b�g�N��
	//-----------------------------------------------------
#ifdef __DIRECT_BOOT_BMENU_ENABLE									// ��NITRO�ݒ�f�[�^�����͎��̃u�[�g���j���[���ڋN���X�C�b�`��ON���H
	if( !TSD_IsSetTP() ||
		!TSD_IsSetLanguage() ||
		!TSD_IsSetDateTime() ||
		!TSD_IsSetUserColor() ||
		!TSD_IsSetNickname() ) {									// TP,����,RTC,�j�b�N�l�[�����Z�b�g����Ă��Ȃ���΁A���S�\�����Q�[�����[�h���s�킸�A�u�[�g���j���[���V���[�g�J�b�g�N���B
		
		if( ( pad.cont & PAD_PRODUCTION_NITRO_SHORTCUT ) == PAD_PRODUCTION_NITRO_SHORTCUT ) {
			other_shortcut_off = TRUE;								// �ʎY�H���p�̃L�[�V���[�g�J�b�g��������Ă�����A�ݒ胁�j���[�N���͂Ȃ��B
		}else if( !SYSM_IsInspectNITROCard() )  {					// �A���A�ʎY�p�̃L�[�V���[�g�J�b�g��������Ă��鎞���ANITRO�����J�[�h���������Ă��鎞�́A�u�[�g���j���[�ւ̃V���[�g�J�b�g�N���͍s��Ȃ��B
			SYSM_SetBootFlag( BFLG_BOOT_BMENU );
			SetLogoEnable( FALSE );
			return TRUE;											// �u�u�[�g���e����v�Ń��^�[��
		}
	}
#endif /* __DIRECT_BOOT_BMENU_ENABLE */
	
	
	//-----------------------------------------------------
	// �L�[�V���[�g�J�b�g�N��
	//-----------------------------------------------------
	if( !other_shortcut_off
//		&& !TSD_IsAutoBoot()
		) {
																	// ���V���[�g�J�b�gON���I�[�g�N��OFF�̎�
		u32 nowBootFlag = 0;
		
		if(pad.cont & PAD_BUTTON_R){								// R�{�^�������N���Ȃ�A���S�\���Ȃ���AGB�Q�[����
			SetLogoEnable( FALSE );
			nowBootFlag = BFLG_BOOT_AGB;
		}else if(pad.cont & PAD_BUTTON_L){							// L�{�^�������N���Ȃ�A���S�\�����NITRO�Q�[����
			nowBootFlag = BFLG_BOOT_NITRO;
		}else if(pad.cont & PAD_BUTTON_B){							// B�{�^�������N���Ȃ�A���S�\����Ƀu�[�g���j���[��
			nowBootFlag = BFLG_BOOT_BMENU;
		}
		if( nowBootFlag ) {
			SYSM_SetBootFlag( nowBootFlag );
			return TRUE;											// �u�u�[�g���e����v�Ń��^�[��
		}
	}
	
	
	//-----------------------------------------------------
	// �����N���I�v�V�����L�����̋���
	//-----------------------------------------------------
#ifndef __SYSM_DEBUG
//	if( TSD_IsAutoBoot() ) {
	if( 0 ) {
		if ( SYSM_IsExistCard() ) {									// NITRO�J�[�h�݂̂̎���NITRO�N��
			SYSM_SetBootFlag( BFLG_BOOT_NITRO );
			return TRUE;											// �u�u�[�g���e����v�Ń��^�[��
		}
	}
#endif /* __SYSM_DEBUG */
#endif
	return FALSE;													// �u�u�[�g���e����v�Ń��^�[��
}


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// V�u�����N�����`�F�b�N�̃Z�b�g
}

