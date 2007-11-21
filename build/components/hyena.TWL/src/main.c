/*---------------------------------------------------------------------------*
  Project:  TwlSDK - components - mongoose.TWL
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
#include    <twl/aes.h>
#include    <twl/mcu.h>
#include    <sysmenu/boot/common/boot.h>
#include    <sysmenu/sysmenu_lib/common/sysmenu_work.h>

#include    <sysmenu/card/common/blowfish.h>
#include    <sysmenu/card/common/Card.h>
#include    "nvram_sp.h"

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
/* [TODO] Work around. Should be defined in wm_sp.h */
#define WM_WL_HEAP_SIZE     0x2100

/* Priorities of each threads */
#define THREAD_PRIO_SPI     2
#define THREAD_PRIO_SND     6
#define THREAD_PRIO_FATFS   8
#define THREAD_PRIO_RTC     12
#define THREAD_PRIO_FS      15
/* OS_THREAD_LAUNCHER_PRIORITY 16 */

/* [TODO] �ȉ��� New WM ���Ɉڍs����ق����D�܂���? */
#define NWM_DMANO                   3
#define THREAD_PRIO_NWM_COMMMAND    6
#define THREAD_PRIO_NWM_EVENT       4
#define THREAD_PRIO_NWM_SDIO        5
#define THREAD_PRIO_NWM_WPA         7

// ROM ���o�^�G���A�̊g������R�[�h
#define ROMHEADER_FOR_CHINA_BIT        0x80
#define ROMHEADER_FOR_KOREA_BIT        0x40

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void         ReadResetParameter( void );
static void         PrintDebugInfo(void);
static OSHeapHandle InitializeAllocateSystem(void);
static void         InitializeFatfs(void);
static void         InitializeNwm(void);
static void         InitializeCdc(void);
static void         DummyThread(void* arg);
static void         ReadUserInfo(void);
static void         VBlankIntr(void);

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
void
TwlSpMain(void)
{
    OSHeapHandle    heapHandle;

    // SYSM���[�N�̃N���A
    MI_CpuClear32( SYSMi_GetWork(), sizeof(SYSM_work) );

    // OS ������
    OS_Init();
    PrintDebugInfo();
	
    // NVRAM ���烆�[�U�[���ǂݏo��
    ReadUserInfo();
    
    // Cold/Hot�X�^�[�g����
	ReadResetParameter();
	SYSMi_GetWork()->isARM9Start = TRUE;				// ��HW_RED_RESERVED��NAND�t�@�[���ŃN���A���Ă����ė~����
	
    // �q�[�v�̈�ݒ�
    {
        void *wram = OS_GetWramSubPrivArenaHi();
        void *mmem = OS_GetSubPrivArenaHi();
        OS_SetSubPrivArenaHi( (void*)SYSM_OWN_ARM7_MMEM_ADDR_END );     // �������z�u���������Ă���̂ŁA�A���[�iHi���ύX���Ȃ��ƃ_���I�I
        OS_SetWramSubPrivArenaHi( (void*)SYSM_OWN_ARM7_WRAM_ADDR_END );
        OS_TPrintf( "MMEM SUBPRV ARENA HI : %08x -> %08x\n", mmem, OS_GetSubPrivArenaHi() );
        OS_TPrintf( "WRAM SUBPRV ARENA HI : %08x -> %08x\n", wram, OS_GetWramSubPrivArenaHi() );
    }
    heapHandle  =   InitializeAllocateSystem();

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
        InitializeNwm();      // NWM ������
#ifndef SDK_NOCRYPTO
        AES_Init();           // AES ������
#endif
    }

    if (OSi_IsCodecTwlMode() == TRUE)
    {
        // CODEC ������
        InitializeCdc();
        // �J����������
        CAMERA_Init();
        /* CODEC �� TWL ���[�h�łȂ��ƃV���b�^�[���������I�ɖ炷
           �@�\���g�p�ł��܂���B���ׁ̈ACODEC �� TWL ���[�h�̏ꍇ
           �ɂ̂݃J�������C�u�������g�p�\�ȏ�Ԃɂ��܂��B */
    }

    // �T�E���h������
    SND_Init(THREAD_PRIO_SND);

    // RTC ������
    RTC_Init(THREAD_PRIO_RTC);

    // ������������
    WVR_Begin(heapHandle);

    // SPI ������
    SPI_Init(THREAD_PRIO_SPI);

    BOOT_Init();

    // ����}���@�\������
//    Cardm_Init();

    // �J�[�h���������Ă�����u�[�g�J�n
//    Card_Boot();

    while (TRUE)
    {
        OS_Halt();
        //---- check reset
        if (OS_IsResetOccurred())
        {
            OS_ResetSystem();
        }
        BOOT_WaitStart();
    }
}


