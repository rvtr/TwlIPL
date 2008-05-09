/*---------------------------------------------------------------------------*
  Project:  TwlSDK - components - armadillo.TWL
  File:     main.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include    <twl/os/common/codecmode.h>
#include    <nitro/pad.h>
#include    <twl/aes.h>
#include    <twl/sea.h>
#include    <twl/fatfs.h>
#include    <twl/nwm.h>
#include    <twl/camera.h>
#include    <twl/mcu.h>
#include    <twl/cdc.h>
#include    <nitro/snd.h>
#include    <twl/snd/ARM7/sndex_api.h>
#include    <twl/rtc.h>
#include    <nitro/wvr.h>
#include    <twl/spi.h>
#include    "nvram_sp.h"
#include    "kami_pxi.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
/* 各スレッド優先度 */
#define THREAD_PRIO_SPI         2
#define THREAD_PRIO_MCU         4 // 暫定
#define THREAD_PRIO_SND         6
#define THREAD_PRIO_NWM_EVENT   7
#define THREAD_PRIO_NWM_SDIO    8
#define THREAD_PRIO_FATFS       8
#define THREAD_PRIO_NWM_COMMAND 9
#define THREAD_PRIO_NWM_WPA     10
#define THREAD_PRIO_RTC         12
#define THREAD_PRIO_SNDEX       14
#define THREAD_PRIO_FS          15
/* OS_THREAD_LAUNCHER_PRIORITY 16 */

/* ROM 内登録エリアの拡張言語コード */
#define ROMHEADER_FOR_CHINA_BIT     0x80
#define ROMHEADER_FOR_KOREA_BIT     0x40

/* 使用 DMA 番号 */
#define DMA_NO_FATFS        FATFS_DMA_4     // = 0
#define DMA_NO_NWM          3

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void         PrintDebugInfo(void);
static OSHeapHandle InitializeAllocateSystem(void);
static void         InitializeFatfs(void);
static void         InitializeNwm(OSHeapHandle hh);
static void         InitializeCdc(void);
static void         DummyThread(void* arg);

static void         ReadUserInfo(void);
#ifdef  NVRAM_CONFIG_DATA_EX_VERSION
static BOOL         IsValidConfigEx(void);
static u16          GetRomValidLanguage(void);
static s32          CheckCorrectNCDEx(NVRAMConfigEx* ncdsp);
#else
static s32          CheckCorrectNCD(NVRAMConfig* ncdsp);
#endif
static void         VBlankIntr(void);
static void AttachAES(void);

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

    /* OS 初期化 */
    OS_Init();
    PrintDebugInfo();

    /* NVRAM からユーザー情報読み出し */
    ReadUserInfo();

    /* ヒープ領域設定 */
    heapHandle  =   InitializeAllocateSystem();

    /* ボタン入力サーチ初期化 */
    (void)PAD_InitXYButton();

    /* 割込み許可 */
    (void)OS_SetIrqFunction(OS_IE_V_BLANK, VBlankIntr);
    (void)OS_EnableIrqMask(OS_IE_V_BLANK);
    (void)GX_VBlankIntr(TRUE);
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // 強制AESモード
    AttachAES();

    /* 各ライブラリ初期化 */
    AES_Init();                                 // AES
    SEA_Init();                                 // SEA
    FS_Init(FS_DMA_NOT_USE);                    // FS for CARD
    FS_CreateReadServerThread(THREAD_PRIO_FS);  // FS for CARD
    InitializeFatfs();                          // FAT-FS
//  InitializeNwm(heapHandle);                  // TWL 無線
    MCU_InitIrq(THREAD_PRIO_MCU);               // マイコン

    if (OSi_IsCodecTwlMode() == TRUE)
    {
        InitializeCdc();                        // CODEC
        CAMERA_Init();                          // カメラ
    }

    SND_Init(THREAD_PRIO_SND);                  // サウンド
    SNDEX_Init(THREAD_PRIO_SNDEX);              // サウンド拡張
    RTC_Init(THREAD_PRIO_RTC);                  // RTC
//  WVR_Begin(heapHandle);                      // NITRO 無線
    SPI_Init(THREAD_PRIO_SPI);

///////////////
#ifndef NAND_INITIALIZER_LIMITED_MODE
    KamiPxiInit();
