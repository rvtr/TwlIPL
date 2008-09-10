/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     main.c
  
  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include    <nitro/types.h>
#include    <twl/init/crt0.h>
#include    <twl/memorymap_sp.h>
#include    <twl/os.h>
#include    <twl/spi.h>
#include    <twl/fatfs.h>
#include    <nitro/pad.h>
#include    <nitro/std.h>
#include    <nitro/snd.h>
#include    <nitro/wvr.h>
#include    <twl/nwm.h>
#include    <twl/camera.h>
#include    <twl/rtc.h>
#include    <nitro/hw/common/lcd.h>
#include    <nitro/gx.h>
#include    <twl/os/common/codecmode.h>
#include    <twl/cdc.h>
#include    <twl/snd/ARM7/sndex_api.h>
#include    <twl/aes.h>
#include    <twl/mcu.h>
#include    "nvram_sp.h"
#include	"address.h"

#ifdef SDK_SEA
#include	<twl/sea.h>
#endif  // ifdef SDK_SEA

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define WM_WL_HEAP_SIZE     0x2100
#define ATH_DRV_HEAP_SIZE   0x5800
#define WPA_HEAP_SIZE       0x1C00

#define MEM_TYPE_WRAM 0
#define MEM_TYPE_MAIN 1

/* Priorities of each threads */
#define THREAD_PRIO_SPI     2
#define THREAD_PRIO_MCU     4 // �b��
#define THREAD_PRIO_SND     6
#define THREAD_PRIO_FATFS   8
#define THREAD_PRIO_AES     12
#define THREAD_PRIO_SEA     12
#define THREAD_PRIO_RTC     12
#define THREAD_PRIO_SNDEX   14
#define THREAD_PRIO_FS      15
/* OS_THREAD_LAUNCHER_PRIORITY 16 */

/* [TODO] �ȉ��� New WM ���Ɉڍs����ق����D�܂���? */
#define NWM_DMANO                   NWMSP_DMA_7
#define THREAD_PRIO_NWM_COMMMAND    9
#define THREAD_PRIO_NWM_EVENT       7
#define THREAD_PRIO_NWM_SDIO        8
#define THREAD_PRIO_NWM_WPA         10

// ROM ���o�^�G���A�̊g������R�[�h
#define ROMHEADER_FOR_CHINA_BIT        0x80
#define ROMHEADER_FOR_KOREA_BIT        0x40

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void         PrintDebugInfo(void);
static OSHeapHandle InitializeAllocateSystem(u8 memType);
static OSHeapHandle InitializeAllocateSystemCore(u8 memType);
#ifdef SDK_TWLHYB
static OSHeapHandle InitializeAllocateSystemCoreEx(u8 memType);
#endif
static void         DummyThread(void* arg);
static void         ReadUserInfo(void);
#ifdef  NVRAM_CONFIG_DATA_EX_VERSION
static BOOL         IsValidConfigEx(void);
static u16          GetRomValidLanguage(void);
static s32          CheckCorrectNCDEx(NVRAMConfigEx * ncdsp);
#else
static s32          CheckCorrectNCD(NVRAMConfig *ncdsp);
#endif
static void         VBlankIntr(void);
static void         InitializeFatfs(void);
static void         InitializeNwm(OSHeapHandle drvHeapHandle, OSHeapHandle wpaHeapHandle);
/*---------------------------------------------------------------------------*
    �O���V���{���Q��
 *---------------------------------------------------------------------------*/
#ifdef  SDK_TWLHYB
extern void         SDK_LTDAUTOLOAD_LTDWRAM_BSS_END(void);
extern void         SDK_LTDAUTOLOAD_LTDMAIN_BSS_END(void);
#endif


