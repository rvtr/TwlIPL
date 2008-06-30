/*---------------------------------------------------------------------------*
  Project:  TwlIPL - gcdfirm - sdmc-launcher-writer
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <firm.h>
#include <twl/mcu.h>

#include <symbols.h>
#include <twl/devices/sdmc/ARM7/sdmc.h>

#define PRINT_DEBUG

#ifndef PRINT_DEBUG
#undef  OS_TPrintf
#undef  OS_PutChar
#define OS_TPrintf(...) ((void)0)
#define OS_PutChar(...) ((void)0)
#endif // PRINT_DEBUG

/*
    デバッグLEDをFINALROMとは別にOn/Offできます。
*/
#define USE_DEBUG_LED

#ifdef USE_DEBUG_LED
static u8 step = 0x00;
#define InitDebugLED()          I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x03, 0x00)
#define SetDebugLED(pattern)    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, (pattern));
#else
#define InitDebugLED()          ((void)0)
#define SetDebugLED(pattern)    ((void)0)
#endif

#define THREAD_PRIO_FATFS   8

#define DMA_FATFS_1         0
#define DMA_FATFS_2         1
#define DMA_CARD            2

static u8* const nor = (u8*)HW_TWL_MAIN_MEM;
static u8* const nand = (u8*)HW_TWL_MAIN_MEM + offsetof(NANDHeader,l);

static OSThread idleThread;
static u64 idleStack[32];
static void IdleThread(void* arg)
{
#pragma unused(arg)
    OS_EnableInterrupts();
    while (1)
    {
        OS_Halt();
    }
}
static void CreateIdleThread(void)
{
    OS_CreateThread(&idleThread, IdleThread, NULL, &idleStack[32], sizeof(idleStack), OS_THREAD_PRIORITY_MAX);
    OS_WakeupThreadDirect(&idleThread);
}

/***************************************************************
    PreInit

    FromBootの対応＆メインメモリの初期化
    OS_Init前なので注意 (ARM9によるメインメモリ初期化で消されないように注意)
***************************************************************/
static void PreInit(void)
{
    // GCDヘッダコピー
    MI_CpuCopyFast( OSi_GetFromBromAddr(), (void*)HW_CARD_ROM_HEADER, HW_CARD_ROM_HEADER_SIZE );
    // NANDコンテキストコピー
    MI_CpuCopyFast( &OSi_GetFromBromAddr()->SDNandContext, (void*)HW_SD_NAND_CONTEXT_BUF, sizeof(SDPortContextData) );
    // FromBrom全消去
    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );
}

/***************************************************************
    PostInit

    各種初期化
***************************************************************/
static void PostInit(void)
{
    MCUi_WriteRegister( MCU_REG_BL_ADDR, MCU_REG_BL_BRIGHTNESS_MASK );
    PM_BackLightOn( TRUE );
    // アイドルスレッドの作成
    CreateIdleThread();
    // XYボタン通知
    PAD_InitXYButton();
    /*
        バッテリー残量チェック
    */
    MCUi_WriteRegister( MCU_REG_MODE_ADDR, MCU_SYSTEMMODE_TWL );   // TWL mode for ES library
    if ( (MCUi_ReadRegister( MCU_REG_POWER_INFO_ADDR ) & MCU_REG_POWER_INFO_LEVEL_MASK) == 0 )
    {
#ifndef SDK_FINALROM
        OS_TPanic("Battery is empty.\n");
#else
        PM_Shutdown();
#endif
    }
}

/***************************************************************
    EraseAll

    不正終了しました
    いろいろ消してください
    DSモードにして終わるのがよいか？
***************************************************************/
static void EraseAll(void)
{
    GCDHeader* const gh = (GCDHeader*)HW_ROM_HEADER_BUF;
    AESi_ResetAesKeyA();
    AESi_ResetAesKeyB();
    AESi_ResetAesKeyC();
    MI_CpuClearFast( nor, (gh->l.nandfirm_size + 512) * 2 );
}

/*
  独自CARDライブラリ
*/

#define CARD_COMMAND_PAGE           0x01000000
#define CARD_COMMAND_MASK           0x07000000
#define CARD_RESET_HI               0x20000000
#define CARD_COMMAND_OP_G_READPAGE  0xB7

static u32                  cache_page;
static u8                   CARDi_cache_buf[CARD_ROM_PAGE_SIZE] ATTRIBUTE_ALIGN(32);

