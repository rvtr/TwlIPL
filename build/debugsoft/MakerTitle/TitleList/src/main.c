/*---------------------------------------------------------------------------*
  Project:  TwlSDK - demos - yasuda - MenuTest
  File:     main.c

  Copyright 2003 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifdef  SDK_TWL
#include <twl.h>
#else
#include <nitro.h>
#endif
#include <nitro/rtc.h>
#include <nitro/std.h>
#include "font.h"
#include "screen.h"
#include "appmain.h"
#include "mycode.h"         //  Makeで自動生成されるはずのファイル.

static void myInit(void);
static void myVBlankIntr(void);
static void InitializeAllocateSystem(void);
static void PrintBootType(void);

extern BOOL PubDataAccessTest(void);

/*---------------------------------------------------------------------------*
  Name:         NitroMain

  Description:  main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
#ifdef  SDK_TWL
void TwlMain(void)
#else
void NitroMain(void)
#endif
{
    u16     trigger;
    u16     preButton = PAD_Read();
    u16     button;

    //---------------- initialize
    myInit();
    PrintBootType();
    
    ClearSubScreen();

    //---------------- main loop
    while (TRUE)
    {
        button = PAD_Read();
        trigger = (u16)((button ^ preButton) & button);
        preButton = button;

        //---- clear screen buffer
        ClearScreen();

        if(trigger & PAD_BUTTON_A)
        {
            DoWritingTest();
        }
        if(trigger & PAD_BUTTON_B)
        {
            DoReadingTest();
        }
        
        if((trigger & PAD_BUTTON_X) && STD_StrCmp("4KAA", MYCODE) == 0)
        {
            DeleteSaveDatas();
        }

        
        //---- display menus
  //      DisplayMenuSet();

        //---- press UP key (toggle menu)
/*        if(trigger & PAD_KEY_UP)
        {
            ChangeMenuItem(MENU_ITEM_UP);
        }

        if(trigger & PAD_KEY_DOWN)
        {
            ChangeMenuItem(MENU_ITEM_DOWN);
        }
        if(trigger & PAD_KEY_RIGHT)
        {
            ExecMenuItemRight();
        }
        if(trigger & PAD_KEY_LEFT)
        {
            ExecMenuItemLeft();
        }

        //---- push A
        if (trigger & PAD_BUTTON_A)
        {
            ExecMenuItem();
        }
        //---- push B
        if (trigger & PAD_BUTTON_B)
        {
            ExecMenuItemB();
        }
        //---- push X
        if (trigger & PAD_BUTTON_X)
        {
            ExecMenuItemX();
        }*/
        
        DrawScreen();
        
        OS_WaitVBlankIntr();
    }
}

//----------------------------------------------------------------
//  myInit
//
void myInit(void)
{
    //---- init
    OS_Init();
    OS_InitTick();
    OS_InitAlarm();
    RTC_Init();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();
	InitializeAllocateSystem();

    //---- init displaying
    GX_SetBankForLCDC(GX_VRAM_LCDC_ALL);
    MI_CpuClearFast((void *)HW_LCDC_VRAM, HW_LCDC_VRAM_SIZE);
    (void)GX_DisableBankForLCDC();

    MI_CpuFillFast((void *)HW_OAM, 192, HW_OAM_SIZE);
    MI_CpuClearFast((void *)HW_PLTT, HW_PLTT_SIZE);
    MI_CpuFillFast((void *)HW_DB_OAM, 192, HW_DB_OAM_SIZE);
    MI_CpuClearFast((void *)HW_DB_PLTT, HW_DB_PLTT_SIZE);

    //---- setting 2D for top screen
    GX_SetBankForBG(GX_VRAM_BG_128_A);

    G2_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                     GX_BG_COLORMODE_16,
                     GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2_SetBG0Priority(0);
    G2_BG0Mosaic(FALSE);
    GX_SetGraphicsMode(GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D);
    GX_SetVisiblePlane(GX_PLANEMASK_BG0);

    GX_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GX_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));



    //---- setting 2D for bottom screen
    GX_SetBankForSubBG(GX_VRAM_SUB_BG_128_C);

    G2S_SetBG0Control(GX_BG_SCRSIZE_TEXT_256x256,
                      GX_BG_COLORMODE_16,
                      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x00000, GX_BG_EXTPLTT_01);
    G2S_SetBG0Priority(0);
    G2S_BG0Mosaic(FALSE);
    GXS_SetGraphicsMode(GX_BGMODE_0);
    GXS_SetVisiblePlane(GX_PLANEMASK_BG0);

    GXS_LoadBG0Char(d_CharData, 0, sizeof(d_CharData));
    GXS_LoadBGPltt(d_PaletteData, 0, sizeof(d_PaletteData));


    //---- screen
    MI_CpuFillFast((void *)gScreen, 0, sizeof(gScreen));
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(&gScreen[0], 0, sizeof(gScreen) / 2);
    GXS_LoadBG0Scr(&gScreen[1], 0, sizeof(gScreen) / 2);

    //---- init interrupt
    OS_SetIrqFunction(OS_IE_V_BLANK, myVBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

	//---- FileSystemInit
    (void)OS_EnableIrqMask(OS_IE_SPFIFO_RECV);
    FS_Init( FS_DMA_NOT_USE );

    //---- start displaying
    GX_DispOn();
    GXS_DispOn();
}

//----------------------------------------------------------------
//  myVBlankIntr
//             vblank interrupt handler
//
static void myVBlankIntr(void)
{
    //---- upload pseudo screen to VRAM
    DC_FlushRange(gScreen, sizeof(gScreen));
    /* DMA操作でIOレジスタへアクセスするのでキャッシュの Wait は不要 */
    // DC_WaitWriteBufferEmpty();
    GX_LoadBG0Scr(&gScreen[0], 0, sizeof(gScreen) / 2);
    GXS_LoadBG0Scr(&gScreen[1], 0, sizeof(gScreen) / 2);


    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem

  Description:  メインメモリ上のアリーナにてメモリ割当てシステムを初期化する。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void InitializeAllocateSystem(void)
{
    void *tempLo;
    OSHeapHandle hh;

    tempLo = OS_InitAlloc(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi(), 1);
    OS_SetArenaLo(OS_ARENA_MAIN, tempLo);
    hh = OS_CreateHeap(OS_ARENA_MAIN, OS_GetMainArenaLo(), OS_GetMainArenaHi());
    if (hh < 0)
    {
        OS_Panic("ARM9: Fail to create heap...\n");
    }
    hh = OS_SetCurrentHeap(OS_ARENA_MAIN, hh);
}

static void PrintBootType(void)
{
    const OSBootType btype = OS_GetBootType();

    switch( btype )
    {
    case OS_BOOTTYPE_ROM:   OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_ROM\n"); break;
    case OS_BOOTTYPE_NAND:  OS_TPrintf("OS_GetBootType = OS_BOOTTYPE_NAND\n"); break;
    default:
        {
            OS_Warning("unknown BootType(=%d)", btype);
        }
        break;
    }
}

