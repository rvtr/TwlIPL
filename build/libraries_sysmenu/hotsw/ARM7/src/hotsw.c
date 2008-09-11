/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     hotsw.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#include    <twl.h>
#include    <twl/os/common/format_rom.h>
#include    <twl/mcu.h>
#include    <nitro/card/types.h>
#include    <firm/os/common/system.h>
#include    <sysmenu.h>
#include    <hotswTypes.h>
#include    <blowfish.h>
#include    <dsCardCommon.h>
#include    <romEmulation.h>
#include    <customNDma.h>
#include    <../build/libraries/mb/common/include/mb_fileinfo.h>

//#define HOTSW_DISABLE_FORCE_CARD_OFF

// カード電源ONからROMヘッダロードまでの期間にスリープに入る時のワンセグ対策しない場合
//#define HOWSW_ENABLE_DEEP_SLEEP_WHILE_INSERT_CARD

// define -------------------------------------------------------------------
#define     CHATTERING_COUNTER                  0x1988      // 100ms分 (0x1988 * 15.3us = 100000us)
//#define   CHATTERING_COUNTER                  0x264c      // 150ms分 (0x264c * 15.3us = 150001us)
//#define   CHATTERING_COUNTER                  0x3310      // 200ms分 (0x3310 * 15.3us = 200001us)
//#define   CHATTERING_COUNTER                  0x4c98      // 300ms分 (0x4c98 * 15.3us = 300002us)

#define     COUNTER_A                           0x264c      // 150ms分 (0x264c * 15.3us = 150001us)

#define     CARD_EXIST_CHECK_POLLING_TIME       100
#define     CARD_INSERT_CHECK_INTERVAL          5

#define     UNDEF_CODE                          0xe7ffdeff  // 未定義コード
#define     ENCRYPT_DEF_SIZE                    0x800       // 2KB  ※ ARM9常駐モジュール先頭2KB

#define     DIGEST_HASH_BLOCK_SIZE_SHA1         (512/8)

#define     SLOT_B_LOCK_BUF                     HW_CTRDG_LOCK_BUF

#define     HOTSW_DSTV_GAME_CODE                'JSNU'

#ifdef SDK_ARM9
#define     reg_HOTSW_EXMEMCNT                  reg_MI_EXMEMCNT
#define     HOTSW_EXMEMCNT_SELB_MASK            REG_MI_EXMEMCNT_MPB_MASK
#define     HOTSW_EXMEMCNT_SELB_SHIFT           REG_MI_EXMEMCNT_MPB_SHIFT
#else  // SDK_ARM7
#define     reg_HOTSW_EXMEMCNT                  reg_MI_EXMEMCNT_L
#define     HOTSW_EXMEMCNT_SELB_MASK            0x0400
#define     HOTSW_EXMEMCNT_SELB_SHIFT           10
#endif

// enum ---------------------------------------------------------------------
typedef enum HotSwCallBackType{
    HOTSW_CHANGE_GAMEMODE = 0,
    HOTSW_CARD_INSERT,
    HOTSW_CARD_PULLOUT
} HotSwCallBackType;

// Function prototype -------------------------------------------------------
static u32 GetMcSlotMode(void);
static void SetMcSlotMode(u32 mode);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCardDet(void);
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err);
#ifndef USE_NEW_DMA
static void InterruptCallbackCardData(void);
#endif

static void LockHotSwRsc(OSLockWord* word);
static void UnlockHotSwRsc(OSLockWord* word);

static void HotSwThread(void *arg);
static void MonitorThread(void *arg);

static void McPowerOn(void);
static void McPowerOff(void);
static void SetMCSCR(void);

static BOOL isTwlModeLoad(void);
static HotSwState ReadSecureModeCardData(void);
static void ClearCardFlgs(void);

static void FinalizeHotSw(HotSwCardState state);
static void ForceNitroModeToFinalize(void);
static void ForceNormalModeToFinalize(void);
static BOOL ChangeGameMode(void);
static void ClearCardIrq(void);
static void ClearAllCardRegister(void);

static HotSwState LoadCardData(void);
static void RegisterRomEmuInfo(void);
static void GenVA_VB_VD(void);

static HotSwState ReadImageReturnErrorCode(void* dest, s32 offset, s32 length, void* arg);
static BOOL ReadImage(void* dest, s32 offset, s32 length, void* arg);

static HotSwState LoadBannerData(void);
static HotSwState CheckCardAuthCode(void);

#ifndef USE_WRAM_LOAD
static HotSwState DecryptObjectFile(void);
static HotSwState LoadStaticModule(void);

static HotSwState CheckStaticModuleHash(void);
static BOOL CheckArm7HashValue(void);
static BOOL CheckArm9HashValue(void);
static BOOL CheckExtArm7HashValue(void);
static BOOL CheckExtArm9HashValue(void);
#else
static void ReadCardData(u32 src, u32 dest, u32 size);
static void SendPxiMessage(HotSwCallBackType type);
#endif

static void DebugPrintErrorMessage(HotSwState state);

HotSwState HOTSWi_RefreshBadBlock(u32 romMode);

static void CheckCardInsert(BOOL cardExist);
static void CheckCardPullOut(BOOL cardExist);
static void SendInsertMessage(void);

static void PulledOutSequence(void);

// Static Values ------------------------------------------------------------
#include <twl/ltdwram_begin.h>

static char                 encrypt_object_key[] ATTRIBUTE_ALIGN(4) = "encryObj";

static u16                  s_RscLockID;
static u16                  s_CardLockID;
static u16					s_PollingLockID;
static u16                  s_bondingOp;

static u32                  s_BootSegBufSize, s_SecureSegBufSize, s_Secure2SegBufSize;

static u32					s_gameID;

static BootSegmentData      *s_pBootSegBuffer;
static u32                  *s_pSecureSegBuffer;
static u32                  *s_pSecure2SegBuffer;

static CardBootData         s_cbData ATTRIBUTE_ALIGN(4);
static SYSMRomEmuInfo       s_romEmuInfo;
static BOOL                 s_isRomEmu;
static BOOL                 s_debuggerFlg;

static BOOL                 s_isPulledOut = TRUE;
static BOOL                 s_pollingThreadSleepFlg = FALSE;

static volatile BOOL		s_isBusyMonitorThread;

// HMACSHA1の鍵
static u8 s_digestDefaultKey[ DIGEST_HASH_BLOCK_SIZE_SHA1 ] = {
    0x21, 0x06, 0xc0, 0xde,
    0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d,
    0x46, 0xf2, 0xed, 0x01,

    0x76, 0xe3, 0xcc, 0x08,
    0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf,
    0x9a, 0x62, 0x78, 0x34,

    0x8f, 0x6d, 0x63, 0x3c,
    0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23,
    0xd2, 0xcf, 0xae, 0xc2,

    0x32, 0x67, 0x8d, 0xfe,
    0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37,
    0x87, 0x46, 0x58, 0x24
};

static CardSecureModeFunction s_funcTable[] = {
    // Game Card
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
    // RomEmulation
    { ReadIDSecure_ROMEMU, ReadSegSecure_ROMEMU, SwitchONPNGSecure_ROMEMU, ChangeModeSecure_ROMEMU }
};

// Global Values ------------------------------------------------------------
BLOWFISH_CTX                HotSwBlowfishInitTableBufDS;
CardThreadData              HotSwThreadData;

#include <twl/ltdwram_end.h>

// ===========================================================================
//  Function Describe
// ===========================================================================

/*---------------------------------------------------------------------------*
  Name:         HOTSW_Init

  Description:
 *---------------------------------------------------------------------------*/
void HOTSW_Init(u32 threadPrio)
{
    SYSM_work* sw = SYSMi_GetWork();

    OS_InitTick();
    OS_InitThread();

#ifndef INITIAL_KEYTABLE_PRELOAD
    // 初期化後に他の用途でWRAM_0を使用できるようにローカルバッファへコピーしておく
    MI_CpuCopyFast((void *)HW_WRAM_0_LTD, &HotSwBlowfishInitTableBufDS, sizeof(BLOWFISH_CTX));
#endif

    // PXI初期化
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_HOTSW, InterruptCallbackPxi);

#ifdef USE_WRAM_LOAD
    while(!PXI_IsCallbackReady(PXI_FIFO_TAG_HOTSW, PXI_PROC_ARM9))
    {
        // do nothing
    }