/*---------------------------------------------------------------------------*
  Name:         TwlSpMain
  Description:  �N���x�N�^�B
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlSpMain(void)
{
    OSHeapHandle    wramHeapHandle, mainHeapHandle;

	
	// OS��������ARM9�Ɠ�������O��SCFG�̏������L�������ɏ����o��
	// ���W�X�^�����R�s�[
	MI_CpuMove16( (void*)REG_ROM_ADDR, DISPINFO_SHARED_SCFG_REG_ADDR, DISPINFO_SHARED_SCFG_REG_SIZE );
	// WRAM�ɑޔ�����Ă镪���R�s�[
	MI_CpuMove16( (void*)HWi_WSYS04_ADDR, DISPINFO_SHARED_SCFG_WRAM_ADDR, DISPINFO_SHARED_SCFG_WRAM_SIZE );

    // OS ������
    OS_Init();
    PrintDebugInfo();

    // NVRAM ���烆�[�U�[���ǂݏo��
    ReadUserInfo();

    // �q�[�v�̈�ݒ�
    wramHeapHandle  =   InitializeAllocateSystem(MEM_TYPE_WRAM);
    mainHeapHandle  =   InitializeAllocateSystem(MEM_TYPE_MAIN);

    // �{�^�����̓T�[�`������
    (void)PAD_InitXYButton();

    // ���荞�݋���
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // �t�@�C���V�X�e��������
    FS_Init(FS_DMA_NOT_USE);
    FS_CreateReadServerThread(THREAD_PRIO_FS);

    if (OS_IsRunOnTwl() == TRUE)
    {
        InitializeFatfs();    // FATFS ������
#ifndef SDK_SEA  // �I�b�菈�u�I
    // NWM ������
#ifdef SDK_TWLLTD
        InitializeNwm(mainHeapHandle, mainHeapHandle);      // LIMITED ���[�h�ł� �����̃q�[�v�� MAIN ����m��
#else
        InitializeNwm(wramHeapHandle, mainHeapHandle);      // HYBRID ���[�h�ł� �����̃q�[�v�� WRAM ����m��
#endif
#endif  // ifndef SDK_SEA
        AES_Init(THREAD_PRIO_AES);      // AES ������

#ifdef SDK_SEA
		SEA_Init(THREAD_PRIO_SEA);
#endif  // ifdef SDK_SEA
        MCU_InitIrq(THREAD_PRIO_MCU);  // MCU ������

        CDC_InitLib();	// CODEC���C�u����������
    }

    if (OSi_IsCodecTwlMode() == TRUE)
    {
        // �J����������
        CAMERA_Init();
        /* CODEC �� TWL ���[�h�łȂ��ƃV���b�^�[���������I�ɖ炷
           �@�\���g�p�ł��܂���B���ׁ̈ACODEC �� TWL ���[�h�̏ꍇ
           �ɂ̂݃J�������C�u�������g�p�\�ȏ�Ԃɂ��܂��B */
    }

    // �T�E���h������
    SND_Init(THREAD_PRIO_SND);
    if (OS_IsRunOnTwl() == TRUE)
    {
        SNDEX_Init(THREAD_PRIO_SNDEX);
    }

    // RTC ������
    RTC_Init(THREAD_PRIO_RTC);

    // ������������
#ifndef SDK_SEA  // �I�b�菈�u�I
    WVR_Begin(wramHeapHandle);
