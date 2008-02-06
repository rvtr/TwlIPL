/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include    <twl_sp.h>
#include    <twl/spi.h>
#include    <twl/camera.h>
#include    <twl/rtc.h>
#include    <twl/fatfs.h>
#include    <twl/os/common/codecmode.h>
#include    <twl/cdc.h>
#include    <twl/exi/ARM7/genPort2.h>
#include    <twl/aes.h>
#include    "kami_pxi.h"
#include    "formatter.h"

/* Priorities of each threads */
#define THREAD_PRIO_SPI     2
#define THREAD_PRIO_SND     6
#define THREAD_PRIO_FATFS   8
#define THREAD_PRIO_RTC     12
#define THREAD_PRIO_FS      15
/* OS_THREAD_LAUNCHER_PRIORITY 16 */

extern void PMi_InitShutdownControl(void);

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystem(void);
static void VBlankIntr(void);
static void InitializeFatfs(void);
static void InitializeCdc(void);
static void DummyThread(void* arg);

/*---------------------------------------------------------------------------*
    �O���V���{���Q��
 *---------------------------------------------------------------------------*/
#ifdef  SDK_TWLHYB
extern void         SDK_LTDAUTOLOAD_LTDWRAM_BSS_END(void);
extern void         SDK_LTDAUTOLOAD_LTDMAIN_BSS_END(void);
#endif

/*---------------------------------------------------------------------------*
  Name:         TwlSpMain

  Description:  Initialize and do main

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlSpMain(void)
{
    OSThread    thread;
    u32         stack[18];

    // OS������
    OS_Init();
    SPI_Init(2);
    OS_InitTick();
    OS_InitAlarm();
    OS_InitThread();
//  (void)PAD_InitXYButton();

    // �q�[�v�̈�ݒ�
    InitializeAllocateSystem();

    // �{�^�����̓T�[�`������
    (void)PAD_InitXYButton();

    // ���荞�݋���
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // �_�~�[�X���b�h�쐬(CDC_Init��FS_Init��OS_Sleep���g�p����Ă��邽�߁j
    OS_CreateThread(&thread, DummyThread, NULL,
        (void*)((u32)stack + (sizeof(u32) * 18)), sizeof(u32) * 18, OS_THREAD_PRIORITY_MAX);
    OS_WakeupThreadDirect(&thread);

    // �t�@�C���V�X�e��������
    FS_Init(FS_DMA_NOT_USE);
    FS_CreateReadServerThread(THREAD_PRIO_FS);

    if (OS_IsRunOnTwl() == TRUE)
    {
        InitializeFatfs();    // FATFS ������
        //InitializeNwm();      // NWM ������
#ifndef SDK_NOCRYPTO
        AES_Init();           // AES ������
#endif
    }

    if (OSi_IsCodecTwlMode() == TRUE)
    {
        // CODEC ������
        InitializeCdc();
        // �J����������
        //CAMERA_Init();
        /* CODEC �� TWL ���[�h�łȂ��ƃV���b�^�[���������I�ɖ炷
           �@�\���g�p�ł��܂���B���ׁ̈ACODEC �� TWL ���[�h�̏ꍇ
           �ɂ̂݃J�������C�u�������g�p�\�ȏ�Ԃɂ��܂��B */
    }

    // �T�E���h������
    SND_Init(THREAD_PRIO_SND);

    // RTC ������
    RTC_Init(THREAD_PRIO_RTC);

    // ������������
    //WVR_Begin(heapHandle);

    // SPI ������
    SPI_Init(THREAD_PRIO_SPI);


    KamiPxiInit();

    // �_�~�[�X���b�h�j��
    OS_KillThread(&thread, NULL);

    while (TRUE)
    {
        OS_Halt();
        //---- check reset
        if (OS_IsResetOccurred())
        {
            OS_ResetSystem();
        }
    }


}
#include    <twl/ltdwram_begin.h>
/*---------------------------------------------------------------------------*
  Name:         InitializeFatfs
  Description:  FATFS���C�u����������������B
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitializeFatfs(void)
{
    OSThread    thread;
    u32         stack[18];

    // �_�~�[�X���b�h�쐬
    OS_CreateThread(&thread, DummyThread, NULL,
        (void*)((u32)stack + (sizeof(u32) * 18)), sizeof(u32) * 18, OS_THREAD_PRIORITY_MAX);
    OS_WakeupThreadDirect(&thread);

    // FATFS���C�u�����̏�����
#ifndef SDK_NOCRYPTO
#ifdef FATFS_AES_MOUNT_FOR_NAND
    if(!FATFS_Init( FATFS_DMA_4, FATFS_DMA_NOT_USE, THREAD_PRIO_FATFS))
#else
    if (FATFS_Init(FATFS_DMA_NOT_USE, FATFS_DMA_NOT_USE, THREAD_PRIO_FATFS))
#endif
#else
    if (FATFS_Init(FATFS_DMA_NOT_USE, FATFS_DMA_NOT_USE, THREAD_PRIO_FATFS))
#endif
    {
        // do nothing
    }

    // �_�~�[�X���b�h�j��
    OS_KillThread(&thread, NULL);
}
#include    <twl/ltdwram_end.h>

#include    <twl/ltdwram_begin.h>
/*---------------------------------------------------------------------------*
  Name:         InitializeCdc
  Description:  CDC���C�u����������������BCDC�������֐����ŃX���b�h�x�~����
                �ׁA�x�~�����삷��_�~�[�̃X���b�h�𗧂Ă�B
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitializeCdc(void)
{
#if 1
    // CODEC ������
    CDC_Init();
    CDC_InitMic();
//    CDCi_DumpRegisters();
#else
    /* [Debug] CODEC �� DS ���[�h�ŏ����� */
    *((u8*)(HW_TWL_ROM_HEADER_BUF + 0x01bf))    &=  ~(0x01);
    CDC_Init();
    CDC_GoDsMode();
    OS_TPrintf("Codec mode changed to DS mode for debug.\n");
