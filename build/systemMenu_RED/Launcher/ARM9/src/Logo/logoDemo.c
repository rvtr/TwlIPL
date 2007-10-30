/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     logoDemo.c

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

#include "logoDemo.h"

// define data--------------------------------------------------------

#define LOGO_DISP_FRAME			60			// ���S�\���t���[����

// ���S�\���X�e�[�^�X�\����
typedef struct LogoStatus {
	s32		state;
	BOOL	enable;
	s32		value_A;
	s32		value_B;
	s32		mainCounter;
}LogoStatus;

// extern data--------------------------------------------------------
extern void LoadLogoData( void );

// function's prototype-----------------------------------------------

// static variables---------------------------------------------------
static LogoStatus s_logo = { 0, TRUE, 0, 0, 0 };

// const data---------------------------------------------------------

static void LogoInit( void )
{
	// ���OFF
	GX_DispOff();
	GXS_DispOff();
	
	// VRAM���蓖�ā��N���A
	GX_SetBankForOBJ( GX_VRAM_OBJ_128_A );						// �@�V�@�@�@�@�@OBJ�p
	GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );
	MI_CpuClearFast( (void *)HW_OBJ_VRAM,  0x1000 );			// OBJ-VRAM �N���A
	MI_DmaFill32( 3, (void *)HW_OAM, 192, HW_OAM_SIZE );		// OAM      �N���A
	
	// ��ʐݒ�
	GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_2, GX_BG0_AS_2D );
	GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_2D );
	G2_SetBlendAlpha( GX_BLEND_PLANEMASK_OBJ, GX_BLEND_PLANEMASK_BD, s_logo.value_A, s_logo.value_B );
	GX_SetVisiblePlane( GX_PLANEMASK_OBJ );
	
	// ���S�f�[�^���[�h
	LoadLogoData();
	
	s_logo.value_A = 0;
	s_logo.value_B = 16;
	G2_ChangeBlendAlpha( s_logo.value_A, s_logo.value_B );
	
	// ���C����ʂ̂�ON
	GX_DispOn();
}


// ���S���C��
int LogoMain()
{
	if( !IsLogoEnable() ) {
		return 1;
	}
	
	switch( s_logo.state ) {
	  case 0:	// �����ݒ�
		LogoInit();
		
		s_logo.mainCounter = 0;
		s_logo.state++;
		break;
		
	  case 1:	// Nintendo���S�t�F�[�h�C��
		if( s_logo.mainCounter++ < 16 ){											// Nintendoۺ� �\��
			G2_ChangeBlendAlpha( ++s_logo.value_A, --s_logo.value_B );
		}else {
			s_logo.mainCounter = 0;
			s_logo.state++;
		}
		break;
		
		
	  case 2:	// Nintendo���S�\��
		if( s_logo.mainCounter++ == LOGO_DISP_FRAME ) {
			s_logo.mainCounter = 0;
			s_logo.state++;
		}
		break;
		
	  case 3:	// Nintendo���S�t�F�[�h�A�E�g
		if( s_logo.mainCounter++ < 16 ) {
			G2_ChangeBlendAlpha( --s_logo.value_A, ++s_logo.value_B );
		}else {
			return 1;
		}
		break;
		
	  default:
		break;
	}
	
	return 0;
}


// ���S�\����OFF�ɂ���B
void SetLogoEnable( BOOL enable )
{
	s_logo.enable = enable;
}

// ���S�\����Ԏ擾
BOOL IsLogoEnable(void)
{
	return s_logo.enable;
}