#endif  // ifdef SDK_SEA

    // SPI ������
    SPI_Init(THREAD_PRIO_SPI);


    while (TRUE)
    {
        OS_Halt();

        //---- check reset
        if (OS_IsResetOccurred())
        {
            //VIB_STOP
            CTRDG_VibPulseEdgeUpdate(NULL);

            OS_ResetSystem();
        }

        //---- check pull out cartridge
        CTRDG_CheckPullOut_Polling();

#ifndef SDK_SMALL_BUILD
        //---- check pull out card
        CARD_CheckPullOut_Polling();
#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:         PrintDebugInfo
  Description:  ARM7 �R���|�[�l���g�̏����f�o�b�O�o�͂���B
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
PrintDebugInfo(void)
{
    if(OS_IsRunOnTwl())
    {
        OS_TPrintf("ARM7: This component is running on TWL.\n");
    }
    else
    {
        OS_TPrintf("ARM7: This component is running on NITRO.\n");
    }
#ifdef  SDK_SEA
#ifdef  SDK_TWLLTD
    OS_TPrintf("ARM7: This component is \"armadillo.TWL\"\n");
#else   /* ifdef SDK_TWLLTD */
#error invalid parameter combination
#endif  /* ifdef SDK_TWLLTD else */
#else   /* ifdef SDK_SEA */
#ifdef  SDK_TWLLTD
    OS_TPrintf("ARM7: This component is \"racoon.TWL\"\n");
#else   /* ifdef SDK_TWLLTD */
#ifdef  SDK_WIRELESS_IN_VRAM
    OS_TPrintf("ARM7: This component is \"ichneumon.TWL\"\n");
#else   /* ifdef SDK_WIRELESS_IN_VRAM */
    OS_TPrintf("ARM7: This component is \"mongoose.TWL\"\n");
#endif  /* ifdef SDK_WIRELESS_IN_VRAM else */
#endif  /* ifdef SDK_TWLLTD else */
#endif  /* ifdef SDK_SEA else */
}

#include    <twl/ltdwram_begin.h>
/*---------------------------------------------------------------------------*
  Name:         InitializeFatfs
  Description:  FATFS���C�u����������������BFATFS�������֐����ŃX���b�h�x�~
                ����ׁA�x�~�����삷��_�~�[�̃X���b�h�𗧂Ă�B
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
    if(!FATFS_Init( FATFS_DMA_4, FATFS_DMA_5, THREAD_PRIO_FATFS))
    {
        // do nothing
    }

    // �_�~�[�X���b�h�j��
    OS_KillThread(&thread, NULL);
}
#include    <twl/ltdwram_end.h>

#include    <twl/ltdwram_begin.h>
/*---------------------------------------------------------------------------*
  Name:         InitializeNwm
  Description:  NWM���C�u����������������B
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitializeNwm(OSHeapHandle drvHeapHandle, OSHeapHandle wpaHeapHandle)
{
    NwmspInit nwmInit;

    nwmInit.dmaNo = NWM_DMANO;
    nwmInit.cmdPrio = THREAD_PRIO_NWM_COMMMAND;
    nwmInit.evtPrio = THREAD_PRIO_NWM_EVENT;
    nwmInit.sdioPrio = THREAD_PRIO_NWM_SDIO;

#ifdef SDK_TWLLTD
    nwmInit.drvHeap.id = OS_ARENA_MAIN_SUBPRIV; /* [TODO] */
#else
    nwmInit.drvHeap.id = OS_ARENA_WRAM_SUBPRIV; /* [TODO] */
#endif
    nwmInit.drvHeap.handle = drvHeapHandle;

    nwmInit.wpaPrio = THREAD_PRIO_NWM_WPA;
    nwmInit.wpaHeap.id = OS_ARENA_MAIN_SUBPRIV; /* [TODO] */
    nwmInit.wpaHeap.handle = wpaHeapHandle;

    NWMSP_Init(&nwmInit);

}
#include    <twl/ltdwram_end.h>

#include    <twl/ltdwram_begin.h>
/*---------------------------------------------------------------------------*
  Name:         DummyThread
  Description:  FATFS���C�u�����ACDC���C�u����������������ۂɗ��Ă�_�~�[��
                �X���b�h�B
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
static OSHeapHandle  InitializeAllocateSystem(u8 memType)
{

    OSHeapHandle    hh;

#ifdef SDK_TWLHYB
    if( OS_IsRunOnTwl() == TRUE)
    {
        hh = InitializeAllocateSystemCoreEx(memType); /* Hybrid �� TWL �œ��삳���� */
    }
    else
