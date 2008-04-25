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

#define DEBUG_MODE

//#define HOTSW_DISABLE_FORCE_CARD_OFF

// カード電源ONからROMヘッダロードまでの期間にスリープに入る時のワンセグ対策しない場合
//#define HOWSW_ENABLE_DEEP_SLEEP_WHILE_INSERT_CARD

// define -------------------------------------------------------------------
#define     CHATTERING_COUNTER                  0x1988      // 100ms分 (0x1988 * 15.3us = 100000us)
#define     COUNTER_A                           0x51C       //  20ms分 ( 0x51C * 15.3us =  20012us)

#define     CARD_EXIST_CHECK_INTERVAL           100

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

// Function prototype -------------------------------------------------------
static BOOL IsSwap(void);
static u32 GetMcSlotShift(void);
static u32 GetMcSlotMask(void);
static u32 GetMcSlotMode(void);
static void SetMcSlotMode(u32 mode);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCard(void);
static void InterruptCallbackCardDet(void);
static void InterruptCallbackNDma(void);
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err);

static void LockHotSwRsc(OSLockWord* word);
static void UnlockHotSwRsc(OSLockWord* word);

static void HotSwThread(void *arg);
static void MonitorThread(void *arg);

static void McPowerOn(void);
static void McPowerOff(void);
static void SetMCSCR(void);

static BOOL isTwlModeLoad(void);
static HotSwState ReadSecureModeCardData(void);
static void ClearCaradFlgs(void);

static void RegisterRomEmuInfo(void);
static void GenVA_VB_VD(void);
static HotSwState DecryptObjectFile(void);
static HotSwState LoadBannerData(void);
static HotSwState LoadStaticModule(void);
static HotSwState LoadCardData(void);
static HotSwState CheckCardAuthCode(void);

static HotSwState CheckStaticModuleHash(void);

static s32 LockExCard(u16 lockID);
static s32 UnlockExCard(u16 lockID);

static BOOL CheckArm7HashValue(void);
static BOOL CheckArm9HashValue(void);
static BOOL CheckExtArm7HashValue(void);
static BOOL CheckExtArm9HashValue(void);

static void ShowRegisterData(void);
static void ShowRomHeaderData(void);
static void DebugPrintErrorMessage(HotSwState state);

HotSwState HOTSWi_RefreshBadBlock(u32 romMode);

// Static Values ------------------------------------------------------------
#include <twl/ltdwram_begin.h>

static char                 encrypt_object_key[] ATTRIBUTE_ALIGN(4) = "encryObj";

static u16                  s_RscLockID;
static u16                  s_CardLockID;
static u16                  s_bondingOp;

static u32                  s_BootSegBufSize, s_SecureSegBufSize, s_Secure2SegBufSize;

static BootSegmentData      *s_pBootSegBuffer;
static u32                  *s_pSecureSegBuffer;
static u32                  *s_pSecure2SegBuffer;

static CardBootData         s_cbData;
static SYSMRomEmuInfo       s_romEmuInfo;
static BOOL                 debuggerFlg;

static BOOL                 s_IsPulledOut = TRUE;
static BOOL                 s_isHotSwBusy = FALSE;

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
    // DS Card Type 1
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
    // DS Card Type 2
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
    // TWL Card Type 1
    { ReadIDSecure, ReadSegSecure, SwitchONPNGSecure, ChangeModeSecure},
    // RomEmulation
    {ReadIDSecure_ROMEMU, ReadSegSecure_ROMEMU, SwitchONPNGSecure_ROMEMU, ChangeModeSecure_ROMEMU}
};

#ifdef DHT_TEST
#include <twl/os/ARM7/debugLED.h>
#include <sysmenu/dht/dht.h>
DHTFile* dht;
static DHTPhase2Work* p2work = (void*)0x02e80000;
static BOOL ReadImage(void* dest, s32 offset, s32 length, void* arg)
{
    HotSwState retval;
    if ( offset % 512 )
    {
        static u8 page_buffer[512];
        u32 page_offset = (u32)(offset & -512);
        u32 buffer_offset = (u32)(offset % 512);
        u32 valid_length = 512 - buffer_offset;
        retval = ReadPageGame((CardBootData*)arg, page_offset, page_buffer, 512);
        if (retval != HOTSW_SUCCESS)
        {
            return FALSE;
        }
        MI_CpuCopy8(page_buffer + buffer_offset, dest, (length < valid_length ? length : valid_length));
        dest = (u8*)dest + valid_length;
        offset += valid_length;
        length -= valid_length;
        if ( length < 0)
        {
            return TRUE;
        }
    }
    retval = ReadPageGame((CardBootData*)arg, (u32)offset, dest, (u32)length);
    return (retval == HOTSW_SUCCESS);
}
#endif

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

#ifndef USE_LOCAL_KEYTABLE
    // 初期化後に他の用途でWRAM_0を使用できるようにローカルバッファへコピーしておく
    MI_CpuCopyFast((void *)HW_WRAM_0_LTD, &HotSwBlowfishInitTableBufDS, sizeof(BLOWFISH_CTX));
