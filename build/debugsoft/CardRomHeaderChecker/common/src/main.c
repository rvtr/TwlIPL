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

#define COLOR_WHITE  (GX_RGBA(31, 31, 31, 1))
#define COLOR_CYAN   (GX_RGBA(0, 31, 31, 1))
#define COLOR_RED    (GX_RGBA(31, 0, 0, 1))
#define COLOR_YELLOW (GX_RGBA(31, 31, 0, 1))

#define TITLE_COLOR  COLOR_YELLOW
#define LABEL_COLOR  COLOR_WHITE
#define VALUE_COLOR  COLOR_WHITE
#define OK_COLOR     COLOR_CYAN
#define NG_COLOR     COLOR_RED

// CRC計算
#define CRC16_INIT_VALUE    0xffff
#define CALC_CRC16_SIZE     0x15e
static u16 CalcCRC16(u16 start, u8 *data, int size);

void TwlMain(void)
{
    ROM_Header  *prhTWL, *prhNTR;
    char         str[100];
    u16          row = 0;
    u16          shift = 8;
    u16          crc;

    OS_Init();
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBitmap();
    DEMOHookConsole();

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 0, 1));
    DEMOSetBitmapGroundColor(DEMO_RGB_CLEAR);
    DEMOStartDisplay();

    // カードROMヘッダ: NTR互換用
    prhNTR = (ROM_Header*)HW_CARD_ROM_HEADER;

    DEMOSetBitmapTextColor(COLOR_YELLOW);
    DEMODrawText( 8, row, "Card ROM Header" );
    row += 2 * shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "Title Name:" );
    MI_CpuClear8( str, 100 );
    MI_CpuCopy8( prhNTR->s.title_name, str, TITLE_NAME_MAX );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, str );
    row += shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "Game Code:" );
    MI_CpuClear8( str, 100 );
    MI_CpuCopy8( prhNTR->s.game_code, str, GAME_CODE_MAX );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, str );
    row += shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "Header CRC:" );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, "0x%04x", prhNTR->s.header_crc16 );
    row += shift;

    crc = CalcCRC16( CRC16_INIT_VALUE, (u8*)prhNTR, CALC_CRC16_SIZE );
    if( crc == prhNTR->s.header_crc16 )
    {
       DEMOSetBitmapTextColor(OK_COLOR);
       DEMODrawText( 96, row, "0x%04x OK", crc );
    }
    else
    {
       DEMOSetBitmapTextColor(NG_COLOR);
       DEMODrawText( 96, row, "0x%04x NG", crc );
    }
    row += 3 * shift;

    // この固定メモリアドレスにTWLカードROMヘッダがある
    // これはカードROMヘッダとは異なる
    // カードROMヘッダ:    NTR互換のための領域でROMヘッダのNTR互換部分だけがある
    // TWLカードROMヘッダ: NTR互換部分に加えてROMヘッダのTWLで追加された部分もある
    prhTWL = (ROM_Header*)HW_TWL_CARD_ROM_HEADER_BUF;

    DEMOSetBitmapTextColor(COLOR_YELLOW);
    DEMODrawText( 8, row, "TWL Card ROM Header" );
    row += 2 * shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "Title Name: " );
    MI_CpuClear8( str, 100 );
    MI_CpuCopy8( prhTWL->s.title_name, str, TITLE_NAME_MAX );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, str );
    row += shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "Game Code: " );
    MI_CpuClear8( str, 100 );
    MI_CpuCopy8( prhTWL->s.game_code, str, GAME_CODE_MAX );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, str );
    row += shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "TitleID_Hi: " );
    prhTWL = (ROM_Header*)HW_TWL_CARD_ROM_HEADER_BUF;
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, "0x%08x", prhTWL->s.titleID_Hi );
    row += shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "TitleID_Lo: " );
    MI_CpuClear8( str, 100 );
    MI_CpuCopy8( prhTWL->s.titleID_Lo, str, 4 );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, str );
    row += shift;

    DEMOSetBitmapTextColor(LABEL_COLOR);
    DEMODrawText( 8, row, "Header CRC:" );
    DEMOSetBitmapTextColor(VALUE_COLOR);
    DEMODrawText( 96, row, "0x%04x", prhTWL->s.header_crc16 );
    row += shift;

    crc = CalcCRC16( CRC16_INIT_VALUE, (u8*)prhTWL, CALC_CRC16_SIZE );
    if( crc == prhTWL->s.header_crc16 )
    {
       DEMOSetBitmapTextColor(OK_COLOR);
       DEMODrawText( 96, row, "0x%04x OK", crc );
    }
    else
    {
       DEMOSetBitmapTextColor(NG_COLOR);
       DEMODrawText( 96, row, "0x%04x NG", crc );
    }
    row += 2 * shift;

    DEMOSetBitmapTextColor(GX_RGBA(31, 31, 31, 1));
    DEMODrawText( 8, row, "End." );
    row += 2 * shift;
    DEMODrawText( 8, row, "Press A to check Self-Jump." );

    while (1)
    {
		DEMOReadKey();
        DEMO_DrawFlip();
        OS_WaitVBlankIntr();
        if(DEMO_IS_TRIG(PAD_BUTTON_A)) OS_RebootSystem();
    }
}


/*---------------------------------------------------------------------------*
 *   Math
 *
 *	u16 CalcCRC16( u16 start, u8 *data, int size )
 *---------------------------------------------------------------------------*/

static u16 crc16_table[16] = {
    0x0000, 0xCC01, 0xD801, 0x1400,
    0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401,
    0x5000, 0x9C01, 0x8801, 0x4400
};

static u16 CalcCRC16(u16 start, u8 *data, int size)
{
    u16     r1;
    u16     total = start;

    while (size-- > 0)
    {
        // 下位4bit
        r1 = crc16_table[total & 0xf];
        total = (u16)((total >> 4) & 0x0fff);
        total = (u16)(total ^ r1 ^ crc16_table[*data & 0xf]);

        // 上位4bit
        r1 = crc16_table[total & 0xf];
        total = (u16)((total >> 4) & 0x0fff);
        total = (u16)(total ^ r1 ^ crc16_table[(*data >> 4) & 0xf]);

        data++;
    }
    return total;
}

/*====== End of main.c ======*/
