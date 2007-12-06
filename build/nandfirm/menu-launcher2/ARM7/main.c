/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - menu-launcher2
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
#include <twl/aes.h>

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

/*
    デバッグLEDをFINALROMとは別にOn/Offできます。
*/
#define USE_DEBUG_LED

//#ifdef SDK_FINALROM // FINALROMで無効化
//#undef PROFILE_ENABLE
//#endif

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
        lo = OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, lo, hi, 1);
        OS_SetArenaLo(OS_ARENA_MAIN_SUBPRIV, lo);
        hh = OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, OS_GetSubPrivArenaLo(), hi);
        OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
    }

    // 3: after OS_CreateHeap
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x85

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
    InitDebugLED();
    SetDebugLED(++step);  // 0x81

    PreInit();

    // 0: before PXI
    PUSH_PROFILE();
    SetDebugLED(++step);  // 0x82

    OS_InitFIRM();
    OS_EnableIrq();
    OS_EnableInterrupts();

    // 1: after PXI
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x83

    PM_InitFIRM();
    AES_Init();

    // 2: after PM_InitFIRM
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x84

    PM_BackLightOn( FALSE );

    if ( !FsInit() )
    {
        OS_TPrintf("Failed to call FsInit().\n");
        goto end;
    }

    // 4: after FS_Init
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x86

    PM_BackLightOn( FALSE );

PXI_RecvID();
SetDebugLED(0x01);
PXI_RecvID();
SetDebugLED(0x02);

    // 5:
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x87

    //PM_BackLightOn( FALSE );

    // 6:
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x88

    //PM_BackLightOn( FALSE );

    // 7:
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x89

    //PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_HEADER )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_HEADER).\n");
        goto end;
    }

    // 8: after PXI
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x8a

    PM_BackLightOn( FALSE );

    AESi_InitKeysFIRM();
    AESi_RecvSeed();

    // 9: after AESi_RecvSeed
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x8b

    PM_BackLightOn( FALSE );

    // 10:
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x8c

    //PM_BackLightOn( FALSE );

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
        MI_CpuCopy8( profile, (void*)0x02000080, sizeof(profile) );
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
    PM_BackLightOn( TRUE ); // last chance

    OS_BootFromFIRM();

end:
    SetDebugLED( (u8)(0xF0 | step));

    EraseAll();

    // failed
//    while (1)
    {
        PXI_NotifyID( FIRM_PXI_ID_ERR );
    }
    OS_Terminate();
}

