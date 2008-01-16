/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - sdmc-launcher
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
#define PRINT_MEMORY_ADDR       0x02000600


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
#define DMA_NO_FATFS        3

extern void*   SDNandContext;  /* NAND初期化パラメータ */

static ROM_Header* const rh= (ROM_Header*)HW_TWL_ROM_HEADER_BUF;

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
/*
  ちゃんとTWLと識別できているかチェック
#ifdef USE_DEBUG_LED
    SetDebugLED(OS_IsRunOnTwl() ? 0xC3 : 0xff);
    OS_SpinWaitCpuCycles(0x1000000);
#endif
*/
}

/***************************************************************
    EraseAll

    不正終了しました
    いろいろ消してください
    DSモードにして終わるのがよいか？
***************************************************************/
static void EraseAll(void)
{
    MI_CpuClearFast( OSi_GetFromFirmAddr(), sizeof(OSFromFirmBuf) );
#ifdef SDK_FINALROM
    MI_CpuClearFast( (void*)HW_TWL_ROM_HEADER_BUF, HW_TWL_ROM_HEADER_BUF_SIZE );
    OS_BootFromFIRM();
#endif
}

void TwlSpMain( void )
{
    int fd; // menu file descriptor

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

    // 2: after PM_InitFIRM
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x84

    PM_BackLightOn( FALSE );

    SDNandContext = &OSi_GetFromFirmAddr()->SDNandContext;
    if ( !FATFS_Init( DMA_NO_FATFS, THREAD_PRIO_FATFS ) )
    {
        OS_TPrintf("Failed to call FATFS_Init().\n");
        goto end;
    }

    // 3: after FS_Init
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x85

    PM_BackLightOn( FALSE );

PXI_RecvID();
SetDebugLED(0x01);
PXI_RecvID();
SetDebugLED(0x02);

    if ( PXI_RecvID() != FIRM_PXI_ID_SET_PATH )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_SET_PATH).\n");
        goto end;
    }

#ifdef USE_IDLE_THREAD
    CreateIdleThread();
#endif

    // 4: after PXI
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x86

    PM_BackLightOn( FALSE );

    if ( (fd = FS_OpenSrl()) < 0 )
    {
        OS_TPrintf("Failed to call FS_OpenSrl().\n");
        goto end;
    }

    // 5: after FS_OpenSrl
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x87

    PM_BackLightOn( FALSE );

    if ( !FS_LoadHeader( fd ) )
    {
        OS_TPrintf("Failed to call FS_LoadHeader().\n");
        goto end;
    }

    // 6: after FS_LoadHeader
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x88

    PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_HEADER )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_HEADER).\n");
        goto end;
    }

    // 7: after PXI
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x89

    PM_BackLightOn( FALSE );

    AESi_InitKeysFIRM();
    AESi_RecvSeed( rh->s.developer_encrypt );

    // 8: after AESi_RecvSeed
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x8a

    PM_BackLightOn( FALSE );

    if ( !FS_LoadStatic( fd ) )
    {
        OS_TPrintf("Failed to call FS_LoadStatic().\n");
        goto end;
    }

    // 9: after FS_LoadStatic
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x8b

    PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_STATIC )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_STATIC).\n");
        goto end;
    }

    // 10: after PXI
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

    PM_BackLightOn( TRUE ); // last chance

    PMi_SetParams( REG_PMIC_BL_BRT_B_ADDR, 22, PMIC_BL_BRT_B_MASK );
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

