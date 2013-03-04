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

// 未実装（現状ではデバッガ接続しないなら選択してもよい）
//#define HYENA_ROMEMU_INFO_FROM_LNCR_PARAM
//#define HYENA_RTC_DEBUG

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define WM_WL_HEAP_SIZE     0x2100
#define ATH_DRV_HEAP_SIZE   0x5800
#define WPA_HEAP_SIZE       0x1C00

#define MEM_TYPE_WRAM 0
#define MEM_TYPE_MAIN 1

/* Priorities of each threads */
#define THREAD_PRIO_MCU     1   /* ハードウェアリセット時に他のスレッドに優先して動く必要アリ */
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

#define NWM_DMANO                   NWMSP_DMA_NOT_USE // NWMのNDMAは使用しない。
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
    外部シンボル参照
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
  Description:  起動ベクタ。
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void
TwlSpMain(void)
{
    OSHeapHandle    wramHeapHandle, mainHeapHandle;

    // SYSMワークのクリア
//	MI_CpuClear32( SYSMi_GetWork(), sizeof(SYSM_work) );		// NANDファームでクリアしているので、いらない。
																// ※もしランチャー上でやるなら、crt0.o内でクリアの記述をしないとAutoloadセグメントに配置されてしまって、うまく動かない点に注意する。
	
    // バックライトON
    while ( (reg_GX_DISPSTAT & REG_GX_DISPSTAT_INI_MASK) == FALSE )
    {
    }
    PMi_SetControl( PMIC_CTL_BKLT1 | PMIC_CTL_BKLT2 );

#ifdef INITIAL_KEYTABLE_PRELOAD
	HOTSW_CopyInitialKeyTable();
#endif
    
    // OS 初期化
    OS_Init();
    PrintDebugInfo();

    // ランチャーバージョンを格納（今のところ、最低でもマウント情報登録前には格納する必要あり）
    *(u8 *)HW_TWL_RED_LAUNCHER_VER = (u8)SYSM_LAUNCHER_VER;

    // ランチャーのマウント情報登録
    SYSMi_SetLauncherMountInfo();

    // ランチャーパラメター取得（Cold/Hotスタート判定含む）
    ReadLauncherParameter();

    // RTCリセット
    ResetRTC();     // 330usくらい

    // NVRAM からユーザー情報読み出し
    ReadUserInfo();

    // NANDのFATALエラー検出
    if( sdmcGetNandLogFatal() != FALSE) {
        /* 故障扱い処理 */
        SYSMi_GetWork()->flags.arm7.isNANDFatalError = TRUE;
    }

    SYSMi_GetWork()->flags.arm7.isARM9Start = TRUE;

    // ヒープ領域設定
#ifndef USE_HYENA_COMPONENT
    OS_SetSubPrivArenaLo( (void*)SDK_STATIC_BSS_END );
#endif
    OS_SetSubPrivArenaHi( (void*)SYSM_OWN_ARM7_MMEM_ADDR_END );     // メモリ配置をいじっているので、アリーナHiも変更しないとダメ！！
    OS_SetWramSubPrivArenaHi( (void*)(SYSM_OWN_ARM7_WRAM_ADDR_END - HW_FIRM_FROM_FIRM_BUF_SIZE) ); // この時点では鍵をつぶさないように
    OS_TPrintf( "MMEM SUBPRV ARENA HI : %08x -> %08x\n", OS_GetSubPrivArenaHi(), OS_GetSubPrivArenaHi() );
    OS_TPrintf( "WRAM SUBPRV ARENA HI : %08x -> %08x\n", OS_GetWramSubPrivArenaHi(), OS_GetWramSubPrivArenaHi() );
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

    // PXIコールバックの設定
    SYSM_InitPXI(THREAD_PRIO_SYSMMCU);

    // ファイルシステム初期化
    FS_Init(FS_DMA_NOT_USE);
    FS_CreateReadServerThread(THREAD_PRIO_FS);

    if (OS_IsRunOnTwl() == TRUE)
    {
        InitializeFatfs();    // FATFS 初期化
        // NWM 初期化
        InitializeNwm(OS_ARENA_MAIN_SUBPRIV, mainHeapHandle, // heap setting for TWL wireless host driver
                      OS_ARENA_MAIN_SUBPRIV, mainHeapHandle  // heap setting for wpa
                      );
#ifndef SDK_NOCRYPTO
        AES_Init(THREAD_PRIO_AES);           // AES 初期化

        {
            // JPEGエンコード用の鍵セット
            SYSMi_SetAESKeysForSignJPEG( (ROM_Header *)HW_TWL_ROM_HEADER_BUF, NULL, NULL );
            // NANDファームがHW_LAUNCHER_DELIVER_PARAM_BUFへのAES_SEEDセットを行ってくれるので、ISデバッガ接続に関係なくSDK_SEA_KEY_STOREへのコピーを行えばよい
            MI_CpuCopyFast( (void *)HW_LAUNCHER_DELIVER_PARAM_BUF, (void *)SDK_SEA_KEY_STORE, HW_LAUNCHER_DELIVER_PARAM_BUF_SIZE );
        }

#ifdef SDK_SEA
        SEA_Init(THREAD_PRIO_SEA);
#endif  // ifdef SDK_SEA
#endif
        MCU_InitIrq(THREAD_PRIO_MCU);  // MCU 初期化

        // ボリューム設定の調整
        AdjustVolume();
    }

    if (OSi_IsCodecTwlMode() == TRUE)
    {
        // CODEC 初期化
        // ランチャーのみCDC_InitForFirstBootで実際にCODECの初期化を行う。
		// アプリ起動時にはCODECは既に初期化されているためmongooseなどでは
		// 簡易的な初期化CDC_InitLibで良い。 2008/07/14
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

    // ランチャーでは、旧無線の初期化はmain loopで行う。

    // SPI 初期化
	// ※ARM9側のOS_Init内のPM_InitでPMのPXIコールバック待ちをしており、ここでARM9と同期が取られる。
    SPI_Init(THREAD_PRIO_SPI);

    BOOT_Init();

    // 活栓挿抜機能初期化
    if( ( SYSM_GetLauncherParamBody()->v1.flags.isValid ) &&
        ( SYSM_GetLauncherParamBody()->v1.flags.bootType != LAUNCHER_BOOTTYPE_ROM ) &&
        ( SYSM_GetLauncherParamBody()->v1.bootTitleID )
        )
#ifdef HYENA_ROMEMU_INFO_FROM_LNCR_PARAM
    {
        // ランチャーパラメータでダイレクトカードブート以外の指定がある時は、活線挿抜をOFFにする。
        SYSMi_GetWork()->flags.hotsw.isEnableHotSW = 0;
    }else {
        // それ以外の時は活線挿抜ON
        SYSMi_GetWork()->flags.hotsw.isEnableHotSW = 1;
    }
#else
    {
        // ランチャーパラメータでダイレクトカードブート以外の指定がある時は、ROMエミュレーション情報のみ必要。
        SYSMi_GetWork()->flags.hotsw.isLoadRomEmuOnly = 1;
    }else {
        // それ以外の時は普通にロード
        SYSMi_GetWork()->flags.hotsw.isLoadRomEmuOnly = 0;
    }
    SYSMi_GetWork()->flags.hotsw.isEnableHotSW = 1;
#endif

    HOTSW_Init(THREAD_PRIO_HOTSW);

    while (TRUE)
    {
        OS_Halt();
        // 無線ファームのロード完了をチェック。
        if (TRUE == NWMSPi_CheckInstalledNotification())
        {
            /* M&M chipでは無線ファームのDL完了 → 旧無線初期化の順序を守る必要あり。
               そのため、新無線のファームDL完了通知をmain loopでチェックし、
               完了が通知されたら旧無線初期化を行った後、NWMに確認通知を送る。[twl-dev:0980] */
            WVR_Begin(wramHeapHandle);
#if 1
            // [TODO:] RC plus branchでは、NWMSPi_NotifyConfirmation()の返り値がないため、返り値評価を行わない。
            //         次期リリースで返り値評価を行うよう変更予定。
            NWMSPi_NotifyConfirmation();
#else
            if (FALSE == NWMSPi_NotifyConfirmation())
            {
                // NWMへのConfirmation通知のためのSendMessageにENQできない状態.
                // (ここに来ることはありえない筈.)
                OS_Panic("ARM7: Failed to complete wireless firmare install.\n");
            }
#endif
        }
        BOOT_WaitStart();
    }
}


// RTCのリセットチェック
static void ResetRTC( void )
{
    SYSM_work* sw = SYSMi_GetWork();

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
        sw->flags.arm7.isResetRTC = TRUE;
    }

    // FOUTが32KHz出力でない場合は、32KHz出力に修正設定する。（無線で使用している）
    {
        RTCRawFout  fout;
        RTC_ReadFout(&fout);
        if( fout.fout != RTC_FOUT_DUTY_32KHZ ) {
            fout.fout = RTC_FOUT_DUTY_32KHZ;
            RTC_WriteFout(&fout);
        }
    }

	// RTC初回データ読み込み
    RTC_ReadDateTime(&sw->Rtc1stData);

    // NTR-IPL同様にアラーム時間をクリア
    //（割り込みイネーブルでないとアクセスできない）
    {
        static RTCRawAlarm alarm = {0,0,0,0,0,0,0,0,0};

        stat2.intr_mode  = RTC_INTERRUPT_MODE_ALARM;                // アラーム割り込みイネーブル
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
        stat2.intr_mode  = RTC_INTERRUPT_MODE_NONE;                  // アラーム割り込みディセーブル
        stat2.intr2_mode = RTC_INTERRUPT_MODE_NONE;
        RTC_WriteStatus2( &stat2 );
    }
}


