/*---------------------------------------------------------------------------*
  Project:  TwlSDK - tests - tmpjumpTest - AppforTmpJump
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifdef SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif

#include "common.h"
#include "screen.h"

/*---------------------------------------------------------------------------*
    �ϐ� ��`
 *---------------------------------------------------------------------------*/
// �L�[����
static KeyInfo  gKey;

/*---------------------------------------------------------------------------*
   Prototype
 *---------------------------------------------------------------------------*/
static void DrawMainScene(void);
void VBlankIntr(void);

#ifdef SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
	InitCommon();
	InitScreen();
	
	GX_DispOn();
	GXS_DispOn();
	
	ClearScreen();
	
	// ��x��ǂ�
	ReadKey(&gKey);
	
	while(TRUE)
	{
		ReadKey(&gKey);
		
		if (gKey.trg & PAD_BUTTON_B)
		{
#ifdef SDK_TWL
			// ���C���A�v���֖߂�
			if ( !OS_ReturnToPrevApplication() )
			{
				PutSubScreen(1, 10, 0xf1, "ERROR!: Failed to return jump");
			}
			break;
#endif
		}
		
		DrawMainScene();
		
		OS_WaitVBlankIntr();
	}
	
	OS_WaitVBlankIntr();
	OS_Terminate();
}

static void DrawMainScene(void)
{
	PutMainScreen(1, 1, 0xf2, "Application on tmp dir");
#ifdef SDK_TWL
	PutMainScreen(1, 20, 0xff, "B : Return to Main App");
#else
	PutMainScreen(1, 20, 0xf8, "Can not return to Main (OK)");
#endif
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr

  Description:  �u�u�����N�����݃n���h���B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void VBlankIntr(void)
{
    // �e�L�X�g�\�����X�V
    UpdateScreen();

    // IRQ �`�F�b�N�t���O���Z�b�g
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

