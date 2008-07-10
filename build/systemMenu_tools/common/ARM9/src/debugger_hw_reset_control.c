/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     debugger_hw_reset_control.c

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

#include <twl.h>
#include "debugger_hw_reset_control.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����萔��`
 *---------------------------------------------------------------------------*/

#define OS_THREAD_PRIORITY_IS_TWL_DEBUGGER_HW_RESET_CONTROL  15

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

vu8       sHwResetEnable = TRUE;
OSThread  sThread;
u32       sStack[1024];
static s32 sLockId;

/*---------------------------------------------------------------------------*
    �֐��錾
 *---------------------------------------------------------------------------*/
static void CardAccessThread(void* arg);

/*---------------------------------------------------------------------------*
  Name:         CardAccessThread

  Description:  5�s���Ƀ_�~�[�̃J�[�h�A�N�Z�X���s���܂��B

  Arguments:    arg -   �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CardAccessThread(void* arg)
{
#pragma unused(arg)

    while (!sHwResetEnable)
    {
		CARD_LockRom((u16)sLockId);
		(void)CARDi_ReadRomID();
		CARD_UnlockRom((u16)sLockId);

		// 5�b�ԃX���[�v
		OS_Sleep(5000);
    }
}

/*---------------------------------------------------------------------------*
  Name:         DEBUGGER_HwResetDisable

  Description:  IS-TWL-DEBUGGER�ł̃n�[�h�E�F�A���Z�b�g���֎~���܂��B
                ���̋@�\�̓f�o�b�K�f�B�[�[�u���t���O���w�肵��SRL 
				�ł̂ݗL���ł��B��������Ƃ��ẮA5�b���ɃJ�[�h�A�N�Z�X��
				�s���X���b�h�𐶐��N�����Ă��܂��BIS-TWL-DEBUGGER��
				�J�[�h�A�N�Z�X���Ď����Ă���10�b�ԃJ�[�h�A�N�Z�X���Ȃ�
				�ꍇ�Ƀn�[�h�E�F�A���Z�b�g��������d�g�݂ɂȂ��Ă��܂��B

  Arguments:   	None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEBUGGER_HwResetDisable( void )
{
	if (sLockId == 0)
	{
		sLockId = OS_GetLockID();
	}

	if (sHwResetEnable)
	{
		sHwResetEnable = FALSE;

	    OS_CreateThread(&sThread, CardAccessThread, NULL,
	        			(void*)((u32)sStack + sizeof(sStack)), sizeof(sStack), 
						OS_THREAD_PRIORITY_IS_TWL_DEBUGGER_HW_RESET_CONTROL);
	    OS_WakeupThreadDirect(&sThread);
	}
}

/*---------------------------------------------------------------------------*
  Name:         DEBUGGER_HwResetEnable

  Description:  IS-TWL-DEBUGGER�ł̃n�[�h�E�F�A���Z�b�g�������܂��B
                ���̋@�\�̓f�o�b�K�f�B�[�[�u���t���O���w�肵��SRL 
				�ł̂ݗL���ł��B���ۂɃn�[�h�E�F�A���Z�b�g���\�ɂȂ�ɂ�
				�ő��10�b������܂��B

  Arguments:   	None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void DEBUGGER_HwResetEnable( void )
{
	if (!sHwResetEnable)
	{
    	sHwResetEnable = TRUE;
		OS_WakeupThreadDirect(&sThread);
		while (!OS_IsThreadTerminated(&sThread)){}
	}
}