#endif
    // PXI初期化
    PXI_Init();
    PXI_SetFifoRecvCallback(PXI_FIFO_TAG_HOTSW, InterruptCallbackPxi);

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
            OS_PutString("Error - Can't Get Lock ID\n");
        }
        s_RscLockID = (u16)tempLockID;

        // カードアクセス用のロックIDの取得
        while((tempLockID = OS_GetLockID()) == OS_LOCK_ID_ERROR){
            OS_PutString("Error - Can't Get Lock ID\n");
        }
        s_CardLockID = (u16)tempLockID;
    }

    // カードデータロード用スレッドの生成
    OS_CreateThread(&HotSwThreadData.hotswThread,
                    HotSwThread,
                    NULL,
                    HotSwThreadData.hotswStack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );

    // カードの状態監視用スレッドの生成 ( DSテレビ対策 )
    // [TODO] 優先度の設定
    OS_CreateThread(&HotSwThreadData.monitorThread,
                    MonitorThread,
                    NULL,
                    HotSwThreadData.monitorStack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );

    // メッセージキューの初期化
    OS_InitMessageQueue( &HotSwThreadData.hotswQueue, &HotSwThreadData.hotswMsgBuffer[0], HOTSW_MSG_BUFFER_NUM );

    // メッセージキューの初期化
    OS_InitMessageQueue( &HotSwThreadData.hotswDmaQueue, &HotSwThreadData.hotswDmaMsgBuffer[0], HOTSW_DMA_MSG_NUM );

    // スレッド起動
    OS_WakeupThreadDirect(&HotSwThreadData.hotswThread);
    OS_WakeupThreadDirect(&HotSwThreadData.monitorThread);

    // Boot Segment バッファの設定
    HOTSW_SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );

    // Secure1 Segment バッファの設定
    HOTSW_SetSecureSegmentBuffer(HOTSW_MODE1, (void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );

    // Secure2 Segment バッファの設定
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
  Name:         LoadCardData

  Description:  カードからデータをロードする

  ※BootSegmentBuffer SecureSegmentBufferの設定を行ってから
  この関数を呼んでください。
 *---------------------------------------------------------------------------*/
static HotSwState LoadCardData(void)
{
    OSTick start;
    HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;
    u32 romMode = HOTSW_ROM_MODE_NULL;

    start = OS_GetTick();

    s_isHotSwBusy = TRUE;

    // カードのロック
#ifndef DEBUG_USED_CARD_SLOT_B_
    CARD_LockRom(s_CardLockID);
#else
    LockExCard(s_CardLockID);
#endif

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
                state  = ReadRomEmulationInfo(&s_romEmuInfo);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // 取得したRomエミュレーション情報を比較
                debuggerFlg = TRUE;
                if ( romEmuInfo->magic_code != SYSM_ROMEMU_INFO_MAGIC_CODE ){
                    debuggerFlg = FALSE;
                }
            }

            // 初回のRomエミュレーション情報を使用
            if(HOTSWi_IsRomEmulation()){
                OS_PutString("Read Emulation ROM\n");
                s_cbData.cardType = ROM_EMULATION;
                s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
                OS_TPrintf("SYSMi_GetWork()->gameCommondParam : 0x%08x\n", s_cbData.gameCommondParam);
            }
            else{
                s_cbData.gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param;
            }
            SYSMi_GetWork()->gameCommondParam = s_cbData.gameCommondParam;

            // ROMヘッダCRCを算出してチェック。NintendoロゴCRCも確認。
            SYSMi_GetWork()->cardHeaderCrc16_bak = SVC_GetCRC16( 65535, s_cbData.pBootSegBuf, 0x015e );
            OS_TPrintf( "RomHeaderCRC16 : calc = %04x  romh = %04x\n",
                        SYSMi_GetWork()->cardHeaderCrc16_bak, s_cbData.pBootSegBuf->rh.s.header_crc16 );

            if( ( SYSMi_GetWork()->cardHeaderCrc16_bak != s_cbData.pBootSegBuf->rh.s.header_crc16 ) ||
                ( 0xcf56 != s_cbData.pBootSegBuf->rh.s.nintendo_logo_crc16 ) ){
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_CRC_CHECK_ERROR : retval;
            }

            // アプリジャンプのデバッグ時にROMエミュレーション情報だけ必要な場合
            if(SYSMi_GetWork()->flags.hotsw.isLoadRomEmuOnly){
                SYSMi_GetWork()->flags.hotsw.isExistCard = TRUE;
                RegisterRomEmuInfo();
                // 排他制御ここまで
                UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
                goto end;
            }
        }

        if( retval == HOTSW_SUCCESS ) {
            // NTRカードかTWLカードか
#ifdef DEBUG_MODE
            if(s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset && s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset)
#else
            if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02)
