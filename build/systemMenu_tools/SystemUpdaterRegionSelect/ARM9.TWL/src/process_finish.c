/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_finish.c

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
#include "font.h"
#include "graphics.h"
#include "keypad.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"
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
  Name:         ProcessNote

  Description:  

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void ProcessFinish(BOOL result)
{
	// TWLの更新処理が完了しました
	CARD_LockRom((u16)gLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_FINISHED);
	CARD_UnlockRom((u16)gLockId);

	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas,  TXT_COLOR_WHITE, 0,  30, 256, 100);
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0, 130, 256,  62);
	OS_WaitVBlankIntr();

	if (result)
	{
		UpdateFreePltt(GX_RGB(0, 21, 0));
		NNS_G2dTextCanvasDrawText(&gTextCanvas, 78, 72,
			TXT_COLOR_FREE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)
			L"Update Success!");
	}
	else
	{
		UpdateFreePltt(GX_RGB(31, 0, 0));
		NNS_G2dTextCanvasDrawText(&gTextCanvas, 78, 72,
			TXT_COLOR_FREE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)
			L"Update Failure!");
	}

	while(1)
	{
		if (result)
		{
			DrawQuad( 0,  30, 256, 131, GX_RGB(0, 21, 0));
		}
		else
		{
			DrawQuad( 0,  30, 256, 131, GX_RGB(31,  0,  0));
		}

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
	    OS_WaitVBlankIntr();

		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		FadeInTick();
	}
}

