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
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static Process sNextProcess;
static int brightness = 16;

/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/

static BOOL MakeETicketFile(void);

/*---------------------------------------------------------------------------*
    プロセス関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         fadeInProcess プロセス

  Description:  フェードインを行い完了後にsNextProcessに遷移します

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
  Name:         fadeOutProcess プロセス

  Description:  フェードアウトを行い完了後にsNextProcessに遷移します

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
    その他関数定義
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*
  Name:         SetNextProcess

  Description:  プロセスの遷移時にフェードを使いたい場合に使用します

  Arguments:    processフェード完了後のプロセス

  Returns:      None.
 *---------------------------------------------------------------------------*/

void SetNextProcess(Process process)
{
	sNextProcess = process;
}