#endif
            {
                OS_TPrintf("TWL Card.\n");
                s_cbData.twlFlg = TRUE;
            }
            else{
#ifdef DHT_TEST
                if ( !s_cbData.pBootSegBuf->rh.s.enable_nitro_whitelist_signature )
#endif
                // NTRカードの場合はRomHeaderバッファの1ページ目以降をクリアしておく。
                MI_CpuClearFast((void *)(SYSM_CARD_ROM_HEADER_BAK + PAGE_SIZE), SYSM_CARD_ROM_HEADER_SIZE - PAGE_SIZE);
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
                OS_PutString("Read Mode : TwlCard\n");

               // Mode2に移行する準備
                s_cbData.modeType = HOTSW_MODE2;

                // ---------------------- Reset ----------------------
                McPowerOff();
                McPowerOn();

                // ---------------------- Normal Mode ----------------------
                state  = ReadIDNormal(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // 先頭1Page分だけでOK。データは読み捨てバッファに
                state  = ReadBootSegNormal(&s_cbData);
                retval = (retval == HOTSW_SUCCESS) ? state : retval;

                // Key Table初期化
                MakeBlowfishTableDS(&s_cbData, 8);

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
            state  = s_funcTable[s_cbData.cardType].ChangeMode_S(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ---------------------- Game Mode ----------------------
            romMode = HOTSW_ROM_MODE_GAME;

            // ID読み込み
            state  = ReadIDGame(&s_cbData);
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // バナーファイルの読み込み
            state  = LoadBannerData();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // 排他制御ここまで(※CRCチェックまでにミスがなかったら、排他制御ここまで)
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            // カードIDの比較をして、一致しなければFALSEを返す
            if(s_cbData.id_scr != s_cbData.id_gam){
                retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;
                goto finalize;
            }

            // 常駐モジュール残りを指定先に転送
            state  = LoadStaticModule();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // ARM9常駐モジュールの先頭2KBの暗号化領域を複合化
            (void)DecryptObjectFile();

            // 認証コード読み込み＆ワーク領域にコピー
            state  = CheckCardAuthCode();
            retval = (retval == HOTSW_SUCCESS) ? state : retval;
        }
        else{
            // 排他制御ここまで(※CRCチェックまでにミスがあったら、ここで開放する)
            UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

            OS_TPrintf("ng... Card Data Load Skip ( state : %d )\n", retval);
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
    HOTSW_WaitDmaCtrl(HOTSW_NDMA_NO);

    // カードアクセス終了確認
    HOTSW_WaitCardCtrl();

    // カードのロック開放(※ロックIDは開放せずに持ち続ける)
#ifndef DEBUG_USED_CARD_SLOT_B_
    CARD_UnlockRom(s_CardLockID);
#else
    UnlockExCard(s_CardLockID);
#endif

    s_isHotSwBusy = FALSE;

//  OS_TPrintf( "Load Card Time : %dms\n\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         isTwlModeLoad

  Description:  カードのロードをTWLモードで行うかDSモードで行うかを決める


  ■ Bonding Op = 0 (製品版)
    ソフトウェア |  　　　 DSカード         |          TWLカード
    ------------------------------------------------------------------------
        DS用     |  DSカード読みシーケンス  |    DSカード読みシーケンス(※1)
       TWL用     |  不正カードフラグ立て    |    TWLカード読みシーケンス
    ハイブリット |  不正カードフラグ立て    |    TWLカード読みシーケンス

  ■ Bonding Op = 0以外 (開発用)
    ソフトウェア |  　　　 DSカード         |          TWLカード
    ------------------------------------------------------------------------
        DS用     |  DSカード読みシーケンス  |    DSカード読みシーケンス(※1)
       TWL用     |  DSカード読みシーケンス  |    TWLカード読みシーケンス
    ハイブリット |  DSカード読みシーケンス  |    TWLカード読みシーケンス


  ※1 [TODO] 動作に関してはカードGと相談して決める
 *---------------------------------------------------------------------------*/
static BOOL isTwlModeLoad(void)
{
    // TWLカード
    if(s_cbData.id_nml & HOTSW_ROMID_TWLROM_MASK){
        // PlatformCodeがTwl or Hybridの場合
        if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
            OS_PutString("TWL Card : TWL Application : Read Sequence -> TWL\n");
            return TRUE;
        }
        else{
            // [TODO] 仕様確認
            OS_PutString("TWL Card : NTR Application : Read Sequence -> NTR\n");
            return FALSE;
        }
    }
    // DSカード
    else{
        // 製品版の場合
        if(s_bondingOp == SCFG_OP_PRODUCT){
            // PlatformCodeがTwl or Hybridの場合
            if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
                OS_PutString("NTR Card : TWL Application : Illegal Card\n");
                s_cbData.illegalCardFlg = TRUE;
                return FALSE;
            }
            else{
                OS_PutString("NTR Card : NTR Application : Read Sequence -> NTR\n");
                return FALSE;
            }
        }
        // 開発用の場合
        else{
            OS_PutString("Bonding Option Development : NTR Card : Read Sequence -> NTR\n");
            return FALSE;
        }
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

    // PNG設定
    state  = s_funcTable[s_cbData.cardType].SetPNG_S(&s_cbData);
    retval = (retval == HOTSW_SUCCESS) ? state : retval;

    // DS側符号生成回路初期値設定 (レジスタ設定)
    SetMCSCR();

    // ID読み込み
    state  = s_funcTable[s_cbData.cardType].ReadID_S(&s_cbData);
    retval = (retval == HOTSW_SUCCESS) ? state : retval;

    // カードIDの比較をして、一致しなければFALSEを返す
    if(s_cbData.id_nml != s_cbData.id_scr){
        retval = (retval == HOTSW_SUCCESS) ? HOTSW_ID_CHECK_ERROR : retval;
    }

    if(retval == HOTSW_SUCCESS){
        // Secure領域のSegment読み込み
        state  = s_funcTable[s_cbData.cardType].ReadSegment_S(&s_cbData);
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
        }
    }

    return retval;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_IsRunOnDebugger

  Description:  ISデバッガ上で動作しているか？
 *---------------------------------------------------------------------------*/
BOOL HOTSWi_IsRunOnDebugger(void)
{
    return debuggerFlg;
}


/*---------------------------------------------------------------------------*
  Name:         HOTSWi_IsRomEmulation

  Description:  ROMをエミュレーションしているか？
 *---------------------------------------------------------------------------*/
BOOL HOTSWi_IsRomEmulation(void)
{
    return debuggerFlg &&
#ifndef DEBUG_USED_CARD_SLOT_B_
           s_romEmuInfo.isEnableSlot1;
#else
           s_romEmuInfo.isEnableSlot2;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         RegisterRomEmuInfo

  Description:  ROMエミュレーション情報を登録
 *---------------------------------------------------------------------------*/
static void RegisterRomEmuInfo(void)
{
    SYSM_work* sw = SYSMi_GetWork();

    // デバッガ情報
    if ( ! sw->flags.hotsw.is1stCardChecked && debuggerFlg )
    {
        MI_CpuCopy8( &s_romEmuInfo, &(sw->romEmuInfo), ROM_EMULATION_DATA_SIZE );
        sw->flags.hotsw.isOnDebugger = debuggerFlg;
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
  Name:         LoadStaticModule

  Description:  ARM7,9の常駐モジュールを展開する関数

  注：ゲームモードになってから呼び出してください
 *---------------------------------------------------------------------------*/
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
#ifdef DHT_TEST
    else
    {
        SVCHMACSHA1Context ctx;
        const u8* hash0;
        const u8* hash1;
        if ( !s_cbData.pBootSegBuf->rh.s.enable_nitro_whitelist_signature ) // ホワイトリストエントリ
        {
            const DHTDatabase* db;
            while (!dht)
            {
                OS_Sleep(1);
            }

            OS_TPrintf("Search DHT...");
            db = DHT_GetDatabase(dht, &s_cbData.pBootSegBuf->rh.s);
            if ( !db )
            {
                OS_TPrintf(" Failed.\n");
OS_SetDebugLED(0xFF);
while(1){ OS_WaitVBlankIntr(); }
                return HOTSW_HASH_CHECK_ERROR;
            }
            OS_TPrintf(" Done.\n");
            hash0 = db->hash[0];
            hash1 = db->hash[1];
        }
        else    // マスタリング済みエントリ
        {
            hash0 = s_cbData.pBootSegBuf->rh.s.nitro_whitelist_phase1_digest;
            hash1 = s_cbData.pBootSegBuf->rh.s.nitro_whitelist_phase2_diegst;
        }

        OS_TPrintf("DHT Pahse1...");
        DHT_CheckHashPhase1Init(&ctx, (ROM_Header_Short*)s_cbData.pBootSegBuf);
        if( s_cbData.pBootSegBuf->rh.s.main_size > SECURE_SEGMENT_SIZE )
        {
            DHT_CheckHashPhase1Update(&ctx, s_cbData.pSecureSegBuf, SECURE_SEGMENT_SIZE);
            DHT_CheckHashPhase1Update(&ctx, (u32 *)(s_cbData.arm9Stc + SECURE_SEGMENT_SIZE), s_cbData.pBootSegBuf->rh.s.main_size - SECURE_SEGMENT_SIZE );
        }
        else
        {
            DHT_CheckHashPhase1Update(&ctx, s_cbData.pSecureSegBuf, s_cbData.pBootSegBuf->rh.s.main_size);
        }

        DHT_CheckHashPhase1Update(&ctx, (u32 *)s_cbData.arm7Stc, s_cbData.pBootSegBuf->rh.s.sub_size);
        if ( !DHT_CheckHashPhase1Final(&ctx, hash0) )
        {
            OS_TPrintf(" Failed.\n");
OS_SetDebugLED(0xAA);
while(1){ OS_WaitVBlankIntr(); }
            return HOTSW_HASH_CHECK_ERROR;
        }
        OS_TPrintf(" Done.\n");

        OS_TPrintf("DHT Pahse2...");
        if ( !DHT_CheckHashPhase2(hash1, &s_cbData.pBootSegBuf->rh.s, p2work, ReadImage, &s_cbData) )
        {
            OS_TPrintf(" Failed.\n");
OS_SetDebugLED(0xCC);
while(1){ OS_WaitVBlankIntr(); }
            return HOTSW_HASH_CHECK_ERROR;
        }
        OS_TPrintf(" Done.\n");
    }
#endif

    return retval;
}


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
        OS_PutString("  ☆ Clone Boot Mode\n");
        SYSMi_GetWork()->cloneBootMode = SYSM_CLONE_BOOT_MODE;
    }else {
        OS_PutString("  □ Other Boot Mode\n");
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
    SDK_ASSERT(size > SECURE_SEGMENT_SIZE);

    if(type == HOTSW_MODE1){
        s_pSecureSegBuffer = (u32 *)buf;
        s_SecureSegBufSize = size;

        s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

        // バッファの初期化
        MI_CpuClear8(s_pSecureSegBuffer, size);
    }
    else{
        s_pSecure2SegBuffer = (u32 *)buf;
        s_Secure2SegBufSize = size;

        s_cbData.pSecure2SegBuf = s_pSecure2SegBuffer;

        // バッファの初期化
        MI_CpuClear8(s_pSecure2SegBuffer, size);
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

    // 乱数を初期化
    // チック＆RTC初回ロード値を種とする。
    // （起動する度に変化するパラメータと組み合わせる。
    //   Vカウンタは2130サイクル変化しないので固定値になりやすい。）
    MATH_InitRand32(&rnd, OS_GetTick() ^ sw->Rtc1stData.words[0] ^ sw->Rtc1stData.words[1]);

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

  注：セキュア領域を読み込んでからこの関数を呼び出してください
 *---------------------------------------------------------------------------*/
static u32 encDestBuf[ENCRYPT_DEF_SIZE/sizeof(u32)];

static HotSwState DecryptObjectFile(void)
{
    u8  i;
    s32 restSize;
    s32 size                = (s32)s_cbData.pBootSegBuf->rh.s.main_size;
    u32 *pEncBuf            = encDestBuf;
    u32 *pEncDes            = s_cbData.pSecureSegBuf;
    BLOWFISH_CTX *tableBufp = &s_cbData.keyTable;
    BOOL exist              = TRUE;
    HotSwState retval       = HOTSW_SUCCESS;

    if (size > ENCRYPT_DEF_SIZE) {
        size = ENCRYPT_DEF_SIZE;
    }
    restSize = size;

    // 読み込んだセキュア領域をバッファから一時バッファにコピー
    MI_CpuCopy32(s_cbData.pSecureSegBuf, pEncBuf, (u32)size);

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
        retval = HOTSW_DATA_DECRYPT_ERROR;

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
#ifndef DEBUG_USED_CARD_SLOT_B_
    u32 mask = (u32)(REG_MI_MC_SL1_CDET_MASK << GetMcSlotShift());
#else
    u32 mask = (u32)(REG_MI_MC_SL2_CDET_MASK >> GetMcSlotShift());
#endif

    if( !(reg_MI_MC1 & mask) ){
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
  Name:         IsSwap

  Description:  SCFG_MC1のSWPフラグを見て、スロットがスワップされているか判定する
 *---------------------------------------------------------------------------*/
static BOOL IsSwap(void)
{
    if( reg_MI_MC1 & REG_MI_MC1_SWP_MASK ){
        return TRUE;
    }
    else{
        return FALSE;
    }
}


/*---------------------------------------------------------------------------*
  Name:         GetMcSlotShift

  Description:  カードスロットのシフトビット数の取得
 *---------------------------------------------------------------------------*/
static u32 GetMcSlotShift(void)
{
    return (u32)(IsSwap() * REG_MI_MC_SL2_CDET_SHIFT);
}


/*---------------------------------------------------------------------------*
  Name:         GetMcSlotMask

  Description:  カードスロットのシフトビット数の取得
 *---------------------------------------------------------------------------*/
static u32 GetMcSlotMask(void)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    return (u32)(REG_MI_MC_SL1_MODE_MASK << GetMcSlotShift());
#else
    return (u32)(REG_MI_MC_SL2_MODE_MASK >> GetMcSlotShift());
#endif
}


/*---------------------------------------------------------------------------*
  Name:         GetMcSlotMode

  Description:  スロットの現在のモードを返す
 *---------------------------------------------------------------------------*/
static u32 GetMcSlotMode(void)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    return (reg_MI_MC1 & GetMcSlotMask()) >> GetMcSlotShift();
#else
    return (reg_MI_MC1 & GetMcSlotMask()) << GetMcSlotShift();
#endif
}


/*---------------------------------------------------------------------------*
  Name:         SetMcSlotMode

  Description:  カードスロットのモード設定
 *---------------------------------------------------------------------------*/
static void SetMcSlotMode(u32 mode)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~GetMcSlotMask()) | (mode << GetMcSlotShift()));
#else
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~GetMcSlotMask()) | (mode >> GetMcSlotShift()));
#endif
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
        // [TODO:]待ち時間は暫定値。金子さんに数値を測定してもらう。
        // VDDの安定期間待ち
        OS_Sleep(100);

        // SCFG_MC1 の Slot Status の M1,M0 を 01 にする
        SetMcSlotMode(SLOT_STATUS_MODE_01);
        // 10ms待ち
        OS_Sleep(10);

        // SCFG_MC1 の Slot Status の M1,M0 を 10 にする
        SetMcSlotMode(SLOT_STATUS_MODE_10);

        // [TODO:]待ち時間は暫定値。金子さんに数値を測定してもらう。
        // RESBを上げるまでの待ち時間
        OS_Sleep(1);

        // リセットをhighに (RESB = 1にする)
        reg_HOTSW_MCCNT1 = RESB_MASK;

        // [TODO:]待ち時間は暫定値。金子さんに数値を測定してもらう。
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
        OS_SpinWait( OS_MSEC_TO_CPUCYC(1) );
    }

    if ( (reg_MI_MC & mask) == (0x0 << shift) )
    {
        reg_MI_MC = (u16)((reg_MI_MC & ~mask) | (0x1 << shift));
        // VDDの安定期間
        OS_SpinWait( OS_MSEC_TO_CPUCYC(100) );

        reg_MI_MC = (u16)((reg_MI_MC & ~mask) | (0x2 << shift));
        // RESBのLow期間
        OS_SpinWait( OS_MSEC_TO_CPUCYC(1) );

        reg_MI_MCCNT1_B = REG_MI_MCCNT1_RESB_MASK;

        // RESBのHigh期間
        OS_SpinWait( OS_MSEC_TO_CPUCYC(100) );
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

  [TODO:]挿抜のフロー・フラグケアetcの確認(今の所、抜き挿ししてもタイトルが更新されない)
 *---------------------------------------------------------------------------*/
static void HotSwThread(void *arg)
{
    #pragma unused( arg )

    HotSwState      retval;
    HotSwMessage    *msg;

    while(1){
        OS_ReceiveMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);

        if( msg->ctrl == TRUE ) {
            // [TODO]とりあえず、ここでHOTSWを抑制した時点でisExistCardがFALSEなら、HOTSWのFinalizeをするようにする。
            SYSMi_GetWork()->flags.hotsw.isEnableHotSW = msg->value;
            // [TODO]カードがあるときとないときで場合分けしてFinalize処理を実装
            //       PXIメッセージを「抑制」と「Finalize」で分けて処理
            //          → １．全てのレジスタをクリアする           (カードがささっていない時)
            //             ２．一度電源を落としてNomalモードにする  (NANDアプリ等を起動する場合)
            //             ３．必要なレジスタを残して、後はクリア   (ささっているカードを起動する場合)
            //
            // NANDアプリ起動時の終了処理が確認できたら、、BOOTAPIでKillThreadでスレッドを殺せるようにしておく。
            // スレッドを殺す前に、IREQとDET割り込みを無効にしておく。
//          HOTSW_Finalize();
        }

        while(1){
            if( !SYSMi_GetWork()->flags.hotsw.isEnableHotSW ) {
                SYSMi_GetWork()->flags.hotsw.is1stCardChecked  = TRUE;
                OS_PutString("### HotSw is restrained...\n");
                break;
            }

            if(HOTSW_IsCardExist()){
                if(!s_IsPulledOut){
                    if(GetMcSlotMode() == SLOT_STATUS_MODE_10){
                        LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                        SYSMi_GetWork()->flags.hotsw.isExistCard         = TRUE;
                        SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;

                        SYSMi_GetWork()->nCardID = s_cbData.id_gam;

                        SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = TRUE;

                        UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);

                        OS_PutString("ok!\n");

                        break;
                    }
                }

                retval = LoadCardData();

                DebugPrintErrorMessage(retval);

                if(retval != HOTSW_SUCCESS){
                    McPowerOff();

                    ClearCaradFlgs();

                    s_IsPulledOut = TRUE;

                    break;
                }

                s_IsPulledOut = FALSE;
            }

            // カードが抜けてたら
            else{
                ClearCaradFlgs();

                MI_CpuClear32(&s_cbData, sizeof(CardBootData));

                MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
                MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);
                MI_CpuClearFast((u32 *)SYSM_CARD_BANNER_BUF, sizeof(TWLBannerFile));

                s_IsPulledOut = TRUE;

                // ワンセグのスリープ時シャットダウン対策を戻す
                MCU_EnableDeepSleepToPowerLine( MCU_PWR_LINE_33, TRUE );

                break;
            }
        }
        SYSMi_GetWork()->flags.hotsw.is1stCardChecked  = TRUE;
    } // while loop
}