#endif
}

/*---------------------------------------------------------------------------*
  Name:         DummyThread
  Description:  CDC���C�u����������������ۂɗ��Ă�_�~�[�̃X���b�h�B
  Arguments:    arg -   �g�p���Ȃ��B
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
DummyThread(void* arg)
{
#pragma unused(arg)
    while (TRUE)
    {
    }
}
#include    <twl/ltdwram_end.h>

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystem
  Description:  �����������ăV�X�e��������������B
  Arguments:    None.
  Returns:      OSHeapHandle - WRAM �A���[�i��Ɋm�ۂ��ꂽ�q�[�v�̃n���h����Ԃ��B
 *---------------------------------------------------------------------------*/
static OSHeapHandle
InitializeAllocateSystem(void){
    OSHeapHandle    hh;

#ifdef  SDK_TWLHYB
    if (OS_IsRunOnTwl() == TRUE)
    {
        void*   basicLo =   (void*)OS_GetSubPrivArenaLo();
        void*   basicHi =   (void*)OS_GetSubPrivArenaHi();
        void*   extraLo =   (void*)MATH_ROUNDUP((u32)SDK_LTDAUTOLOAD_LTDMAIN_BSS_END, 32);
        void*   extraHi =   (void*)MATH_ROUNDDOWN(HW_MAIN_MEM_SUB, 32);

#if SDK_DEBUG
        // debug information
        OS_TPrintf("ARM7: MAIN arena basicLo = %p\n", basicLo);
        OS_TPrintf("ARM7: MAIN arena basicHi = %p\n", basicHi);
        OS_TPrintf("ARM7: MAIN arena extraLo = %p\n", extraLo);
        OS_TPrintf("ARM7: MAIN arena extraHi = %p\n", extraHi);
#endif

        // �A���[�i�� 0 �N���A
        MI_CpuClear8(basicLo, (u32)basicHi - (u32)basicLo);
        MI_CpuClear8(extraLo, (u32)extraHi - (u32)extraLo);

        // ���������蓖�ď�����
        if ((u32)basicLo < (u32)extraLo)
        {
            basicLo =   OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, basicLo, extraHi, 1);
            // �A���[�i���ʃA�h���X��ݒ�
            OS_SetArenaLo(OS_ARENA_MAIN_SUBPRIV, basicLo);
        }
        else
        {
            extraLo =   OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, extraLo, basicHi, 1);
        }

        // �q�[�v�쐬
        hh  =   OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, basicLo, basicHi);

        if (hh < 0)
        {
            OS_Panic("ARM7: Failed to create MAIN heap.\n");
        }

        // �q�[�v�Ɋg���u���b�N��ǉ�
        OS_AddToHeap(OS_ARENA_MAIN_SUBPRIV, hh, extraLo, extraHi);
    }
    else
