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

#define DMA_NO_FS			1

/*---------------------------------------------------------------------------*
    �ϐ� ��`
 *---------------------------------------------------------------------------*/
// �L�[����
static KeyInfo  gKey;

static const char gRomPath[32] = "rom:/dummy.srl";
static u64 gTargetId = 0x0003000434333441;

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
	
	FS_Init(DMA_NO_FS);
	
	GX_DispOn();
	GXS_DispOn();
	
	ClearScreen();
	
	// ��x��ǂ�
	ReadKey(&gKey);
	
	while(TRUE)
	{
		ReadKey(&gKey);
		
		if (gKey.trg & PAD_BUTTON_A)
		{
#ifdef SDK_TWL
			BOOL success = TRUE;
			FSFile src, dest;
			void *buf;
			s32 len = 0;
			
			// srl �� NAND ��� OS_TMP_APP_PATH �փR�s�[
			FS_DeleteFile(OS_TMP_APP_PATH);
			FS_CreateFile(OS_TMP_APP_PATH, FS_PERMIT_R | FS_PERMIT_W);
			FS_InitFile( &src );
			FS_InitFile( &dest );
			if ( !FS_OpenFileEx( &src, gRomPath, FS_FILEMODE_R ) ) success = FALSE;
			len = (s32)FS_GetFileLength( &src );
			
			buf = OS_AllocFromMain((u32)len);
			if (buf == NULL)
			{
				success = FALSE;
				break;
			}
			
			if ( -1 == FS_ReadFile( &src, buf, len ) )
			{
				success = FALSE;
				break;
			}
			
			if ( !FS_CloseFile( &src ) ) success = FALSE;
			
			if ( !FS_OpenFileEx( &dest, OS_TMP_APP_PATH, FS_FILEMODE_W ) ) success = FALSE;
			if ( -1 == FS_WriteFile( &dest, buf, len ) ) success = FALSE;
			if ( !FS_CloseFile( &dest ) ) success = FALSE;
			
			if ( !success ) break;
			
			// TMP �W�����v�����݂�
			if ( !OS_DoApplicationJump( gTargetId, OS_APP_JUMP_TMP) )
			{
				PutSubScreen(1, 10, 0xf1, "ERROR!: Failed to TMP jump");
			}
			break;
#endif
		}
		
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
	PutMainScreen(1, 18, 0xff, "A : try TMP jump");
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