#endif
///////////////

    while (TRUE)
    {
        OS_Halt();
    
        /* ソフトウェアリセット要求は監視しない */
        /* AGB カートリッジの挿抜チェックは行わない */
        /* DS カードの挿抜チェックは行わない */
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
    OS_TPrintf("ARM7: This component is running on TWL.\n");
    OS_TPrintf("ARM7: This component is \"armadillo.TWL\".\n");
}

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
    void*           subLo   =   (void*)OS_GetWramSubArenaLo();
    void*           subHi   =   (void*)OS_GetWramSubArenaHi();
    void*           privLo  =   (void*)OS_GetWramSubPrivArenaLo();
    void*           privHi  =   (void*)OS_GetWramSubPrivArenaHi();

    if (((u32)privLo == HW_PRV_WRAM) && ((u32)subHi == HW_PRV_WRAM) && ((u32)subLo < HW_PRV_WRAM))
    {
        /* SUB アリーナを SUBPRIV アリーナに吸収 */
        OS_SetWramSubArenaHi(subLo);
        OS_SetWramSubPrivArenaLo(subLo);
        privLo  =   subLo;
    }

    /* アリーナを 0 クリア */
    MI_CpuClear8(privLo, (u32)privHi - (u32)privLo);

    /* ヒープ作成初期化 */
    privLo  =   OS_InitAlloc(OS_ARENA_WRAM_SUBPRIV, privLo, privHi, 1);
    hh  =   OS_CreateHeap(OS_ARENA_WRAM_SUBPRIV, privLo, privHi);
    if (hh < 0)
    {
        OS_Panic("ARM7: Failed to create heap.\n");
    }

    /* カレントヒープに設定 */
    (void)OS_SetCurrentHeap(OS_ARENA_WRAM_SUBPRIV, hh);

    /* ヒープサイズの確認 */
    {
        u32     heapSize;
    
        heapSize    =   (u32)OS_CheckHeap(OS_ARENA_WRAM_SUBPRIV, hh);
    
        if (heapSize <= 0)
        {
            OS_Panic("ARM7: Failed to create heap.\n");
        }
        OS_TPrintf("ARM7: heap size [%d]\n", heapSize);
    }

    return hh;
}

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

    /* ダミースレッド作成 */
    OS_CreateThread(&thread, DummyThread, NULL,
        (void*)((u32)stack + (sizeof(u32) * 18)), sizeof(u32) * 18, OS_THREAD_PRIORITY_MAX);
    OS_WakeupThreadDirect(&thread);

    /* FATFS 初期化 */
    /* [TODO] DMA は NOT_USE のままで良い？ */
    if(!FATFS_Init(DMA_NO_FATFS, FATFS_DMA_NOT_USE, THREAD_PRIO_FATFS))
    {
        /* do nothing */
    }

    /* ダミースレッド破棄 */
    OS_KillThread(&thread, NULL);
}

/*---------------------------------------------------------------------------*
  Name:         InitializeNwm
  Description:  NWMライブラリを初期化する。
  Arguments:    hh  -   利用可能なヒープのハンドルを指定。
  Returns:      None.
 *---------------------------------------------------------------------------*/
static void
InitializeNwm(OSHeapHandle hh)
{
    NwmspInit   nwmInit;

    /* [TODO] 確保しているヒープ領域が新無線一式が必要としているメモリ量以上かのチェックが必要 */

    nwmInit.cmdPrio         =   THREAD_PRIO_NWM_COMMAND;
    nwmInit.evtPrio         =   THREAD_PRIO_NWM_EVENT;
    nwmInit.sdioPrio        =   THREAD_PRIO_NWM_SDIO;

    nwmInit.dmaNo           =   DMA_NO_NWM;
    nwmInit.drvHeap.id      =   OS_ARENA_WRAM_SUBPRIV;
    nwmInit.drvHeap.handle  =   hh;

#ifdef  WPA_BUILT_IN    // WPA が組み込まれる場合、以下のメンバが追加される
    nwmInit.wpaPrio         =   THREAD_PRIO_NWM_WPA;
    nwmInit.wpaHeap.id      =   OS_ARENA_WRAM_SUBPRIV;
    nwmInit.wpaHeap.handle  =   hh;
#endif

    NWMSP_Init(&nwmInit);
}

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

	// ランチャー経由で起動した場合はCODECは既に初期化されているため
	// コンポーネントがCODECを初期化する必要はありません。
	// 将来的にはバッサリと切る必要がありますが、
	// 暫定的にI2Sが有効かどうかでCODECが初期化済みかどうかを判定します。
	if (reg_SND_SMX_CNT & REG_SND_SMX_CNT_E_MASK)
	{
		return;
	}

    /* ダミースレッド作成 */
    OS_CreateThread(&thread, DummyThread, NULL,
        (void*)((u32)stack + (sizeof(u32) * 18)), sizeof(u32) * 18, OS_THREAD_PRIORITY_MAX);
    OS_WakeupThreadDirect(&thread);

    /* CODEC 初期化 */
    CDC_Init();
    CDC_InitMic();

    /* ダミースレッド破棄 */
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

//#ifdef  WM_PRECALC_ALLOWEDCHANNEL
//extern u16 WMSP_GetAllowedChannel(u16 bitField);
//#endif
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
/*
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
*/
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
extern void PM_SelfBlinkProc(void);

static void
VBlankIntr(void)
{
    if (PMi_Initialized)
    {
        PM_SelfBlinkProc();
    }
}

/*---------------------------------------------------------------------------*
  Name:         AttachAES

  Description:  強制的にAESマウントするようにします。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
extern BOOL sdmc_aes_attach_for_nand;   // 強制AESマウント用

static void AttachAES(void)
{
    // 強制AESマウント
    sdmc_aes_attach_for_nand = TRUE;
}