#endif
    {
        void*   lo  =   (void*)OS_GetSubPrivArenaLo();
        void*   hi  =   (void*)OS_GetSubPrivArenaHi();

        // �A���[�i�� 0 �N���A
        MI_CpuClear8(lo, (u32)hi - (u32)lo);

        // ���������蓖�ď�����
        lo  =   OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, lo, hi, 1);
        // �A���[�i���ʃA�h���X��ݒ�
        OS_SetArenaLo(OS_ARENA_MAIN_SUBPRIV, lo);

        // �q�[�v�쐬
        hh  =   OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, lo, hi);

        if (hh < 0)
        {
            OS_Panic("ARM7: Failed to MAIN create heap.\n");
        }
    }
    // �J�����g�q�[�v�ɐݒ�
    (void)OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
    // �q�[�v�T�C�Y�̊m�F
    {
        u32     heapSize;

        heapSize    =   (u32)OS_CheckHeap(OS_ARENA_MAIN_SUBPRIV, hh);
        OS_TPrintf("ARM7: MAIN heap size is %d\n", heapSize);
    }
#ifdef  SDK_TWLHYB
    if (OS_IsRunOnTwl() == TRUE)
    {
        void*   basicLo =   (void*)OS_GetWramSubPrivArenaLo();
        void*   basicHi =   (void*)OS_GetWramSubPrivArenaHi();
        void*   extraLo =   (void*)MATH_ROUNDUP((u32)SDK_LTDAUTOLOAD_LTDWRAM_BSS_END, 32);
        void*   extraHi =   (void*)MATH_ROUNDDOWN(HW_WRAM_A_HYB_END, 32);

#if SDK_DEBUG
        // debug information
        OS_TPrintf("ARM7: WRAM arena basicLo = %p\n", basicLo);
        OS_TPrintf("ARM7: WRAM arena basicHi = %p\n", basicHi);
        OS_TPrintf("ARM7: WRAM arena extraLo = %p\n", extraLo);
        OS_TPrintf("ARM7: WRAM arena extraHi = %p\n", extraHi);
#endif

        // �A���[�i�� 0 �N���A
        MI_CpuClear8(basicLo, (u32)basicHi - (u32)basicLo);
        MI_CpuClear8(extraLo, (u32)extraHi - (u32)extraLo);

        // ���������蓖�ď�����
        if ((u32)basicLo < (u32)extraLo)
        {
            basicLo =   OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV, basicLo, extraHi, 1);
            // �A���[�i���ʃA�h���X��ݒ�
            OS_SetArenaLo(OS_ARENA_WRAM_SUBPRIV, basicLo);
        }
        else
        {
            extraLo =   OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV, extraLo, basicHi, 1);
        }

        // �q�[�v�쐬
        hh  =   OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV, basicLo, basicHi);

        if (hh < 0)
        {
            OS_Panic("ARM7: Failed to WRAM create heap.\n");
        }

        // �q�[�v�Ɋg���u���b�N��ǉ�
        OS_AddToHeap(OS_ARENA_WRAM_SUBPRIV, hh, extraLo, extraHi);
    }
    else
#endif
    {
        void*   lo  =   (void*)OS_GetWramSubPrivArenaLo();
        void*   hi  =   (void*)OS_GetWramSubPrivArenaHi();

        // �A���[�i�� 0 �N���A
        MI_CpuClear8(lo, (u32)hi - (u32)lo);

        // ���������蓖�ď�����
        lo  =   OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV, lo, hi, 1);
        // �A���[�i���ʃA�h���X��ݒ�
        OS_SetArenaLo(OS_ARENA_WRAM_SUBPRIV, lo);

        // �q�[�v�쐬
        hh  =   OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV, lo, hi);

        if (hh < 0)
        {
            OS_Panic("ARM7: Failed to WRAM create heap.\n");
        }
    }

    // �J�����g�q�[�v�ɐݒ�
    (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);

    // �q�[�v�T�C�Y�̊m�F
    {
        u32     heapSize;

        heapSize    =   (u32)OS_CheckHeap(OS_ARENA_WRAM_SUBPRIV, hh);
        if (WM_WL_HEAP_SIZE > heapSize)
        {
            OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, WM_WL_HEAP_SIZE);
        }
        OS_TPrintf("ARM7: WRAM heap size is %d\n", heapSize);
    }

    return hh;
}

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr
  Description:  V �u�����N���荞�݃x�N�^�B
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
extern BOOL PMi_Initialized;
void    PM_SelfBlinkProc(void);

static void
VBlankIntr(void)
{
    if (PMi_Initialized)
    {
        PM_SelfBlinkProc();
    }
}
