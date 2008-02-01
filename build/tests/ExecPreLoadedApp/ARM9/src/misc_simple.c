/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     misc.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-01-30#$
  $Rev: 547 $
  $Author: yoshida_teruhisa $
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "misc_simple.h"
#include "data.h"

// define data-----------------------------------------------------------------


// extern data-----------------------------------------------------------------

// define data-----------------------------------------------------------------


// function's prototype-------------------------------------------------------


// global variable-------------------------------------------------------------


// static variable-------------------------------------------------------------
static u16 sScrnBuf[2][SCREEN_SIZE];      // Buffer for screen data(BG #0)

// const data------------------------------------------------------------------

void myInit( void )
{
    //---------------------------------------------------------------------------
    // Initialize:
    // They enable IRQ interrupts, initialize VRAM, and set BG #0 for text mode.
    //---------------------------------------------------------------------------
    DEMOInitCommon();
    DEMOInitVRAM();
    DEMOInitDisplayBG0Only();
    DEMOInitDisplaySubBG0Only();

    //---------------------------------------------------------------------------
    // Transmitting the character data and the palette data
    //---------------------------------------------------------------------------
    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                     GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x04000, GX_BG_EXTPLTT_01);
    
    GX_LoadBG0Char(d_64_256_bg_schDT, 0, sizeof(d_64_256_bg_schDT));
    GX_LoadBGPltt(d_64_256_bg_sclDT, 0, sizeof(d_64_256_bg_sclDT));
    
    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                     GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x08000, GX_BG_EXTPLTT_01);
                     
    GXS_LoadBG0Char(d_64_256_bg_schDT, 0, sizeof(d_64_256_bg_schDT));
    GXS_LoadBGPltt(d_64_256_bg_sclDT, 0, sizeof(d_64_256_bg_sclDT));

    GX_SetVisiblePlane(GX_PLANEMASK_BG0|GX_PLANEMASK_BG2);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0|GX_PLANEMASK_BG2);
    
    G2_SetBG0Priority(0);
    G2S_SetBG0Priority(0);
    
    GX_SetDispSelect(GX_DISP_SELECT_SUB_MAIN);

    DEMOStartDisplay();
}

void myPreMain( void )
{
	// 描画前クリア処理。まぁどのシーンでも使うので。
    myDp_Cls16(MAIN_SCREEN);
    myDp_Cls16(SUB_SCREEN);
    
    DEMOReadKey();
}

void myProMain( void )
{
    // Store the data onto the main memory, and invalidate the cache.
    // キャッシュONになってると、CPUとDMAの見てるメモリの値が違って、悲しい事が起きる
    DC_FlushRange(sScrnBuf[0], sizeof(sScrnBuf[0]));
    DC_FlushRange(sScrnBuf[1], sizeof(sScrnBuf[1]));
	// 描画後フリップ処理。まぁどのシーンでも使うので。
    OS_WaitVBlankIntr();           // Waiting the end of VBlank interrupt
    GX_LoadBG0Scr(sScrnBuf[0], 0, sizeof(sScrnBuf[0]));
	GXS_LoadBG0Scr(sScrnBuf[1], 0, sizeof(sScrnBuf[1]));
}

void myDp_Cls16(MyScreen scr)
{
	if( !(scr < SCREEN_MAX) ) return;
	MI_CpuClear8((void *)sScrnBuf[scr],SCREEN_SIZE * sizeof(u16));
}

void myDp_Cls8(MyScreen scr)
{
	if( !(scr < SCREEN_MAX) ) return;
	MI_CpuClear8((void *)sScrnBuf[scr],SCREEN_SIZE * sizeof(u8));
}

void myDp_Putchar(s32 x, s32 y, u8 color, MyScreen scr, u8 c)
{
	if( !(scr < SCREEN_MAX) ) return;
	sScrnBuf[scr][(y * 32) + x] = (u16)((color << 12) | c);
}

void myDp_Write(s32 x, s32 y, u8 color, MyScreen scr, char* str)
{
    while (*str) {
        while (x >= 32) {
            x -= 32;
            y++;
        }
        while (y >= 24) {
            return; // out of range
        }
        myDp_Putchar(x, y, color, scr, (u8)*str++);
        x++;
    }
}

void myDp_Printf(s32 x, s32 y, u8 color, MyScreen scr, const char *s, ...)
{
    va_list vlist;
    char temp[SCREEN_SIZE+1];
    temp[sizeof(temp)-1] = 0;
    va_start(vlist, s);
    vsnprintf(temp, sizeof(temp) - 1, s, vlist);
    va_end(vlist);
    myDp_Write(x, y, color, scr, temp);
}

void myDp_DrawMenu( u16 csr, MyScreen scr, const MenuParam *param )
{
	int l;
	for( l=0; l<param->num; l++)
	{
		myDp_Printf( param->pos[l].x, param->pos[l].y,
					(u8)( param->pos[l].enable ? ( l==csr ? param->select_color : param->normal_color ) : param->disable_color ),
					scr, ( l==csr ? "=>%s" : "  %s" ), param->str_elem[l] );
	}
}