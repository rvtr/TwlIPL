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
#include    <twl/hw/common/mmap_wramEnv.h>
#include    <sysmenu.h>
#include    <sysmenu/mcu.h>
#include    <firm/memorymap.h>
#include    "nvram_sp.h"
#include    "pm_pmic.h"
#include    "internal_api.h"
#ifdef SDK_SEA
#include    <twl/sea.h>
#endif  // ifdef SDK_SEA

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
/* [TODO] Work around. Should be defined in wm_sp.h */
#define WM_WL_HEAP_SIZE     0x2100

/* Priorities of each threads */
#define THREAD_PRIO_SPI     2
#define THREAD_PRIO_MCU     4 // 暫定
#define THREAD_PRIO_SYSMMCU 6
#define THREAD_PRIO_SND     6
#define THREAD_PRIO_FATFS   8
#define THREAD_PRIO_HOTSW   11
#define THREAD_PRIO_RTC     12
#define THREAD_PRIO_SNDEX   14
#define THREAD_PRIO_FS      15
/* OS_THREAD_LAUNCHER_PRIORITY 16 */

/* [TODO] 以下は New WM 側に移行するほうが好ましい? */
#define NWM_DMANO                   3
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
static void         ResetRTC( void );
static void         ReadLauncherParameter( void );
static void         PrintDebugInfo(void);
static OSHeapHandle InitializeAllocateSystem(void);
static void         InitializeFatfs(void);
static void         InitializeNwm(void);
static void         InitializeCdc(void);
static void         DummyThread(void* arg);
static void         ReadUserInfo(void);
static void         VBlankIntr(void);

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
void
TwlSpMain(void)
{
    OSHeapHandle    heapHandle;

    // SYSMワークのクリア
    MI_CpuClear32( SYSMi_GetWork(), sizeof(SYSM_work) );

    // バックライトON
    while ( (reg_GX_DISPSTAT & REG_GX_DISPSTAT_INI_MASK) == FALSE )
    {
    }
    PMi_SetControl( PMIC_CTL_BKLT1 | PMIC_CTL_BKLT2 );

    // OS 初期化
    OS_Init();
    OS_InitTick();
    PrintDebugInfo();

    // ランチャーパラメター取得（Cold/Hotスタート判定含む）
    ReadLauncherParameter();

    // RTCリセット
    ResetRTC();     // 330usくらい

    // NVRAM からユーザー情報読み出し
    ReadUserInfo();

    // [TODO:] カード電源ONして、ROMヘッダのみリード＆チェックくらいはやっておきたい

    SYSMi_GetWork()->flags.common.isARM9Start = TRUE;                // [TODO:] HW_RED_RESERVEDはNANDファームでクリアしておいて欲しい

    // ヒープ領域設定
    {
        void *wram = OS_GetWramSubPrivArenaHi();
        void *mmem = OS_GetSubPrivArenaHi();
        OS_SetSubPrivArenaHi( (void*)SYSM_OWN_ARM7_MMEM_ADDR_END );     // メモリ配置をいじっているので、アリーナHiも変更しないとダメ！！
        OS_SetWramSubPrivArenaHi( (void*)(SYSM_OWN_ARM7_WRAM_ADDR_END - HW_FIRM_FROM_FIRM_BUF_SIZE) ); // この時点では鍵をつぶさないように
        OS_TPrintf( "MMEM SUBPRV ARENA HI : %08x -> %08x\n", mmem, OS_GetSubPrivArenaHi() );
        OS_TPrintf( "WRAM SUBPRV ARENA HI : %08x -> %08x\n", wram, OS_GetWramSubPrivArenaHi() );
    }
    heapHandle  =   InitializeAllocateSystem();

    // ボタン入力サーチ初期化
    (void)PAD_InitXYButton();

    // 割り込み許可
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // PXIコールバックの設定
    SYSM_InitPXI(THREAD_PRIO_SYSMMCU);

    // ファイルシステム初期化
    FS_Init(FS_DMA_NOT_USE);
    FS_CreateReadServerThread(THREAD_PRIO_FS);

    if (OS_IsRunOnTwl() == TRUE)
    {
        OSTick start = OS_GetTick();
        InitializeFatfs();    // FATFS 初期化
        OS_TPrintf( "FATFS init time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
        InitializeNwm();      // NWM 初期化
#ifndef SDK_NOCRYPTO
        AES_Init();           // AES 初期化
#ifdef SDK_SEA
        SEA_Init();
#endif  // ifdef SDK_SEA
#endif
        MCU_InitIrq(THREAD_PRIO_MCU);  // MCU 初期化
    }

    if (OSi_IsCodecTwlMode() == TRUE)
    {
        // CODEC 初期化
        InitializeCdc();
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
    WVR_Begin(heapHandle);

    // SPI 初期化
    SPI_Init(THREAD_PRIO_SPI);

    BOOT_Init();

    // 活栓挿抜機能初期化
    if( ( SYSM_GetLauncherParamBody()->v1.flags.isValid ) &&
        ( SYSM_GetLauncherParamBody()->v1.flags.bootType != LAUNCHER_BOOTTYPE_ROM ) &&
        ( SYSM_GetLauncherParamBody()->v1.bootTitleID )
        ) {
        // ランチャーパラメータでダイレクトカードブート以外の指定がある時は、活線挿抜をOFFにする。
        SYSMi_GetWork()->flags.hotsw.isEnableHotSW = 0;
    }else {
        // それ以外の時は活線挿抜ON
        SYSMi_GetWork()->flags.hotsw.isEnableHotSW = 1;
    }
	
	// [TODO]アプリジャンプ有効で、カードブートでない時は、最初からHOTSW_Initを呼ばないようにしたい。
    HOTSW_Init(THREAD_PRIO_HOTSW);
	
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


// RTCのリセットチェック
static void ResetRTC( void )
{
    // ランチャーでリセットを検出するためにこの処理をしているが、RTC_Init内でも同じことをしているので、ちょっと無駄。
    RTCRawStatus1 stat1;
    RTCRawStatus2 stat2;
    RTC_ReadStatus1( &stat1 );
    RTC_ReadStatus2( &stat2 );
    // リセット、電源投入、電源電圧低下、ICテストの各フラグを確認
    if ( stat1.reset || stat1.poc || stat1.bld || stat2.test )
    {
        // リセット実行
        stat1.reset = 1;
        RTC_WriteStatus1( &stat1 );
        SYSMi_GetWork()->flags.common.isResetRTC = TRUE;
    }
}


// ランチャーパラメータのリードおよびHot/Coldスタート判定
void ReadLauncherParameter( void )
{
    BOOL hot;
    SYSMi_GetWork()->flags.common.isValidLauncherParam = OS_ReadLauncherParameter( (LauncherParam *)&(SYSMi_GetWork()->launcherParam), &hot );
    SYSMi_GetWork()->flags.common.isHotStart = hot;
    // メインメモリのリセットパラメータをクリアしておく
    MI_CpuClear32( SYSMi_GetLauncherParamAddr(), 0x100 );
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
    OS_TPrintf("ARM7: This component is \"hyena.TWL\"\n");
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
#ifndef SDK_NOCRYPTO
    if(!FATFS_Init( FATFS_DMA_4, FATFS_DMA_NOT_USE, THREAD_PRIO_FATFS))
#else
    if (FATFS_Init(FATFS_DMA_NOT_USE, FATFS_DMA_NOT_USE, THREAD_PRIO_FATFS))
#endif
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
InitializeNwm(void)
{
    NwmspInit nwmInit;

    OSHeapHandle heapHandle;
    void*   Lo =   (void*)OS_GetSubPrivArenaLo();
    void*   Hi =   (void*)OS_GetSubPrivArenaHi();
    heapHandle  =   OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, Lo, Hi);

    /* [TODO] 確保したヒープ領域が新無線一式が必要としているメモリ量以上かのチェックが必要 */

    nwmInit.dmaNo = NWM_DMANO;
    nwmInit.cmdPrio = THREAD_PRIO_NWM_COMMMAND;
    nwmInit.evtPrio = THREAD_PRIO_NWM_EVENT;
    nwmInit.sdioPrio = THREAD_PRIO_NWM_SDIO;
    nwmInit.drvHeap.id = OS_ARENA_MAIN_SUBPRIV; /* [TODO] */
    nwmInit.drvHeap.handle = heapHandle;
#ifdef WPA_BUILT_IN /* WPA が組み込まれる場合、以下のメンバが追加される */
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
  Description:  CDCライブラリを初期化する。CDC初期化関数内でスレッド休止する
                為、休止中動作するダミーのスレッドを立てる。
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitializeCdc(void)
{
    OSThread    thread;
    u32         stack[18];

    // ダミースレッド作成
    OS_CreateThread(&thread, DummyThread, NULL,
        (void*)((u32)stack + (sizeof(u32) * 18)), sizeof(u32) * 18, OS_THREAD_PRIORITY_MAX);
    OS_WakeupThreadDirect(&thread);

#if 1
    // CODEC 初期化
    CDC_Init();
    CDC_InitMic();
//    CDCi_DumpRegisters();
#else
    /* [Debug] CODEC を DS モードで初期化 */
    *((u8*)(HW_TWL_ROM_HEADER_BUF + 0x01bf))    &=  ~(0x01);
    CDC_Init();
    CDC_GoDsMode();
    OS_TPrintf("Codec mode changed to DS mode for debug.\n");
#endif

    // ダミースレッド破棄
    OS_KillThread(&thread, NULL);
}

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
        if ((ATH_DRV_HEAP_SIZE + WPA_HEAP_SIZE) > heapSize)
        {
            OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, ATH_DRV_HEAP_SIZE + WPA_HEAP_SIZE);
        }
        OS_TPrintf("ARM7: MAIN heap size is %d (before AddToHead)\n", heapSize);
    }

        // ヒープに拡張ブロックを追加
        OS_AddToHeap(OS_ARENA_MAIN_SUBPRIV, hh, extraLo, extraHi);
    }
    else
#endif
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
        if (WM_WL_HEAP_SIZE > heapSize)
        {
            OS_Panic("Insufficient heap size. (0x%x < 0x%x)\n", heapSize, WM_WL_HEAP_SIZE);
        }
        OS_TPrintf("ARM7: WRAM heap size is %d (before AddToHeap)\n", heapSize);
    }

        // ヒープに拡張ブロックを追加
        OS_AddToHeap(OS_ARENA_WRAM_SUBPRIV, hh, extraLo, extraHi);
    }
    else
#endif
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

  Description:  NVRAMからユーザー情報を読み出し、共有領域に展開する。
                ミラーリングされているバッファが両方壊れている場合は、
                共有領域のユーザー情報格納場所をクリアする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ReadUserInfo(void)
{
    u8     *p = OS_GetSystemWork()->nvramUserInfo;

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