/*---------------------------------------------------------------------------*
  Name:        ClearCaradFlgs

  Description: カードデータバッファのバッファとフラグの後始末
 *---------------------------------------------------------------------------*/
static void ClearCaradFlgs(void)
{
    // フラグ処理
    LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
    SYSMi_GetWork()->flags.hotsw.isExistCard         = FALSE;
    SYSMi_GetWork()->flags.hotsw.isValidCardBanner   = FALSE;
    SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
    SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = FALSE;
    UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
}


/*---------------------------------------------------------------------------*
  [TODO:] Finalize処理の前にやるべきこと

・NANDアプリ起動時、カード電源OFF

・TWL NANDアプリなら
        ゲームカードONフラグ(NANDのRomHeader TWL拡張諸フラグ)を見て、
        NANDアプリ起動後もカード電源(OFF後)ONにしてNormalモードにする

・NTR NANDアプリなら
        カード種別問わず、GAMEモードに遷移(NTR互換のRomHeaderのみ読む)
        拡張領域残しておいてもよいかも
        TWLカードがささっていて、Game2モードになっていたら、
        電源OFFしてONして、Gameモードに遷移させる。
 *---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*
  Name:        MonitorThread

  Description: 実際のカード状態とHotSwThreadで状態を比べて、違いがあった場合は
               メッセージを送る

  s_IsPulledOut : True  -> カードなし       HOTSW_IsCardExist : True  -> カードあり
                  False -> カードあり                           False -> カードなし
 *---------------------------------------------------------------------------*/
