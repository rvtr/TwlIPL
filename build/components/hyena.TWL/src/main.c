/*---------------------------------------------------------------------------*
  Project:  TwlIPL - components - hyena.TWL
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
#include    <twl/nwm/ARM7/ForLauncher/nwm_sp_init_for_launcher.h>
#include    <twl/camera.h>
#include    <twl/rtc.h>
#include    <nitro/hw/common/lcd.h>
#include    <nitro/gx.h>
#include    <twl/os/common/codecmode.h>
#include    <twl/cdc.h>
#include    <twl/snd/ARM7/sndex_api.h>
#include    <twl/aes.h>
#include    <twl/mcu.h>
#include    <twl/hw/common/mmap_wramEnv.h>
#include    <sysmenu.h>
#include    <sysmenu/mcu.h>
#include    <firm/memorymap.h>
#include    "pm_pmic.h"
#include    "internal_api.h"
#include    "nvram_sp.h"
#include    "twl/sea.h"

// �������i����ł̓f�o�b�K�ڑ����Ȃ��Ȃ�I�����Ă��悢�j
//#define HYENA_ROMEMU_INFO_FROM_LNCR_PARAM
//#define HYENA_RTC_DEBUG

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define WM_WL_HEAP_SIZE     0x2100
#define ATH_DRV_HEAP_SIZE   0x5800
#define WPA_HEAP_SIZE       0x1C00

#define MEM_TYPE_WRAM 0
#define MEM_TYPE_MAIN 1

/* Priorities of each threads */
#define THREAD_PRIO_MCU     1   /* �n�[�h�E�F�A���Z�b�g���ɑ��̃X���b�h�ɗD�悵�ē����K�v�A�� */
#define THREAD_PRIO_SPI     2
#define THREAD_PRIO_SYSMMCU 6
#define THREAD_PRIO_SND     6
#define THREAD_PRIO_FATFS   8
#define THREAD_PRIO_HOTSW   11
#define THREAD_PRIO_AES     12
#define THREAD_PRIO_SEA     12
#define THREAD_PRIO_RTC     12
#define THREAD_PRIO_SNDEX   14
#define THREAD_PRIO_FS      15
/* OS_THREAD_LAUNCHER_PRIORITY 16 */

#define NWM_DMANO                   NWMSP_DMA_NOT_USE // NWM��NDMA�͎g�p���Ȃ��B
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
static void         ResetRTC( void );
static void         ReadLauncherParameter( void );
static void         PrintDebugInfo(void);
static OSHeapHandle InitializeAllocateSystem(u8 memType);
static OSHeapHandle InitializeAllocateSystemCore(u8 memType);
#ifdef SDK_TWLHYB
static OSHeapHandle InitializeAllocateSystemCoreEx(u8 memType);
#endif
static void         DummyThread(void* arg);
static void         ReadUserInfo(void);
static void         VBlankIntr(void);
static void         InitializeFatfs(void);
static void         InitializeNwm(OSArenaId drvArenaId, OSHeapHandle drvHeapHandle,
                                  OSArenaId wpaArenaId, OSHeapHandle wpaHeapHandle);
static void         InitializeCdc();
static void         AdjustVolume(void);
/*---------------------------------------------------------------------------*
    �O���V���{���Q��
 *---------------------------------------------------------------------------*/
#ifdef  SDK_TWLHYB
extern void         SDK_LTDAUTOLOAD_LTDWRAM_BSS_END(void);
extern void         SDK_LTDAUTOLOAD_LTDMAIN_BSS_END(void);
#endif
extern void         SDK_SEA_KEY_STORE(void);
extern void         SDK_STATIC_BSS_END(void);

extern BOOL sdmcGetNandLogFatal( void );