// ランチャーパラメータのリードおよびHot/Coldスタート判定
void ReadLauncherParameter( void )
{
    BOOL hot;
    SYSMi_GetWork()->flags.arm7.isValidLauncherParam = OS_ReadLauncherParameter( (LauncherParam *)&(SYSMi_GetWork()->launcherParam), &hot );
    SYSMi_GetWork()->flags.arm7.isHotStart = hot;
    // メインメモリのランチャーパラメータをクリアしておく
    MI_CpuClearFast( (void*)HW_PARAM_LAUNCH_PARAM, HW_PARAM_LAUNCH_PARAM_SIZE );
    // Coldスタート時はアプリパラメータもクリア
    if ( ! hot )
    {
        MI_CpuClearFast( (void*)HW_PARAM_DELIVER_ARG, HW_PARAM_DELIVER_ARG_SIZE );
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
#ifdef USE_HYENA_COMPONENT
    OS_TPrintf("ARM7: This component is \"hyena.TWL\"\n");
#else
    OS_TPrintf("ARM7: This component is \"jackal.TWL\"\n");
#endif
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
    if(!FATFS_Init( FATFS_DMA_4, FATFS_DMA_5, THREAD_PRIO_FATFS))
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

    /* 新無線初期化(ランチャー専用) */
    NWMSP_InitForLauncher(&nwmInit);

}
#include    <twl/ltdwram_end.h>

#include    <twl/ltdwram_begin.h>
/*---------------------------------------------------------------------------*
  Name:         InitializeCdc
  Description:  CDCライブラリを初期化する。
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitializeCdc(void)
{
	u32 spiLockId;

	// CODECアクセス用のSPIロックIDを取得する
	spiLockId = (u32)OS_GetLockID();
	if (spiLockId == OS_LOCK_ID_ERROR)
	{
        OS_Warning("OS_GetLockID failed.\n");
	}

	SPI_Lock(spiLockId);    	// CODEC用SPI排他ロック
    CDC_InitForFirstBoot();     // ※ランチャー特殊処理。
	SPI_Unlock(spiLockId);  	// CODEC用SPI排他ロック

	OS_Sleep(50);				// CDC_InitForFirstBootでのPowerOnTime+デポップ期間

	SPI_Lock(spiLockId);    	// CODEC用SPI排他ロック
    CDC_DisableExternalDepop(); // 外部デポップ回路を無効にします
	SPI_Unlock(spiLockId);  	// CODEC用SPI排他ロック
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

#ifdef  SDK_TWLHYB
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

  Description:  NVRAMからユーザー情報を読み出し、共有領域に展開する。
                ミラーリングされているバッファが両方壊れている場合は、
                共有領域のユーザー情報格納場所をクリアする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ReadUserInfo(void)
{
	// ※hyenaでは、NITRO本体設定データはリードしない。
	
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
  Name:         AdjustVolume

  Description:  32段階のボリュームを8段階に量子化する

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
