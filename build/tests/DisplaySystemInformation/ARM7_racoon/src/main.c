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
    定数定義
 *---------------------------------------------------------------------------*/
#define WM_WL_HEAP_SIZE     0x2100
#define ATH_DRV_HEAP_SIZE   0x5800
#define WPA_HEAP_SIZE       0x1C00

#define MEM_TYPE_WRAM 0
#define MEM_TYPE_MAIN 1

/* Priorities of each threads */
#define THREAD_PRIO_SPI     2
#define THREAD_PRIO_MCU     4 // 暫定
#define THREAD_PRIO_SND     6
#define THREAD_PRIO_FATFS   8
#define THREAD_PRIO_AES     12
#define THREAD_PRIO_SEA     12
#define THREAD_PRIO_RTC     12
#define THREAD_PRIO_SNDEX   14
#define THREAD_PRIO_FS      15
/* OS_THREAD_LAUNCHER_PRIORITY 16 */

/* [TODO] 以下は New WM 側に移行するほうが好ましい? */
#define NWM_DMANO                   NWMSP_DMA_7
#define THREAD_PRIO_NWM_COMMMAND    9
#define THREAD_PRIO_NWM_EVENT       7
#define THREAD_PRIO_NWM_SDIO        8
#define THREAD_PRIO_NWM_WPA         10

// ROM 内登録エリアの拡張言語コード
#define ROMHEADER_FOR_CHINA_BIT        0x80
#define ROMHEADER_FOR_KOREA_BIT        0x40

/*---------------------------------------------------------------------------*
    内部関数定義
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
    外部シンボル参照
 *---------------------------------------------------------------------------*/
#ifdef  SDK_TWLHYB
extern void         SDK_LTDAUTOLOAD_LTDWRAM_BSS_END(void);
extern void         SDK_LTDAUTOLOAD_LTDMAIN_BSS_END(void);
#endif


/*---------------------------------------------------------------------------*
  Name:         TwlSpMain
  Description:  起動ベクタ。
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void TwlSpMain(void)
{
    OSHeapHandle    wramHeapHandle, mainHeapHandle;

	
	// OS初期化でARM9と同期する前にSCFGの情報を共有メモリに書き出す
	// レジスタ情報をコピー
	MI_CpuMove16( (void*)REG_ROM_ADDR, DISPINFO_SHARED_SCFG_REG_ADDR, DISPINFO_SHARED_SCFG_REG_SIZE );
	// WRAMに退避されてる分もコピー
	MI_CpuMove16( (void*)HWi_WSYS04_ADDR, DISPINFO_SHARED_SCFG_WRAM_ADDR, DISPINFO_SHARED_SCFG_WRAM_SIZE );

    // OS 初期化
    OS_Init();
    PrintDebugInfo();

    // NVRAM からユーザー情報読み出し
    ReadUserInfo();

    // ヒープ領域設定
    wramHeapHandle  =   InitializeAllocateSystem(MEM_TYPE_WRAM);
    mainHeapHandle  =   InitializeAllocateSystem(MEM_TYPE_MAIN);

    // ボタン入力サーチ初期化
    (void)PAD_InitXYButton();

    // 割り込み許可
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // ファイルシステム初期化
    FS_Init(FS_DMA_NOT_USE);
    FS_CreateReadServerThread(THREAD_PRIO_FS);

    if (OS_IsRunOnTwl() == TRUE)
    {
        InitializeFatfs();    // FATFS 初期化
#ifndef SDK_SEA  // ！暫定処置！
    // NWM 初期化
#ifdef SDK_TWLLTD
        InitializeNwm(mainHeapHandle, mainHeapHandle);      // LIMITED モードでは 無線のヒープを MAIN から確保
#else
        InitializeNwm(wramHeapHandle, mainHeapHandle);      // HYBRID モードでは 無線のヒープを WRAM から確保
#endif
#endif  // ifndef SDK_SEA
        AES_Init(THREAD_PRIO_AES);      // AES 初期化

#ifdef SDK_SEA
		SEA_Init(THREAD_PRIO_SEA);
#endif  // ifdef SDK_SEA
        MCU_InitIrq(THREAD_PRIO_MCU);  // MCU 初期化

        CDC_InitLib();	// CODECライブラリ初期化
    }

    if (OSi_IsCodecTwlMode() == TRUE)
    {
        // カメラ初期化
        CAMERA_Init();
        /* CODEC が TWL モードでないとシャッター音を強制的に鳴らす
           機能が使用できません。この為、CODEC が TWL モードの場合
           にのみカメラライブラリを使用可能な状態にします。 */
    }

    // サウンド初期化
    SND_Init(THREAD_PRIO_SND);
    if (OS_IsRunOnTwl() == TRUE)
    {
        SNDEX_Init(THREAD_PRIO_SNDEX);
    }

    // RTC 初期化
    RTC_Init(THREAD_PRIO_RTC);

    // 旧無線初期化