static void MonitorThread(void *arg)
{
    #pragma unused( arg )

    BOOL isPullOutNow;

    while(1){
        // カードデータロード中は待機
        do{
            OS_Sleep(CARD_EXIST_CHECK_INTERVAL);
        }
        while(s_isHotSwBusy);

        // 現在カードが抜けているか
        isPullOutNow = !HOTSW_IsCardExist();

        // 状態の比較
        if(s_IsPulledOut != isPullOutNow){
            OSIntrMode enabled = OS_DisableInterrupts();

            // 本当は抜けてた場合
            if(isPullOutNow){
                HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].ctrl  = FALSE;
                HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].value = 0;
                HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].type  = HOTSW_PULLOUT;

                // メッセージをキューの先頭に入れる
                OS_JamMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

                // メッセージインデックスをインクリメント
                HotSwThreadData.idx_pulledOut = (HotSwThreadData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

                OS_PutString(">>> Card State Error : PulledOut\n");
            }

            // 本当は挿さっていた場合
            else{
                HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].ctrl  = FALSE;
                HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].value = 0;
                HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert].type  = HOTSW_INSERT;

                // メッセージをキューの先頭に入れる
                OS_JamMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswInsertMsg[HotSwThreadData.idx_insert], OS_MESSAGE_NOBLOCK);

                // メッセージインデックスをインクリメント
                HotSwThreadData.idx_insert = (HotSwThreadData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;

                OS_PutString(">>> Card State Error : Insert\n");
            }

            (void)OS_RestoreInterrupts( enabled );
        }
    }
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackCard

  Description: カードB抜け割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCard(void)
{
    // スロット電源ON時は強制OFF
    // （エミュレーションROMは旧コネクタでの強制OFFでデータが化けることがある）
#ifndef HOTSW_DISABLE_FORCE_CARD_OFF
//    if ( ! HOTSWi_IsRomEmulation() )
    {
        u32 mode = GetMcSlotMode();
        if(mode == SLOT_STATUS_MODE_01 || mode == SLOT_STATUS_MODE_10){
            SetMcSlotMode(SLOT_STATUS_MODE_11);
        }
        OS_TPrintf("slot status: %x\n", mode);
    }
#endif

    HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].ctrl  = FALSE;
    HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].value = 0;
    HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut].type  = HOTSW_PULLOUT;

    // メッセージ送信
    OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPulledOutMsg[HotSwThreadData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

    // メッセージインデックスをインクリメント
    HotSwThreadData.idx_pulledOut = (HotSwThreadData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

    OS_PutString("○\n");
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

    OS_PutString("●\n");
}


/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackPxi

  Description:  PXI割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)