#endif
    {
        hh = InitializeAllocateSystemCore(memType);     /* Hybrid �� DS �œ��삳���� or Limited */
    }

    return hh;
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystemCore
  Description:  �����������ăV�X�e��������������B
                Hybrid �� DS �œ��삳�����ꍇ�ALimited �� TWL �œ��삳�����ꍇ�ɓ���
  Arguments:    None.
  Returns:      OSHeapHandle - WRAM �A���[�i��Ɋm�ۂ��ꂽ�q�[�v�̃n���h����Ԃ��B
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystemCore(u8 memType)
{
    OSHeapHandle    hh;

    /* MAIN */
    if(memType == MEM_TYPE_MAIN)
    {
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

            if( heapSize <= 0) /* �q�[�v�̈�̊m�ۂɎ��s */
            {
                OS_Panic("ARM7: Failed to MAIN create heap.\n");
            }

    #ifdef SDK_TWLLTD
            {
                if ((ATH_DRV_HEAP_SIZE + WPA_HEAP_SIZE) > heapSize)
                {
                    OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, ATH_DRV_HEAP_SIZE + WPA_HEAP_SIZE);
                }
            }
    #endif
            OS_TPrintf("ARM7: MAIN heap size is %d\n", heapSize);
        }
    }

    /* WRAM */
    if( memType == MEM_TYPE_WRAM)
    {
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

            if( heapSize <= 0) /* �q�[�v�̈�̊m�ۂɎ��s */
            {
                OS_Panic("ARM7: Failed to MAIN create heap.\n");
            }

            if (WM_WL_HEAP_SIZE > heapSize)
            {
                OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, WM_WL_HEAP_SIZE);
            }
            OS_TPrintf("ARM7: WRAM heap size is %d\n", heapSize);
        }
    }
    return hh;
}

#ifdef SDK_TWLHYB
#include    <twl/ltdwram_begin.h>
/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystemCoreEx
  Description:  �����������ăV�X�e��������������B
                Hybrid �� TWL �œ��삳�����ꍇ�ɓ���
  Arguments:    None.
  Returns:      OSHeapHandle - WRAM �A���[�i��Ɋm�ۂ��ꂽ�q�[�v�̃n���h����Ԃ��B
 *---------------------------------------------------------------------------*/
static OSHeapHandle InitializeAllocateSystemCoreEx(u8 memType)
{
    OSHeapHandle    hh;

    if(memType == MEM_TYPE_MAIN)
    {
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

            // �q�[�v�T�C�Y�̊m�F
            {
                u32     heapSize;

                heapSize    =   (u32)OS_CheckHeap(OS_ARENA_MAIN_SUBPRIV, hh);

                if( heapSize <= 0) /* �q�[�v�̈�̊m�ۂɎ��s */
                {
                    OS_Panic("ARM7: Failed to MAIN create heap.\n");
                }

                OS_TPrintf("ARM7: MAIN heap size is %d (before AddToHead)\n", heapSize);
            }

            // �q�[�v�Ɋg���u���b�N��ǉ�
            OS_AddToHeap(OS_ARENA_MAIN_SUBPRIV, hh, extraLo, extraHi);
        }
        // �J�����g�q�[�v�ɐݒ�
        (void)OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
        // �q�[�v�T�C�Y�̊m�F
        {
            u32     heapSize;

            heapSize    =   (u32)OS_CheckHeap(OS_ARENA_MAIN_SUBPRIV, hh);

            if( heapSize <= 0) /* �q�[�v�̈�̊m�ۂɎ��s */
            {
                OS_Panic("ARM7: Failed to MAIN create heap.\n");
            }

            if ((WPA_HEAP_SIZE) > heapSize)
            {
                OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, WPA_HEAP_SIZE);
            }
            OS_TPrintf("ARM7: MAIN heap size is %d\n", heapSize);
        }
    }

    if(memType == MEM_TYPE_WRAM)
    {
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

            // �q�[�v�T�C�Y�̊m�F
            {
                u32     heapSize;

                heapSize    =   (u32)OS_CheckHeap(OS_ARENA_WRAM_SUBPRIV, hh);

                if( heapSize <= 0) /* �q�[�v�̈�̊m�ۂɎ��s */
                {
                    OS_Panic("ARM7: Failed to WRAM create heap.\n");
                }

                if (WM_WL_HEAP_SIZE > heapSize)
                {
                    OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, WM_WL_HEAP_SIZE);
                }
                OS_TPrintf("ARM7: WRAM heap size is %d (before AddToHeap)\n", heapSize);
            }

            // �q�[�v�Ɋg���u���b�N��ǉ�
            OS_AddToHeap(OS_ARENA_WRAM_SUBPRIV, hh, extraLo, extraHi);
        }

        // �J�����g�q�[�v�ɐݒ�
        (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);

        // �q�[�v�T�C�Y�̊m�F
        {
            u32     heapSize;

            heapSize    =   (u32)OS_CheckHeap(OS_ARENA_WRAM_SUBPRIV, hh);

            if( heapSize <= 0) /* �q�[�v�̈�̊m�ۂɎ��s */
            {
                OS_Panic("ARM7: Failed to WRAM create heap.\n");
            }

            if (ATH_DRV_HEAP_SIZE + WM_WL_HEAP_SIZE > heapSize)
            {
                OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, WM_WL_HEAP_SIZE + ATH_DRV_HEAP_SIZE );
            }
            OS_TPrintf("ARM7: WRAM heap size is %d\n", heapSize);
        }
    }

    return hh;
}
#include    <twl/ltdwram_end.h>
#endif