/*---------------------------------------------------------------------------*
  Name:         CARDi_SetRomOp

  Description:  カードコマンド設定

  Arguments:    command    コマンド
                offset     転送ページ数

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_SetRomOp(u32 command, u32 offset)
{
    u32     cmd1 = (u32)((offset >> 8) | (command << 24));
    u32     cmd2 = (u32)((offset << 24));
    // 念のため前回のROMコマンドの完了待ち。
    while ((reg_MI_MCCNT1 & REG_MI_MCCNT1_START_MASK) != 0)
    {
    }
    // マスターイネーブル。
    reg_MI_MCCNT0 = (u16)(REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK |
                          (reg_MI_MCCNT0 & ~REG_MI_MCCNT0_SEL_MASK));
    // コマンド設定。
    reg_MI_MCCMD0 = MI_HToBE32(cmd1);
    reg_MI_MCCMD1 = MI_HToBE32(cmd2);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_GetRomFlag

  Description:  カードコマンドコントロールパラメータを取得

  Arguments:    flag       カードデバイスへ発行するコマンドのタイプ
                           (CARD_COMMAND_PAGE / CARD_COMMAND_ID /
                            CARD_COMMAND_STAT / CARD_COMMAND_REFRESH)

  Returns:      カードコマンドコントロールパラメータ
 *---------------------------------------------------------------------------*/
