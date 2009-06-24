/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - ts_dev9
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <firm.h>
#include  "font.h"
#include  "screen.h"

//#define PRINT_DEBUG

#ifndef PRINT_DEBUG
#undef  OS_TPrintf
#define OS_TPrintf(...) ((void)0)
#endif // PRINT_DEBUG

static void myInit(void);
static void myVBlankIntr(void);

/***************************************************************
    PreInit

    FromBootの対応＆OS_Init前に必要なメインメモリの初期化
***************************************************************/
static void PreInit(void)
{
    /*
     メインメモリ関連
    */
    // SHARED領域クリア
    MI_CpuClearFast((void*)HW_WRAM_EX_LOCK_BUF,         (HW_WRAM_EX_LOCK_BUF_END - HW_WRAM_EX_LOCK_BUF));
    MI_CpuClearFast((void*)HW_BIOS_EXCP_STACK_MAIN,     (HW_REAL_TIME_CLOCK_BUF - HW_BIOS_EXCP_STACK_MAIN));
    MI_CpuClearFast((void*)HW_PXI_SIGNAL_PARAM_ARM9,    (HW_MMEMCHECKER_MAIN - HW_PXI_SIGNAL_PARAM_ARM9));
    MI_CpuClearFast((void*)HW_ROM_HEADER_BUF,           (HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF));

    // FromBrom全消去
    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );

    // ブートタイプの変更
    ( (OSBootInfo *)OS_GetBootInfo() )->boot_type = OS_BOOTTYPE_NAND;
}

/***************************************************************
    PostInit

    各種初期化
***************************************************************/
static void PostInit(void)
{
}

/***************************************************************
    EraseAll

    不正終了しました
    いろいろ消してください
    DSモードにして終わるのがよいか？
***************************************************************/
static void EraseAll(void)
{
}

void TwlMain( void )
{
    u32 len;
#define X_OFF   2
    s16 y = 2;

    PreInit();
    myInit();
    PostInit();

    //---- clear screen buffer
    ClearScreen();

    PrintString( X_OFF, y, FONT_CYAAN, "NAND Firm Writer" );
    PrintString( X_OFF+18, y++, FONT_YELLOW, "%s", __DATE__ );
    PrintString( X_OFF+21, y++, FONT_YELLOW, "%s", __TIME__ );
    OS_WaitVBlankIntr();
    y++;

    PXI_RecvStream(&len, sizeof(len));
    PrintString( X_OFF, y++, FONT_WHITE, "Firm length: %d bytes", len );
    OS_WaitVBlankIntr();
    y++;

    PrintString( X_OFF, y++, FONT_WHITE, "Load NAND Firm..." );
    OS_WaitVBlankIntr();

    if ( PXI_RecvID() != FIRM_PXI_ID_NULL )
    {
        goto err;
    }
    PrintString( X_OFF+20, y++, FONT_GREEN, "Done." );
    PrintString( X_OFF, y++, FONT_WHITE, "Write NAND Firm..." );
    OS_WaitVBlankIntr();

    if ( PXI_RecvID() != FIRM_PXI_ID_NULL )
    {
        goto err;
    }
    PrintString( X_OFF+20, y++, FONT_GREEN, "Done." );
    PrintString( X_OFF, y++, FONT_WHITE, "Verify NAND Firm..." );
    OS_WaitVBlankIntr();

    if ( PXI_RecvID() != FIRM_PXI_ID_NULL )
    {
        goto err;
    }
    PrintString( X_OFF+20, y++, FONT_GREEN, "Done." );
    PrintString( X_OFF, y++, FONT_WHITE, "Load SRL Header..." );
    OS_WaitVBlankIntr();

    if ( PXI_RecvID() != FIRM_PXI_ID_NULL )
    {
        goto err;
    }
    PrintString( X_OFF+20, y++, FONT_GREEN, "Done." );

    PrintString( X_OFF, y++, FONT_WHITE, "Load SRL Static Data..." );
    OS_WaitVBlankIntr();

    if ( PXI_RecvID() != FIRM_PXI_ID_NULL )
    {
        goto err;
    }
    PrintString( X_OFF+20, y++, FONT_GREEN, "Done." );

    PrintString( X_OFF, 20, FONT_GREEN, "SUCCESS ALL!" );
    do
    {
        OS_WaitVBlankIntr();
    }
    while ( PAD_DetectFold() && (PAD_Read() & PAD_ALL_MASK) == (PAD_BUTTON_START|PAD_BUTTON_SELECT|PAD_BUTTON_X));

    PXI_NotifyID( FIRM_PXI_ID_NULL );
    OS_Terminate();

err:
    PrintString( X_OFF+20, y++, FONT_RED, "Failed." );
    OS_WaitVBlankIntr();
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    OS_Terminate();
}

//----------------------------------------------------------------
//  myInit
//
void myInit(void)
{
    //---- init
    OS_InitFIRM();
    OS_InitTick();
    OS_InitAlarm();
    FX_Init();
    GX_Init();
    GX_DispOff();
    GXS_DispOff();

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
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    //---- init interrupt
    OS_SetIrqFunction(OS_IE_V_BLANK, myVBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

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
    GX_LoadBG0Scr(gScreen, 0, sizeof(gScreen));
    GXS_LoadBG0Scr(gScreen, 0, sizeof(gScreen));

    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

