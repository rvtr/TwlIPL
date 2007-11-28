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
#include <twl/os/ARM7/debugLED.h>

#define FATFS_HEAP_SIZE     (1024)   // FATFS用ヒープ (サイズ調整必要)

#define THREAD_PRIO_FS      15
#define THREAD_PRIO_FATFS   8
#define FS_DMA_NO           3

static u8 fatfsHeap[FATFS_HEAP_SIZE] __attribute__ ((aligned (32)));

/*
    PROFILE_ENABLE を定義するとある程度のパフォーマンスチェックができます。
    利用するためには、main.cかどこかに、u32 profile[256]; u32 pf_cnt = 0; を
    定義する必要があります。
*/
#define PROFILE_ENABLE

#ifdef SDK_FINALROM // FINALROMで無効化
#undef PROFILE_ENABLE
#endif

#ifdef PROFILE_ENABLE
#define PROFILE_MAX  16
u32 profile[PROFILE_MAX];
u32 pf_cnt = 0;
#endif

#ifndef SDK_FINALROM
static u8 step = 0x80;
#endif

/***************************************************************
    PreInit

    FromBootの対応＆メインメモリの初期化
    OS_Init前なので注意 (ARM9によるメインメモリ初期化で消されないように注意)
***************************************************************/
static void PreInit(void)
{
    /*
        FromBrom関連
    */
    if ( !OSi_FromBromToMenu() )
    {
        OS_Terminate();
    }
    /*
        リセットパラメータ(1バイト)を共有領域(4バイト)にコピー
    */
#define FIRM_AVAILABLE_BIT  0x80000000UL
    *(u32*)HW_RESET_PARAMETER_BUF = (u32)MCUi_ReadRegister( MCU_REG_TEMP_ADDR ) | FIRM_AVAILABLE_BIT;
    /*
        バッテリー残量チェック
    */
    //if ( MCUi_ReadRegister( MCU_REG_BATTELY ) < 0x02 )
    //if ( MCUi_ReadRegister( MCU_REG_IRQ ) & MCU_IRQ_NO_BATTELY )
}

/***************************************************************
    EraseAll

    不正終了しました
    いろいろ消してください
    DSモードにして終わるのがよいか？
***************************************************************/
static void EraseAll(void)
{
#ifdef SDK_FINALROM
    MI_CpuClearFast( (void*)HW_TWL_ROM_HEADER_BUF, HW_TWL_ROM_HEADER_BUF_SIZE );
    OS_BootFromFIRM();
#endif
}

/***************************************************************
    FsInit

    FS周りの初期化
***************************************************************/
extern void*   SDNandContext;  /* NAND初期化パラメータ */
static BOOL FsInit(void)
{
    /* FATFSライブラリ用にカレントヒープに設定 */
    /* WRAM上のfatfsHeapをメインメモリヒープとして登録している */
    {
        OSHeapHandle hh;
        u8     *lo = (u8*)fatfsHeap;
        u8     *hi = (u8*)fatfsHeap + FATFS_HEAP_SIZE;
//MI_CpuFillFast(fatfsHeap, 0xcccccccc, FATFS_HEAP_SIZE);
        lo = OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, lo, hi, 1);
        OS_SetArenaLo(OS_ARENA_MAIN_SUBPRIV, lo);
        hh = OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, OS_GetSubPrivArenaLo(), hi);
        OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
    }

#ifdef PROFILE_ENABLE
    // 3: after OS_CreateHeap
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step); // 0x85

    SDNandContext = &OSi_GetFromFirmAddr()->SDNandContext;

    //FS_Init( FS_DMA_NO ); // just CARD_Init
    //FS_CreateReadServerThread( THREAD_PRIO_FS );  // just CARD_SetThreadPriority

    if ( !FATFS_Init( FATFS_DMA_NOT_USE, THREAD_PRIO_FATFS ) )
    {
        return FALSE;
    }

    return TRUE;
}

void TwlSpMain( void )
{
    int fd; // menu file descriptor

    // OS_InitDebugLED and OS_SetDebugLED are able to call after OS_Init
#ifndef SDK_FINALROM
    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x03, 0x00);
    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, ++step);  // 0x81
#endif

    PreInit();

#ifndef SDK_FINALROM
    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, ++step);  // 0x82
#endif
#ifdef PROFILE_ENABLE
    // 0: before PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    OS_InitFIRM();
    OS_EnableIrq();
    OS_EnableInterrupts();

#ifdef PROFILE_ENABLE
    // 1: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step); // 0x83

    PM_InitFIRM();

#ifdef PROFILE_ENABLE
    // 2: after PM_InitFIRM
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step); // 0x84
    PM_BackLightOn( FALSE );

    if ( !FsInit() )
    {
        OS_TPrintf("Failed to call FsInit().\n");
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 4: after FS_Init
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step); // 0x86
    PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_SET_PATH )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_SET_PATH).\n");
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 5: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step); // 0x87
    PM_BackLightOn( FALSE );

    if ( (fd = FS_OpenSrl()) < 0 )
    {
        OS_TPrintf("Failed to call FS_OpenSrl().\n");
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 6: after FS_OpenSrl
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step); // 0x88
    PM_BackLightOn( FALSE );

    if ( !FS_LoadHeader( fd ) )
    {
        OS_TPrintf("Failed to call FS_LoadHeader().\n");
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 7: after FS_LoadHeader
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step); // 0x89
    PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_HEADER )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_HEADER).\n");
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 8: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step); // 0x8a
    PM_BackLightOn( FALSE );

    if ( !FS_LoadStatic( fd ) )
    {
        OS_TPrintf("Failed to call FS_LoadStatic().\n");
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 9: after FS_LoadStatic
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step); // 0x8b
    PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_STATIC )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_STATIC).\n");
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 15: after PXI
    pf_cnt = PROFILE_MAX-1;
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
    {
        int i;
        PXI_RecvID();
        OS_TPrintf("\n[ARM7] Begin\n");
        for (i = 0; i < PROFILE_MAX; i++)
        {
            OS_TPrintf("0x%08X\n", profile[i]);
        }
        OS_TPrintf("\n[ARM7] End\n");
    }
#endif
    OS_SetDebugLED( 0 );
    PM_BackLightOn( TRUE ); // last chance

    OS_BootFromFIRM();

end:
    OS_SetDebugLED( (u8)(0xF0 | step));

    EraseAll();

    // failed
    while (1)
    {
        PXI_NotifyID( FIRM_PXI_ID_ERR );
    }
}