/*---------------------------------------------------------------------------*
  Name:         TwlSpMain
  Description:  �N���x�N�^�B
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
TwlSpMain(void)
{
    OSHeapHandle    wramHeapHandle, mainHeapHandle;

    // SYSM���[�N�̃N���A
//	MI_CpuClear32( SYSMi_GetWork(), sizeof(SYSM_work) );		// NAND�t�@�[���ŃN���A���Ă���̂ŁA����Ȃ��B
																// �����������`���[��ł��Ȃ�Acrt0.o���ŃN���A�̋L�q�����Ȃ���Autoload�Z�O�����g�ɔz�u����Ă��܂��āA���܂������Ȃ��_�ɒ��ӂ���B
	
    // �o�b�N���C�gON
    while ( (reg_GX_DISPSTAT & REG_GX_DISPSTAT_INI_MASK) == FALSE )
    {
    }
    PMi_SetControl( PMIC_CTL_BKLT1 | PMIC_CTL_BKLT2 );

#ifdef INITIAL_KEYTABLE_PRELOAD
	HOTSW_CopyInitialKeyTable();
#endif
    
    // OS ������
    OS_Init();
    PrintDebugInfo();

    // �����`���[�o�[�W�������i�[�i���̂Ƃ���A�Œ�ł��}�E���g���o�^�O�ɂ͊i�[����K�v����j
    *(u8 *)HW_TWL_RED_LAUNCHER_VER = (u8)SYSM_LAUNCHER_VER;

    // �����`���[�̃}�E���g���o�^
    SYSMi_SetLauncherMountInfo();

    // �����`���[�p�����^�[�擾�iCold/Hot�X�^�[�g����܂ށj
    ReadLauncherParameter();

    // RTC���Z�b�g
    ResetRTC();     // 330us���炢

    // NVRAM ���烆�[�U�[���ǂݏo��
    ReadUserInfo();

    // NAND��FATAL�G���[���o
    if( sdmcGetNandLogFatal() != FALSE) {
        /* �̏ስ������ */
        SYSMi_GetWork()->flags.arm7.isNANDFatalError = TRUE;
    }

    SYSMi_GetWork()->flags.arm7.isARM9Start = TRUE;

    // �q�[�v�̈�ݒ�
#ifndef USE_HYENA_COMPONENT
    OS_SetSubPrivArenaLo( (void*)SDK_STATIC_BSS_END );