#pragma unused(err)
    HotSwPxiMessage d;

    d.data = data;

    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].ctrl  = (d.msg.ctrl) ? TRUE : FALSE;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].value = d.msg.value;
    HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl].type  = HOTSW_CONTROL;

    // メッセージ送信
    OS_SendMessage(&HotSwThreadData.hotswQueue, (OSMessage *)&HotSwThreadData.hotswPxiMsg[HotSwThreadData.idx_ctrl], OS_MESSAGE_NOBLOCK);

    // メッセージインデックスをインクリメント
    HotSwThreadData.idx_ctrl = (HotSwThreadData.idx_ctrl+1) % HOTSW_CTRL_MSG_NUM;
}


/*---------------------------------------------------------------------------*
  Name:         AllocateExCardBus

  Description:  アクセス権を設定する
 *---------------------------------------------------------------------------*/
#ifdef  SDK_ARM9
static inline void SetExCardProcessor(MIProcessor proc)
{
    reg_HOTSW_EXMEMCNT =
        (u16)((reg_HOTSW_EXMEMCNT & ~HOTSW_EXMEMCNT_SELB_MASK) | (proc << HOTSW_EXMEMCNT_SELB_SHIFT));
}
#endif


/*---------------------------------------------------------------------------*
  Name:         AllocateExCardBus

  Description:  スロットBへのアクセス権を設定する
 *---------------------------------------------------------------------------*/