#ifndef SDK_SEA  // ！暫定処置！
    WVR_Begin(wramHeapHandle);
#endif  // ifdef SDK_SEA

    // SPI 初期化
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
  Description:  ARM7 コンポーネントの情報をデバッグ出力する。
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
  Description:  FATFSライブラリを初期化する。FATFS初期化関数内でスレッド休止
                する為、休止中動作するダミーのスレッドを立てる。
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitializeFatfs(void)
{
    OSThread    thread;
    u32         stack[18];

    // ダミースレッド作成
    OS_CreateThread(&thread, DummyThread, NULL,
        (void*)((u32)stack + (sizeof(u32) * 18)), sizeof(u32) * 18, OS_THREAD_PRIORITY_MAX);
    OS_WakeupThreadDirect(&thread);


    // FATFSライブラリの初期化
    if(!FATFS_Init( FATFS_DMA_4, FATFS_DMA_5, THREAD_PRIO_FATFS))
    {
        // do nothing
    }

    // ダミースレッド破棄
    OS_KillThread(&thread, NULL);
}
#include    <twl/ltdwram_end.h>

#include    <twl/ltdwram_begin.h>
/*---------------------------------------------------------------------------*
  Name:         InitializeNwm
  Description:  NWMライブラリを初期化する。
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
  Description:  FATFSライブラリ、CDCライブラリを初期化する際に立てるダミーの
                スレッド。
  Arguments:    arg -   使用しない。
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
  Description:  メモリ割当てシステムを初期化する。
  Arguments:    None.
  Returns:      OSHeapHandle - WRAM アリーナ上に確保されたヒープのハンドルを返す。
 *---------------------------------------------------------------------------*/
static OSHeapHandle  InitializeAllocateSystem(u8 memType)
{

    OSHeapHandle    hh;

#ifdef SDK_TWLHYB
    if( OS_IsRunOnTwl() == TRUE)
    {
        hh = InitializeAllocateSystemCoreEx(memType); /* Hybrid を TWL で動作させる */
    }
    else
#endif
    {
        hh = InitializeAllocateSystemCore(memType);     /* Hybrid を DS で動作させる or Limited */
    }

    return hh;
}

