/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - FS - overlay
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/


#include <nitro.h>

#include "DEMO.h"
#include "loadSharedFont.h"

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  アプリケーションメインエントリ

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void NitroMain(void)
{
	
    OS_Init();
	OS_InitTick();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

     {
        OSHeapHandle hh;
        void   *tmp;
        tmp = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
        OS_SetArenaLo(OS_ARENA_MAIN, tmp);
        hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
        if (hh < 0)
        {
            OS_TPanic("ARM9: Fail to create heap...\n");
        }
        (void)OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
    }
	
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    FS_Init(FS_DMA_NOT_USE);

    OS_TPrintf("--------------------------------\n"
               "Shared Font sample.\n");

	if( LoadSharedFontInit() ) {
		
		while( !IsFinishedLoadSharedFont() ) {
		    OS_WaitVBlankIntr();
		}
		
	    // 結果表示
	    {
			int     i;
	        int     ox = 10;
	        int     oy = 60;
	        DEMOFillRect(0, 0, GX_LCD_SIZE_X, GX_LCD_SIZE_Y, DEMO_RGB_CLEAR);
	        DEMOSetBitmapTextColor(GX_RGBA(0, 31, 0, 1));
	        DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
	        DEMODrawFrame(ox, oy, 240, 10 + ( OS_SHARED_FONT_CN_KR_MAX + 1 ) * 10, GX_RGBA( 0, 31, 0, 1));
	        for (i = 0; i < OS_SHARED_FONT_CN_KR_MAX + 1; ++i)
	        {
	            DEMODrawText(ox + 10, oy + 5 + i * 10, "%s load %s",
	                         str_fontname[ i ], g_isSucceededLoad[ i ] ? "suceeded" : "failed");
	        }
	    }
	    DEMO_DrawFlip();
	    OS_WaitVBlankIntr();
	}
	
    OS_Terminate();
}