#ifdef  WM_PRECALC_ALLOWEDCHANNEL
extern u16 WMSP_GetAllowedChannel(u16 bitField);
#endif
/*---------------------------------------------------------------------------*
  Name:         ReadUserInfo

  Description:  NVRAM���烆�[�U�[����ǂݏo���A���L�̈�ɓW�J����B
                �~���[�����O����Ă���o�b�t�@���������Ă���ꍇ�́A
                ���L�̈�̃��[�U�[���i�[�ꏊ���N���A����B

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ReadUserInfo(void)
{
    s32     offset;
#ifdef  NVRAM_CONFIG_DATA_EX_VERSION
    NVRAMConfigEx temp[2];
#else
    NVRAMConfig temp[2];
#endif
    s32     check;
    u8     *p = OS_GetSystemWork()->nvramUserInfo;

    // �I�t�Z�b�g�ǂݏo��
#ifdef  NVRAM_CONFIG_CONST_ADDRESS
    offset = NVRAM_CONFIG_DATA_ADDRESS_DUMMY;
#else
    NVRAM_ReadDataBytes(NVRAM_CONFIG_DATA_OFFSET_ADDRESS, NVRAM_CONFIG_DATA_OFFSET_SIZE,
                        (u8 *)(&offset));
    offset <<= NVRAM_CONFIG_DATA_OFFSET_SHIFT;
#endif

#ifdef  NVRAM_CONFIG_DATA_EX_VERSION
    // �~���[���ꂽ�Q�̃f�[�^��ǂݏo��
    NVRAM_ReadDataBytes((u32)offset, sizeof(NVRAMConfigEx), (u8 *)(&temp[0]));
    NVRAM_ReadDataBytes((u32)(offset + SPI_NVRAM_PAGE_SIZE), sizeof(NVRAMConfigEx),
                        (u8 *)(&temp[1]));
    // �Q�̓��ǂ�����g�������f
    check = CheckCorrectNCDEx(temp);
#else
    // �~���[���ꂽ�Q�̃f�[�^��ǂݏo��
    NVRAM_ReadDataBytes((u32)offset, sizeof(NVRAMConfig), (u8 *)(&temp[0]));
    NVRAM_ReadDataBytes((u32)(offset + SPI_NVRAM_PAGE_SIZE), sizeof(NVRAMConfig), (u8 *)(&temp[1]));
    // �Q�̓��ǂ�����g�������f
    check = CheckCorrectNCD(temp);
#endif

    if (check >= 3)
    {
        // �A�v���P�[�V�����̋N����}��
        MI_CpuFill32(p, 0xffffffff, sizeof(NVRAMConfig));
    }
    else if (check)
    {
        s32     i;

        // �j�b�N�l�[����␳
        if (temp[check - 1].ncd.owner.nickname.length < NVRAM_CONFIG_NICKNAME_LENGTH)
        {
            for (i = NVRAM_CONFIG_NICKNAME_LENGTH;
                 i > temp[check - 1].ncd.owner.nickname.length; i--)
            {
                temp[check - 1].ncd.owner.nickname.str[i - 1] = 0x0000;
            }
        }
        // �R�����g��␳
        if (temp[check - 1].ncd.owner.comment.length < NVRAM_CONFIG_COMMENT_LENGTH)
        {
            for (i = NVRAM_CONFIG_COMMENT_LENGTH; i > temp[check - 1].ncd.owner.comment.length;
                 i--)
            {
                temp[check - 1].ncd.owner.comment.str[i - 1] = 0x0000;
            }
        }
        // ���L�̈�ɃX�g�A
        MI_CpuCopy32(&temp[check - 1], p, sizeof(NVRAMConfig));
    }
    else
    {
        // ���L�̈���N���A
        MI_CpuClear32(p, sizeof(NVRAMConfig));
    }

    // ����MAC�A�h���X�����[�U�[���̌��ɓW�J
    {
        u8      wMac[6];

        // NVRAM����MAC�A�h���X��ǂݏo��
        NVRAM_ReadDataBytes(NVRAM_CONFIG_MACADDRESS_ADDRESS, 6, wMac);
        // �W�J��A�h���X���v�Z
        p = (u8 *)((u32)p + ((sizeof(NVRAMConfig) + 3) & ~0x00000003));
        // ���L�̈�ɓW�J
        MI_CpuCopy8(wMac, p, 6);
    }

#ifdef  WM_PRECALC_ALLOWEDCHANNEL
    // �g�p�\�`�����l������g�p���`�����l�����v�Z
    {
        u16     enableChannel;
        u16     allowedChannel;

        // �g�p�\�`�����l����ǂݏo��
        NVRAM_ReadDataBytes(NVRAM_CONFIG_ENABLECHANNEL_ADDRESS, 2, (u8 *)(&enableChannel));
        // �g�p���`�����l�����v�Z
        allowedChannel = WMSP_GetAllowedChannel((u16)(enableChannel >> 1));
        // �W�J��A�h���X���v�Z(MAC�A�h���X�̌���2�o�C�g)
        p = (u8 *)((u32)p + 6);
        // ���L�̈�ɓW�J
        *((u16 *)p) = allowedChannel;
    }
#endif
}

#ifdef  NVRAM_CONFIG_DATA_EX_VERSION
/*---------------------------------------------------------------------------*
  Name:         IsValidConfigEx

  Description:  ���[�U�[��񂪊g���R���t�B�O�ɑΉ����Ă��邩�ǂ����𒲍�����B

  Arguments:    None.

  Returns:      BOOL    - �g�����[�U�[��񂪗L���ȏꍇ��TRUE��Ԃ��B
                          �����ł���ꍇ��FALSE��Ԃ��B
 *---------------------------------------------------------------------------*/
