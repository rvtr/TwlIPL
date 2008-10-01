/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_check_log.c

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
#include "font.h"
#include "graphics.h"
#include "keypad.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"
#include "kami_global.h"

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
static void DrawAlready(SystemUpdaterLog* log);

/*---------------------------------------------------------------------------*
  Name:         ProcessCheckLog

  Description:  

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
void ProcessCheckLog(void)
{
	// �i�X�V�\�����j
	//  1.���O�����݂��Ȃ�
	//  2.���O�����݂��A���O�ɋL�ڂ̃}�W�b�N�R�[�h���s���i����SystemUpdater���s��̏�ԁj
	//  2.���O�����݂��A���O�ɋL�ڂ̃}�W�b�N�R�[�h�������������O�ɋL�ڂ� 
    //    SDK & IPL �̃o�[�W������ SystemUpdater �̂���ȉ��ł���

#ifdef IGNORE_VERSION_CHECK
	if( 0 )
#endif // IGNORE_VERSION_CHECK
	{
		SystemUpdaterLog log;
		FSFile file;
		FS_InitFile( &file );

		if (FS_OpenFileEx(&file, SYSTEM_UPDATER_LOG_PATH, FS_FILEMODE_R) == TRUE)
		{
			DC_InvalidateRange(&log, sizeof(log));

			if (FS_ReadFile(&file, &log, sizeof(log)) == sizeof(log))
			{
				// ���O���[�h����
				OS_Printf("[%d, %d]\n", log.sdk_version, log.ipl_version);

				// ����SystemUpdater���s��ԂłȂ����Ƃ��}�W�b�N�R�[�h�Ŕ��ʂ���
				if (log.magic_code == SYSTEM_UPDATER_MAGIC_CODE)
				{
					// �}�W�b�N�R�[�h�ASDK�o�[�W�����AIPL�o�[�W�����̊m�F
					if (log.sdk_version > atoi(g_strSDKSvnRevision) || 
						log.ipl_version > atoi(g_strIPLSvnRevision))
					{
						// �X�V�s��
						DrawAlready(&log);
					}
				}
			}
			else
			{
				// ���O���[�h���s
				OS_Warning("Failure! FS_ReadFile");
			}

			FS_CloseFile(&file);
		}
	}
}

/*---------------------------------------------------------------------------*
  Name:         DrawAlready

  Description:  Already��\�����܂�

  Arguments:   

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void DrawAlready(SystemUpdaterLog* log)
{
	char string1[256];
	u16  string2[256];

	MI_CpuClear8(string1, sizeof(string1));
	MI_CpuClear8(string2, sizeof(string2));

	// ���ɃA�b�v�f�[�g�ς�
	CARD_LockRom((u16)gLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_ALREADY);
	CARD_UnlockRom((u16)gLockId);

	STD_TSPrintf(string1, "This machine has already\nbeen updated.\n\n    ver: %d %d", log->sdk_version, log->ipl_version);
	STD_ConvertStringSjisToUnicode(string2, NULL, string1, NULL, NULL);

	NNS_G2dTextCanvasDrawText(&gTextCanvas, 48, 58,
		TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char *)string2);

	while(1)
	{
	}
}
