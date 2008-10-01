/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_hwinfo.c

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

#include <stdlib.h>	// atoi
#include <twl.h>
#include <twl/nam.h>
#include "font.h"
#include "graphics.h"
#include "keypad.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"
#include "kami_global.h"
#include "kami_pxi.h"
#include "kami_font.h"
#include "hw_info.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �O���[�o���ϐ���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����萔��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessHwinfo

  Description:  

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
BOOL ProcessHwinfo(void)
{
	const int MAX_RETRY_COUNT = 2;
	BOOL hw_info_result;
	int i;

	// �S�n�[�h�E�F�A���̍X�V
	for (i=0;i<MAX_RETRY_COUNT;i++)
	{
		hw_info_result = WriteHWInfoFile(gRegion, OS_IsForceDisableWireless());
		if (hw_info_result)
		{
			kamiFontPrintfConsole( FONT_COLOR_GREEN, "Write Hardware Info Success.\n");			
			break;
		}
		else
		{
			kamiFontPrintfConsole(CONSOLE_RED, "Write Hardware Info Retry!\n");
		}
	}

	if ( hw_info_result)
	{
		return TRUE;
	}
	{
		kamiFontPrintfConsole(FONT_COLOR_RED, "Write Hardware Info Failure!\n");			
		return FALSE;
	}
}
