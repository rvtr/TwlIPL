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
#include	<twl/hw/common/mmap_wramEnv.h>
#include    <sysmenu.h>
#include    "nvram_sp.h"

/*---------------------------------------------------------------------------*
    定数定義
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
static void         SetSCFGWork( void );
static void			ResetRTC( void );
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
	
	// MMEMサイズチェックは、ARM7の_start内でやっているので、ノーケアでOK.
	// SCFGレジスタ→HWi_WSYS04 etc.→system shared領域への値セットは、ランチャー起動時点では行われていないので、
	// ランチャー自身がこれらの値を使うには、自身でこれらの値をセットしてやる必要がある。
	// ランチャーからアプリを起動する際には、reboot.cが値を再セットしてくれる。
	SetSCFGWork();	// [TODO]未デバッグ
	
    // OS 初期化
    OS_Init();
	OS_InitTick();
    PrintDebugInfo();
	
    // ランチャーパラメター取得（Cold/Hotスタート判定含む）
	ReadLauncherParameter();
	
	// RTCリセット
	ResetRTC();		// 330usくらい
	
    // NVRAM からユーザー情報読み出し
    ReadUserInfo();
    
	// [TODO:] カード電源ONして、ROMヘッダのみリード＆チェックくらいはやっておきたい
	
	SYSMi_GetWork()->isARM9Start = TRUE;				// [TODO:] HW_RED_RESERVEDはNANDファームでクリアしておいて欲しい
	
    // ヒープ領域設定
    {
        void *wram = OS_GetWramSubPrivArenaHi();
        void *mmem = OS_GetSubPrivArenaHi();
        OS_SetSubPrivArenaHi( (void*)SYSM_OWN_ARM7_MMEM_ADDR_END );     // メモリ配置をいじっているので、アリーナHiも変更しないとダメ！！
        OS_SetWramSubPrivArenaHi( (void*)SYSM_OWN_ARM7_WRAM_ADDR_END );
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
#endif
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

    // RTC 初期化
    RTC_Init(THREAD_PRIO_RTC);

    // 旧無線初期化
    WVR_Begin(heapHandle);

    // SPI 初期化
    SPI_Init(THREAD_PRIO_SPI);

    BOOT_Init();

    // 活栓挿抜機能初期化
    HOTSW_Init();
	
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


// システム領域(WRAM & MMEM)にSCFG情報をセット [TODO:]最終的にNANDファームからブートされたらいらないかも
static void SetSCFGWork( void )
{
	// SCFGレジスタが有効な場合のみセット
	if( reg_SCFG_EXT & REG_SCFG_EXT_CFG_MASK ) {
		// WRAMのシステム領域にセット
		u32 *wsys4 = (void*)HWi_WSYS04_ADDR;
		u8  *wsys8 = (void*)HWi_WSYS08_ADDR;
		u8  *wsys9 = (void*)HWi_WSYS09_ADDR;
		// copy scfg registers
		*wsys4 = reg_SCFG_EXT;
		*wsys8 = (u8)(((reg_SCFG_OP & REG_SCFG_OP_OPT_MASK)) |
						((reg_SCFG_A9ROM & (REG_SCFG_A9ROM_RSEL_MASK | REG_SCFG_A9ROM_SEC_MASK)) << (HWi_WSYS08_ROM_ARM9RSEL_SHIFT - REG_SCFG_A9ROM_RSEL_SHIFT)) |
						((reg_SCFG_A7ROM & (REG_SCFG_A7ROM_RSEL_MASK | REG_SCFG_A7ROM_FUSE_MASK)) << (HWi_WSYS08_ROM_ARM7RSEL_SHIFT - REG_SCFG_A7ROM_RSEL_SHIFT)) |
						((reg_SCFG_WL & REG_SCFG_WL_OFFB_MASK) << (HWi_WSYS08_WL_OFFB_SHIFT - REG_SCFG_WL_OFFB_SHIFT))
						);
		*wsys9 = (u8)((*wsys9 & (HWi_WSYS09_JTAG_DSPJE_MASK | HWi_WSYS09_JTAG_CPUJE_MASK | HWi_WSYS09_JTAG_ARM7SEL_MASK)) |
						((reg_SCFG_JTAG & (REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK))) |
						((reg_SCFG_JTAG & REG_SCFG_JTAG_DSPJE_MASK) >> (REG_SCFG_JTAG_DSPJE_SHIFT - HWi_WSYS09_JTAG_DSPJE_SHIFT)) | 
						((reg_SCFG_CLK & (REG_SCFG_CLK_AESHCLK_MASK | REG_SCFG_CLK_SD2HCLK_MASK | REG_SCFG_CLK_SD1HCLK_MASK)) << (HWi_WSYS09_CLK_SD1HCLK_SHIFT - REG_SCFG_CLK_SD1HCLK_SHIFT)) | 
						((reg_SCFG_CLK & (REG_SCFG_CLK_SNDMCLK_MASK | REG_SCFG_CLK_WRAMHCLK_MASK)) >> (REG_SCFG_CLK_WRAMHCLK_SHIFT - HWi_WSYS09_CLK_WRAMHCLK_SHIFT))
						);
		
		// MMEMのシステム領域にコピー
		MI_CpuCopy8( (void*)HWi_WSYS04_ADDR, (void *)HW_SYS_CONF_BUF, 6 );
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
		SYSMi_GetWork()->isResetRTC = TRUE;
	}
}


// ランチャーパラメータのリードおよびHot/Coldスタート判定
void ReadLauncherParameter( void )
{
	BOOL hot;
    SYSMi_GetWork()->isValidLauncherParam = OS_ReadLauncherParameter( (LauncherParam *)&(SYSMi_GetWork()->launcherParam), &hot );
    SYSMi_GetWork()->isHotStart = hot;
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
  Description:  FATFSライブラリを初期化する。
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitializeFatfs(void)
{
    // FATFSライブラリの初期化
    if (FATFS_Init(FATFS_DMA_NOT_USE, THREAD_PRIO_FATFS))
    {
        // do nothing
    }
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
  Description:  CDCライブラリを初期化する際に立てるダミーのスレッド。
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
	u8 *p;
	
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
