/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_note.c

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
void ProcessNote(void)
{
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas,  TXT_COLOR_WHITE, 0,  30, 256, 100);
	OS_WaitVBlankIntr();
	NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0, 130, 256,  62);
	OS_WaitVBlankIntr();

//    NNS_G2dTextCanvasDrawText(&gTextCanvas, 20, 40,
//        TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT,
//        (const char *)L"Please Select System Menu Region."
//    );

    NNS_G2dTextCanvasDrawText(&gTextCanvas2, 60, 140,
        TXT_COLOR_BLACK_BASE, TXT_DRAWTEXT_FLAG_DEFAULT,
        (const char *)
		L"\xe000 ok! start update.\n"
		L"\xe001 cancel.\n"
    );
/*
	NNS_G2dTextCanvasDrawText(&gTextCanvas, 50, 40,
		TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)
		L"         -- Note --"
	);

	NNS_G2dTextCanvasDrawText(&gTextCanvas, 44, 60,
		TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)
		L"・All data in nand flash \n"
		L"  memory will be lost."
	);
*/

	NNS_G2dTextCanvasDrawText(&gTextCanvas, 80, 50,
		TXT_COLOR_FREE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)
		sRegionStringArray[gRegion]
	);

	NNS_G2dTextCanvasDrawText(&gTextCanvas, 44, 80,
		TXT_COLOR_WHITE_BASE, TXT_DRAWTEXT_FLAG_DEFAULT, (const char*)
		L"・Do not shut down while\n"
		L"  update is processing."
	);

	while(1)
	{
		G3X_Reset();
		G3_Identity();
		G3_PolygonAttr(GX_LIGHTMASK_NONE, GX_POLYGONMODE_DECAL, GX_CULL_NONE, 0, 31, 0);

		kamiPadRead();

		if (FadeInTick())
		{
			if (kamiPadIsTrigger(PAD_BUTTON_B))
			{
				while (!FadeOutTick())
				{
	    			OS_WaitVBlankIntr();
				}
				ProcessCancel((const char *)L"\n   Update was Canceled.");
			}
			else if (kamiPadIsTrigger(PAD_BUTTON_A))
			{
				while (!FadeOutTick())
				{
	    			OS_WaitVBlankIntr();
				}
				NNS_G2dCharCanvasClearArea(&gCanvas2, TXT_COLOR_BLACK, 0, 130, 256,  62);
				return;
			}
		}
		DrawQuadWithColors(   0,  50, 128, 62, GX_RGB(31, 31, 31), GX_RGB(22, 28, 31));
		DrawQuadWithColors( 128,  50, 256, 62, GX_RGB(22, 28, 31), GX_RGB(31, 31, 31));

		G3_SwapBuffers(GX_SORTMODE_AUTO, GX_BUFFERMODE_W);
	    OS_WaitVBlankIntr();
	}
}