#endif

    // 割り込みマスクの設定
    SetInterrupt();

    // 割り込みの有効化
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

    // チャッタリングカウンタの値を設定
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~REG_MI_MC1_CC_MASK) |
                       (CHATTERING_COUNTER << REG_MI_MC1_CC_SHIFT));

    // Counter-Aの値を設定
    reg_MI_MC2 = COUNTER_A;

    // Bonding Optionの取得
    s_bondingOp = SCFG_REG_GetBondingOption();

    // 構造体の初期化
    MI_CpuClear8(&s_cbData, sizeof(CardBootData));

    // カードスレッド用構造体の初期化
    MI_CpuClear8(&HotSwThreadData, sizeof(CardThreadData));

    // HotSwリソースの排他制御用Lock IDの取得(開放しないで持ち続ける)
    {
        s32 tempLockID;
        // ARM9と排他制御用のロックIDの取得
        while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
            // do nothing
        }
        s_RscLockID = (u16)tempLockID;

        // カードアクセス用のロックIDの取得
        while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
            // do nothing
        }
        s_CardLockID = (u16)tempLockID;

        // [Debug]
        // ポーリングスレッド用のロックIDの取得
        while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
            // do nothing
        }
        s_PollingLockID = (u16)tempLockID;
    }

    // カードの状態監視用スレッドの生成 ( DSテレビ対策 )
    OS_CreateThread(&HotSwThreadData.monitorThread,
                    MonitorThread,
                    NULL,
                    HotSwThreadData.monitorStack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );

    // カードデータロード用スレッドの生成 ※カード状態監視スレッドと優先度同じだけど、後に作成したこっちのスレッドが優先される
    OS_CreateThread(&HotSwThreadData.hotswThread,
                    HotSwThread,
                    NULL,
                    HotSwThreadData.hotswStack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );

    // メッセージキューの初期化
    OS_InitMessageQueue( &HotSwThreadData.hotswQueue,            &HotSwThreadData.hotswMsgBuffer[0],         HOTSW_MSG_BUFFER_NUM );
    OS_InitMessageQueue( &HotSwThreadData.hotswDmaQueue,         &HotSwThreadData.hotswDmaMsgBuffer[0],      HOTSW_DMA_MSG_NUM );
    OS_InitMessageQueue( &HotSwThreadData.hotswPollingCtrlQueue, &HotSwThreadData.hotswPollingCtrlBuffer[0], HOTSW_POLLING_CTRL_BUFFER_NUM );

    // スレッド起動
    OS_WakeupThreadDirect(&HotSwThreadData.hotswThread);
    OS_WakeupThreadDirect(&HotSwThreadData.monitorThread);

    // バッファの設定
    HOTSW_SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_APP_ROM_HEADER_SIZE );
    HOTSW_SetSecureSegmentBuffer(HOTSW_MODE1, (void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );
    HOTSW_SetSecureSegmentBuffer(HOTSW_MODE2, (void *)SYSM_CARD_TWL_SECURE_BUF, SECURE_AREA_SIZE );

    // カードが挿さってあったらスレッドを起動する
    if(HOTSW_IsCardExist()){
        // メッセージ送信
        OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

        // メッセージインデックスをインクリメント
        HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;
    }
    else{
        sw->flags.hotsw.is1stCardChecked  = TRUE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_CopyInitialKeyTable

  Description:  DS互換BlowfishテーブルをWRAM経由でローカルにコピーする

  ※この関数はHOTSW_Initが呼ばれる前に呼んで下さい。
 *---------------------------------------------------------------------------*/
#ifdef INITIAL_KEYTABLE_PRELOAD
void HOTSW_CopyInitialKeyTable(void)
{
    // ARM9でKey Tableの用意が出来るまでポーリング
    while( !SYSMi_GetWork()->flags.hotsw.isKeyTableLoadReady ){
        OS_SpinWait( 0x400 );
    }

    // 初期化後に他の用途でWRAM_0を使用できるようにローカルバッファへコピーしておく
    MI_CpuCopyFast((void *)HW_WRAM_0_LTD, &HotSwBlowfishInitTableBufDS, sizeof(BLOWFISH_CTX));
}
#endif


/*---------------------------------------------------------------------------*
  Name:         LoadCardData

  Description:  カードからデータをロードする

  ※BootSegmentBuffer SecureSegmentBufferの設定を行ってから
  この関数を呼んでください。
 *---------------------------------------------------------------------------*/
static HotSwState LoadCardData(void)
{
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;
    u32 romMode = HOTSW_ROM_MODE_NULL;
    
    // カードのロック
    CARD_LockRom(s_CardLockID);

    // カード電源リセット
    McPowerOff();
#ifndef HOWSW_ENABLE_DEEP_SLEEP_WHILE_INSERT_CARD
    MCU_EnableDeepSleepToPowerLine( MCU_PWR_LINE_33, FALSE );
#endif
    McPowerOn();

    // バッファを設定
    s_cbData.pBootSegBuf   = s_pBootSegBuffer;
    s_cbData.pSecureSegBuf = s_pSecureSegBuffer;
    s_cbData.pSecure2SegBuf= s_pSecure2SegBuffer;

    // ロード処理開始
    if(HOTSW_IsCardAccessible()){
        s_cbData.modeType = HOTSW_MODE1;

        // カード側でKey Tableをロードする
        state  = LoadTable();
        retval = (retval == HOTSW_SUCCESS) ? state : retval;

        // ---------------------- Normal Mode ----------------------
        romMode = HOTSW_ROM_MODE_NORMAL;

        // カードID読み込み
        state  = ReadIDNormal(&s_cbData);
        retval = (retval == HOTSW_SUCCESS) ? state : retval;

        // カードタイプを判別をして、使う関数を切替える IDの最上位ビットが1なら3DM
        s_cbData.cardType = (s_cbData.id_nml & HOTSW_ROMID_1TROM_MASK) ? DS_CARD_TYPE_2 : DS_CARD_TYPE_1;

        {
            SYSMRomEmuInfo *romEmuInfo = (void *)&s_romEmuInfo;
            BOOL enableDeepSleep = TRUE;

            // バナーリードが完了して、フラグ処理が終わるまでARM9と排他制御する
            LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            // Boot Segment読み込み
            state  = ReadBootSegNormal(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ワンセグ差込み時はカードバス電源をディープスリープ（PFM）に入れない
            // （スリープ時シャットダウン対策。カードロード中にスリープしても良いように早目に設定）
            if ( *(u32*)(s_cbData.pBootSegBuf->rh.s.game_code) == HOTSW_DSTV_GAME_CODE )
            {
                enableDeepSleep = FALSE;
            }
            MCU_EnableDeepSleepToPowerLine( MCU_PWR_LINE_33, enableDeepSleep );

            // ARM9/7で不整合が発生しないようにRomエミュレーション情報ロードは初回のみ
            if ( ! SYSMi_GetWork()->flags.hotsw.is1stCardChecked )
            {
                // Romエミュレーション情報を取得
                //（製品本体なら内部で構造体をクリアするだけ）
                state  = ReadRomEmulationInfo(&s_romEmuInfo);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // 取得したRomエミュレーション情報を比較
                s_debuggerFlg = TRUE;
                if ( romEmuInfo->magic_code != SYSM_ROMEMU_INFO_MAGIC_CODE ){
                    s_debuggerFlg = FALSE;
                }
            }
            // ※ROMエミュレーション情報を読まなければ、デバッガ上でカードアクセスができなくなるため、ランチャー自身をデバッグできなくなる。
            //   よって、ROMエミュレーション情報リード部分をデバッガビルド時以外に切ることはできない。

            // 初回のRomエミュレーション情報を使用
            s_isRomEmu = FALSE;
            if(HOTSWi_IsRomEmulation()){
                s_isRomEmu = TRUE;
                HOTSW_PutString("Read Emulation ROM\n");
                s_cbData.cardType = ROM_EMULATION;
                s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
                HOTSW_TPrintf("SYSMi_GetWork()->gameCommondParam : 0x%08x\n", s_cbData.gameCommondParam);
            }
            else{
                s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param;
            }
            SYSMi_GetWork()->gameCommondParam = s_cbData.gameCommondParam;

            // CRCチェック
            if( !UTL_CheckAppCRC16( &s_cbData.pBootSegBuf->rh.s ) ) {
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_CRC_CHECK_ERROR : retval;
            }

            if(s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL){
                s_cbData.twlFlg = TRUE;
            }

            // アプリジャンプのデバッグ時にROMエミュレーション情報だけ必要な場合
            if(SYSMi_GetWork()->flags.hotsw.isLoadRomEmuOnly){
                SYSMi_GetWork()->flags.hotsw.isExistCard = TRUE;
                RegisterRomEmuInfo();
                // 排他制御ここまで
                UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
                goto finalize;
            }
        }

        if( retval == HOTSW_SUCCESS ) {
            if(!s_cbData.twlFlg){
                if ( !s_cbData.pBootSegBuf->rh.s.exFlags.enable_nitro_whitelist_signature )
                // NTRカードの場合はRomHeaderバッファの1ページ目以降をクリアしておく。
                MI_CpuClearFast((void *)(SYSM_CARD_ROM_HEADER_BAK + PAGE_SIZE), SYSM_APP_ROM_HEADER_SIZE - PAGE_SIZE);
            }

            // SecureコマンドのPNG_ONコマンドetc用のレイテンシを求める(Latency1とLatency2を足す)
            s_cbData.secureLatency = AddLatency2ToLatency1(s_cbData.pBootSegBuf->rh.s.secure_cmd_param);

            // Key Table初期化
            MakeBlowfishTableDS(&s_cbData, 8);

            // コマンド認証値・コマンドカウンタ初期値・PNジェネレータ初期値の生成
            GenVA_VB_VD();

            // セキュアモードに移行
            state  = ChangeModeNormal(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ---------------------- Secure Mode ----------------------
            romMode = HOTSW_ROM_MODE_SECURE;

            // SecureモードのIDとSecureSegmentを読み込む
            state  = ReadSecureModeCardData();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ★TWLカード対応 一旦リセット後Secure2モードに移行
            // SCFG
            if((s_cbData.isLoadTypeTwl = isTwlModeLoad()) == TRUE){
                HOTSW_PutString("Read Mode : TwlCard\n");

               // Mode2に移行する準備
                s_cbData.modeType = HOTSW_MODE2;

                // ---------------------- Reset ----------------------
                McPowerOff();
                McPowerOn();

                // ---------------------- Normal Mode ----------------------
                // 先頭1Page分だけでOK。データは読み捨てバッファに
                state  = ReadBootSegNormal(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // Key Table初期化
                MakeBlowfishTableTWL(&s_cbData, 8, s_bondingOp);

                // コマンド認証値・コマンドカウンタ初期値・PNジェネレータ初期値の生成
                GenVA_VB_VD();

                // セキュア２モードに移行
                state  = ChangeModeNormal2(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // ---------------------- Secure2 Mode ----------------------
                // Secure2モードのIDとSecureSegmentを読み込む
                state  = ReadSecureModeCardData();
                retval = (retval == HOTSW_SUCCESS) ? state : retval;
            }

            // ゲームモードに移行
            state  = s_funcTable[s_isRomEmu].ChangeMode_S(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ---------------------- Game Mode ----------------------
            romMode = HOTSW_ROM_MODE_GAME;

            if(retval != HOTSW_SUCCESS || s_cbData.illegalCardFlg){
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_ILLEGAL_CARD_ERROR : retval;

                // 排他制御ここまで
                UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                goto finalize;
            }

            // ID読み込み
            state  = ReadIDGame(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // カードIDの比較をして、一致しなければFALSEを返す
            {
                u32 secure_ID = (s_cbData.modeType == HOTSW_MODE1) ? s_cbData.id_scr : s_cbData.id_scr2;
                if(secure_ID != s_cbData.id_gam){
                    retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;

                    // 排他制御ここまで
                    UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                    goto finalize;
                }
            }

            // バナーファイルの読み込み
            state  = LoadBannerData();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // 排他制御ここまで(※CRCチェックまでにミスがなかったら、排他制御ここまで)
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            // 認証コード読み込み＆ワーク領域にコピー
            state  = CheckCardAuthCode();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

#ifndef USE_WRAM_LOAD
            // 常駐モジュール残りを指定先に転送
            state  = LoadStaticModule();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ARM9常駐モジュールの先頭2KBの暗号化領域を複合化
            (void)DecryptObjectFile();
#endif
        }
        else{
            // 排他制御ここまで(※CRCチェックまでにミスがあったら、ここで開放する)
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            HOTSW_TPrintf("ng... Card Data Load Skip ( state : %d )\n", retval);
        }
    }
    else{
        state = HOTSW_PULLED_OUT_ERROR;
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
    }

finalize:
    if( retval == HOTSW_SUCCESS )
    {
        // バッドブロックを置換
        state = HOTSWi_RefreshBadBlock(romMode);
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
    }

end:
    // カードDMA終了確認
#ifdef USE_NEW_DMA
    HOTSW_WaitNDmaCtrl(HOTSW_NDMA_NO);
#else
    HOTSW_WaitDmaCtrl(HOTSW_DMA_NO);
#endif

    // カードアクセス終了確認
    HOTSW_WaitCardCtrl();

    // カードのロック開放(※ロックIDは開放せずに持ち続ける)
    CARD_UnlockRom(s_CardLockID);
    
    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         isTwlModeLoad

  Description:  カードのロードをTWLモードで行うかDSモードで行うかを決める

  ※ 読み込みシーケンスの詳細は docs/ゲームカード/カード読み込みシーケンス表.xls 参照
 *---------------------------------------------------------------------------*/
static BOOL isTwlModeLoad(void)
{
    // TWLカード
    if(s_cbData.id_nml & HOTSW_ROMID_TWLROM_MASK){
        // NANDアプリの場合
        if(s_cbData.pBootSegBuf->rh.s.titleID_Hi & TITLE_ID_HI_MEDIA_MASK){
            return FALSE;
        }
        else{
            // PlatformCodeがTwl or Hybridの場合
            if(s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL){
                return TRUE;
            }
            else{
                return FALSE;
            }
        }
    }
    // DSカード
    else{
        // PlatformCodeがTwl or Hybridの場合
        if(s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL){
            // 製品用本体 (最終的には開発用本体も)
#ifdef HOTSW_FINAL_VERSION
            if(s_bondingOp == SCFG_OP_PRODUCT || !s_debuggerFlg)
#else
            if(s_bondingOp == SCFG_OP_PRODUCT)
#endif
            {
                s_cbData.illegalCardFlg = TRUE;
            }
        }

        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ReadSecureModeCardData

  Description:  セキュアモードの各種読み込みを行う関数

  ※ s_cbData.modeTypeの設定はこの関数の外で行う
 *---------------------------------------------------------------------------*/
static HotSwState ReadSecureModeCardData(void)
{
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;

    u32 secure_ID;

    // PNG設定
    state  = s_funcTable[s_isRomEmu].SetPNG_S(&s_cbData);
    retval = (retval == HOTSW_SUCCESS) ? state : retval;

    // DS側符号生成回路初期値設定 (レジスタ設定)
    SetMCSCR();

    // ID読み込み
    state  = s_funcTable[s_isRomEmu].ReadID_S(&s_cbData);
    retval = (retval == HOTSW_SUCCESS) ? state : retval;

    // カードIDの比較をして、一致しなければFALSEを返す
    secure_ID = (s_cbData.modeType == HOTSW_MODE1) ? s_cbData.id_scr : s_cbData.id_scr2;
    if(s_cbData.id_nml != secure_ID){
        retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;
    }

    if(retval == HOTSW_SUCCESS){
        // Secure領域のSegment読み込み
        state  = s_funcTable[s_isRomEmu].ReadSegment_S(&s_cbData);
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_RefreshBadBlock

  Description:  ノーマルモードまたはゲームモードでバッドブロックを置換
 *---------------------------------------------------------------------------*/
HotSwState HOTSWi_RefreshBadBlock(u32 romMode)
{
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;

    HotSwState (*pReadStatus)(CardBootData *cbd);
    HotSwState (*pRefreshBadBlock)(CardBootData *cbd);

    if ( ! ( romMode == HOTSW_ROM_MODE_NORMAL || romMode == HOTSW_ROM_MODE_GAME ) )
    {
        return HOTSW_SUCCESS;
    }

    if ( romMode == HOTSW_ROM_MODE_NORMAL )
    {
        pReadStatus = ReadStatusNormal;
        pRefreshBadBlock = RefreshBadBlockNormal;
    }
    else
    if ( romMode == HOTSW_ROM_MODE_GAME )
    {
        pReadStatus = ReadStatusGame;
        pRefreshBadBlock = RefreshBadBlockGame;
    }

    // ステータス対応ROMのみステータス読み込み
    if ( s_cbData.id_nml & HOTSW_ROMID_RFSSUP_MASK )
    {
        state = pReadStatus(&s_cbData);
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
        // 要求レベルに関わらずバッドブロックを置換（製品カードでは滅多に発生しない）
        if ( s_cbData.romStatus & (HOTSW_ROMST_RFS_WARN_L1_MASK | HOTSW_ROMST_RFS_WARN_L2_MASK) )
        {
            state = pRefreshBadBlock(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;
            while (1)
            {
                state = pReadStatus(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;
                if ( s_cbData.romStatus & HOTSW_ROMST_RFS_READY_MASK ||
                     (retval != HOTSW_SUCCESS) )
                {
                    break;
                }
                OS_Sleep(1);
            }
        }
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_IsRomEmulation

  Description:  ROMをエミュレーションしているか？
 *---------------------------------------------------------------------------*/
BOOL HOTSWi_IsRomEmulation(void)
{
    return s_debuggerFlg && s_romEmuInfo.isEnableSlot1;
}

/*---------------------------------------------------------------------------*
  Name:         RegisterRomEmuInfo

  Description:  ROMエミュレーション情報を登録
 *---------------------------------------------------------------------------*/
static void RegisterRomEmuInfo(void)
{
    SYSM_work* sw = SYSMi_GetWork();

    // デバッガ情報
    if ( ! sw->flags.hotsw.is1stCardChecked && s_debuggerFlg )
    {
        MI_CpuCopy8( &s_romEmuInfo, &(sw->romEmuInfo), ROM_EMULATION_DATA_SIZE );
        sw->flags.hotsw.isOnDebugger = s_debuggerFlg;
    }

    sw->flags.hotsw.isCardStateChanged = TRUE;
    sw->flags.hotsw.is1stCardChecked   = TRUE;
}


/*---------------------------------------------------------------------------*
  Name:         LoadBannerData

  Description:  バナーデータを読み込む

  注：ゲームモードになってから呼び出してください
 *---------------------------------------------------------------------------*/
static HotSwState LoadBannerData(void)
{
    BOOL state;
    HotSwState retval = HOTSW_SUCCESS;
    SYSM_work* sw = SYSMi_GetWork();

    // バナーリード
    if( s_cbData.pBootSegBuf->rh.s.banner_offset ) {
        retval  = ReadPageGame(&s_cbData,   s_cbData.pBootSegBuf->rh.s.banner_offset,
                                            (u32 *)SYSM_CARD_BANNER_BUF,
                                            sizeof(TWLBannerFile) );

        // バナーリードが成功していたら各種フラグTRUE その他の場合はFALSE (この関数の外で排他制御されているからここでは排他制御しないでOK)
        state = (retval == HOTSW_SUCCESS) ? TRUE : FALSE;
        sw->flags.hotsw.isValidCardBanner  = state;
        sw->flags.hotsw.isExistCard        = state;
    }
    else{
        // バナーデータが登録されていない場合 (この関数の外で排他制御されているからここでは排他制御しないでOK)
        sw->flags.hotsw.isValidCardBanner  = FALSE;
        sw->flags.hotsw.isExistCard        = TRUE;
    }

    if ( sw->flags.hotsw.isExistCard )
    {
        sw->flags.hotsw.isInspectCard = s_cbData.pBootSegBuf->rh.s.inspect_card;
    }
    else
    {
        sw->flags.hotsw.isInspectCard = FALSE;
    }

    // デバッガ情報を登録
    RegisterRomEmuInfo();

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         ReadImageReturnErrorCode

  Description:  カードから中途半端なサイズ(page途中)のデータを読み出す関数
                エラーコードを返す
 *---------------------------------------------------------------------------*/
static HotSwState ReadImageReturnErrorCode(void* dest, s32 offset, s32 length, void* arg)
{
    HotSwState retval = HOTSW_SUCCESS;

    static u8 page_buffer[512];
    u32 page_offset = (u32)(offset & -512);
    u32 buffer_offset = (u32)(offset % 512);
    u32 valid_length = 512 - buffer_offset;
    u32 remain_length;

    // 開始アドレスがページの途中
    if ( offset % 512 )
    {
        retval = ReadPageGame((CardBootData*)arg, page_offset, page_buffer, 512);

        if (retval != HOTSW_SUCCESS)
        {
            return retval;
        }

        MI_CpuCopy8(page_buffer + buffer_offset, dest, (length < valid_length ? length : valid_length));

        dest = (u8*)dest + valid_length;
        offset += valid_length;
        length -= valid_length;
        if ( length < 0)
        {
            return retval;
        }
    }

    remain_length = (u32)(length % 512);
    retval = ReadPageGame((CardBootData*)arg, (u32)offset, dest, (u32)(length - remain_length));

    if (retval != HOTSW_SUCCESS)
    {
        return retval;
    }

    // ケツがページ途中
    if( remain_length ){
        dest   = (u8*)dest + (length - remain_length);
        offset += length - remain_length;

        retval = ReadPageGame((CardBootData*)arg, (u32)offset, page_buffer, 512);

        if (retval != HOTSW_SUCCESS)
        {
            return retval;
        }

        MI_CpuCopy8(page_buffer, dest, remain_length);
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         ReadImage

  Description:  カードから中途半端なサイズ(page途中)のデータを読み出す関数
                成功するとTRUEを返す
 *---------------------------------------------------------------------------*/
static BOOL ReadImage(void* dest, s32 offset, s32 length, void* arg)
{
    return (ReadImageReturnErrorCode(dest, offset, length, arg) == HOTSW_SUCCESS);
}


/*---------------------------------------------------------------------------*
  Name:         ReadCardData

  Description:  ARM9から通知された範囲のデータをカードから読み込み
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
static void ReadCardData(u32 src, u32 dest, u32 size)
{
    u32 sendSize = 0;
    HotSwState state = HOTSW_SUCCESS;

    if(!SYSMi_GetWork()->flags.hotsw.isCardGameMode){
        state = HOTSW_MODE_ERROR;
    }

    if(SYSMi_GetWork()->flags.hotsw.isCardStateChanged){
        state = HOTSW_PULLED_OUT_ERROR;
    }
    
    // カードのロック
    CARD_LockRom(s_CardLockID);

    // [Debug]
    ReadIDGame(&s_cbData);
    if(s_cbData.id_gam != s_gameID){
		state = HOTSW_GAMEMODE_ID_CHECK_ERROR;
    }

	// [Debug]
    if(!(reg_MI_EXMEMCNT_L & REG_MI_EXMEMCNT_L_MP_MASK)){
		state = HOTSW_BUS_LOCK_ERROR;
    }
    
    while(size > 0 && state == HOTSW_SUCCESS){
        // --- Boot Segment
        if(src >= HOTSW_BOOTSEGMENT_AREA_OFS && src < HOTSW_KEYTABLE_AREA_OFS){
            sendSize = ((src + size) > HOTSW_KEYTABLE_AREA_OFS) ? HOTSW_KEYTABLE_AREA_OFS - src : size;
            MI_CpuCopy8((u32 *)(SYSM_CARD_ROM_HEADER_BAK + (src - HOTSW_BOOTSEGMENT_AREA_OFS)), (u32 *)dest, sendSize);
        }

        // --- Key Table
        else if(src >= HOTSW_KEYTABLE_AREA_OFS && src < HOTSW_SECURE_AREA_OFS){
            sendSize = ((src + size) > HOTSW_SECURE_AREA_OFS) ? HOTSW_SECURE_AREA_OFS - src : size;
            state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
        }
        // --- Secure Segment
        else if(src >= HOTSW_SECURE_AREA_OFS && src < HOTSW_GAME_AREA_OFS){
            sendSize = ((src + size) > HOTSW_GAME_AREA_OFS) ? HOTSW_GAME_AREA_OFS - src : size;
            MI_CpuCopy8((u32 *)((u32)s_cbData.pSecureSegBuf + (src - HOTSW_SECURE_AREA_OFS)), (u32 *)dest, sendSize);
        }
        // --- Game Segment
        else if(src >= HOTSW_GAME_AREA_OFS){
            // KeyTable2が設定されている場合
            if(s_cbData.isLoadTypeTwl){
                u32 keyTable2Adr = (u32)s_cbData.pBootSegBuf->rh.s.twl_card_keytable_area_rom_offset * TWLCARD_BORDER_OFFSET;
                u32 Secure2Adr   = keyTable2Adr + HOTSW_SECURE2_AREA_OFS;
                u32 Game2Adr     = keyTable2Adr + HOTSW_GAME2_AREA_OFS;

                // --- Game Segment
                if(src < keyTable2Adr){
                    sendSize = ((src + size) > keyTable2Adr) ? keyTable2Adr - src : size;
                    state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
                }
                // --- Key Table2
                else if(src >= keyTable2Adr && src < Secure2Adr){
                    sendSize = ((src + size) > Secure2Adr) ? Secure2Adr - src : size;
                    state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
                }
                // --- Secure2 Segment
                else if(src >= Secure2Adr && src < Game2Adr){
                    sendSize = ((src + size) > Game2Adr) ? Game2Adr - src : size;
                    MI_CpuCopy8((u32 *)((u32)s_cbData.pSecure2SegBuf + (src - Secure2Adr)), (u32 *)dest, sendSize);
                }
                // --- Game2 Segment
                else{
                    sendSize = size;
                    state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
                }
            }
            // --- Game Segment
            else{
                sendSize = size;
                state = ReadImageReturnErrorCode((u32 *)dest, (s32)src, (s32)sendSize, &s_cbData);
            }
        }

        size -= sendSize;
        src  += sendSize;
        dest += sendSize;
    }

    // [Debug]
    ReadIDGame(&s_cbData);
    if(s_cbData.id_gam != s_gameID){
        if(state == HOTSW_SUCCESS){
        	state = HOTSW_GAMEMODE_ID_CHECK_ERROR;
        }
    }
    
    // カードのアンロック
    CARD_UnlockRom(s_CardLockID);
    
    {
        HotSwPxiMessageForArm9  msg;
        CardDataReadState       retval;

        switch(state){
          case HOTSW_SUCCESS:
            retval = CARD_READ_SUCCESS;
            break;

          case HOTSW_TIME_OUT:
            retval = CARD_READ_TIME_OUT;
            break;

          case HOTSW_PULLED_OUT_ERROR:
            retval = CARD_READ_PULLED_OUT_ERROR;
            break;

          case HOTSW_BUFFER_OVERRUN_ERROR:
            retval = CARD_READ_BUFFER_OVERRUN_ERROR;
            break;

          case HOTSW_MODE_ERROR:
            retval = CARD_READ_MODE_ERROR;
            break;

          case HOTSW_GAMEMODE_ID_CHECK_ERROR:
            retval = CARD_READ_ID_CHECK_ERROR;
            break;

          case HOTSW_BUS_LOCK_ERROR:
            retval = CARD_READ_BUS_LOCK_ERROR;
            break;
            
          default:
            retval = CARD_READ_UNEXPECTED_ERROR;
            break;
        }

        MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm9));

        msg.msg.read   = TRUE;
        msg.msg.result = (u8)retval;

        while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
        {
            // do nothing
        }
    }
}
#endif


/*---------------------------------------------------------------------------*
  Name:         LoadStaticModule

  Description:  ARM7,9の常駐モジュールを展開する関数

  注：ゲームモードになってから呼び出してください
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static HotSwState LoadStaticModule(void)
{
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;
    u32 arm9StcEnd    = s_cbData.pBootSegBuf->rh.s.main_rom_offset     + s_cbData.pBootSegBuf->rh.s.main_size;
    u32 arm9LtdStcEnd = s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset + s_cbData.pBootSegBuf->rh.s.main_ltd_size;
    u32 secure2SegEnd = (u32)(s_cbData.pBootSegBuf->rh.s.twl_card_keytable_area_rom_offset * TWLCARD_BORDER_OFFSET + SECURE_SEGMENT_SIZE);

    // 配置先と再配置情報を取得 & Arm9の常駐モジュール残りを指定先に転送
    s_cbData.arm9Stc = (u32)s_cbData.pBootSegBuf->rh.s.main_ram_address;
    if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_STATIC, &s_cbData.arm9Stc, s_cbData.pBootSegBuf->rh.s.main_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC] , s_cbData.twlFlg)){
        if(arm9StcEnd > SECURE_SEGMENT_END){
            state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.main_rom_offset + SECURE_SEGMENT_SIZE, (u32 *)(s_cbData.arm9Stc + SECURE_SEGMENT_SIZE), MATH_ROUNDUP( arm9StcEnd, SYSM_ALIGNMENT_LOAD_MODULE ) - SECURE_SEGMENT_END);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;
        }
    }
    else{
        retval = (retval == HOTSW_SUCCESS) ? HOTSW_BUFFER_OVERRUN_ERROR : retval;
    }
    if(retval != HOTSW_SUCCESS){
        return retval;
    }

    // 配置先と再配置情報を取得 & Arm7の常駐モジュールを指定先に転送
    s_cbData.arm7Stc = (u32)s_cbData.pBootSegBuf->rh.s.sub_ram_address;
    if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_STATIC, &s_cbData.arm7Stc, s_cbData.pBootSegBuf->rh.s.sub_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_STATIC], s_cbData.twlFlg)){
        state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.sub_rom_offset, (u32 *)s_cbData.arm7Stc, MATH_ROUNDUP( s_cbData.pBootSegBuf->rh.s.sub_size, SYSM_ALIGNMENT_LOAD_MODULE ) );
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
    }
    else{
        retval = (retval == HOTSW_SUCCESS) ? HOTSW_BUFFER_OVERRUN_ERROR : retval;
    }
    if(retval != HOTSW_SUCCESS){
        return retval;
    }

    // 拡張常駐モジュールの読み込み
    if( s_cbData.twlFlg ) {
        u32 size = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ? s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;
        s_cbData.arm9Ltd = (u32)s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address;
        // 配置先と再配置情報を取得 & Arm9の常駐モジュールを指定先に転送
        if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_LTD_STATIC, &s_cbData.arm9Ltd, s_cbData.pBootSegBuf->rh.s.main_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC] , TRUE)){
            if(!s_cbData.isLoadTypeTwl){
                // Secure2領域読み
                state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset, (u32 *)SYSM_CARD_TWL_SECURE_BUF, MATH_ROUNDUP( size, SYSM_ALIGNMENT_LOAD_MODULE ));
                retval = (retval == HOTSW_SUCCESS) ? state : retval;
            }

            // (Arm9Ltd領域 - Secure2領域)分の読み込み
            if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ) {
                state  = ReadPageGame(&s_cbData,        s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset + SECURE_SEGMENT_SIZE,
                                                (u32 *)(s_cbData.arm9Ltd                               + SECURE_SEGMENT_SIZE),
                                                        MATH_ROUNDUP( s_cbData.pBootSegBuf->rh.s.main_ltd_size, SYSM_ALIGNMENT_LOAD_MODULE )       - size);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;
            }
        }
        else{
            retval = (retval == HOTSW_SUCCESS) ? HOTSW_BUFFER_OVERRUN_ERROR : retval;
        }

        if(retval != HOTSW_SUCCESS){
            return retval;
        }

        // 配置先と再配置情報を取得 & Arm7の常駐モジュールを指定先に転送
        s_cbData.arm7Ltd = (u32)s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address;
        if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_LTD_STATIC, &s_cbData.arm7Ltd, s_cbData.pBootSegBuf->rh.s.sub_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_LTD_STATIC], TRUE)){
            state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset, (u32 *)s_cbData.arm7Ltd, MATH_ROUNDUP( s_cbData.pBootSegBuf->rh.s.sub_ltd_size, SYSM_ALIGNMENT_LOAD_MODULE ) );
            retval = (retval == HOTSW_SUCCESS) ? state : retval;
        }
        else{
            retval = (retval == HOTSW_SUCCESS) ? HOTSW_BUFFER_OVERRUN_ERROR : retval;
        }
        if(retval != HOTSW_SUCCESS){
            return retval;
        }

        // セキュア領域先頭2K分のハッシュ値を求めて、Work領域にコピー
        {
            SVCHMACSHA1Context hash;

            // ハッシュ初期化
            SVC_HMACSHA1Init( &hash, s_digestDefaultKey, sizeof(s_digestDefaultKey) );

            // セキュア領域先頭2kb分UpDate
            SVC_HMACSHA1Update( &hash, s_cbData.pSecureSegBuf, ENCRYPT_DEF_SIZE );

            // Arm9と排他制御する
            LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);

            // ハッシュコンテキストをWork領域にコピー
            MI_CpuCopy8( &hash, &SYSMi_GetWork2()->hmac_sha1_context, sizeof(SVCHMACSHA1Context) );

            // Arm9との排他制御、ここまで
            UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
        }

//#define MY_DEBUG
#ifdef  MY_DEBUG
        (void)CheckStaticModuleHash();
#endif
    }

    return retval;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckCardAuthCode

  Description:  Rom Headerの認証コードアドレスを読んで、クローンブート対応か判定する
 *---------------------------------------------------------------------------*/
static HotSwState CheckCardAuthCode(void)
{
    u32 authBuf[PAGE_SIZE/sizeof(u32)];
    u32 auth_offset   = s_cbData.pBootSegBuf->rh.s.rom_valid_size ? s_cbData.pBootSegBuf->rh.s.rom_valid_size : 0x01000000;
    u32 page_offset   = auth_offset & 0xFFFFFE00;
    HotSwState retval = HOTSW_SUCCESS;

    u8  *p = (u8 *)authBuf;

    if(!HOTSW_IsCardAccessible()){
        return HOTSW_PULLED_OUT_ERROR;
    }

    retval = ReadPageGame( &s_cbData, page_offset, authBuf, MB_AUTHCODE_SIZE );

    p += auth_offset & 0x000001FF;
    if( *p++ == 'a' && *p == 'c' ) {
        HOTSW_PutString("  ☆ Clone Boot Mode\n");
        SYSMi_GetWork()->cloneBootMode = SYSM_CLONE_BOOT_MODE;
    }else {
        HOTSW_PutString("  □ Other Boot Mode\n");
        SYSMi_GetWork()->cloneBootMode = SYSM_OTHER_BOOT_MODE;
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_SetBootSegmentBuffer

  Description:  Boot Segment バッファの指定

  注：カードデータロード中は呼び出さないようにする
 *---------------------------------------------------------------------------*/
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size)
{
    SDK_ASSERT(size > BOOT_SEGMENT_SIZE);

    s_pBootSegBuffer = (BootSegmentData *)buf;
    s_BootSegBufSize = size;

    s_cbData.pBootSegBuf = s_pBootSegBuffer;

    // バッファの初期化
    MI_CpuClear8(s_pBootSegBuffer, size);
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_SetSecureSegmentBuffer

  Description:  Secure Segment バッファの指定

  注：カードデータロード中は呼び出さないようにする
 *---------------------------------------------------------------------------*/
void HOTSW_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size)
{
    HOTSWi_SetSecureSegmentBuffer(type, buf, size);
    // バッファの初期化
    MI_CpuClear8(s_pSecureSegBuffer, size);
}

void HOTSWi_SetSecureSegmentBuffer(ModeType type ,void* buf, u32 size)
{
    SDK_ASSERT(size > SECURE_SEGMENT_SIZE);

    if(type == HOTSW_MODE1){
        s_pSecureSegBuffer = (u32 *)buf;
        s_SecureSegBufSize = size;

        s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

    }
    else{
        s_pSecure2SegBuffer = (u32 *)buf;
        s_Secure2SegBufSize = size;

        s_cbData.pSecure2SegBuf = s_pSecure2SegBuffer;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GenVA_VB_VD

  Description:  コマンド認証値・コマンドカウンタ・PNジェネレータ初期値の生成
 *---------------------------------------------------------------------------*/
static void GenVA_VB_VD(void)
{
    SYSM_work* sw = SYSMi_GetWork();
    u32 dummy = 0;
    MATHRandContext32 rnd;
    u64 fuse64 = SCFG_ReadFuseData();
    u32 fuse32[2];

    // 念のためeFuseIDを推測できなくする
    fuse32[0] = ((u32*)&fuse64)[0];
    fuse32[1] = ((u32*)&fuse64)[1];
    EncryptByBlowfish(&s_cbData.keyTable, &fuse32[0], &fuse32[1]);

    // 乱数を初期化
    // チック＆RTC初回ロード値＆ゲームコード＆eFuseIDを種とする。
    // （起動する度に変化するパラメータと組み合わせる。
    //   Vカウンタは2130サイクル変化しないので固定値になりやすい。）
    // 起動毎、ゲームカード毎、本体毎にスクランブルが変化することになる。
    MATH_InitRand32(&rnd, OS_GetTick() ^ sw->Rtc1stData.words[0] ^ sw->Rtc1stData.words[1] ^ *(u32*)(s_cbData.pBootSegBuf->rh.s.game_code) ^ fuse32[0] ^ fuse32[1]);

    s_cbData.vae = MATH_Rand32(&rnd, 0);
    s_cbData.vbi = MATH_Rand32(&rnd, 0);
    s_cbData.vd  = MATH_Rand32(&rnd, 0);
    dummy        = MATH_Rand32(&rnd, 0);

    EncryptByBlowfish(&s_cbData.keyTable, &s_cbData.vae, &s_cbData.vbi);
    EncryptByBlowfish(&s_cbData.keyTable, &s_cbData.vd , &dummy);

    s_cbData.vae &= 0xffffff;
    s_cbData.vbi &= 0xfffff;
    s_cbData.vd  &= 0xffffff;
}


/*---------------------------------------------------------------------------*
  Name:         DecryptObjectFile

  Description:  セキュア領域先頭2KBの暗号化領域を復号化
 *---------------------------------------------------------------------------*/
static u32 encDestBuf[ENCRYPT_DEF_SIZE/sizeof(u32)];

#ifndef USE_WRAM_LOAD
static HotSwState DecryptObjectFile(void)
#else
BOOL HOTSW_DecryptObjectFile(void* dest)
#endif
{
    u8  i;
    s32 restSize;
    s32 size                = (s32)s_cbData.pBootSegBuf->rh.s.main_size;
    u32 *pEncBuf            = encDestBuf;
#ifndef USE_WRAM_LOAD
    u32 *pEncDes            = s_cbData.pSecureSegBuf;
#else
    u32 *pEncDes            = (u32 *)dest;
#endif
    BLOWFISH_CTX *tableBufp = &s_cbData.keyTable;
    BOOL exist              = TRUE;
#ifndef USE_WRAM_LOAD
    HotSwState retval       = HOTSW_SUCCESS;
#else
    BOOL       retval       = TRUE;
#endif

    if (size > ENCRYPT_DEF_SIZE) {
        size = ENCRYPT_DEF_SIZE;
    }
    restSize = size;

    // 読み込んだセキュア領域をバッファから一時バッファにコピー
    MI_CpuCopy32(pEncDes, pEncBuf, (u32)size);

    // セキュア領域先頭8バイトをBlowfishで複合化
    DecryptByBlowfish(&s_cbData.keyTable, &(pEncBuf)[1], &(pEncBuf)[0]);

    // Key Tableを変換
    s_cbData.keyBuf[1] = (s_cbData.keyBuf[1] << 1);
    s_cbData.keyBuf[2] = (s_cbData.keyBuf[2] >> 1);
    InitBlowfishKeyAndTableDS(&s_cbData.keyTable, s_cbData.keyBuf, 8);

    // もう一度セキュア領域先頭8バイトをBlowfishで複合化
    DecryptByBlowfish(&s_cbData.keyTable, &(pEncBuf)[1], &(pEncBuf)[0]);
    for ( i=0; i<8; i++ ){
        // 先頭8バイトが複合化の結果 "encryObj" となっていたら複合化成功
        if ( encrypt_object_key[i] != ((char*)pEncBuf)[i] ){
            exist = FALSE;
            break;
        }
    }

    // 暗号化オブジェクト有効時
    if ( exist ){
        u32 *bufp  = pEncBuf;

        bufp[0] = UNDEF_CODE;
        bufp[1] = UNDEF_CODE;
        while ((restSize -= 8) > 0) {
            bufp += 2;  // 復号処理
            DecryptByBlowfish(tableBufp, &(bufp)[1], &(bufp)[0]);
        }
    }
    else{
#ifndef USE_WRAM_LOAD
        retval = HOTSW_DATA_DECRYPT_ERROR;
#else
        retval = FALSE;
#endif
        MI_NDmaFill( HOTSW_NDMA_NO, pEncBuf, UNDEF_CODE, (u32)size ); // 未定義コードでクリア
    }
    MI_CpuCopy32(pEncBuf, pEncDes, (u32)size);

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         LockHotSwRsc

  Description:  共有ワークのリソースの排他制御用　lockを行う
 *---------------------------------------------------------------------------*/
static void LockHotSwRsc(OSLockWord* word)
{
    while(OS_TryLockByWord( s_RscLockID, word, NULL ) != OS_LOCK_SUCCESS){
        OS_Sleep(1);
    }
}


/*---------------------------------------------------------------------------*
  Name:         UnlockHotSwRsc

  Description:  共有ワークのリソースの排他制御用　Unlockを行う
 *---------------------------------------------------------------------------*/
static void UnlockHotSwRsc(OSLockWord* word)
{
    OS_UnlockByWord( s_RscLockID, word, NULL );
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_IsCardExist

  Description:  SCFG_MC1のCDETフラグを見て、カードの存在判定を行う
 *---------------------------------------------------------------------------*/
BOOL HOTSW_IsCardExist(void)
{
    if( !(reg_MI_MC1 & REG_MI_MC_SL1_CDET_MASK) ){
        return TRUE;
    }
    else{
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSW_IsCardAccessible

  Description:  SCFG_MC1のCDETフラグとM(モード)を見て、カードスロットにアクセスできる状態か判定する
 *---------------------------------------------------------------------------*/
BOOL HOTSW_IsCardAccessible(void)
{
    if( HOTSW_IsCardExist() && (GetMcSlotMode() == SLOT_STATUS_MODE_10)){
        return TRUE;
    }
    else{
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GetMcSlotMode

  Description:  スロットの現在のモードを返す
 *---------------------------------------------------------------------------*/
static u32 GetMcSlotMode(void)
{
    return reg_MI_MC1 & REG_MI_MC_SL1_MODE_MASK;
}


/*---------------------------------------------------------------------------*
  Name:         SetMcSlotMode

  Description:  カードスロットのモード設定
 *---------------------------------------------------------------------------*/
static void SetMcSlotMode(u32 mode)
{
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~REG_MI_MC_SL1_MODE_MASK) | mode);
}


/*---------------------------------------------------------------------------*
  Name:        McPowerOn

  Description: スロット電源ON
 *---------------------------------------------------------------------------*/
static void McPowerOn(void)
{
    // Counter-Aカウンタ設定値到達まで待つ
    while(GetMcSlotMode() == SLOT_STATUS_MODE_11){
        OS_Sleep(1);
    }

    if(GetMcSlotMode() == SLOT_STATUS_MODE_00){
        // 3DMのリセット待ち
        OS_Sleep(1);

        // SCFG_MC1 の Slot Status の M1,M0 を 01 にする
        SetMcSlotMode(SLOT_STATUS_MODE_01);

        // VDDの安定期間待ち
        OS_Sleep(10);

        // SCFG_MC1 の Slot Status の M1,M0 を 10 にする
        SetMcSlotMode(SLOT_STATUS_MODE_10);

        // RESBを上げるまでの待ち時間
        OS_Sleep(27);

        // リセットをhighに (RESB = 1にする)
        reg_HOTSW_MCCNT1 = RESB_MASK;

        // カードへ最初のコマンドを送るまでの待ち時間
        OS_Sleep(120);
    }
}


/*---------------------------------------------------------------------------*
  Name:        McPowerOff

  Description: スロット電源OFF
 *---------------------------------------------------------------------------*/
static void McPowerOff(void)
{
    // Counter-Aカウンタ設定値到達まで待つ
    while(GetMcSlotMode() == SLOT_STATUS_MODE_11){
        OS_Sleep(1);
    }

    if(GetMcSlotMode() == SLOT_STATUS_MODE_10){
        // SCFG_MC1 の Slot Status の M1,M0 を 11 にする
        SetMcSlotMode(SLOT_STATUS_MODE_11);

        // SCFG_MC1 の Slot Status の M1,M0 が 00 になるまでポーリング
        while(GetMcSlotMode() != SLOT_STATUS_MODE_00){
            OS_Sleep(1);
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_TurnCardPowerOn

  Description:  デバッガ通信用にカードスロットの電源をONにする。

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void HOTSWi_TurnCardPowerOn(u32 slot)
{
    u32 shift;
    u32 mask = slot >= 2 ? (u32)REG_MI_MC_SL2_CDET_MASK : (u32)REG_MI_MC_SL1_CDET_MASK;

    // カードが差さっていなければONしない
    if ( reg_MI_MC & mask )
    {
        return;
    }

    shift = slot >= 2 ? (u32)REG_MI_MC_SL2_MODE_SHIFT : (u32)REG_MI_MC_SL1_MODE_SHIFT;
    mask  = slot >= 2 ? (u32)REG_MI_MC_SL2_MODE_MASK : (u32)REG_MI_MC_SL1_MODE_MASK;

    // 電源が落ちている最中なら待つ
    if ( (reg_MI_MC & mask) == (0x3 << shift) )
    {
        OS_Sleep(1);
    }

    if ( (reg_MI_MC & mask) == (0x0 << shift) )
    {
        // 3DMのリセット待ち
        OS_Sleep(1);

        reg_MI_MC = (u16)((reg_MI_MC & ~mask) | (0x1 << shift));
        // VDDの安定期間
        OS_Sleep(10);

        reg_MI_MC = (u16)((reg_MI_MC & ~mask) | (0x2 << shift));
        // RESBのLow期間
        OS_Sleep(27);

        reg_MI_MCCNT1_B = REG_MI_MCCNT1_RESB_MASK;
        // RESBのHigh期間
        // カードへ最初のコマンドを送るまでの待ち時間
        OS_Sleep(120);
    }
}


/*---------------------------------------------------------------------------*
  Name:         SetMCSCR

  Description:  符号生成回路初期値設定レジスタを設定する

  ※注：この関数はセキュアモードで、sPNG_ONコマンドを実行してから呼び出してください。
 *---------------------------------------------------------------------------*/
static void SetMCSCR(void)
{
    static u32 pnbL          = 0x879b9b05;
    static u8  pnbH          = 0x5c;
    static u8  pnaL1         = 0x60;
    static u8  pnaL0Table[8] = { 0xe8, 0x4d, 0x5a, 0xb1, 0x17, 0x8f, 0x99, 0xd5 };

    u32 pnaL = s_cbData.vd << 15 | pnaL1 << 8 | pnaL0Table[(s_cbData.pBootSegBuf->rh.s.rom_type & 0x7)];
    u8  pnaH = (u8)((s_cbData.vd >> 17) & 0x7f);

    // SCR A
    reg_HOTSW_MCSCR0 = pnaL;

    // SCR B
    reg_HOTSW_MCSCR1 = pnbL;

    // [d0 -d6 ] -> SCR A
    // [d16-d22] -> SCR B
    reg_HOTSW_MCSCR2 = (u32)(pnaH | pnbH << 16);

    // MCCNT1 レジスタ設定 (SCR = 1に)
    reg_HOTSW_MCCNT1 = SCR_MASK;
}


/*---------------------------------------------------------------------------*
  Name:        HotSwThread

  Description: カード抜け・挿し処理スレッド
 *---------------------------------------------------------------------------*/
static void HotSwThread(void *arg)
{
    #pragma unused( arg )

    HotSwState              retval;
    HotSwMessageForArm7     *msg;

    while(1){
        OS_ReceiveMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);

        SYSMi_GetWork()->flags.hotsw.isBusyHotSW = TRUE;

        if( msg->ctrl == TRUE ) {
            SYSMi_GetWork()->flags.hotsw.isEnableHotSW = msg->value;
            s_pollingThreadSleepFlg = msg->value ? FALSE : TRUE;

            if(msg->value){
#ifndef USE_WRAM_LOAD
                s_isPulledOut = TRUE;
#endif
                OS_SendMessage(&HotSwThreadData.hotswPollingCtrlQueue,
                               (OSMessage *)&HotSwThreadData.hotswPollingCtrlMsg[HotSwThreadData.idx_polling],
                               OS_MESSAGE_NOBLOCK);
                HotSwThreadData.idx_polling = (HotSwThreadData.idx_polling+1) % HOTSW_POLLING_CTRL_BUFFER_NUM;
            }
            else{
                if(msg->finalize == FALSE){
                    ClearCardFlgs();
                }
            }
        }

        if( msg->finalize == TRUE && msg->ctrl == FALSE) {
            FinalizeHotSw( msg->state );
        }

#ifdef USE_WRAM_LOAD
        if( msg->read == TRUE ){
            ReadCardData(SYSMi_GetWork()->cardReadParam.src,
                         SYSMi_GetWork()->cardReadParam.dest,
                         SYSMi_GetWork()->cardReadParam.size);

            SYSMi_GetWork()->flags.hotsw.isBusyHotSW = FALSE;

            continue;
        }

        if( msg->type == HOTSW_INSERT ){
            SendPxiMessage(HOTSW_CARD_INSERT);
        }
#endif

        while(1){
            if( !SYSMi_GetWork()->flags.hotsw.isEnableHotSW ) {
                HOTSW_PutString("### HotSw is restrained...\n");
                break;
            }

            if(HOTSW_IsCardExist()){
                if(!s_isPulledOut){
                    if(GetMcSlotMode() == SLOT_STATUS_MODE_10){
                        // [Debug]
						s_gameID = s_cbData.id_gam;
                        
                        LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                        if( msg->ctrl && msg->value ){
                            SYSMi_GetWork()->flags.hotsw.isExistCard        = TRUE;
                            SYSMi_GetWork()->flags.hotsw.isCardStateChanged = TRUE;
                            if( s_cbData.pBootSegBuf->rh.s.banner_offset ){
                                SYSMi_GetWork()->flags.hotsw.isValidCardBanner = TRUE;
                            }
                        }
                        SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = TRUE;
#ifdef USE_WRAM_LOAD
                        SYSMi_GetWork()->flags.hotsw.isCardGameMode      = TRUE;
#endif
                        UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
#ifdef USE_WRAM_LOAD
                        SendPxiMessage(HOTSW_CHANGE_GAMEMODE);
#endif
                        HOTSW_PutString("ok!\n");

                        break;
                    }
                }

                retval = LoadCardData();

                DebugPrintErrorMessage(retval);

                s_isPulledOut = FALSE;

                // エラー処理
                if(retval != HOTSW_SUCCESS){
                    McPowerOff();
                    PulledOutSequence();
                    break;
                }
            }

            // カードが抜けてたら
            else{
                PulledOutSequence();
                break;
            }
        } // Card Read while loop

        if( !SYSMi_GetWork()->flags.hotsw.is1stCardChecked ){
            LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
            SYSMi_GetWork()->flags.hotsw.is1stCardChecked = TRUE;
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
        }

        SYSMi_GetWork()->flags.hotsw.isBusyHotSW       = FALSE;
    } // while loop
}


/*---------------------------------------------------------------------------*
  Name:        PulledOutSequence

  Description: カード抜け処理
 *---------------------------------------------------------------------------*/
static void PulledOutSequence(void)
{
#ifdef USE_WRAM_LOAD
    SendPxiMessage(HOTSW_CARD_PULLOUT);
#endif

    ClearCardFlgs();

    MI_CpuClear32(&s_cbData, sizeof(CardBootData));

    MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
    MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);
    MI_CpuClearFast(s_pSecure2SegBuffer, s_Secure2SegBufSize);
    MI_CpuClearFast((u32 *)SYSM_CARD_BANNER_BUF, sizeof(TWLBannerFile));

    s_isPulledOut = TRUE;

	// [Debug]
    s_gameID = 0;
    
    // ワンセグのスリープ時シャットダウン対策を戻す
    MCU_EnableDeepSleepToPowerLine( MCU_PWR_LINE_33, TRUE );
}


/*---------------------------------------------------------------------------*
  Name:        ClearCardFlgs

  Description: カードデータバッファのバッファとフラグの後始末
 *---------------------------------------------------------------------------*/
static void ClearCardFlgs(void)
{
    // フラグ処理
    LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
    SYSMi_GetWork()->flags.hotsw.isExistCard         = FALSE;
    SYSMi_GetWork()->flags.hotsw.isValidCardBanner   = FALSE;
    SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
    SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = FALSE;
#ifdef USE_WRAM_LOAD
    SYSMi_GetWork()->flags.hotsw.isCardGameMode      = FALSE;
#endif
    UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
}


/*---------------------------------------------------------------------------*
  Name:        FinalizeHotSw

  Description: アプリ起動時に、活線挿抜関係の後始末を行う。
 *---------------------------------------------------------------------------*/
static void FinalizeHotSw(HotSwCardState state)
{
    ROM_Header* rh = (void*)SYSM_APP_ROM_HEADER_BUF;
    static BOOL finalized = FALSE;

    if(finalized){
        return;
    }
    finalized = TRUE;

    // カード挿入検出停止
    (void)OS_DisableIrqMask( HOTSW_IF_CARD_DET );

    // ポーリングスレッドが動作中は待つ
    while(s_isBusyMonitorThread){
		OS_Sleep(1);
    }

    // ポーリングスレッドを消去
    OS_KillThread( &HotSwThreadData.monitorThread, NULL );

    SYSMi_GetWork()->appCardID = 0;

    // カードアクセスできないなら、(一応)スロット電源OFFしてレジスタクリア
    if(!HOTSW_IsCardAccessible()){
        McPowerOff();
        ClearAllCardRegister();
        goto final;
    }

    switch(state){
        // Slot Power Off
      case HOTSW_CARD_STATE_POWER_OFF:
        McPowerOff();
        break;

        // Normalモードに移行
      case HOTSW_CARD_STATE_NORMAL_MODE:
        ForceNormalModeToFinalize();
        break;

        // Gameモードに移行
      case HOTSW_CARD_STATE_GAME_MODE:
        ForceNitroModeToFinalize();
        break;

        // 状態キープ
      case HOTSW_CARD_STATE_KEEP:
        SYSMi_GetWork()->appCardID = s_cbData.id_gam;
        break;

        // else
      default:
        ClearAllCardRegister();
        McPowerOff();
    }

final:
    ClearCardIrq();

    // デバッガではTWLカードスロット２を電源ON
    //（既にONなら何もしない）
    // NANDアプリ起動時はデフォルトではカードI/FをOFFにするが、
    // デバッガモニタではカードI/Fが必要なためスロット２を使用
    if ( SYSM_IsRunOnDebugger() )
    {
        HOTSWi_TurnCardPowerOn( 2 );
    }

    // 終了完了通知
    SYSMi_GetWork()->gameCommondParam = s_cbData.gameCommondParam;
    SYSMi_GetWork()->flags.hotsw.isFinalized = TRUE;
}


/*---------------------------------------------------------------------------*
  Name:        ForceNitroModeToFinalize

  Description: Nitro互換のGame Modeに強制移行させる
 *---------------------------------------------------------------------------*/
static void ForceNitroModeToFinalize(void)
{
    s_cbData.id_gam = 0;

    if(!ChangeGameMode()){
        ClearAllCardRegister();
        McPowerOff();

        HOTSW_PutString("Failed To Change Game Mode... Card Slot Power Off\n");
    }

    SYSMi_GetWork()->appCardID = s_cbData.id_gam;
}


/*---------------------------------------------------------------------------*
  Name:        ForceNormalModeToFinalize

  Description: Normal Modeに強制移行させる
 *---------------------------------------------------------------------------*/
static void ForceNormalModeToFinalize(void)
{
    CARD_LockRom(s_CardLockID);

    McPowerOff();  // 既にOFFになっているため実質的には無効
    McPowerOn();

    s_cbData.modeType = HOTSW_MODE2;
    (void)LoadTable();
    (void)ReadIDNormal(&s_cbData);
    (void)ReadBootSegNormal(&s_cbData);

    CARD_UnlockRom(s_CardLockID);

    s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
    SYSMi_GetWork()->appCardID = s_cbData.id_nml;

    HOTSW_PutString("- game card on flg is TRUE : now Normal Mode\n");
}


/*---------------------------------------------------------------------------*
  Name:        ChangeGameMode

  Description: Nitro互換のRomHeaderを読んで、Game Modeに移行させる
 *---------------------------------------------------------------------------*/
static BOOL ChangeGameMode(void)
{
    HotSwState state;

    CARD_LockRom(s_CardLockID);

    McPowerOff();
    McPowerOn();

    s_cbData.pBootSegBuf = s_pBootSegBuffer;
    s_cbData.modeType = HOTSW_MODE1;

    // ---------------------- Normal Mode ----------------------
    state = LoadTable();
    state = ReadIDNormal(&s_cbData);
    state = ReadBootSegNormal(&s_cbData);

    if(s_isRomEmu){
        s_cbData.cardType = ROM_EMULATION;
        s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
    }
    else{
        s_cbData.cardType = (s_cbData.id_nml & HOTSW_ROMID_1TROM_MASK) ? DS_CARD_TYPE_2 : DS_CARD_TYPE_1;
        s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param;
    }
    s_cbData.secureLatency = AddLatency2ToLatency1(s_cbData.pBootSegBuf->rh.s.secure_cmd_param);

    MI_CpuClear32(&s_cbData.keyTable, sizeof(BLOWFISH_CTX));
    MI_CpuClear32(s_cbData.keyBuf, sizeof(s_cbData.keyBuf));

    MakeBlowfishTableDS(&s_cbData, 8);
    GenVA_VB_VD();
    state = ChangeModeNormal(&s_cbData);

    // ---------------------- Secure Mode ----------------------
    state = s_funcTable[s_isRomEmu].SetPNG_S(&s_cbData);
    SetMCSCR();
    state = s_funcTable[s_isRomEmu].ChangeMode_S(&s_cbData);

    // ---------------------- Game Mode ----------------------
    state = ReadIDGame(&s_cbData);

    if(s_cbData.id_nml != s_cbData.id_gam){
        state = HOTSW_ID_CHECK_ERROR;
    }

    HOTSW_TPrintf("Card Normal ID : 0x%08x\n", s_cbData.id_nml);
    HOTSW_TPrintf("Card Game   ID : 0x%08x\n", s_cbData.id_gam);

#ifdef USE_NEW_DMA
    HOTSW_WaitNDmaCtrl(HOTSW_NDMA_NO);
#else
    HOTSW_WaitDmaCtrl(HOTSW_DMA_NO);
#endif

    HOTSW_WaitCardCtrl();

    CARD_UnlockRom(s_CardLockID);

    if(state == HOTSW_SUCCESS){
        return TRUE;
    }
    else{
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:        SendPxiMessage

  Description: ARM9にメッセージを送る
 *---------------------------------------------------------------------------*/
#ifdef USE_WRAM_LOAD
static void SendPxiMessage(HotSwCallBackType type)
{
    HotSwPxiMessageForArm9  msg;

    MI_CpuClear8( &msg, sizeof(HotSwPxiMessageForArm9));

    switch(type){
      case HOTSW_CHANGE_GAMEMODE:
        msg.msg.mode = TRUE;
        break;

      case HOTSW_CARD_INSERT:
        msg.msg.insert = TRUE;
        break;

      case HOTSW_CARD_PULLOUT:
        msg.msg.pullout = TRUE;
        break;

      default:
        return;
    }

    while (PXI_SendWordByFifo(PXI_FIFO_TAG_HOTSW, msg.data, FALSE) != PXI_FIFO_SUCCESS)
    {
        // do nothing
    }
}
#endif


/*---------------------------------------------------------------------------*
  Name:        ClearCardIrq

  Description: カード関連の割り込みをクリアする
 *---------------------------------------------------------------------------*/
#define REGCLEAR_16     0x0000
#define REGCLEAR_32     0x00000000UL

static void ClearCardIrq(void)
{
    // HotSwで使っている割り込みを無効にする
    OSIntrMode irq = OS_DisableInterrupts();
    (void)OS_DisableIrqMask( HOTSW_IF_CARD_DET );
    (void)OS_ResetRequestIrqMask( HOTSW_IF_CARD_DET );
    (void)OS_RestoreInterrupts( irq );
}


/*---------------------------------------------------------------------------*
  Name:        ClearAllCardRegister

  Description: カード関連の全てのレジスタをクリアする。
 *---------------------------------------------------------------------------*/
static void ClearAllCardRegister(void)
{
    ClearCardIrq();

    // コマンド設定レジスタをクリア [各32bit]
    reg_HOTSW_MCCMD0 = REGCLEAR_32;
    reg_HOTSW_MCCMD1 = REGCLEAR_32;

    // メモリカードイネーブルとか割り込みイネーブルとかをクリア [16bit中 d15-d8が関係あり]
    reg_HOTSW_MCCNT0 &= HOTSW_E2PROM_CTRL_MASK;

    // latencyとかstartフラグとかのレジスタをクリア [32bit]
    reg_HOTSW_MCCNT1 = REGCLEAR_32;

    // カードからのデータがたまるレジスタをクリア [32bit]
    reg_HOTSW_MCD1 = REGCLEAR_32;

    // 符号生成回路初期値設定レジスタをクリア [各32bit]
    reg_HOTSW_MCSCR0 = REGCLEAR_32;
    reg_HOTSW_MCSCR1 = REGCLEAR_32;
    reg_HOTSW_MCSCR2 = REGCLEAR_32;

    // Slot Status,SWPをクリア [d15-d0 Slot Status,SWP]
    reg_MI_MC1 &= REG_MI_MC1_CC_MASK;
}


/*---------------------------------------------------------------------------*
  Name:        MonitorThread

  Description: 実際のカード状態とHotSwThreadで状態を比べて、違いがあった場合は
               メッセージを送る

  s_isPulledOut : True  -> カードなし       HOTSW_IsCardExist : True  -> カードあり
                  False -> カードあり                           False -> カードなし
 *---------------------------------------------------------------------------*/
static void MonitorThread(void *arg)
{
    #pragma unused( arg )

    u32  count = 0;
    BOOL isCardExist;

    while(1){
        // カードデータロード中は待機
        do{
            OS_Sleep(CARD_EXIST_CHECK_POLLING_TIME);

            count++;
        }
        while( SYSMi_GetWork()->flags.hotsw.isBusyHotSW );

        // ポーリングスレッド抑制フラグが上がってたら、スリープ。抑制フラグが下りたら起床。
        if(s_pollingThreadSleepFlg){
            OSMessage msg;
            OS_ReceiveMessage(&HotSwThreadData.hotswPollingCtrlQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);
        }

		s_isBusyMonitorThread = TRUE;
        
        isCardExist = HOTSW_IsCardExist();

        CheckCardPullOut(isCardExist);

        if(count >= CARD_INSERT_CHECK_INTERVAL){
            CheckCardInsert(isCardExist);
            count = 0;
        }

        s_isBusyMonitorThread = FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:        CheckCardInsert

  Description: カードは挿さっているのに、ランチャーが認識してなかったらメッセージを送る
 *---------------------------------------------------------------------------*/
static void CheckCardInsert(BOOL cardExist)
{
    // カードは挿さっているのに、ランチャーが認識してなかったらメッセージを送る
    if(cardExist && s_isPulledOut){
        OSIntrMode enabled = OS_DisableInterrupts();

        SendInsertMessage();

        (void)OS_RestoreInterrupts( enabled );
    }
    // カードは挿さっていて、ランチャーが認識していたらGameモードのID読みをして、抜けてないか調べる
    else if(cardExist && !s_isPulledOut && !SYSMi_GetWork()->flags.hotsw.isBusyHotSW){
		OS_PutString("GameMode ID Check...\n");
        
        // カードのロック
    	CARD_LockRom(s_PollingLockID);

        ReadIDGame(&s_cbData);

        if(s_cbData.id_gam != s_gameID){
            OSIntrMode enabled;
            
			McPowerOff();

            enabled = OS_DisableInterrupts();
            
			PulledOutSequence();
			SendInsertMessage();
            
            (void)OS_RestoreInterrupts( enabled );
        }
        
	    // カードのロック開放(※ロックIDは開放せずに持ち続ける)
    	CARD_UnlockRom(s_PollingLockID);
    }
}


static void SendInsertMessage(void)
{
	HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].ctrl  = FALSE;
	HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].value = 0;
	HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].type  = HOTSW_INSERT;

	// メッセージをキューの先頭に入れる
	OS_JamMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

	// メッセージインデックスをインクリメント
	HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;
}


/*---------------------------------------------------------------------------*
  Name:        CheckCardPullOut

  Description: カードは抜かれているのに、ランチャーが認識していたらメッセージを送る
 *---------------------------------------------------------------------------*/
static void CheckCardPullOut(BOOL cardExist)
{
    if(!cardExist && !s_isPulledOut){
        OSIntrMode enabled;

#ifndef HOTSW_DISABLE_FORCE_CARD_OFF
        {
            u32 mode = GetMcSlotMode();
            if(mode == SLOT_STATUS_MODE_01 || mode == SLOT_STATUS_MODE_10){
                SetMcSlotMode(SLOT_STATUS_MODE_11);
            }
        }
#endif
        enabled = OS_DisableInterrupts();

        HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].ctrl  = FALSE;
        HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].value = 0;
        HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].type  = HOTSW_PULLOUT;

        // メッセージをキューの先頭に入れる
        OS_JamMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

        // メッセージインデックスをインクリメント
        HotSwThreadData.idx_pulledOut = (HotSwThreadData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

        (void)OS_RestoreInterrupts( enabled );
    }
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackCardDet

  Description:  カードB挿し割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCardDet(void)
{
    HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].ctrl  = FALSE;
    HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].value = 0;
    HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].type  = HOTSW_INSERT;

    // メッセージ送信
    OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

    // メッセージインデックスをインクリメント
    HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;

    HOTSW_PutString("●\n");
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackPxi

  Description:  PXI割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)
#pragma unused(err)
    HotSwPxiMessageForArm7 d;

    d.data = data;

#ifndef USE_WRAM_LOAD
    HOTSW_TPrintf("... Pxi Message - value:%x  ctrl:%x  finalize:%x  bootType:%x\n",
                                d.msg.value, d.msg.ctrl, d.msg.finalize, d.msg.cardState);
#else
    HOTSW_TPrintf("... Pxi Message - value:%x  ctrl:%x  finalize:%x  read:%x  bootType:%x\n",
                                d.msg.value, d.msg.ctrl, d.msg.finalize, d.msg.read, d.msg.cardState);

    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].read      = (d.msg.read) ? TRUE : FALSE;
#endif
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].ctrl      = (d.msg.ctrl) ? TRUE : FALSE;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].finalize  = (d.msg.finalize) ? TRUE : FALSE;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].value     = d.msg.value;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].state     = (HotSwCardState)d.msg.cardState;

    // メッセージ送信
    OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl], OS_MESSAGE_NOBLOCK);

    // メッセージインデックスをインクリメント
    HotSwThreadData.idx_ctrl = (HotSwThreadData.idx_ctrl+1) % HOTSW_CTRL_MSG_NUM;
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackCardData

  Description:  カードB データ転送終了割り込みハンドラ
 *---------------------------------------------------------------------------*/
#ifndef USE_NEW_DMA
static void InterruptCallbackCardData(void)
{
    // DMA強制終了
    MI_StopDma(HOTSW_DMA_NO);

    // メッセージ送信
    OS_SendMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&HotSwThreadData.hotswDmaMsg[HotSwThreadData.idx_dma], OS_MESSAGE_NOBLOCK);

    // メッセージインデックスをインクリメント
    HotSwThreadData.idx_dma = (HotSwThreadData.idx_dma+1) % HOTSW_DMA_MSG_NUM;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         SetInterruptCallback
                SetInterruptCallbackEx

  Description:  割り込みコールバック関数と割り込み許可の設定を行う
 *---------------------------------------------------------------------------*/
static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func )
{
    (void)OS_SetIrqFunction(intr_bit, func);
    (void)OS_EnableIrqMask(intr_bit);
}

static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func )
{
    (void)OS_SetIrqFunctionEx(intr_bit, func);
    (void)OS_EnableIrqMaskEx(intr_bit);
}


/*---------------------------------------------------------------------------*
  Name:         SetInterrupt

  Description:  割り込みコールバック関数を一度に設定する関数
 *---------------------------------------------------------------------------*/
static void SetInterrupt(void)
{
    SetInterruptCallback( OS_IE_CARD_A_DET  , InterruptCallbackCardDet );

#ifdef USE_NEW_DMA
    (void)OS_EnableIrqMask(OS_IE_NDMA2);
#else
    SetInterruptCallback( OS_IE_CARD_DATA  , InterruptCallbackCardData );
#endif
}


/*---------------------------------------------------------------------------*
  Name:         CheckStaticModuleHash

  Description:  常駐モジュールのハッシュチェックを行う
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
#include <twl/os/common/systemCall.h>
static HotSwState CheckStaticModuleHash(void)
{
    BOOL flg = TRUE;

    // Arm9常駐モジュール Hash値のチェック
    if(!CheckArm9HashValue()){
        flg = FALSE;
        HOTSW_PutString("×Arm9 Static Module Hash Check Error...\n");
    }

    // Arm7常駐モジュール Hash値のチェック
    if(!CheckArm7HashValue()){
        flg = FALSE;
        HOTSW_PutString("×Arm7 Static Module Hash Check Error...\n");
    }

    // Arm9拡張常駐モジュール Hash値のチェック
    if(!CheckExtArm9HashValue()){
        flg = FALSE;
        HOTSW_PutString("×Arm9 Ltd Static Module Hash Check Error...\n");
    }

    // Arm7拡張常駐モジュール Hash値のチェック
    if(!CheckExtArm7HashValue()){
        flg = FALSE;
        HOTSW_PutString("×Arm7 Ltd Static Module Hash Check Error...\n");
    }

    if(flg){
        HOTSW_PutString("*** Static Module Load was Completed!!\n");
    }

    return flg ? HOTSW_SUCCESS : HOTSW_HASH_CHECK_ERROR;
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckArm7HashValue

  Description:  Arm7常駐モジュールのハッシュチェック
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static BOOL CheckArm7HashValue(void)
{
    u8      sha1data[DIGEST_SIZE_SHA1];
    BOOL    retval = TRUE;

    // クリア
    MI_CpuClear8(sha1data, sizeof(sha1data));

    // ARM7常駐モジュールのHash値照合
    SVC_CalcHMACSHA1( sha1data,
                      (u32 *)(s_cbData.arm7Stc),
                      s_cbData.pBootSegBuf->rh.s.sub_size,
                      s_digestDefaultKey,
                      sizeof(s_digestDefaultKey) );

    return SVC_CompareSHA1( sha1data, s_cbData.pBootSegBuf->rh.s.sub_static_digest );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckArm9HashValue

  Description:  Arm9常駐モジュールのハッシュチェック

  ※ 先頭2Kの復号化が行われる前のデータのハッシュを比べる
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static BOOL CheckArm9HashValue(void)
{
    u8      sha1data[DIGEST_SIZE_SHA1];
    BOOL    retval = TRUE;
    SVCHMACSHA1Context hash;

    // クリア
    MI_CpuClear8(sha1data, sizeof(sha1data));

    // ハッシュ初期化
    SVC_HMACSHA1Init( &hash, s_digestDefaultKey, sizeof(s_digestDefaultKey) );

    // セキュア領域分UpDate
    SVC_HMACSHA1Update( &hash, s_cbData.pSecureSegBuf, SECURE_SEGMENT_SIZE );

    // ゲーム領域分UpDate
    SVC_HMACSHA1Update( &hash, (u32 *)(s_cbData.arm9Stc + SECURE_SEGMENT_SIZE), s_cbData.pBootSegBuf->rh.s.main_size - SECURE_SEGMENT_SIZE );

    // Hash値取得
    SVC_HMACSHA1GetHash( &hash, sha1data );

    return SVC_CompareSHA1( sha1data, s_cbData.pBootSegBuf->rh.s.main_static_digest );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckExtArm7HashValue

  Description:  Arm7拡張常駐モジュールのハッシュチェック
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static BOOL CheckExtArm7HashValue(void)
{
    u8      sha1data[DIGEST_SIZE_SHA1];
    BOOL    retval = TRUE;

    // クリア
    MI_CpuClear8(sha1data, sizeof(sha1data));

    // ARM7常駐モジュールのHash値照合
    SVC_CalcHMACSHA1( sha1data,
                      (u32 *)s_cbData.arm7Ltd,
                      s_cbData.pBootSegBuf->rh.s.sub_ltd_size,
                      s_digestDefaultKey,
                      sizeof(s_digestDefaultKey) );

    return SVC_CompareSHA1( sha1data, s_cbData.pBootSegBuf->rh.s.sub_ltd_static_digest );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         CheckExtArm9HashValue

  Description:  Arm9拡張常駐モジュールのハッシュチェック
 *---------------------------------------------------------------------------*/
#ifndef USE_WRAM_LOAD
static BOOL CheckExtArm9HashValue(void)
{
    u8      sha1data[DIGEST_SIZE_SHA1];
    u32     size;
    BOOL    retval = TRUE;
    SVCHMACSHA1Context hash;

    // Arm9拡張常駐モジュールのセキュア領域分のサイズを取得
    size = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ?
             s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;

    // クリア
    MI_CpuClear8(sha1data, sizeof(sha1data));

    // ハッシュ初期化
    SVC_HMACSHA1Init( &hash, s_digestDefaultKey, sizeof(s_digestDefaultKey) );

    // セキュア領域分UpDate
    SVC_HMACSHA1Update( &hash, (u32 *)SYSM_CARD_TWL_SECURE_BUF, size );

    // ゲーム領域分UpDate (Arm9拡張常駐モジュールがSecure領域で収まってたらここは飛ばす)
    if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ){
        SVC_HMACSHA1Update( &hash, (u32 *)(s_cbData.arm9Ltd + SECURE_SEGMENT_SIZE), s_cbData.pBootSegBuf->rh.s.main_ltd_size - size );
    }

    // Hash値取得
    SVC_HMACSHA1GetHash( &hash, sha1data );

    return SVC_CompareSHA1( sha1data, s_cbData.pBootSegBuf->rh.s.main_ltd_static_digest );
}
#endif


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_GetCardBootData

  Description:
 *---------------------------------------------------------------------------*/
void *HOTSWi_GetCardBootData(void)
{
    return &s_cbData;
}


/*---------------------------------------------------------------------------*
  Name:         DebugPrintErrorMessage

  Description:
 *---------------------------------------------------------------------------*/
static void DebugPrintErrorMessage(HotSwState state)
{
    switch(state){
      case HOTSW_SUCCESS:
        HOTSW_PutString("   - Success\n");
        break;

      case HOTSW_TIME_OUT:
        HOTSW_PutString("   - Error 1 : TimeOut\n");
        break;

      case HOTSW_CARD_LOCK_ERROR:
        HOTSW_PutString("   - Error 2 : Slot Lock\n");
        break;

      case HOTSW_CRC_CHECK_ERROR:
        HOTSW_PutString("   - Error 3 : CRC Check\n");
        break;

      case HOWSW_REGION_CHECK_ERROR:
        HOTSW_PutString("   - Error 3 : Region Check\n");
        break;

      case HOTSW_HASH_CHECK_ERROR:
        HOTSW_PutString("   - Error 4 : Hash Check\n");
        break;

      case HOTSW_ID_CHECK_ERROR:
        HOTSW_PutString("   - Error 5 : ID Check\n");
        break;

      case HOTSW_PULLED_OUT_ERROR:
        HOTSW_PutString("   - Error 6 : Pulled Out\n");
        break;

      case HOTSW_DATA_DECRYPT_ERROR:
        HOTSW_PutString("   - Error 7 : Data Decrypt\n");
        break;

      case HOTSW_BUFFER_OVERRUN_ERROR:
        HOTSW_PutString("   - Error 8 : Buffer OverRun\n");
        break;

      case HOTSW_UNEXPECTED_ERROR:
        HOTSW_PutString("   - Error 9 : Unexpected\n");
        break;

      default :
        HOTSW_PutString("   - illigal Error\n");
        break;
    }
}