static void AllocateExCardBus(void)
{
#ifdef  SDK_ARM9
    // preset reset flag with status of disable interrupts in OSi_DoTryLockByWord
    if ( ! ( reg_MI_MC & REG_MI_MC_SL2_CDET_MASK ) )
    {
        reg_MI_MCCNT1 |= REG_MI_MCCNT2_RESB_MASK;
    }
    SetExCardProcessor(MI_PROCESSOR_ARM9);    // Arm9側で動作している場合
#endif
}


/*---------------------------------------------------------------------------*
  Name:         FreeExCardBus

  Description:  スロットBへのアクセス権を設定する
 *---------------------------------------------------------------------------*/
static void FreeExCardBus(void)
{
#ifdef  SDK_ARM9
    SetExCardProcessor(MI_PROCESSOR_ARM7);    // Card for SUB
#endif
}


/*---------------------------------------------------------------------------*
  Name:         LockSlotB

  Description:  スロットBをロックする
 *---------------------------------------------------------------------------*/
static s32 LockExCard(u16 lockID)
{
    return OS_LockByWord(lockID, (OSLockWord *)SLOT_B_LOCK_BUF, AllocateExCardBus);
}


/*---------------------------------------------------------------------------*
  Name:         UnlockSlotB

  Description:  スロットBをロックする
 *---------------------------------------------------------------------------*/
static s32 UnlockExCard(u16 lockID)
{
    return OS_UnlockByWord(lockID, (OSLockWord *)SLOT_B_LOCK_BUF, FreeExCardBus);
}


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
#ifndef DEBUG_USED_CARD_SLOT_B_
    SetInterruptCallback( OS_IE_CARD_A_IREQ , InterruptCallbackCard );
    SetInterruptCallback( OS_IE_CARD_A_DET  , InterruptCallbackCardDet );
    (void)OS_EnableIrqMask(OS_IE_NDMA2);
#else
    SetInterruptCallback( OS_IE_CARD_B_IREQ , InterruptCallbackCard );
    SetInterruptCallback( OS_IE_CARD_B_DET  , InterruptCallbackCardDet );
    (void)OS_EnableIrqMask(OS_IE_NDMA2);
#endif
}


#include <twl/os/common/systemCall.h>
/*---------------------------------------------------------------------------*
  Name:         CheckStaticModuleHash

  Description:  常駐モジュールのハッシュチェックを行う
 *---------------------------------------------------------------------------*/
static HotSwState CheckStaticModuleHash(void)
{
    BOOL flg = TRUE;

    // Arm9常駐モジュール Hash値のチェック
    if(!CheckArm9HashValue()){
        flg = FALSE;
        OS_PutString("×Arm9 Static Module Hash Check Error...\n");
    }

    // Arm7常駐モジュール Hash値のチェック
    if(!CheckArm7HashValue()){
        flg = FALSE;
        OS_PutString("×Arm7 Static Module Hash Check Error...\n");
    }

    // Arm9拡張常駐モジュール Hash値のチェック
    if(!CheckExtArm9HashValue()){
        flg = FALSE;
        OS_PutString("×Arm9 Ltd Static Module Hash Check Error...\n");
    }

    // Arm7拡張常駐モジュール Hash値のチェック
    if(!CheckExtArm7HashValue()){
        flg = FALSE;
        OS_PutString("×Arm7 Ltd Static Module Hash Check Error...\n");
    }

    if(flg){
        OS_PutString("*** Static Module Load was Completed!!\n");
    }

    return flg ? HOTSW_SUCCESS : HOTSW_HASH_CHECK_ERROR;
}


/*---------------------------------------------------------------------------*
  Name:         CheckArm7HashValue

  Description:  Arm7常駐モジュールのハッシュチェック
 *---------------------------------------------------------------------------*/
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


