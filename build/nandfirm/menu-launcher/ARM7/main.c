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

#define FATFS_HEAP_SIZE     (64*1024)   // FATFS�p�q�[�v (�T�C�Y�����K�v)

#define BOOT_DEVICE     FATFS_MEDIA_TYPE_NAND
#define PARTITION_NO    0                       // �Ώۃp�[�e�B�V����

#define DRIVE_LETTER    'A'                     // �}�E���g��h���C�u��
#define DRIVE_NO        (DRIVE_LETTER - 'A')    // �}�E���g��h���C�u�ԍ�

static u8 fatfsHeap[FATFS_HEAP_SIZE] __attribute__ ((aligned (32)));

#ifndef SDK_FINALROM
static u8 step = 0x80;
#endif

/*
    PROFILE_ENABLE ���`����Ƃ�����x�̃p�t�H�[�}���X�`�F�b�N���ł��܂��B
    ���p���邽�߂ɂ́Amain.c���ǂ����ɁAu32 profile[256]; u32 pf_cnt = 0; ��
    ��`����K�v������܂��B
*/
#define PROFILE_ENABLE

#ifdef SDK_FINALROM // FINALROM�Ŗ�����
#undef PROFILE_ENABLE
#endif

#ifdef PROFILE_ENABLE
#define PROFILE_MAX  256
u32 profile[PROFILE_MAX];
u32 pf_cnt = 0;
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
    *(u32*)HW_RESET_PARAMETER_BUF = (u32)MCUi_ReadRegister( MCU_REG_TEMP_ADDR );
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
    // TODO
#endif
}

/***************************************************************
    Fatfs4nandInit

    FATFS����̏����� for NAND
***************************************************************/
#if 1   /* 0: FATFS���K�i���p�� */
#else
extern void*   SDNandContext;  /* NAND�������p�����[�^ */
extern BOOL FATFSi_rtfs_init( void );
extern int  FATFSi_sdmcInit( int dmaNo );
extern int  FATFSi_nandRtfsAttach( int driveno, int partition );
static void IdleThreadFunc(void* arg)
{
    OSThread* pThread = arg;
    OS_EnableInterrupts();
    while (1)
    {
        OS_CheckStack(pThread);
        OS_Halt();
    }
}
static void CreateIdleThread(void)
{
    static u32 stack[32];
    static OSThread idle;
    OS_EnableIrq();
    OS_EnableInterrupts();
    OS_CreateThread(&idle, IdleThreadFunc, &idle, stack + 32, sizeof(stack), 31);
    OS_WakeupThreadDirect(&idle);
}
#endif
static BOOL Fatfs4nandInit(void)
{
    /* FATFS���C�u�����p�ɃJ�����g�q�[�v�ɐݒ� */
    /* WRAM���fatfsHeap�����C���������q�[�v�Ƃ��ēo�^���Ă��� */
    {
        OSHeapHandle hh;
        u8     *lo = (u8*)fatfsHeap;
        u8     *hi = (u8*)fatfsHeap + FATFS_HEAP_SIZE;
        lo = OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, lo, hi, 1);
        OS_SetArenaLo(OS_ARENA_MAIN_SUBPRIV, lo);
        hh = OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, OS_GetSubPrivArenaLo(), hi);
        OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
    }

    OS_SetDebugLED(++step);

#if 1   /* 0: FATFS���K�i���p�� */
    if ( !FATFS_InitFIRM( &(OSi_GetFromFirmAddr()->SDNandContext) ) )
#else
    SDNandContext = &OSi_GetFromFirmAddr()->SDNandContext;
    CreateIdleThread();
    /* RTFS���C�u������������ */
    /* SD�h���C�o������ */
    if( !FATFSi_rtfs_init() || FATFSi_sdmcInit(1) != 0)
#endif
    {
        return FALSE;
    }

#ifdef PROFILE_ENABLE
    // 3: after FATFS
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step);
#if 1   /* 0: FATFS���K�i���p�� */
    if ( !FATFS_MountDriveFIRM( DRIVE_NO, BOOT_DEVICE, PARTITION_NO ) )
#else
    if ( !FATFSi_nandRtfsAttach( DRIVE_NO, PARTITION_NO ) )
#endif
    {
        return FALSE;
    }
#ifdef PROFILE_ENABLE
    // 4: after Mount
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step);
    PM_BackLightOn( FALSE );

    if ( !FATFS_OpenRecentMenu( DRIVE_NO ) )
    {
        return FALSE;
    }
    return TRUE;
}

void TwlSpMain( void )
{
    // OS_InitDebugLED and OS_SetDebugLED are able to call after OS_Init
#ifndef SDK_FINALROM
    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x03, 0x00);
    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, ++step);
#endif

    PreInit();

#ifndef SDK_FINALROM
    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, ++step);
#endif
#ifdef PROFILE_ENABLE
    // 0: before PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    OS_InitFIRM();

#ifdef PROFILE_ENABLE
    // 1: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    OS_SetDebugLED(++step);

    PM_InitFIRM();
    PM_BackLightOn( FALSE );

#ifdef PROFILE_ENABLE
    // 2: after PM
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    if ( !Fatfs4nandInit() )
    {
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 5: after Open
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    OS_SetDebugLED(++step);

    PM_BackLightOn( FALSE );

    if ( !FATFS_LoadHeader() || !FATFS_LoadStatic() )
    {
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 127: before Boot
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
    OS_SetDebugLED(++step);

    PM_BackLightOn( TRUE ); // last chance

    OS_BootFromFIRM();

end:
    OS_SetDebugLED( (u8)(0xF0 | step));

    EraseAll();

    // failed
    while (1)
    {
        PXI_NotifyID( FIRM_PXI_ID_NULL );
    }
}

