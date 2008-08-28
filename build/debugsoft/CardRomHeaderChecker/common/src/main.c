/*---------------------------------------------------------------------------*
  Project:  TwlSDK - template - demos
  File:     main.c

  Copyright 2003-2005,2008 Nintendo.  All rights reserved.

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
#include <twl/os/common/format_rom.h>
#include <twl/hw/common/mmap_shared.h>
#include "DEMO.h"

#define LABEL_COLOR (GX_RGBA(0, 31, 31, 1))
#define VALUE_COLOR (GX_RGBA(31, 31, 31, 1))

void TwlMain(void)
{
    ROM_Header  *prh;
    char         str[100];
    u16          row = 0;
    u16          shift = 16;

    OS_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // この固定メモリアドレスにカードROMヘッダがある
    prh = (ROM_Header*)HW_TWL_CARD_ROM_HEADER_BUF;

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
    DEMODrawText( 8, 0, "Card ROM Header Checker" );

    row = 24;
    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "Title Name: " );
    MI_CpuClear8( str, 100 );
    MI_CpuCopy8( prh->s.title_name, str, TITLE_NAME_MAX );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, str );
    row += shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "Game Code: " );
    MI_CpuClear8( str, 100 );
    MI_CpuCopy8( prh->s.game_code, str, GAME_CODE_MAX );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, str );
    row += shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "TitleID_Hi: " );
    prh = (ROM_Header*)HW_TWL_CARD_ROM_HEADER_BUF;
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, "0x%08x", prh->s.titleID_Hi );
    row += shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "TitleID_Lo: " );
    MI_CpuClear8( str, 100 );
    MI_CpuCopy8( prh->s.titleID_Lo, str, 4 );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, str );
    row += shift;

    row += shift;
    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
    DEMODrawText( 8, row, "End." );

    while (1)
    {
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
    }
}

/*====== End of main.c ======*/
