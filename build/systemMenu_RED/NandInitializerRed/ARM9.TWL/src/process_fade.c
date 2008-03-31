/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_fade.c

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
#include "process_fade.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static Process sNextProcess;
static int brightness = 16;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL MakeETicketFile(void);

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         fadeInProcess �v���Z�X

  Description:  �t�F�[�h�C�����s���������sNextProcess�ɑJ�ڂ��܂�

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/
void* fadeInProcess(void)
{
	if (--brightness < 0)
	{
		brightness = 0;
		return sNextProcess;
	}
	else
	{
		GXS_SetMasterBrightness(brightness);
		return fadeInProcess;
	}
}

/*---------------------------------------------------------------------------*
  Name:         fadeOutProcess �v���Z�X

  Description:  �t�F�[�h�A�E�g���s���������sNextProcess�ɑJ�ڂ��܂�

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/
void* fadeOutProcess(void)
{
	if (++brightness > 16)
	{
		brightness = 16;
		return sNextProcess;
	}
	else
	{
		GXS_SetMasterBrightness(brightness);
		return fadeOutProcess;
	}
}

/*---------------------------------------------------------------------------*
    ���̑��֐���`
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         SetNextProcess

  Description:  �v���Z�X�̑J�ڎ��Ƀt�F�[�h���g�������ꍇ�Ɏg�p���܂�

  Arguments:    process�t�F�[�h������̃v���Z�X

  Returns:      None.
 *---------------------------------------------------------------------------*/

void SetNextProcess(Process process)
{
	sNextProcess = process;
}

