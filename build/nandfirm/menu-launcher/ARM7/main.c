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

#define FATFS_HEAP_SIZE     (1024)   // FATFS�p�q�[�v (�T�C�Y�����K�v)

#define THREAD_PRIO_FS      15
#define THREAD_PRIO_FATFS   8
#define FS_DMA_NO           3

static u8 fatfsHeap[FATFS_HEAP_SIZE] __attribute__ ((aligned (32)));

/*
    PROFILE_ENABLE ���`����Ƃ�����x�̃p�t�H�[�}���X�`�F�b�N���ł��܂��B
    ���p���邽�߂ɂ́Amain.c���ǂ����ɁAu32 profile[256]; u32 pf_cnt = 0; ��
    ��`����K�v������܂��B
*/
#define PROFILE_ENABLE

/*
    �f�o�b�OLED��FINALROM�Ƃ͕ʂ�On/Off�ł��܂��B
*/
#define USE_DEBUG_LED

//#ifdef SDK_FINALROM // FINALROM�Ŗ�����
//#undef PROFILE_ENABLE
//#undef USE_DEBUG_LED
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

    FromBoot�̑Ή������C���������̏�����
    OS_Init�O�Ȃ̂Œ��� (ARM9�ɂ�郁�C���������������ŏ�����Ȃ��悤�ɒ���)
***************************************************************/
static void PreInit(void)
{
    /*
        FromBrom�֘A
    */
    if ( !OSi_FromBromToMenu() )
    {
        OS_Terminate();
    }
    /*
        ���Z�b�g�p�����[�^(1�o�C�g)�����L�̈�(4�o�C�g)�ɃR�s�[
    */
#define FIRM_AVAILABLE_BIT  0x80000000UL
    *(u32*)HW_RESET_PARAMETER_BUF = (u32)MCUi_ReadRegister( MCU_REG_TEMP_ADDR ) | FIRM_AVAILABLE_BIT;
    /*
        �o�b�e���[�c�ʃ`�F�b�N
    */
    //if ( MCUi_ReadRegister( MCU_REG_BATTELY ) < 0x02 )
    //if ( MCUi_ReadRegister( MCU_REG_IRQ ) & MCU_IRQ_NO_BATTELY )
}

/***************************************************************
    EraseAll

    �s���I�����܂���
    ���낢������Ă�������
    DS���[�h�ɂ��ďI���̂��悢���H
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

    FS����̏�����
***************************************************************/
extern void*   SDNandContext;  /* NAND�������p�����[�^ */
static BOOL FsInit(void)
{
    /* FATFS���C�u�����p�ɃJ�����g�q�[�v�ɐݒ� */
    /* WRAM���fatfsHeap�����C���������q�[�v�Ƃ��ēo�^���Ă��� */
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

static u32 systemId;
static void PxiSystemCallback( PXIFifoTag tag, u32 data, BOOL err )
{
    (void)tag;
    (void)err;
    systemId = data;
}

static void IdleThread(void* arg)
{
    OS_EnableInterrupts();
    while ( 1 )
    {
        OS_Halt();
    }
}
static OSThread idle;
static u32 idleStack[16] ATTRIBUTE_ALIGN(32);
static void CreateIdleThread( void )
{
    OS_CreateThread( &idle, IdleThread, NULL, &idleStack[16], sizeof(idleStack), OS_THREAD_PRIORITY_MAX );
    OS_WakeupThreadDirect( &idle );
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
PXI_RecvID();
SetDebugLED(0x03);
PXI_RecvID();
SetDebugLED(0x04);


    if ( PXI_RecvID() != FIRM_PXI_ID_SET_PATH )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_SET_PATH).\n");
        goto end;
    }

    CreateIdleThread();

    // 5: after PXI
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x87

    PM_BackLightOn( FALSE );

    if ( (fd = FS_OpenSrl()) < 0 )
    {
        OS_TPrintf("Failed to call FS_OpenSrl().\n");
        goto end;
    }

    // 6: after FS_OpenSrl
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x88

    PM_BackLightOn( FALSE );

    if ( !FS_LoadHeader( fd ) )
    {
        OS_TPrintf("Failed to call FS_LoadHeader().\n");
        goto end;
    }

    // 7: after FS_LoadHeader
    PUSH_PROFILE();
    SetDebugLED(++step); // 0x89

    PM_BackLightOn( FALSE );

    if ( PXI_RecvID() != FIRM_PXI_ID_DONE_HEADER )
    {
        OS_TPrintf("PXI_RecvID() was received invalid value (!=FIRM_PXI_ID_DONE_HEADER).\n");
        goto end;
    }

    // 8: after PXI
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

