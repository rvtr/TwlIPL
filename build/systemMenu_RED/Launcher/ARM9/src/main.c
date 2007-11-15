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

// const data------------------------------------------------------------------


// ���C��
void TwlMain( void )
{
	enum {
		START = 0,
		LOGODEMO = 1,
		LAUNCHER_INIT = 2,
		LAUNCHER = 3,
		AUTHENTICATE = 4,
		BOOT = 5,
		STOP = 6
	};
	u32 state = START;
	TitleProperty *pBootTitle = NULL;
	TitleProperty pTitleList[ LAUNCHER_TITLE_LIST_NUM ];
	
	// �V�X�e�����j���[������----------
	SYSM_Init( Alloc, Free );											// OS_Init�̑O�ŃR�[���B
	
	// OS������------------------------
    OS_Init();
	
	(void)OS_EnableIrq();
	(void)OS_EnableInterrupts();
	
	FS_Init( FS_DMA_NOT_USE );
    GX_Init();
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
				state = AUTHENTICATE;
			}
			break;
		case AUTHENTICATE:
			switch ( SYSM_LoadAndAuthenticateTitle( pBootTitle ) ) {	// �A�v�����[�h���F��	�������Fnever return
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


// ============================================================================
// ���荞�ݏ���
// ============================================================================

// V�u�����N���荞��
static void INTR_VBlank(void)
{
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);								// V�u�����N�����`�F�b�N�̃Z�b�g
}

