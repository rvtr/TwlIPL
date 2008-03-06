/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - menu-launcher
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
#include <firm.h>
#include <twl/mcu.h>

/*
    PROFILE_ENABLE を定義するとある程度のパフォーマンスチェックができます。
    利用するためには、main.cかどこかに、u32 profile[256]; u32 pf_cnt = 0; を
    定義する必要があります。
*/
#define PROFILE_ENABLE

/*
    デバッグLEDをFINALROMとは別にOn/Offできます。
*/
#define USE_DEBUG_LED

/*
    PRINT_MEMORY_ADDR を定義すると、そのアドレスからSPrintfを行います(このファイルのみ)
    FINALROM版でもコードが残るので注意してください。
*/
#define PRINT_MEMORY_ADDR       0x02FFC800


#ifdef PROFILE_ENABLE
#define PROFILE_MAX  16
u32 profile[PROFILE_MAX];
u32 pf_cnt = 0;
#define PUSH_PROFILE()  (profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick()))
#else
#define PUSH_PROFILE()  ((void)0)
#endif

#ifdef USE_DEBUG_LED
static u8 step = 0x80;
#define InitDebugLED()          I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x03, 0x00)
#define SetDebugLED(pattern)    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, (pattern));
#else
#define InitDebugLED()          ((void)0)
#define SetDebugLED(pattern)    ((void)0)
#endif

#ifdef PRINT_MEMORY_ADDR
static char* debugPtr = (char*)PRINT_MEMORY_ADDR;
#undef OS_TPrintf
//#define OS_TPrintf(...) (debugPtr = (char*)((u32)(debugPtr + STD_TSPrintf(debugPtr, __VA_ARGS__) + 0xf) & ~0xf))
#define OS_TPrintf(...) (debugPtr += STD_TSPrintf(debugPtr, __VA_ARGS__))
#endif

#define THREAD_PRIO_FATFS   8
#define DMA_FATFS_1         0
#define DMA_FATFS_2         1

extern void*   SDNandContext;  /* NAND初期化パラメータ */

static ROM_Header* const rh= (ROM_Header*)HW_TWL_ROM_HEADER_BUF;

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
    /*
        FromBrom関連
    */
    if ( !OSi_FromBromToMenu() )
    {
        OS_Terminate();
    }
    /*
        リセットパラメータ(1バイト)を共有領域(1バイト)にコピー
    */
#define HOTSTART_FLAG_ENABLE    0x80
    if ( IS_OLD_MCU )   // MCU旧バージョン対策
    {
        *(u8 *)HW_NAND_FIRM_HOTSTART_FLAG = (u8)(MCUi_ReadRegister( (u16)(MCU_OLD_REG_TEMP_ADDR + OS_MCU_RESET_VALUE_OFS) ) | HOTSTART_FLAG_ENABLE);
    }
    else
    {
        *(u8 *)HW_NAND_FIRM_HOTSTART_FLAG = (u8)(MCUi_ReadRegister( (u16)(MCU_REG_TEMP_ADDR + OS_MCU_RESET_VALUE_OFS) ) | HOTSTART_FLAG_ENABLE);
    }
}

/***************************************************************
    PostInit

    各種初期化
***************************************************************/
static void PostInit(void)
{
    // PMICの設定 for old version
    PM_InitFIRM();
    // AESの初期化
    AES_Init(); // for encrypted NAND
    // アイドルスレッドの作成
    CreateIdleThread();
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
    AESi_ResetAesKeyA();
    AESi_ResetAesKeyB();
    AESi_ResetAesKeyC();
    MI_CpuClearFast( OSi_GetFromFirmAddr(), sizeof(OSFromFirmBuf) );
#ifdef SDK_FINALROM
    MI_CpuClearFast( (void*)HW_TWL_ROM_HEADER_BUF, HW_TWL_ROM_HEADER_BUF_SIZE );
    OS_BootFromFIRM();
#endif
}

void TwlSpMain( void )
{
    int fd; // menu file descriptor

#ifdef PROFILE_ENABLE
    // 0: bootrom
    profile[pf_cnt++] = OS_TicksToMicroSecondsBROM32(OS_GetTick());
#endif
    InitDebugLED();
    SetDebugLED(++step);  // 0x81

    PreInit();
#ifdef PROFILE_ENABLE
    // 1: after PreInit
    profile[pf_cnt++] = OS_TicksToMicroSecondsBROM32(OS_GetTick());
#endif
    SetDebugLED(++step);  // 0x82

    OS_InitFIRM();
    OS_EnableIrq();
    OS_EnableInterrupts();
    // 2: after OS_InitFIRM
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x83

    PostInit();
    // 3: after PostInit
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x84

//    PM_BackLightOn( FALSE );

    SDNandContext = &OSi_GetFromFirmAddr()->SDNandContext;
    if ( !FATFS_Init( DMA_FATFS_1, DMA_FATFS_2, THREAD_PRIO_FATFS ) )
    {
        OS_TPrintf("Failed to call FATFS_Init().\n");
        goto end;
    }
    // 4: after FATFS_Init
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x85

//    PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_SET_PATH )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_SET_PATH).\n");
        goto end;
    }
    // 5: after PXI
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x86

//    PM_BackLightOn( FALSE );

    if ( (fd = FS_OpenSrl()) < 0 )
    {
        OS_TPrintf("Failed to call FS_OpenSrl().\n");
        goto end;
    }
    // 6: after FS_OpenSrl
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x87

//    PM_BackLightOn( FALSE );

    if ( !FS_LoadHeader( fd ) )
    {
        OS_TPrintf("Failed to call FS_LoadHeader().\n");
        goto end;
    }
    // 7: after FS_LoadHeader
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x88

//    PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_HEADER )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_HEADER).\n");
        goto end;
    }
    // 8: after PXI
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x89

//    PM_BackLightOn( FALSE );

    AESi_InitKeysFIRM();
    AESi_InitSeed();
    // 9: after AESi_InitSeed
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x8a

//    PM_BackLightOn( FALSE );

    if ( !FS_LoadStatic( fd ) )
    {
        OS_TPrintf("Failed to call FS_LoadStatic().\n");
        goto end;
    }
    // 10: after FS_LoadStatic
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x8b

//    PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_STATIC )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_STATIC).\n");
        goto end;
    }
    // 11: after PXI
    PUSH_PROFILE();

#ifdef PROFILE_ENABLE
    {
        int i;
        PXI_RecvID();
        OS_TPrintf("\n[ARM7] Begin\n");
        for (i = 0; i < PROFILE_MAX; i++)
        {
//            OS_TPrintf("0x%08X\n", profile[i]);
            if ( !profile[i] ) break;
            OS_TPrintf("%2d: %7d usec", i, profile[i]);
            if (i)
            {
                OS_TPrintf(" ( %7d usec )\n", profile[i]-profile[i-1]);
            }
            else
            {
                OS_TPrintf("\n");
            }
        }
        OS_TPrintf("\n[ARM7] End\n");
    }
#endif
    SetDebugLED( 0 );

//    PM_BackLightOn( TRUE ); // last chance

    OS_BootFromFIRM();

end:
    SetDebugLED( (u8)(0xF0 | step));

    EraseAll();

    // failed
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    OS_Terminate();
}