#endif
    OS_SetSubPrivArenaHi( (void*)SYSM_OWN_ARM7_MMEM_ADDR_END );     // �������z�u���������Ă���̂ŁA�A���[�iHi���ύX���Ȃ��ƃ_���I�I
    OS_SetWramSubPrivArenaHi( (void*)(SYSM_OWN_ARM7_WRAM_ADDR_END - HW_FIRM_FROM_FIRM_BUF_SIZE) ); // ���̎��_�ł͌����Ԃ��Ȃ��悤��
    OS_TPrintf( "MMEM SUBPRV ARENA HI : %08x -> %08x\n", OS_GetSubPrivArenaHi(), OS_GetSubPrivArenaHi() );
    OS_TPrintf( "WRAM SUBPRV ARENA HI : %08x -> %08x\n", OS_GetWramSubPrivArenaHi(), OS_GetWramSubPrivArenaHi() );
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

    // PXI�R�[���o�b�N�̐ݒ�
    SYSM_InitPXI(THREAD_PRIO_SYSMMCU);

    // �t�@�C���V�X�e��������
    FS_Init(FS_DMA_NOT_USE);
    FS_CreateReadServerThread(THREAD_PRIO_FS);

    if (OS_IsRunOnTwl() == TRUE)
    {
        InitializeFatfs();    // FATFS ������
        // NWM ������
        InitializeNwm(OS_ARENA_MAIN_SUBPRIV, mainHeapHandle, // heap setting for TWL wireless host driver
                      OS_ARENA_MAIN_SUBPRIV, mainHeapHandle  // heap setting for wpa
                      );
#ifndef SDK_NOCRYPTO
        AES_Init(THREAD_PRIO_AES);           // AES ������

        {
            // JPEG�G���R�[�h�p�̌��Z�b�g
            SYSMi_SetAESKeysForSignJPEG( (ROM_Header *)HW_TWL_ROM_HEADER_BUF, NULL, NULL );
            // NAND�t�@�[����HW_LAUNCHER_DELIVER_PARAM_BUF�ւ�AES_SEED�Z�b�g���s���Ă����̂ŁAIS�f�o�b�K�ڑ��Ɋ֌W�Ȃ�SDK_SEA_KEY_STORE�ւ̃R�s�[���s���΂悢
            MI_CpuCopyFast( (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, (void *)SDK_SEA_KEY_STORE, HW_LAUNCHER_DELIVER_PARAM_BUF_SIZE );
        }

#ifdef SDK_SEA
        SEA_Init(THREAD_PRIO_SEA);
#endif  // ifdef SDK_SEA
#endif
        MCU_InitIrq(THREAD_PRIO_MCU);  // MCU ������

        // �{�����[���ݒ�̒���
        AdjustVolume();
    }

    if (OSi_IsCodecTwlMode() == TRUE)
    {
        // CODEC ������
        // �����`���[�̂�CDC_InitForFirstBoot�Ŏ��ۂ�CODEC�̏��������s���B
		// �A�v���N�����ɂ�CODEC�͊��ɏ���������Ă��邽��mongoose�Ȃǂł�
		// �ȈՓI�ȏ�����CDC_InitLib�ŗǂ��B 2008/07/14
        InitializeCdc();

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

    // �����`���[�ł́A�������̏�������main loop�ōs���B

    // SPI ������
	// ��ARM9����OS_Init����PM_Init��PM��PXI�R�[���o�b�N�҂������Ă���A������ARM9�Ɠ����������B
    SPI_Init(THREAD_PRIO_SPI);

    BOOT_Init();

    // ����}���@�\������
    if( ( SYSM_GetLauncherParamBody()->v1.flags.isValid ) &&
        ( SYSM_GetLauncherParamBody()->v1.flags.bootType != LAUNCHER_BOOTTYPE_ROM ) &&
        ( SYSM_GetLauncherParamBody()->v1.bootTitleID )
        )
#ifdef HYENA_ROMEMU_INFO_FROM_LNCR_PARAM
    {
        // �����`���[�p�����[�^�Ń_�C���N�g�J�[�h�u�[�g�ȊO�̎w�肪���鎞�́A�����}����OFF�ɂ���B
        SYSMi_GetWork()->flags.hotsw.isEnableHotSW = 0;
    }else {
        // ����ȊO�̎��͊����}��ON
        SYSMi_GetWork()->flags.hotsw.isEnableHotSW = 1;
    }
#else
    {
        // �����`���[�p�����[�^�Ń_�C���N�g�J�[�h�u�[�g�ȊO�̎w�肪���鎞�́AROM�G�~�����[�V�������̂ݕK�v�B
        SYSMi_GetWork()->flags.hotsw.isLoadRomEmuOnly = 1;
    }else {
        // ����ȊO�̎��͕��ʂɃ��[�h
        SYSMi_GetWork()->flags.hotsw.isLoadRomEmuOnly = 0;
    }
    SYSMi_GetWork()->flags.hotsw.isEnableHotSW = 1;
#endif

    HOTSW_Init(THREAD_PRIO_HOTSW);

    while (TRUE)
    {
        OS_Halt();
        // �����t�@�[���̃��[�h�������`�F�b�N�B
        if (TRUE == NWMSPi_CheckInstalledNotification())
        {
            /* M&M chip�ł͖����t�@�[����DL���� �� �������������̏��������K�v����B
               ���̂��߁A�V�����̃t�@�[��DL�����ʒm��main loop�Ń`�F�b�N���A
               �������ʒm���ꂽ�狌�������������s������ANWM�Ɋm�F�ʒm�𑗂�B[twl-dev:0980] */
            WVR_Begin(wramHeapHandle);
#if 1
            // [TODO:] RC plus branch�ł́ANWMSPi_NotifyConfirmation()�̕Ԃ�l���Ȃ����߁A�Ԃ�l�]�����s��Ȃ��B
            //         ���������[�X�ŕԂ�l�]�����s���悤�ύX�\��B
            NWMSPi_NotifyConfirmation();
#else
            if (FALSE == NWMSPi_NotifyConfirmation())
            {
                // NWM�ւ�Confirmation�ʒm�̂��߂�SendMessage��ENQ�ł��Ȃ����.
                // (�����ɗ��邱�Ƃ͂��肦�Ȃ���.)
                OS_Panic("ARM7: Failed to complete wireless firmare install.\n");
            }
#endif
        }
        BOOT_WaitStart();
    }
}


// RTC�̃��Z�b�g�`�F�b�N
static void ResetRTC( void )
{
    SYSM_work* sw = SYSMi_GetWork();

    // �����`���[�Ń��Z�b�g�����o���邽�߂ɂ��̏��������Ă��邪�ARTC_Init���ł��������Ƃ����Ă���̂ŁA������Ɩ��ʁB
    RTCRawStatus1 stat1;
    RTCRawStatus2 stat2;
    RTC_ReadStatus1( &stat1 );
    RTC_ReadStatus2( &stat2 );

    // ���Z�b�g�A�d�������A�d���d���ቺ�AIC�e�X�g�̊e�t���O���m�F
    if ( stat1.reset || stat1.poc || stat1.bld || stat2.test )
    {
        // ���Z�b�g���s
        stat1.reset = 1;
        RTC_WriteStatus1( &stat1 );
        sw->flags.arm7.isResetRTC = TRUE;
    }

    // FOUT��32KHz�o�͂łȂ��ꍇ�́A32KHz�o�͂ɏC���ݒ肷��B�i�����Ŏg�p���Ă���j
    {
        RTCRawFout  fout;
        RTC_ReadFout(&fout);
        if( fout.fout != RTC_FOUT_DUTY_32KHZ ) {
            fout.fout = RTC_FOUT_DUTY_32KHZ;
            RTC_WriteFout(&fout);
        }
    }

	// RTC����f�[�^�ǂݍ���
    RTC_ReadDateTime(&sw->Rtc1stData);

    // NTR-IPL���l�ɃA���[�����Ԃ��N���A
    //�i���荞�݃C�l�[�u���łȂ��ƃA�N�Z�X�ł��Ȃ��j
    {
        static RTCRawAlarm alarm = {0,0,0,0,0,0,0,0,0};

        stat2.intr_mode  = RTC_INTERRUPT_MODE_ALARM;                // �A���[�����荞�݃C�l�[�u��
        stat2.intr2_mode = TRUE;
        RTC_WriteStatus2( &stat2 );
        
        (void)RTC_WriteAlarm1( &alarm );
        (void)RTC_WriteAlarm2( &alarm );
#ifdef HYENA_RTC_DEBUG
        {
            static RTCRawAlarm rd_alarm = {1,1,1,1,1,1,1,1,1};
            (void)RTC_ReadAlarm1( &rd_alarm );
            (void)RTC_ReadAlarm2( &rd_alarm );
        }
#endif
        stat2.intr_mode  = RTC_INTERRUPT_MODE_NONE;                  // �A���[�����荞�݃f�B�Z�[�u��
        stat2.intr2_mode = RTC_INTERRUPT_MODE_NONE;
        RTC_WriteStatus2( &stat2 );
    }
}


// �����`���[�p�����[�^�̃��[�h�����Hot/Cold�X�^�[�g����
void ReadLauncherParameter( void )
{
    BOOL hot;
    SYSMi_GetWork()->flags.arm7.isValidLauncherParam = OS_ReadLauncherParameter( (LauncherParam *)&(SYSMi_GetWork()->launcherParam), &hot );
    SYSMi_GetWork()->flags.arm7.isHotStart = hot;
    // ���C���������̃����`���[�p�����[�^���N���A���Ă���
    MI_CpuClearFast( (void*)HW_PARAM_LAUNCH_PARAM, HW_PARAM_LAUNCH_PARAM_SIZE );
    // Cold�X�^�[�g���̓A�v���p�����[�^���N���A
    if ( ! hot )
    {
        MI_CpuClearFast( (void*)HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_SIZE );
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
#ifdef USE_HYENA_COMPONENT
    OS_TPrintf("ARM7: This component is \"hyena.TWL\"\n");
#else
    OS_TPrintf("ARM7: This component is \"jackal.TWL\"\n");
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
    if(!FATFS_Init( FATFS_DMA_4, FATFS_DMA_5, THREAD_PRIO_FATFS))
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
InitializeNwm(OSArenaId drvArenaId, OSHeapHandle drvHeapHandle,
              OSArenaId wpaArenaId, OSHeapHandle wpaHeapHandle)
{
    NwmspInit nwmInit;

    nwmInit.dmaNo = NWM_DMANO;
    nwmInit.cmdPrio = THREAD_PRIO_NWM_COMMMAND;
    nwmInit.evtPrio = THREAD_PRIO_NWM_EVENT;
    nwmInit.sdioPrio = THREAD_PRIO_NWM_SDIO;
    nwmInit.drvHeap.id = drvArenaId;
    nwmInit.drvHeap.handle = drvHeapHandle;

    nwmInit.wpaPrio = THREAD_PRIO_NWM_WPA;
    nwmInit.wpaHeap.id = wpaArenaId;
    nwmInit.wpaHeap.handle = wpaHeapHandle;

    /* �V����������(�����`���[��p) */
    NWMSP_InitForLauncher(&nwmInit);

}
#include    <twl/ltdwram_end.h>

#include    <twl/ltdwram_begin.h>
/*---------------------------------------------------------------------------*
  Name:         InitializeCdc
  Description:  CDC���C�u����������������B
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitializeCdc(void)
{
	u32 spiLockId;

	// CODEC�A�N�Z�X�p��SPI���b�NID���擾����
	spiLockId = (u32)OS_GetLockID();
	if (spiLockId == OS_LOCK_ID_ERROR)
	{
        OS_Warning("OS_GetLockID failed.\n");
	}

	SPI_Lock(spiLockId);    	// CODEC�pSPI�r�����b�N
    CDC_InitForFirstBoot();     // �������`���[���ꏈ���B
	SPI_Unlock(spiLockId);  	// CODEC�pSPI�r�����b�N

	OS_Sleep(50);				// CDC_InitForFirstBoot�ł�PowerOnTime+�f�|�b�v����

	SPI_Lock(spiLockId);    	// CODEC�pSPI�r�����b�N
    CDC_DisableExternalDepop(); // �O���f�|�b�v��H�𖳌��ɂ��܂�
	SPI_Unlock(spiLockId);  	// CODEC�pSPI�r�����b�N
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

#ifdef  SDK_TWLHYB
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

            if ((ATH_DRV_HEAP_SIZE + WPA_HEAP_SIZE) > heapSize)
            {
                OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, ATH_DRV_HEAP_SIZE + WPA_HEAP_SIZE);
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

        if (WM_WL_HEAP_SIZE > heapSize)
        {
            OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, WM_WL_HEAP_SIZE);
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
	// ��hyena�ł́ANITRO�{�̐ݒ�f�[�^�̓��[�h���Ȃ��B
	
    u8     *p = OS_GetSystemWork()->nvramUserInfo;

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
  Name:         AdjustVolume

  Description:  32�i�K�̃{�����[����8�i�K�ɗʎq������

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void AdjustVolume(void)
{
    u8 volume = MCU_GetVolume();
    u8 adjust;
    if ( volume < 2 )
    {
        adjust = 0;
    }
    else if ( volume < 5 )
    {
        adjust = 2;
    }
    else if ( volume < 9 )
    {
        adjust = 6;
    }
    else if ( volume < 14 )
    {
        adjust = 11;
    }
    else if ( volume < 19 )
    {
        adjust = 16;
    }
    else if ( volume < 24 )
    {
        adjust = 21;
    }
    else if ( volume < 29 )
    {
        adjust = 26;
    }
    else
    {
        adjust = 31;
    }
    OS_TPrintf("Current volume: %d.\n", volume);
    if ( volume != adjust )
    {
        OS_TPrintf("Volume adjusts to %d.\n", adjust);
        MCU_SetVolume(adjust);
    }
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
