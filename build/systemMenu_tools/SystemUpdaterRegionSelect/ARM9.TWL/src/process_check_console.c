/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     check_console.c

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
#include "kami_global.h"
#include "kami_font.h"
#include "kami_pxi.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����萔��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/
static RunningConsole sRunning = UNKNOWN;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/
static RunningConsole CheckConsole(void);

/*---------------------------------------------------------------------------*
    �֐���`
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         ProcessCheckConsole

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void ProcessCheckConsole(void)
{
	BOOL isAdapter;	
	u16 batLevel;

	sRunning = CheckConsole();

	switch (sRunning)
	{
	case IS_TWL_DEBUGGER:
		kamiFontPrintfConsole( FONT_COLOR_GREEN, "Running on IS_TWL_DEBUGGER.\n");
		break;
	case IS_TWL_CAPTURE:
		kamiFontPrintfConsole( FONT_COLOR_GREEN, "Running on IS_TWL_CAPTURE.\n");
		break;
	case TWL:
		kamiFontPrintfConsole( FONT_COLOR_GREEN, "Running on TWL CONSOLE.\n");
		break;
	case UNKNOWN:
		kamiFontPrintfConsole( FONT_COLOR_GREEN, "Running on UNKNOWN.\n");
		break;
	}
/*
#ifdef SYSM_BUILD_FOR_DEBUGGER
	// �f�o�b�K����SystemUpdater�͎��@�ƃL���v�`���ł͓��삳���Ȃ�
	if (sRunning != IS_TWL_DEBUGGER)
	{
		ProcessCancel((const char *)
			L" Sorry,\n"
			L" This SystemUpdater can not\n"
			L" execute on TWL-CONSOLE.     "
		);
	}
#else
    // ���@����SystemUpdater�̓f�o�b�K�ł͓��삳���Ȃ�
	if (sRunning == IS_TWL_DEBUGGER)
	{
		ProcessCancel((const char *)
			L" Sorry,\n"
			L" This SystemUpdater can not\n"
			L" execute on IS-TWL-DEBUGGER. "
		);
	}
#endif  // SYSM_BUILD_FOR_DEBUGGER
*/
    // UNKNOWN�͓��삳���Ȃ�
	if (sRunning == UNKNOWN)
	{
		ProcessCancel((const char *)
			L" Sorry,\n"
			L" This SystemUpdater can not\n"
			L" execute on UNKNOWN CONSOLE. "
		);
	}

    // �d�r�c�ʂ����Ȃ���Γ��삳���Ȃ�
	while (PM_GetBatteryLevel( &batLevel ) != PM_RESULT_SUCCESS)
	{
		OS_Sleep(1);
	}
	while (PM_GetACAdapter( &isAdapter ) != PM_RESULT_SUCCESS)
	{
		OS_Sleep(1);
	}
	if (((batLevel <= 2) && ! isAdapter) ||
		 (batLevel <= 1))		
	{
		ProcessCancel((const char *)
			L" Sorry,\n"
			L" This SystemUpdater can not\n"
			L" execute if battery is low.  "
		);
	}

	kamiFontLoadScreenData();
}

/*---------------------------------------------------------------------------*
  Name:         CheckConsole

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
static RunningConsole CheckConsole(void)
{
	u32 console = OS_GetRunningConsoleType();
	RunningConsole running = UNKNOWN;

	// SystemUpdater�̓f�o�b�O�s�ō쐬����邽��OS_CONSOLE_TWL���擾�����
	// �Ԕ��ɃJ�[�h��}����SystemUpdater�����s�����ꍇ�����l�i�A��OS_CONSOLE_TWLTYPE_RETAIL�ɂ͂Ȃ�Ȃ��j
	// �f�o�b�K���ǂ����̔���̓������T�C�Y�`�F�b�N�ɂ��s��
	// �O�̂���OS_CONSOLE_TWLTYPE_RETAIL�łȂ����Ƃ��m�F����

	if ((console & OS_CONSOLE_SIZE_MASK) == OS_CONSOLE_SIZE_32MB)
	{
		if ((console & OS_CONSOLE_TWLTYPE_MASK) != OS_CONSOLE_TWLTYPE_RETAIL)
		{
			IsToolType type;
			kamiGetIsToolType(&type);
			if (type == IS_TOOL_TYPE_DEBUGGER)
			{
				running = IS_TWL_DEBUGGER;
			}
			else if (type == IS_TOOL_TYPE_ERROR) // TS�{�[�h�v���X + ���d�l�f�o�b�K
			{
				running = IS_TWL_DEBUGGER;
			}
			else if (type == IS_TOOL_TYPE_CAPTURE)
			{
				running = IS_TWL_CAPTURE;
			}
		}
	}
	else if ((console & OS_CONSOLE_MASK) == OS_CONSOLE_TWL)
	{
		IsToolType type;
		kamiGetIsToolType(&type);
		if (type == IS_TOOL_TYPE_CAPTURE)
		{
			running = IS_TWL_CAPTURE;
		}
		else
		{
			running = TWL;
		}
	}

	return running;
}

/*---------------------------------------------------------------------------*
  Name:         GetConsole

  Description:  

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
RunningConsole GetConsole(void)
{
	return sRunning;
}