/*---------------------------------------------------------------------------*
  Name:         CheckArm9HashValue

  Description:  Arm9常駐モジュールのハッシュチェック

  ※ 先頭2Kの復号化が行われる前のデータのハッシュを比べる
 *---------------------------------------------------------------------------*/
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


/*---------------------------------------------------------------------------*
  Name:         CheckExtArm7HashValue

  Description:  Arm7拡張常駐モジュールのハッシュチェック
 *---------------------------------------------------------------------------*/
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


/*---------------------------------------------------------------------------*
  Name:         CheckExtArm9HashValue

  Description:  Arm9拡張常駐モジュールのハッシュチェック
 *---------------------------------------------------------------------------*/
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






// **************************************************************************
//
//                             Debug用表示関数
//
// **************************************************************************
/*---------------------------------------------------------------------------*
  Name:         ShowRomHeaderData

  Description:
 *---------------------------------------------------------------------------*/
static void ShowRomHeaderData(void)
{
    OS_TPrintf("\nDebug Data -------------------------------\n");
    OS_TPrintf("1. Normal Mode ID  : 0x%08x\n"  , s_cbData.id_nml);
    OS_TPrintf("2. Secure Mode ID  : 0x%08x\n"  , s_cbData.id_scr);
    OS_TPrintf("3. Game   Mode ID  : 0x%08x\n"  , s_cbData.id_gam);

    OS_TPrintf("title Name         : %s\n",     s_pBootSegBuffer->rh.s.title_name);
    OS_TPrintf("initial Code       : %x\n\n",   *(u32 *)s_pBootSegBuffer->rh.s.game_code);

    OS_TPrintf("platform Code      : 0x%02x\n\n", s_cbData.pBootSegBuf->rh.s.platform_code);

    OS_TPrintf("main rom offset    : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_rom_offset);
    OS_TPrintf("main entry addr    : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_entry_address);
    OS_TPrintf("main ram   addr    : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_ram_address);
    OS_TPrintf("main size          : 0x%08x\n\n", s_cbData.pBootSegBuf->rh.s.main_size);

    OS_TPrintf("sub  rom offset    : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_rom_offset);
    OS_TPrintf("sub  entry addr    : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_entry_address);
    OS_TPrintf("sub  ram   addr    : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_ram_address);
    OS_TPrintf("sub  size          : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_size);

    if(s_cbData.twlFlg){
    OS_TPrintf("\nLtd main rom offset: 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset);
    OS_TPrintf("Ltd main ram   addr: 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address);
    OS_TPrintf("Ltd main size      : 0x%08x\n\n", s_cbData.pBootSegBuf->rh.s.main_ltd_size);

    OS_TPrintf("Ltd Sub rom offset : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset);
    OS_TPrintf("Ltd Sub ram   addr : 0x%08x\n"  , s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address);
    OS_TPrintf("Ltd Sub size       : 0x%08x\n", s_cbData.pBootSegBuf->rh.s.sub_ltd_size);
    }

    OS_TPrintf("------------------------------------------\n\n");
}

/*---------------------------------------------------------------------------*
  Name:         ShowRegisterData

  Description:
 *---------------------------------------------------------------------------*/
static void ShowRegisterData(void)
{
    OS_TPrintf("----------------------------------------------------------\n");
    OS_TPrintf("拡張機能制御レジスタ         (MC_B(d24))   : %08x\n", reg_SCFG_EXT);
    OS_TPrintf("MC I/F制御レジスタ１         (slot status) : %08x\n", reg_MI_MC1);
    OS_TPrintf("MC I/F制御レジスタ２         (Counter-A)   : %04x\n", reg_MI_MC2);
    OS_TPrintf("MC コントロールレジスタ0     (SEL etc)     : %04x\n", reg_HOTSW_MCCNT0);
    OS_TPrintf("MC コントロールレジスタ1     (START etc)   : %08x\n", reg_HOTSW_MCCNT1);
    OS_TPrintf("----------------------------------------------------------\n");
}

/*---------------------------------------------------------------------------*
  Name:         DebugPrintErrorMessage

  Description:
 *---------------------------------------------------------------------------*/
static void DebugPrintErrorMessage(HotSwState state)
{
    switch(state){
      case HOTSW_SUCCESS:
        OS_PutString("   - Success\n");
        break;

      case HOTSW_TIME_OUT:
        OS_PutString("   - Error 1 : TimeOut\n");
        break;

      case HOTSW_CARD_LOCK_ERROR:
        OS_PutString("   - Error 2 : Slot Lock\n");
        break;

      case HOTSW_CRC_CHECK_ERROR:
        OS_PutString("   - Error 3 : CRC Check\n");
        break;

      case HOTSW_HASH_CHECK_ERROR:
        OS_PutString("   - Error 4 : Hash Check\n");
        break;

      case HOTSW_ID_CHECK_ERROR:
        OS_PutString("   - Error 5 : ID Check\n");
        break;

      case HOTSW_PULLED_OUT_ERROR:
        OS_PutString("   - Error 6 : Pulled Out\n");
        break;

      case HOTSW_DATA_DECRYPT_ERROR:
        OS_PutString("   - Error 7 : Data Decrypt\n");
        break;

      case HOTSW_BUFFER_OVERRUN_ERROR:
        OS_PutString("   - Error 8 : Buffer OverRun\n");
        break;

      case HOTSW_UNEXPECTED_ERROR:
        OS_PutString("   - Error 9 : Unexpected\n");
        break;

      default :
        OS_PutString("   - illigal Error\n");
        break;
    }
}