/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_select_region.c

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
#include "kami_global.h"
#include "debugger_hw_reset_control.h"
#include "debugger_card_rom.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    グローバル変数定義
 *---------------------------------------------------------------------------*/

OSTWLRegion gRegion = OS_TWL_REGION_JAPAN;

/*---------------------------------------------------------------------------*
    内部定数定義
 *---------------------------------------------------------------------------*/

static const u16 POS_Y_JAPAN     =  52;
static const u16 POS_Y_AMERICA   =  66;
static const u16 POS_Y_EUROPE    =  80;
static const u16 POS_Y_AUSTRALIA =  94;

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static const u16 sPosArray[4] =
{
	POS_Y_JAPAN,
	POS_Y_AMERICA,
	POS_Y_EUROPE,
	POS_Y_AUSTRALIA
};

const u16* sRegionStringArray[4] =
{
	L"region Japan",
	L"region America",
	L"region Europe",
	L"region Australia"
};

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessSelectRegion

  Description:  

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
void ProcessSelectRegion(void)
{
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas,  TXT_COLOR_WHITE, 0,  30, 256, 100);
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0, 130, 256,  62);
	OS_WaitVBlankIntr();

    NNS_G2dTextCanvasDrawText(&gTextCanvas2, 60, 140,
        TXT_COLOR_BLACK_BASE, TXT_DRAWTEXT_FLAG_DEFAULT,
        (const char *)
		L"\xe006 change region.\n"
		L"\xe000 choice.\n"
		L"\xe001 cancel.\n"
    );

	// 液晶を見てください。
	CARD_LockRom((u16)gLockId);
	(void)CARDi_ReadRomIDCoreEx(DEBUGGER_COMMAND_LOOK_SCREEN);
	CARD_UnlockRom((u16)gLockId);

	gRegion = OS_GetRegion();

	while(1)
	{
		s32 i;
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		kamiPadRead();

		if (FadeInMaster())
		{
			if (kamiPadIsRepeatTrigger(PAD_KEY_DOWN))
			{
				if (++gRegion > OS_TWL_REGION_AUSTRALIA)
				{
					gRegion = OS_TWL_REGION_JAPAN;
				}
			}
			else if (kamiPadIsRepeatTrigger(PAD_KEY_UP))
			{
				if (--gRegion < OS_TWL_REGION_JAPAN)
				{
					gRegion = OS_TWL_REGION_AUSTRALIA;
				}
			}

			if (kamiPadIsTrigger(PAD_BUTTON_B))
			{
				while (!FadeOutTick())
				{
				    OS_WaitVBlankIntr();
				}
				ProcessCancel((const char *)L"\n    Update was Canceld.");
			}
			else if (kamiPadIsTrigger(PAD_BUTTON_A))
			{
				break;
			}
		}

		for (i=0;i<OS_TWL_REGION_AUSTRALIA+1;i++)
		{
			if (gRegion != i)
			{
			    NNS_G2dTextCanvasDrawText(&gTextCanvas, 76, sPosArray[i], TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)sRegionStringArray[i]);
			}
			else
			{
			    NNS_G2dTextCanvasDrawText(&gTextCanvas, 76, sPosArray[i], TXT_COLOR_FREE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)sRegionStringArray[i]);
			}
		}

		DrawQuadWithColors(   0,  (s16)(sPosArray[gRegion]+2),  84, (s16)(sPosArray[gRegion]+12), GX_RGB(31, 31, 31), GX_RGB(22, 28, 31));
		DrawQuadWithColors(  84,  (s16)(sPosArray[gRegion]+2), 172, (s16)(sPosArray[gRegion]+12), GX_RGB(22, 28, 31), GX_RGB(22, 28, 31));
		DrawQuadWithColors( 172,  (s16)(sPosArray[gRegion]+2), 256, (s16)(sPosArray[gRegion]+12), GX_RGB(22, 28, 31), GX_RGB(31, 31, 31));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);

	    OS_WaitVBlankIntr();
	}

	while (!FadeOutTick())
	{
	    OS_WaitVBlankIntr();
	}
}