/*---------------------------------------------------------------------------*
  Name:         InitializeAllocateSystemCore
  Description:  メモリ割当てシステムを初期化する。
                Hybrid を DS で動作させた場合、Limited を TWL で動作させた場合に動作
  Arguments:    None.
  Returns:      OSHeapHandle - WRAM アリーナ上に確保されたヒープのハンドルを返す。
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

            // アリーナを 0 クリア
            MI_CpuClear8(lo, (u32)hi - (u32)lo);

            // メモリ割り当て初期化
            lo  =   OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, lo, hi, 1);
            // アリーナ下位アドレスを設定
            OS_SetArenaLo(OS_ARENA_MAIN_SUBPRIV, lo);

            // ヒープ作成
            hh  =   OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, lo, hi);

            if (hh < 0)
            {
                OS_Panic("ARM7: Failed to MAIN create heap.\n");
            }
        }
        // カレントヒープに設定
        (void)OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
        // ヒープサイズの確認
        {
            u32     heapSize;

            heapSize    =   (u32)OS_CheckHeap(OS_ARENA_MAIN_SUBPRIV, hh);

            if( heapSize <= 0) /* ヒープ領域の確保に失敗 */
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

            // アリーナを 0 クリア
            MI_CpuClear8(lo, (u32)hi - (u32)lo);

            // メモリ割り当て初期化
            lo  =   OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV, lo, hi, 1);
            // アリーナ下位アドレスを設定
            OS_SetArenaLo(OS_ARENA_WRAM_SUBPRIV, lo);

            // ヒープ作成
            hh  =   OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV, lo, hi);

            if (hh < 0)
            {
                OS_Panic("ARM7: Failed to WRAM create heap.\n");
            }
        }

        // カレントヒープに設定
        (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);

        // ヒープサイズの確認
        {
            u32     heapSize;

            heapSize    =   (u32)OS_CheckHeap(OS_ARENA_WRAM_SUBPRIV, hh);

            if( heapSize <= 0) /* ヒープ領域の確保に失敗 */
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
  Description:  メモリ割当てシステムを初期化する。
                Hybrid を TWL で動作させた場合に動作
  Arguments:    None.
  Returns:      OSHeapHandle - WRAM アリーナ上に確保されたヒープのハンドルを返す。
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
            // アリーナを 0 クリア
            MI_CpuClear8(basicLo, (u32)basicHi - (u32)basicLo);
            MI_CpuClear8(extraLo, (u32)extraHi - (u32)extraLo);

            // メモリ割り当て初期化
            if ((u32)basicLo < (u32)extraLo)
            {
                basicLo =   OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, basicLo, extraHi, 1);
                // アリーナ下位アドレスを設定
                OS_SetArenaLo(OS_ARENA_MAIN_SUBPRIV, basicLo);
            }
            else
            {
                extraLo =   OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, extraLo, basicHi, 1);
            }

            // ヒープ作成
            hh  =   OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, basicLo, basicHi);

            if (hh < 0)
            {
                OS_Panic("ARM7: Failed to create MAIN heap.\n");
            }

            // ヒープサイズの確認
            {
                u32     heapSize;

                heapSize    =   (u32)OS_CheckHeap(OS_ARENA_MAIN_SUBPRIV, hh);

                if( heapSize <= 0) /* ヒープ領域の確保に失敗 */
                {
                    OS_Panic("ARM7: Failed to MAIN create heap.\n");
                }

                OS_TPrintf("ARM7: MAIN heap size is %d (before AddToHead)\n", heapSize);
            }

            // ヒープに拡張ブロックを追加
            OS_AddToHeap(OS_ARENA_MAIN_SUBPRIV, hh, extraLo, extraHi);
        }
        // カレントヒープに設定
        (void)OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
        // ヒープサイズの確認
        {
            u32     heapSize;

            heapSize    =   (u32)OS_CheckHeap(OS_ARENA_MAIN_SUBPRIV, hh);

            if( heapSize <= 0) /* ヒープ領域の確保に失敗 */
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

            // アリーナを 0 クリア
            MI_CpuClear8(basicLo, (u32)basicHi - (u32)basicLo);
            MI_CpuClear8(extraLo, (u32)extraHi - (u32)extraLo);

            // メモリ割り当て初期化
            if ((u32)basicLo < (u32)extraLo)
            {
                basicLo =   OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV, basicLo, extraHi, 1);
                // アリーナ下位アドレスを設定
                OS_SetArenaLo(OS_ARENA_WRAM_SUBPRIV, basicLo);
            }
            else
            {
                extraLo =   OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV, extraLo, basicHi, 1);
            }

            // ヒープ作成
            hh  =   OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV, basicLo, basicHi);

            if (hh < 0)
            {
                OS_Panic("ARM7: Failed to WRAM create heap.\n");
            }

            // ヒープサイズの確認
            {
                u32     heapSize;

                heapSize    =   (u32)OS_CheckHeap(OS_ARENA_WRAM_SUBPRIV, hh);

                if( heapSize <= 0) /* ヒープ領域の確保に失敗 */
                {
                    OS_Panic("ARM7: Failed to WRAM create heap.\n");
                }

                if (WM_WL_HEAP_SIZE > heapSize)
                {
                    OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, WM_WL_HEAP_SIZE);
                }
                OS_TPrintf("ARM7: WRAM heap size is %d (before AddToHeap)\n", heapSize);
            }

            // ヒープに拡張ブロックを追加
            OS_AddToHeap(OS_ARENA_WRAM_SUBPRIV, hh, extraLo, extraHi);
        }

        // カレントヒープに設定
        (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);

        // ヒープサイズの確認
        {
            u32     heapSize;

            heapSize    =   (u32)OS_CheckHeap(OS_ARENA_WRAM_SUBPRIV, hh);

            if( heapSize <= 0) /* ヒープ領域の確保に失敗 */
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

  Description:  NVRAMからユーザー情報を読み出し、共有領域に展開する。
                ミラーリングされているバッファが両方壊れている場合は、
                共有領域のユーザー情報格納場所をクリアする。

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

    // オフセット読み出し
#ifdef  NVRAM_CONFIG_CONST_ADDRESS
    offset = NVRAM_CONFIG_DATA_ADDRESS_DUMMY;
#else
    NVRAM_ReadDataBytes(NVRAM_CONFIG_DATA_OFFSET_ADDRESS, NVRAM_CONFIG_DATA_OFFSET_SIZE,
                        (u8 *)(&offset));
    offset <<= NVRAM_CONFIG_DATA_OFFSET_SHIFT;
#endif

#ifdef  NVRAM_CONFIG_DATA_EX_VERSION
    // ミラーされた２つのデータを読み出し
    NVRAM_ReadDataBytes((u32)offset, sizeof(NVRAMConfigEx), (u8 *)(&temp[0]));
    NVRAM_ReadDataBytes((u32)(offset + SPI_NVRAM_PAGE_SIZE), sizeof(NVRAMConfigEx),
                        (u8 *)(&temp[1]));
    // ２つの内どちらを使うか判断
    check = CheckCorrectNCDEx(temp);
#else
    // ミラーされた２つのデータを読み出し
    NVRAM_ReadDataBytes((u32)offset, sizeof(NVRAMConfig), (u8 *)(&temp[0]));
    NVRAM_ReadDataBytes((u32)(offset + SPI_NVRAM_PAGE_SIZE), sizeof(NVRAMConfig), (u8 *)(&temp[1]));
    // ２つの内どちらを使うか判断
    check = CheckCorrectNCD(temp);
#endif

    if (check >= 3)
    {
        // アプリケーションの起動を抑制
        MI_CpuFill32(p, 0xffffffff, sizeof(NVRAMConfig));
    }
    else if (check)
    {
        s32     i;

        // ニックネームを補正
        if (temp[check - 1].ncd.owner.nickname.length < NVRAM_CONFIG_NICKNAME_LENGTH)
        {
            for (i = NVRAM_CONFIG_NICKNAME_LENGTH;
                 i > temp[check - 1].ncd.owner.nickname.length; i--)
            {
                temp[check - 1].ncd.owner.nickname.str[i - 1] = 0x0000;
            }
        }
        // コメントを補正
        if (temp[check - 1].ncd.owner.comment.length < NVRAM_CONFIG_COMMENT_LENGTH)
        {
            for (i = NVRAM_CONFIG_COMMENT_LENGTH; i > temp[check - 1].ncd.owner.comment.length;
                 i--)
            {
                temp[check - 1].ncd.owner.comment.str[i - 1] = 0x0000;
            }
        }
        // 共有領域にストア
        MI_CpuCopy32(&temp[check - 1], p, sizeof(NVRAMConfig));
    }
    else
    {
        // 共有領域をクリア
        MI_CpuClear32(p, sizeof(NVRAMConfig));
    }

    // 無線MACアドレスをユーザー情報の後ろに展開
    {
        u8      wMac[6];

        // NVRAMからMACアドレスを読み出し
        NVRAM_ReadDataBytes(NVRAM_CONFIG_MACADDRESS_ADDRESS, 6, wMac);
        // 展開先アドレスを計算
        p = (u8 *)((u32)p + ((sizeof(NVRAMConfig) + 3) & ~0x00000003));
        // 共有領域に展開
        MI_CpuCopy8(wMac, p, 6);
    }

#ifdef  WM_PRECALC_ALLOWEDCHANNEL
    // 使用可能チャンネルから使用許可チャンネルを計算
    {
        u16     enableChannel;
        u16     allowedChannel;

        // 使用可能チャンネルを読み出し
        NVRAM_ReadDataBytes(NVRAM_CONFIG_ENABLECHANNEL_ADDRESS, 2, (u8 *)(&enableChannel));
        // 使用許可チャンネルを計算
        allowedChannel = WMSP_GetAllowedChannel((u16)(enableChannel >> 1));
        // 展開先アドレスを計算(MACアドレスの後ろの2バイト)
        p = (u8 *)((u32)p + 6);
        // 共有領域に展開
        *((u16 *)p) = allowedChannel;
    }
#endif
}

