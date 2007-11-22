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
static void INTR_VBlank( void );

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------
static TitleProperty s_titleList[ LAUNCHER_TITLE_LIST_NUM ];

// const data------------------------------------------------------------------

// ���C��
void TwlMain( void )
{
	enum {
		LOGODEMO_INIT = 0,
		LOGODEMO = 1,
		LAUNCHER_INIT = 2,
		LAUNCHER = 3,
		LOAD_START = 4,
		LOADING = 5,
		AUTHENTICATE = 6,
		BOOT = 7,
		STOP = 8
	};
	u32 state = LOGODEMO_INIT;
	TitleProperty *pBootTitle = NULL;
	OSTick start, end = 0;
	
	// �V�X�e�����j���[������----------
	SYSM_Init( Alloc, Free );											// OS_Init�̑O�ŃR�[���B
	
	// OS������------------------------
    OS_Init();
	OS_InitTick();
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
	pBootTitle = SYSM_ReadParameters();							// �{�̐ݒ�f�[�^�A���Z�b�g�p�����[�^�A
																// ����N���V�[�P���X����A
																// �����p�I�[�g�N���J�[�h����A�ʎY���C���p�L�[�V���[�g�J�b�g�N�����蓙�̃��[�h
	
	// �u�_�C���N�g�u�[�g�łȂ��v�Ȃ�ANAND & �J�[�h�A�v�����X�g�擾
	if( !pBootTitle ) {
		(void)SYSM_GetNandTitleList( s_titleList, LAUNCHER_TITLE_LIST_NUM );	// NAND�A�v�����X�g�̎擾�i�����A�v����s_titleList[1]����i�[�����j
		(void)SYSM_GetCardTitleList( s_titleList );				// �J�[�h�A�v�����X�g�̎擾�i�J�[�h�A�v����s_titleList[0]�Ɋi�[�����j
	}
	
	// �u�_�C���N�g�u�[�g�łȂ��v��������
	// �u�_�C���N�g�u�[�g�����A���S�f���\���v�̎��A�e�탊�\�[�X�̃��[�h------------
	if( !pBootTitle ||
		( pBootTitle && !SYSM_IsLogoDemoSkip() ) ) {
//		FS_ReadContentFile( ContentID );						// �^�C�g�������\�[�X�t�@�C���̃��[�h
//		FS_ReadSharedContentFile( ContentID );					// ���L�R���e���g�t�@�C���̃��[�h
	}
	
	// �J�n�X�e�[�g�̔���--------------
	
	if( pBootTitle ) {
		// �_�C���N�g�u�[�g�Ȃ�A���S�A�����`���[���΂��ă��[�h�J�n
		state = LOAD_START;
	}else if( SYSM_IsLogoDemoSkip() ) {
		// ���S�f���X�L�b�v���w�肳��Ă�����A�����`���[�N��
		state = LAUNCHER_INIT;
	}else {
		// �����Ȃ��Ȃ�A���S�f���N��
		state = LOGODEMO_INIT;
	}
	
	// ���C�����[�v--------------------
	while( 1 ) {
		OS_WaitIrq(1, OS_IE_V_BLANK);							// V�u�����N���荞�ݑ҂�
		
		ReadKeyPad();											// �L�[���͂̎擾
		ReadTP();												// TP���͂̎擾
		
		switch( state ) {
		case LOGODEMO_INIT:
			LogoInit();
			state = LOGODEMO;
			break;
		case LOGODEMO:
			if( LogoMain() ) {
				state = LAUNCHER_INIT;
			}
			break;
		case LAUNCHER_INIT:
			LauncherInit( s_titleList );
			state = LAUNCHER;
			break;
		case LAUNCHER:
			pBootTitle = LauncherMain( s_titleList );
			if( pBootTitle ) {
				state = LOAD_START;
			}
			break;
		case LOAD_START:
			SYSM_StartLoadTitle( pBootTitle );
			state = LOADING;
			
			start = OS_GetTick();
			
			break;
		case LOADING:
			if( LauncherFadeout( s_titleList ) &&
				SYSM_IsLoadTitleFinished( pBootTitle ) ) {
				state = AUTHENTICATE;
			}
			
			if( ( end == 0 ) &&
				SYSM_IsLoadTitleFinished( pBootTitle ) ) {
				end = OS_GetTick();
				OS_TPrintf( "Load Time : %dms\n", OS_TicksToMilliSeconds( end - start ) );
			}
			break;
		case AUTHENTICATE:
			switch ( SYSM_AuthenticateTitle( pBootTitle ) ) {	// �A�v���F�؁��u�[�g	�������Fnever return
			case AUTH_RESULT_TITLE_LOAD_FAILED:
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
		(void)SYSM_GetCardTitleList( s_titleList );
	}
}


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// V�u�����N�����`�F�b�N�̃Z�b�g
}