SDK_INLINE u32 CARDi_GetRomFlag(u32 flag)
{
    u32     rom_ctrl = *(vu32 *)(HW_CARD_ROM_HEADER + 0x60);
    return (u32)(flag | REG_MI_MCCNT1_START_MASK | CARD_RESET_HI | (rom_ctrl & ~CARD_COMMAND_MASK));
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_StartRomPageTransfer

  Description:  ROMページ転送を開始。

  Arguments:    offset     転送元のROMオフセット

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void CARDi_StartRomPageTransfer(u32 offset)
{
    u8 op = CARD_COMMAND_OP_G_READPAGE;
    CARDi_SetRomOp(op, offset);
    reg_MI_MCCNT1 = CARDi_GetRomFlag(CARD_COMMAND_PAGE);
}

/*---------------------------------------------------------------------------*
  Name:         CARDi_ReadRomWithCPU

  Description:  CPUを使用してROM転送。
                キャッシュやページ単位の制限を考慮する必要は無いが
                転送完了まで関数がブロッキングする点に注意。

  Arguments:    userdata          (他のコールバックとして使用するためのダミー)
                buffer            転送先バッファ
                offset            転送元ROMオフセット
                length            転送サイズ

  Returns:      None.
 *---------------------------------------------------------------------------*/
static int CARDi_ReadRomWithCPU(void *userdata, void *buffer, u32 offset, u32 length)
{
    int     retval = (int)length;
    // 頻繁に使用するグローバル変数をローカル変数へキャッシュ。
    u32         cachedPage = cache_page;
    u8  * const cacheBuffer = CARDi_cache_buf;
    while (length > 0)
    {
        // ROM転送は常にページ単位。
        u8     *ptr = (u8 *)buffer;
        u32     n = CARD_ROM_PAGE_SIZE;
        u32     pos = MATH_ROUNDDOWN(offset, CARD_ROM_PAGE_SIZE);
        // 以前のページと同じならばキャッシュを使用。
        if (pos == cachedPage)
        {
            ptr = cacheBuffer;
        }
        else
        {
            // バッファへ直接転送できないならキャッシュへ転送。
            if(((pos != offset) || (((u32)buffer & 3) != 0) || (length < n)))
            {
                cachedPage = pos;
                ptr = cacheBuffer;
            }
            // 4バイト整合の保証されたバッファへCPUで直接リード。
            CARDi_StartRomPageTransfer(pos);
            {
                u32     word = 0;
                for (;;)
                {
                    // 1ワード転送完了を待つ。
                    u32     ctrl = reg_MI_MCCNT1;
                    if ((ctrl & REG_MI_MCCNT1_RDY_MASK) != 0)
                    {
                        // データを読み出し、必要ならバッファへ格納。
                        u32     data = reg_MI_MCD1;
                        if (word < (CARD_ROM_PAGE_SIZE / sizeof(u32)))
                        {
                            ((u32 *)ptr)[word++] = data;
                        }
                    }
                    // 1ページ転送完了なら終了。
                    if ((ctrl & REG_MI_MCCNT1_START_MASK) == 0)
                    {
                        break;
                    }
                }
            }
        }
        // キャッシュ経由ならキャッシュから転送。
        if (ptr == cacheBuffer)
        {
            u32     mod = offset - pos;
            n = MATH_MIN(length, CARD_ROM_PAGE_SIZE - mod);
            MI_CpuCopy8(cacheBuffer + mod, buffer, n);
        }
        buffer = (u8 *)buffer + n;
        offset += n;
        length -= n;
    }
    // ローカル変数からグローバル変数へ反映。
    cache_page = cachedPage;
    (void)userdata;
    return retval;
}


extern SDMC_ERR_CODE FATFSi_sdmcGoIdle(u16 ports, void (*func1)(),void (*func2)());

void TwlSpMain( void )
{
    GCDHeader* const gh = &OSi_GetFromBromAddr()->header.gcd;
    u32 offset = gh->l.nandfirm_offset;
    u32 size = gh->l.nandfirm_size;
    u32 nsize = size - offsetof(NANDHeader,l);  // size to write to nand
    u32 sectors = (nsize + 511)/512;
    u8* nor2 = nor + size;      // buffer to verify
    u8* nand2 = nand + size;    // buffer to verify

    SdmcResultInfo  sdResult;

    InitDebugLED();
    SetDebugLED(++step);  // 0x01

    PreInit();
    SetDebugLED(++step);  // 0x02

    OS_InitFIRM();
    OS_EnableIrq();
    OS_EnableInterrupts();
    SetDebugLED(++step);  // 0x03

    PostInit();
    SetDebugLED(++step);  // 0x04

    // NAND初期化
    if (SDMC_NORMAL != FATFSi_sdmcInit( (SDMC_DMA_NO)DMA_FATFS_1, (SDMC_DMA_NO)DMA_FATFS_2 ))
    {
        OS_TPrintf("Failed to call FATFSi_sdmcInit().\n");
        goto err;
    }
    FATFSi_sdmcGoIdle( 2, NULL, NULL );
    SetDebugLED(++step);  // 0x05

    // CARD初期化
    SetDebugLED(++step);  // 0x06

    PXI_SendStream(&size, sizeof(size));

    if ( size < sizeof(NANDHeader) )
    {
        OS_TPrintf("No NAND firm is there.\n");
        goto err;
    }
    SetDebugLED(++step);  // 0x07

    // read all
    *(u32*)nor = 0;
    CARDi_ReadRomWithCPU( NULL, nor, offset, size );
    SetDebugLED(++step);  // 0x08

    PXI_NotifyID( FIRM_PXI_ID_NULL );

    // write NOR
    NVRAMi_Write( 0, nor, sizeof(NORHeaderDS));
    SetDebugLED(++step);  // 0x09

    {   // write boot_nandfirm flag
        s32 tmp = -1;
        NVRAMi_Write( 0x2ff, &tmp, 1 );
    }
    SetDebugLED(++step);  // 0x0a

    // write NAND
    if (SDMC_NORMAL != FATFSi_sdmcWriteFifo( nand, sectors, 1, SDMC_PORT_NAND, &sdResult ))
    {
        OS_TPrintf("Failed to call FATFSi_sdmcWriteFifo() to write header.\n");
        goto err;
    }

    SetDebugLED(++step);  // 0x0b

    PXI_NotifyID( FIRM_PXI_ID_NULL );

    // verify NOR
    NVRAMi_Read( 0, nor2, sizeof(NORHeaderDS) );
    if ( MI_CpuComp8( nor, nor2, sizeof(NORHeaderDS) ) )
    {
        OS_TPrintf("Failed to verify firm data in NOR.\n");
        goto err;
    }
    SetDebugLED(++step);  // 0x0c

    // verify NAND
    if ( FATFSi_sdmcReadFifo( nand2, sectors, 1, SDMC_PORT_NAND, &sdResult ) )
    {
        OS_TPrintf("Failed to call FATFSi_sdmcReadFifo() to write header.\n");
        goto err;
    }
    SetDebugLED(++step);  // 0x0d
    if ( MI_CpuComp8( nand, nand2, nsize ) )
    {
        OS_TPrintf("Failed to verify firm data in NAND.\n");
        goto err;
    }
    SetDebugLED(0);  // 0x00

    PXI_NotifyID( FIRM_PXI_ID_NULL );

    if ( PXI_RecvID() != FIRM_PXI_ID_NULL )
    {
        goto err;
    }

    OS_TPrintf("Success all.\n");
    MCUi_WriteRegister( MCU_REG_COMMAND_ADDR, MCU_REG_COMMAND_RESET_MASK );
    OS_Terminate();

err:
    SetDebugLED((u8)(step|0xF0));
    EraseAll();
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    OS_Terminate();
}