#ifdef  NVRAM_CONFIG_DATA_EX_VERSION
/*---------------------------------------------------------------------------*
  Name:         IsValidConfigEx

  Description:  ユーザー情報が拡張コンフィグに対応しているかどうかを調査する。

  Arguments:    None.

  Returns:      BOOL    - 拡張ユーザー情報が有効な場合にTRUEを返す。
                          無効である場合はFALSEを返す。
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

  Description:  ROM内登録エリアの情報から、拡張言語コードの対応言語ビットマップ
                に関する情報を抽出する。

  Arguments:    None.

  Returns:      u16     - DSカード、もしくは マルチブートバイナリが対応している
                          言語コードのビットマップを返す。DSカードが拡張言語
                          コードに対応していない場合は 0 を返す。
 *---------------------------------------------------------------------------*/
static u16 GetRomValidLanguage(void)
{
    u16     ret = 0x0000;
    u8      langBit = OS_GetSystemWork()->rom_header[0x1d];

    // ROM内登録エリアの拡張言語コードを確認
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

  Description:  ミラーリングされているユーザー情報のどちらを使うべきか判定する。

  Arguments:    nvdsp   - 比較するコンフィグデータ２つの配列。

  Returns:      s32     - 0: 両方不適切。
                          1: 配列[ 0 ]が適切。
                          2: 配列[ 1 ]が適切。
                          3: アプリの起動を抑制すべき。
 *---------------------------------------------------------------------------*/
static s32 CheckCorrectNCDEx(NVRAMConfigEx * ncdsp)
{
    u16     i;
    u16     calc_crc;
    s32     crc_flag = 0;
    u16     saveCount;

    // IPLが拡張言語コードに対応しているか
    if (IsValidConfigEx())
    {
        // IPLが拡張言語コードに対応している場合
        u16     rom_valid_language = GetRomValidLanguage();

        for (i = 0; i < 2; i++)
        {
            calc_crc = SVC_GetCRC16(0xffff, (void *)(&ncdsp[i].ncd), sizeof(NVRAMConfigData));
            if ((ncdsp[i].crc16 == calc_crc) && (ncdsp[i].saveCount < NVRAM_CONFIG_SAVE_COUNT_MAX))
            {
                // CRC が正しく saveCount 値が 0x80 未満のデータを正当と判断
                calc_crc =
                    SVC_GetCRC16(0xffff, (void *)(&ncdsp[i].ncd_ex), sizeof(NVRAMConfigDataEx));
                if ((ncdsp[i].crc16_ex == calc_crc)
                    && ((0x0001 << ncdsp[i].ncd_ex.language) &
                        (ncdsp[i].ncd_ex.valid_language_bitmap)))
                {
                    // 拡張データ用 CRC が正しく、設定言語コードが対応言語コードに含まれる場合に正当と判断
                    if (rom_valid_language & ncdsp[i].ncd_ex.valid_language_bitmap)
                    {
                        // 拡張言語コードで通常言語コードを上書き
                        ncdsp[i].ncd.option.language = ncdsp[i].ncd_ex.language;
                    }
                    if (rom_valid_language & (0x0001 << NVRAM_CONFIG_LANG_CHINESE) & ~ncdsp[i].
                        ncd_ex.valid_language_bitmap)
                    {
                        // ROM 内登録エリアに"中国語"拡張言語コードが設定されているが、
                        // IPL2の対応言語コードに"中国語"拡張言語コードが含まれない場合は起動を抑制
                        return 3;
                    }
                    crc_flag |= (1 << i);
                }
            }
        }
    }
    else
    {
        // IPLが拡張言語コードに対応していない場合
        u16     rom_valid_language = GetRomValidLanguage();

        if (rom_valid_language & (0x0001 << NVRAM_CONFIG_LANG_CHINESE))
        {
            // ROM 内登録エリアに"中国語"拡張言語コードが設定されている場合は起動を抑制
            return 3;
        }
        for (i = 0; i < 2; i++)
        {
            calc_crc = SVC_GetCRC16(0xffff, (void *)(&ncdsp[i].ncd), sizeof(NVRAMConfigData));
            if ((ncdsp[i].crc16 == calc_crc) && (ncdsp[i].saveCount < NVRAM_CONFIG_SAVE_COUNT_MAX))
            {
                // CRC が正しく saveCount 値が 0x80 未満のデータを正当と判断
                crc_flag |= (1 << i);
            }
        }
    }

    // 正当なデータのうちどのデータが有効かを判定する。
    switch (crc_flag)
    {
    case 1:
    case 2:
        // 片方のCRCだけ正常
        return crc_flag;

    case 3:
        // 両方ともCRCが正しければどちらが最新のデータか判断する。
        saveCount = (u8)((ncdsp[0].saveCount + 1) & NVRAM_CONFIG_SAVE_COUNT_MASK);
        if (saveCount == ncdsp[1].saveCount)
        {
            return 2;
        }
        return 1;
    }

    // 両方ともCRCが不正
    return 0;
}

#else
/*---------------------------------------------------------------------------*
  Name:         CheckCorrectNCD

  Description:  ミラーリングされているユーザー情報のどちらを使うべきか判定する。

  Arguments:    nvdsp   - 比較するコンフィグデータ２つの配列。

  Returns:      s32     - 0: 両方不適切。
                          1: 配列[ 0 ]が適切。
                          2: 配列[ 1 ]が適切。
 *---------------------------------------------------------------------------*/
static s32 CheckCorrectNCD(NVRAMConfig *ncdsp)
{
    u16     i;
    u16     calc_crc;
    s32     crc_flag = 0;
    u16     saveCount;

    // 各ミラーデータのCRC & saveCount正当性チェック
    for (i = 0; i < 2; i++)
    {
        calc_crc = SVC_GetCRC16(0xffff, (void *)(&ncdsp[i].ncd), sizeof(NVRAMConfigData));

        if ((ncdsp[i].crc16 == calc_crc) && (ncdsp[i].saveCount < NVRAM_CONFIG_SAVE_COUNT_MAX))
        {
            // CRCが正しく、saveCount値が0x80未満のデータを正当と判断。
            crc_flag |= (1 << i);
        }
    }

    // 正当なデータのうちどのデータが有効かを判定する。
    switch (crc_flag)
    {
    case 1:
    case 2:
        // 片方のCRCだけ正常
        return crc_flag;

    case 3:
        // 両方ともCRCが正しければどちらが最新のデータか判断する。
        saveCount = (u8)((ncdsp[0].saveCount + 1) & NVRAM_CONFIG_SAVE_COUNT_MASK);
        if (saveCount == ncdsp[1].saveCount)
        {
            return 2;
        }
        return 1;
    }

    // 両方ともCRCが不正
    return 0;
}
#endif

/*---------------------------------------------------------------------------*
  Name:         VBlankIntr
  Description:  V ブランク割り込みベクタ。
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
