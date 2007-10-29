/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     DS_Setting.c

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
#include "main.h"
#include "DS_Setting.h"

// define data-----------------------------------------------------------------

// function's prototype--------------------------------------------------------
int DS_SettingMain();
void VBlankIntr_bm(void);

// extern data-----------------------------------------------------------------

// global variable-------------------------------------------------------------

// static variable-------------------------------------------------------------

// const data------------------------------------------------------------------


// ============================================================================
// function's description
// ============================================================================

int DS_SettingMain()
{
	(void)OS_DisableIrq();
	
//	GXS_DispOff();													// LCDC OFF
	
//	reg_GX_POWCNT = 0x7fff;											// �\����ʂ���LCD�ɐ؂�ւ�
	
	OS_Printf("ARM9 bootMenu start.\n");
	
	//---- VRAM �N���A
//	GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);							// VRAM�N���A�̂��߂Ɉꎞ�I��LCDC��VRAM��S�Ċ��蓖�Ă�B
//	MI_CpuClearFast((void*)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
//	(void)GX_DisableBankForLCDC();
	
	//---- OAM�ƃp���b�g�N���A
//	MI_CpuFillFast( (void*)HW_DB_OAM, 192, HW_DB_OAM_SIZE );
//	MI_CpuClearFast((void*)HW_DB_PLTT,     HW_PLTT_SIZE);
	
	//---- VRAM�̊��蓖��
//	GX_SetBankForBG(GX_VRAM_BG_64_E); 								// VRAM���蓖��
//	GX_SetBankForOBJ(GX_VRAM_OBJ_32_FG);
	
	//---- �O���t�B�b�N�X�\�����[�h�ɂ���
//	GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
	
	//---- BG1�̐ݒ�
//	G2_SetBG1Control(GX_BG_SCRSIZE_TEXT_256x256, GX_BG_COLORMODE_16, GX_BG_SCRBASE_0xf000,
//					 GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01 );
//	G2_SetBG1Priority(3);											// BG�R���g���[�� �Z�b�g
	
	//---- OBJ,BG1�̕\���̂�ON
	GXS_SetVisiblePlane(GX_PLANEMASK_OBJ | GX_PLANEMASK_BG1);
	
	//---- OBJ��2D�}�b�v���[�h�Ŏg�p
	GXS_SetOBJVRamModeChar(GX_OBJVRAMMODE_CHAR_2D);
	
	//---- �f�[�^���[�h
//	MI_CpuCopyFast(myChar,	   (void *)HW_BG_VRAM,	sizeof(myChar));	//  BG�L�����N�^ �Z�b�g
//	MI_CpuCopyFast(myChar,	   (void *)HW_OBJ_VRAM,	sizeof(myChar));	// OBJ�L�����N�^ �Z�b�g
//	MI_CpuCopyFast(myPlttData, (void *)HW_BG_PLTT,	sizeof(myPlttData));//  BG�p���b�g   �Z�b�g
//	MI_CpuCopyFast(myPlttData, (void *)HW_OBJ_PLTT,	sizeof(myPlttData));// OBJ�p���b�g   �Z�b�g
	
	//----  V�u�����N�����؂�ւ�
	(void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr_bm);
	(void)OS_EnableIrq();
	
	//---- �\���J�n
	OS_WaitIrq(1, OS_IE_V_BLANK);
//	SVC_WaitVBlankIntr();
//	GXS_DispOn();
	
	//---- ���C�����[�v�O����
	SEQ_MainMenu_init();
	
	//================ ���C�����[�v
	while(1)
	{
		OS_WaitIrq(1, OS_IE_V_BLANK);
//		SVC_WaitVBlankIntr();										// V�u�����N�����I���҂�
		ReadKeyPad();
		mf_KEYPAD_rapid();
		
		(void)nowProcess();											// ���C���v���Z�X���s
		
		if (PAD_DetectFold() == TRUE) {								// �X���[�v���[�h�ւ̑J��
			SYSM_GoSleepMode();
		}
		
		OS_PrintServer();											// ARM7����̃v�����g�f�o�b�O����������
	}
}

//=============================================================================
// ���荞�݃��[�`��
//=============================================================================

// V�u�����N���荞��
void VBlankIntr_bm(void)
{
	//---- OAM�ABG-VRAM�̍X�V
	DC_FlushRange(oamBakS, sizeof(oamBakS));
	MI_CpuCopyFast(oamBakS,(void*)HW_DB_OAM, sizeof(oamBakS));
	DC_FlushRange(bgBakS,  sizeof(bgBakS));
	MI_CpuCopyFast(bgBakS, (void*)(HW_DB_BG_VRAM+0xf000), sizeof(bgBakS));
	//---- ���荞�݃`�F�b�N�t���O
	OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}