static BOOL IsValidConfigEx(void)
{
    u8      ipl2_type;

    NVRAM_ReadDataBytes(NVRAM_CONFIG_IPL2_TYPE_ADDRESS, NVRAM_CONFIG_IPL2_TYPE_SIZE, &ipl2_type);
    if (ipl2_type == NVRAM_CONFIG_IPL2_TYPE_NORMAL)
    {
        return FALSE;
    }
    if (ipl2_type & NVRAM_CONFIG_IPL2_TYPE_EX_MASK)
    {
        return TRUE;
    }
    return FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         GetRomValidLanguage

  Description:  ROM���o�^�G���A�̏�񂩂�A�g������R�[�h�̑Ή�����r�b�g�}�b�v
                �Ɋւ�����𒊏o����B

  Arguments:    None.

  Returns:      u16     - DS�J�[�h�A�������� �}���`�u�[�g�o�C�i�����Ή����Ă���
                          ����R�[�h�̃r�b�g�}�b�v��Ԃ��BDS�J�[�h���g������
                          �R�[�h�ɑΉ����Ă��Ȃ��ꍇ�� 0 ��Ԃ��B
 *---------------------------------------------------------------------------*/
static u16 GetRomValidLanguage(void)
{
    u16     ret = 0x0000;
    u8      langBit = OS_GetSystemWork()->rom_header[0x1d];

    // ROM���o�^�G���A�̊g������R�[�h���m�F
    if (langBit == ROMHEADER_FOR_CHINA_BIT)
    {
        // for CHINA
        ret |= (0x0001 << NVRAM_CONFIG_LANG_CHINESE);
    }
    else if (langBit == ROMHEADER_FOR_KOREA_BIT)
    {
        // for KOREA
        ret |= (0x0001 << NVRAM_CONFIG_LANG_HANGUL);
    }
    return ret;
}

/*---------------------------------------------------------------------------*
  Name:         CheckCorrectNCDEx

  Description:  �~���[�����O����Ă��郆�[�U�[���̂ǂ�����g���ׂ������肷��B

  Arguments:    nvdsp   - ��r����R���t�B�O�f�[�^�Q�̔z��B

  Returns:      s32     - 0: �����s�K�؁B
                          1: �z��[ 0 ]���K�؁B
                          2: �z��[ 1 ]���K�؁B
                          3: �A�v���̋N����}�����ׂ��B
 *---------------------------------------------------------------------------*/
static s32 CheckCorrectNCDEx(NVRAMConfigEx * ncdsp)
{
    u16     i;
    u16     calc_crc;
    s32     crc_flag = 0;
    u16     saveCount;

    // IPL���g������R�[�h�ɑΉ����Ă��邩
    if (IsValidConfigEx())
    {
        // IPL���g������R�[�h�ɑΉ����Ă���ꍇ
        u16     rom_valid_language = GetRomValidLanguage();

        for (i = 0; i < 2; i++)
        {
            calc_crc = SVC_GetCRC16(0xffff, (void *)(&ncdsp[i].ncd), sizeof(NVRAMConfigData));
            if ((ncdsp[i].crc16 == calc_crc) && (ncdsp[i].saveCount < NVRAM_CONFIG_SAVE_COUNT_MAX))
            {
                // CRC �������� saveCount �l�� 0x80 �����̃f�[�^�𐳓��Ɣ��f
                calc_crc =
                    SVC_GetCRC16(0xffff, (void *)(&ncdsp[i].ncd_ex), sizeof(NVRAMConfigDataEx));
                if ((ncdsp[i].crc16_ex == calc_crc)
                    && ((0x0001 << ncdsp[i].ncd_ex.language) &
                        (ncdsp[i].ncd_ex.valid_language_bitmap)))
                {
                    // �g���f�[�^�p CRC ���������A�ݒ茾��R�[�h���Ή�����R�[�h�Ɋ܂܂��ꍇ�ɐ����Ɣ��f
                    if (rom_valid_language & ncdsp[i].ncd_ex.valid_language_bitmap)
                    {
                        // �g������R�[�h�Œʏ팾��R�[�h���㏑��
                        ncdsp[i].ncd.option.language = ncdsp[i].ncd_ex.language;
                    }
                    if (rom_valid_language & (0x0001 << NVRAM_CONFIG_LANG_CHINESE) & ~ncdsp[i].
                        ncd_ex.valid_language_bitmap)
                    {
                        // ROM ���o�^�G���A��"������"�g������R�[�h���ݒ肳��Ă��邪�A
                        // IPL2�̑Ή�����R�[�h��"������"�g������R�[�h���܂܂�Ȃ��ꍇ�͋N����}��
                        return 3;
                    }
                    crc_flag |= (1 << i);
                }
            }
        }
    }
    else
    {
        // IPL���g������R�[�h�ɑΉ����Ă��Ȃ��ꍇ
        u16     rom_valid_language = GetRomValidLanguage();

        if (rom_valid_language & (0x0001 << NVRAM_CONFIG_LANG_CHINESE))
        {
            // ROM ���o�^�G���A��"������"�g������R�[�h���ݒ肳��Ă���ꍇ�͋N����}��
            return 3;
        }
        for (i = 0; i < 2; i++)
        {
            calc_crc = SVC_GetCRC16(0xffff, (void *)(&ncdsp[i].ncd), sizeof(NVRAMConfigData));
            if ((ncdsp[i].crc16 == calc_crc) && (ncdsp[i].saveCount < NVRAM_CONFIG_SAVE_COUNT_MAX))
            {
                // CRC �������� saveCount �l�� 0x80 �����̃f�[�^�𐳓��Ɣ��f
                crc_flag |= (1 << i);
            }
        }
    }

    // �����ȃf�[�^�̂����ǂ̃f�[�^���L�����𔻒肷��B
    switch (crc_flag)
    {
    case 1:
    case 2:
        // �Е���CRC��������
        return crc_flag;

    case 3:
        // �����Ƃ�CRC����������΂ǂ��炪�ŐV�̃f�[�^�����f����B
        saveCount = (u8)((ncdsp[0].saveCount + 1) & NVRAM_CONFIG_SAVE_COUNT_MASK);
        if (saveCount == ncdsp[1].saveCount)
        {
            return 2;
        }
        return 1;
    }

    // �����Ƃ�CRC���s��
    return 0;
}

#else
/*---------------------------------------------------------------------------*
  Name:         CheckCorrectNCD

  Description:  �~���[�����O����Ă��郆�[�U�[���̂ǂ�����g���ׂ������肷��B

  Arguments:    nvdsp   - ��r����R���t�B�O�f�[�^�Q�̔z��B

  Returns:      s32     - 0: �����s�K�؁B
                          1: �z��[ 0 ]���K�؁B
                          2: �z��[ 1 ]���K�؁B
 *---------------------------------------------------------------------------*/
static s32 CheckCorrectNCD(NVRAMConfig *ncdsp)
{
    u16     i;
    u16     calc_crc;
    s32     crc_flag = 0;
    u16     saveCount;

    // �e�~���[�f�[�^��CRC & saveCount�������`�F�b�N
    for (i = 0; i < 2; i++)
    {
        calc_crc = SVC_GetCRC16(0xffff, (void *)(&ncdsp[i].ncd), sizeof(NVRAMConfigData));

        if ((ncdsp[i].crc16 == calc_crc) && (ncdsp[i].saveCount < NVRAM_CONFIG_SAVE_COUNT_MAX))
        {
            // CRC���������AsaveCount�l��0x80�����̃f�[�^�𐳓��Ɣ��f�B
            crc_flag |= (1 << i);
        }
    }

    // �����ȃf�[�^�̂����ǂ̃f�[�^���L�����𔻒肷��B
    switch (crc_flag)
    {
    case 1:
    case 2:
        // �Е���CRC��������
        return crc_flag;

    case 3:
        // �����Ƃ�CRC����������΂ǂ��炪�ŐV�̃f�[�^�����f����B
        saveCount = (u8)((ncdsp[0].saveCount + 1) & NVRAM_CONFIG_SAVE_COUNT_MASK);
        if (saveCount == ncdsp[1].saveCount)
        {
            return 2;
        }
        return 1;
    }

    // �����Ƃ�CRC���s��
    return 0;
}
#endif

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
