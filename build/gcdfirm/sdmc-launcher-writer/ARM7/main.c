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

// MCU旧バージョン対策
#if SDK_TS_VERSION <= 200
static u8 version = 0;
#define IS_OLD_MCU  (version ? (version < 0x20) : ((version=MCUi_ReadRegister( MCU_REG_VER_INFO_ADDR )) < 0x20))
#else
#define IS_OLD_MCU  FALSE
#define MCU_OLD_REG_TEMP_ADDR   MCU_REG_TEMP_ADDR   // avoid compiler error
#endif

/***************************************************************
    PreInit

    FromBootの対応＆メインメモリの初期化
    OS_Init前なので注意 (ARM9によるメインメモリ初期化で消されないように注意)
***************************************************************/
static void PreInit(void)
{
    GCDHeader* const gh = &OSi_GetFromBromAddr()->header.gcd;
    /*
        バッテリー残量チェック
    */
    if ( !IS_OLD_MCU )   // MCU旧バージョン対策
    {
        if ( (MCUi_ReadRegister( MCU_REG_POWER_INFO_ADDR ) & MCU_REG_POWER_INFO_LEVEL_MASK) == 0 )
        {
#ifndef SDK_FINALROM
            OS_TPanic("Battery is empty.\n");
#else
            PM_Shutdown();
#endif
        }
    }
    MI_CpuCopyFast( gh, (void*)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END - HW_ROM_HEADER_BUF );
//    MI_CpuCopyFast( gh, (void*)HW_CARD_ROM_HEADER, HW_CARD_ROM_HEADER_SIZE );
    // FromBrom全消去
    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );
}

/***************************************************************
    PostInit

    各種初期化
***************************************************************/
static void PostInit(void)
{
#if SDK_TS_VERSION <= 200
    // PMICの設定 for old version
    PM_InitFIRM();
#endif
#if SDK_TS_VERSION <= 200
    PMi_SetParams( REG_PMIC_BL_BRT_A_ADDR, PMIC_BACKLIGHT_BRIGHT_DEFAULT, PMIC_BL_BRT_A_MASK );
    PMi_SetParams( REG_PMIC_BL_BRT_B_ADDR, PMIC_BACKLIGHT_BRIGHT_DEFAULT, PMIC_BL_BRT_B_MASK );
#else
    MCUi_WriteRegister( MCU_REG_BL_ADDR, MCU_REG_BL_BRIGHTNESS_MASK );
#endif
    PM_BackLightOn( TRUE );
    // アイドルスレッドの作成
    CreateIdleThread();
    // XYボタン通知
    PAD_InitXYButton();
    /*
        バッテリー残量チェック
    */
    if ( !IS_OLD_MCU )   // MCU旧バージョン対策
    {
        if ( (MCUi_ReadRegister( MCU_REG_POWER_INFO_ADDR ) & MCU_REG_POWER_INFO_LEVEL_MASK) == 0 )
        {
#ifndef SDK_FINALROM
            OS_TPanic("Battery is empty.\n");
#else
            PM_Shutdown();
#endif
        }
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
    GCDHeader* const gh = &OSi_GetFromBromAddr()->header.gcd;
    AESi_ResetAesKeyA();
    AESi_ResetAesKeyB();
    AESi_ResetAesKeyC();
    MI_CpuClearFast( nor, (gh->l.nandfirm_size + 512) * 2 );
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

    s32             lock_id;
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
    FATFSi_sdmcSelect( SDMC_PORT_NAND );
    SetDebugLED(++step);  // 0x05

    // CARD初期化
    CARD_Init();
    CARD_Enable(TRUE);
    lock_id = OS_GetLockID();
    CARD_LockRom((u16)lock_id);
    SetDebugLED(++step);  // 0x06

    PXI_SendStream(&size, sizeof(size));

    if ( size < sizeof(NANDHeader) )
    {
        OS_TPrintf("No NAND firm is there.\n");
        goto err;
    }
    SetDebugLED(++step);  // 0x07

    // read all
    CARD_ReadRom( DMA_CARD, (void*)offset, nor, size );
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
    if ( FATFSi_sdmcWriteFifo( nand, sectors, 1, NULL, &sdResult ) )
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
    if ( FATFSi_sdmcReadFifo( nand2, sectors, 1, NULL, &sdResult ) )
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


