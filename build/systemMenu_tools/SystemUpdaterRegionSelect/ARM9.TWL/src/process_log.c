/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_log.c

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
#include "kami_font.h"
#include "kami_global.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    グローバル変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessLog

  Description:  

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void ProcessLog(void)
{
	SystemUpdaterLog log;

	(void)FS_DeleteFile(SYSTEM_UPDATER_LOG_PATH);

	if (FS_CreateFileAuto(SYSTEM_UPDATER_LOG_PATH, FS_PERMIT_R | FS_PERMIT_W))
	{
		FSFile file;
		FS_InitFile( &file );
		if (FS_OpenFileEx(&file, SYSTEM_UPDATER_LOG_PATH, FS_FILEMODE_W))
		{
			log.magic_code  = SYSTEM_UPDATER_MAGIC_CODE;
			log.sdk_version = atoi(g_strSDKSvnRevision);
			log.ipl_version = atoi(g_strIPLSvnRevision);
			
			DC_FlushRange(&log, sizeof(log));

			if (FS_WriteFile(&file, (void*)&log, sizeof(log) ) == -1)
			{
				kamiFontPrintfConsole(FONT_COLOR_RED, "Failure : FS_WriteFile\n");
			}
			FS_CloseFile(&file);
		}
	}
	else
	{
		kamiFontPrintfConsole(FONT_COLOR_RED, "Failure : FS_CreateFileAuto\n");
	}
}

