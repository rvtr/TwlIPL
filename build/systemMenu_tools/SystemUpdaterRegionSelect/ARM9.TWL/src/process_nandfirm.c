/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_nandfirm.c

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
#include "kami_write_nandfirm.h"

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
  Name:         ProcessNandfirm

  Description:  

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
BOOL ProcessNandfirm(void)
{
    FSFile  dir;
    FSDirectoryEntryInfo   info[1];
	char full_path[FS_ENTRY_LONGNAME_MAX+6];
	const int MAX_RETRY_COUNT = 2;
	BOOL result = FALSE;
	BOOL find = FALSE;
	int i;

	// �K�؂ȃf�B���N�g�����J��
	STD_TSNPrintf(full_path, sizeof(full_path), "rom:/data/common/");

	FS_InitFile(&dir);
	if (!FS_OpenDirectory(&dir, full_path, FS_FILEMODE_R))
	{
    	kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory()\n");
		return FALSE;
	}

	// .nand�t�@�C�����������ăC���|�[�g
    while (FS_ReadDirectory(&dir, info))
    {
        if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
        {
			char* pExtension;

			// �g���q�̃`�F�b�N
			pExtension = STD_SearchCharReverse( info->longname, '.');
			if (pExtension)
			{
				if (!STD_CompareString( pExtension, ".nand") || !STD_CompareString( pExtension, ".NAND")  )
				{
					STD_TSNPrintf(full_path, sizeof(full_path), "rom:/data/common/%s", info->longname);
					find = TRUE;
					break;
				}
			}
        }
	}

	if (find)
	{
		for (i=0;i<MAX_RETRY_COUNT;i++)
		{
			result = kamiWriteNandfirm(full_path, OS_AllocFromMain, OS_FreeToMain);
			if (result)
			{
				kamiFontPrintfConsole(FONT_COLOR_GREEN, "Firm Update Success.\n");			
				break;
			}
			else
			{
				kamiFontPrintfConsole(CONSOLE_RED, "Write Firm Retry!\n");
			}
		}
		if (result == FALSE)
		{
			kamiFontPrintfConsole(FONT_COLOR_RED, "Firm Update Failure!\n");
		}
	}

	OS_WaitVBlankIntr();
	kamiFontLoadScreenData();
	return result;
}