// Hot/Cold�X�^�[�g���肨��у��Z�b�g�p�����[�^�̃��[�h
static void ReadResetParameter( void )
{
    // Hot/Cold�X�^�[�g����
#ifdef SDK_FINALROM
    if( SYSMi_GetMCUFreeRegisterValue() == 0 )          // �}�C�R���t���[���W�X�^�l��"0"�Ȃ�Cold�X�^�[�g
#else
    if( 1 )                                             // IS�f�o�b�K�ł̃f�o�b�O���쎞�ɏ�Ƀz�b�g�X�^�[�g���肳���̂�h��
#endif
    {
        u8 data = 1;
        MCU_SetFreeRegisters( 0, &data, 1 );            // �}�C�R���t���[���W�X�^�Ƀz�b�g�X�^�[�g�t���O���Z�b�g
        SYSMi_GetWork()->isHotStart = FALSE;
    }else {
        SYSMi_GetWork()->isHotStart = TRUE;
        // ���Z�b�g�p�����[�^�L������
        if( ( STD_StrNCmp( (const char *)&SYSMi_GetResetParamAddr()->header.magicCode,
                             SYSM_RESET_PARAM_MAGIC_CODE,
                             SYSM_RESET_PARAM_MAGIC_CODE_LEN ) == 0 ) &&
              ( SYSMi_GetResetParamAddr()->header.bodyLength > 0 ) &&
              ( SVC_GetCRC16( 65535, &SYSMi_GetResetParamAddr()->body, SYSMi_GetResetParamAddr()->header.bodyLength ) )
              ) {
            // ���Z�b�g�p�����[�^���L���Ȃ�A���[�N�ɑޔ�
            MI_CpuCopy32 ( SYSMi_GetResetParamAddr(), &SYSMi_GetWork()->resetParam, sizeof(ResetParam) );
            SYSMi_GetWork()->isValidResetParam = TRUE;
        }
    }
    // ���C���������̃��Z�b�g�p�����[�^���N���A���Ă���
    MI_CpuClear32( SYSMi_GetResetParamAddr(), 0x100 );
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
#ifdef  SDK_TWLLTD
    OS_TPrintf("ARM7: This component is \"racoon.TWL\"\n");
#else   /* SDK_TWLHYB */
#ifdef  SDK_WIRELESS_IN_VRAM
    OS_TPrintf("ARM7: This component is \"ichneumon.TWL\"\n");
#else
    OS_TPrintf("ARM7: This component is \"mongoose.TWL\"\n");
#endif
#endif
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
    // FATFS���C�u�����̏�����
    if (FATFS_Init(FATFS_DMA_NOT_USE, THREAD_PRIO_FATFS))
    {
        // do nothing
    }
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
InitializeNwm(void)
{
    NwmspInit nwmInit;

    OSHeapHandle heapHandle;
    void*   Lo =   (void*)OS_GetSubPrivArenaLo();
    void*   Hi =   (void*)OS_GetSubPrivArenaHi();
    heapHandle  =   OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, Lo, Hi);

    /* [TODO] �m�ۂ����q�[�v�̈悪�V�����ꎮ���K�v�Ƃ��Ă��郁�����ʈȏォ�̃`�F�b�N���K�v */

    nwmInit.dmaNo = NWM_DMANO;
    nwmInit.cmdPrio = THREAD_PRIO_NWM_COMMMAND;
    nwmInit.evtPrio = THREAD_PRIO_NWM_EVENT;
    nwmInit.sdioPrio = THREAD_PRIO_NWM_SDIO;
	nwmInit.drvHeap.id = OS_ARENA_MAIN_SUBPRIV; /* [TODO] */
	nwmInit.drvHeap.handle = heapHandle;
#ifdef WPA_BUILT_IN /* WPA ���g�ݍ��܂��ꍇ�A�ȉ��̃����o���ǉ������ */
    nwmInit.wpaPrio = THREAD_PRIO_NWM_WPA;
	nwmInit.wpaHeap.id = OS_ARENA_MAIN_SUBPRIV; /* [TODO] */
	nwmInit.wpaHeap.handle = heapHandle;
#endif
    NWMSP_Init(&nwmInit);

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
    OSThread    thread;
    u32         stack[18];

    // �_�~�[�X���b�h�쐬
    OS_CreateThread(&thread, DummyThread, NULL,
        (void*)((u32)stack + (sizeof(u32) * 18)), sizeof(u32) * 18, OS_THREAD_PRIORITY_MAX);
    OS_WakeupThreadDirect(&thread);

    // CODEC ������
    CDC_Init();
    CDC_InitMic();
//    CDCi_DumpRegisters();

    // �_�~�[�X���b�h�j��
    OS_KillThread(&thread, NULL);
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
InitializeAllocateSystem(void)
{
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
	u8 *p;
	
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
