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
#include 	<twl.h>
#include 	<twl/os/common/format_rom.h>
#include	<nitro/card/types.h>
#include	<sysmenu.h>
#include 	<hotswTypes.h>
#include	<blowfish.h>
#include	<dsCardCommon.h>
#include	<romEmulation.h>
#include	<customNDma.h>
#include 	<../build/libraries/mb/common/include/mb_fileinfo.h>

#define DEBUG_MODE

// define -------------------------------------------------------------------
#define		CHATTERING_COUNTER					0x600
#define		COUNTER_A							0x100

#define 	UNDEF_CODE							0xe7ffdeff	// 未定義コード
#define 	ENCRYPT_DEF_SIZE					0x800		// 2KB  ※ ARM9常駐モジュール先頭2KB

#define		DIGEST_HASH_BLOCK_SIZE_SHA1			(512/8)

#define 	SLOT_B_LOCK_BUF						HW_CTRDG_LOCK_BUF

#ifdef SDK_ARM9
#define 	reg_HOTSW_EXMEMCNT		 			reg_MI_EXMEMCNT
#define		HOTSW_EXMEMCNT_SELB_MASK			REG_MI_EXMEMCNT_MPB_MASK
#define		HOTSW_EXMEMCNT_SELB_SHIFT 			REG_MI_EXMEMCNT_MPB_SHIFT
#else  // SDK_ARM7
#define 	reg_HOTSW_EXMEMCNT		 			reg_MI_EXMEMCNT_L
#define		HOTSW_EXMEMCNT_SELB_MASK			0x0400
#define		HOTSW_EXMEMCNT_SELB_SHIFT 			10								 
#endif

// Function prototype -------------------------------------------------------
static BOOL IsSwap(void);
static u32 GetMcSlotShift(void);
static u32 GetMcSlotMask(void);
static void SetMcSlotMode(u32 mode);
static BOOL CmpMcSlotMode(u32 mode);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCard(void);
static void InterruptCallbackCardDet(void);
static void InterruptCallbackNDma(void);
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err);

static void LockHotSwRsc(OSLockWord* word);
static void UnlockHotSwRsc(OSLockWord* word);

static void McThread(void *arg);
static void McPowerOn(void);
static void McPowerOff(void);
static void SetMCSCR(void);

static void GenVA_VB_VD(void);
static HotSwState DecryptObjectFile(void);
static HotSwState LoadBannerData(void);
static HotSwState LoadStaticModule(void);
static HotSwState LoadCardData(void);
static HotSwState CheckCardAuthCode(void);

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

static char 				encrypt_object_key[] ATTRIBUTE_ALIGN(4) = "encryObj";
static char					rom_emu_info[] ATTRIBUTE_ALIGN(4)	    = "TWLD";

static u16					s_RscLockID;
static u16					s_CardLockID;

static u32					s_BootSegBufSize, s_SecureSegBufSize, s_Secure2SegBufSize;

static BootSegmentData		*s_pBootSegBuffer;		// カード抜けてもバッファの場所覚えとく
static u32					*s_pSecureSegBuffer;	// カード抜けてもバッファの場所覚えとく
static u32					*s_pSecure2SegBuffer;	// カード抜けてもバッファの場所覚えとく

static CardBootData			s_cbData;

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

static CardBootFunction  	s_funcTable[] = {
	// DS Card Type 1
    {				ReadBootSegNormal, 	ChangeModeNormal,						// Normalモード関数
     ReadIDSecure, 	ReadSegSecure, 	  	SwitchONPNGSecure, ChangeModeSecure,	// Secureモード関数
     ReadIDGame,    ReadPageGame},												// Game  モード関数
	// DS Card Type 2
    {				ReadBootSegNormal, 	ChangeModeNormal,						// Normalモード関数
     ReadIDSecure, 	ReadSegSecure, 	  	SwitchONPNGSecure, ChangeModeSecure,	// Secureモード関数
     ReadIDGame,    ReadPageGame},												// Game  モード関数
	// TWL Card Type 1
    {				ReadBootSegNormal, 	ChangeModeNormal,						// Normalモード関数
     ReadIDSecure, 	ReadSegSecure, 	  	SwitchONPNGSecure, ChangeModeSecure,	// Secureモード関数
     ReadIDGame,    ReadPageGame},												// Game  モード関数
	// RomEmulation
    {				ReadBootSegNormal, 	ChangeModeNormal,											// Normalモード関数
     ReadIDSecure_ROMEMU, ReadSegSecure_ROMEMU, SwitchONPNGSecure_ROMEMU, ChangeModeSecure_ROMEMU,	// Secureモード関数
     ReadIDGame,    ReadPageGame},																	// Game  モード関数
};

// Global Values ------------------------------------------------------------
BLOWFISH_CTX 				GCDi_BlowfishInitTableBufDS;
CardThreadData				s_ctData;

#include <twl/ltdwram_end.h>

// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         HOTSW_Init
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void HOTSW_Init(u32 threadPrio)
{
	OS_InitTick();
    OS_InitThread();

#ifndef USE_LOCAL_KEYTABLE
    // 初期化後に他の用途でWRAM_0を使用できるようにローカルバッファへコピーしておく
    MI_CpuCopyFast((void *)HW_WRAM_0_LTD, &GCDi_BlowfishInitTableBufDS, sizeof(BLOWFISH_CTX));
#endif
    // PXI初期化
	PXI_Init();
	PXI_SetFifoRecvCallback(PXI_FIFO_TAG_HOTSW, InterruptCallbackPxi);
    
	// 割り込みマスクの設定
	SetInterrupt();

    // 割り込みの有効化
    (void)OS_EnableIrq();
    (void)OS_EnableInterrupts();

#ifdef SDK_ARM7
	// チャッタリングカウンタの値を設定
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~REG_MI_MC1_CC_MASK) |
                       (CHATTERING_COUNTER << REG_MI_MC1_CC_SHIFT));

	// Counter-Aの値を設定
    reg_MI_MC2 = COUNTER_A;
#else
    // PXI経由でARM7にチャッタリングカウンタ・カウンタAの値を設定してもらう。設定されるまで待つ。

#endif
    
	// カードブート用構造体の初期化
	MI_CpuClear8(&s_cbData, sizeof(CardBootData));

    // カードスレッド用構造体の初期化
	MI_CpuClear8(&s_ctData, sizeof(CardThreadData));

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
    
	// カードブート用スレッドの生成
	OS_CreateThread(&s_ctData.thread,
                    McThread,
                    NULL,
                    s_ctData.stack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    threadPrio
                    );

    // メッセージキューの初期化
	OS_InitMessageQueue( &s_ctData.hotswQueue, &s_ctData.hotswMsgBuffer[0], HOTSW_MSG_BUFFER_NUM );

    // メッセージキューの初期化
	OS_InitMessageQueue( &s_ctData.hotswDmaQueue, &s_ctData.hotswDmaMsgBuffer[0], HOTSW_DMA_MSG_NUM );
    
    // スレッド起動
    OS_WakeupThreadDirect(&s_ctData.thread);

    // Boot Segment バッファの設定
	HOTSW_SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );

    // Secure1 Segment バッファの設定
    HOTSW_SetSecureSegmentBuffer(HOTSW_MODE1, (void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );

    // Secure2 Segment バッファの設定
    HOTSW_SetSecureSegmentBuffer(HOTSW_MODE2, (void *)SYSM_CARD_TWL_SECURE_BUF, SECURE_AREA_SIZE );
    
    // カードが挿さってあったらスレッドを起動する
	if(HOTSW_IsCardExist()){
		// メッセージ送信
    	OS_SendMessage(&s_ctData.hotswQueue, (OSMessage)&s_ctData.hotswInsertMsg[s_ctData.idx_insert], OS_MESSAGE_NOBLOCK);

        // メッセージインデックスをインクリメント
        s_ctData.idx_insert = (s_ctData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;
	}
    else{
		SYSMi_GetWork()->flags.hotsw.is1stCardChecked  = TRUE;
    }
}

/* -----------------------------------------------------------------
 * LoadCardData関数
 *
 * カードからデータをロードする
 *
 * ※BootSegmentBuffer SecureSegmentBufferの設定を行ってから
 *   この関数を呼んでください。
 * ----------------------------------------------------------------- */
static HotSwState LoadCardData(void)
{
	OSTick start;
	HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;
    u32 romMode = HOTSW_ROM_MODE_NULL;

    start = OS_GetTick();

	// カードのロック
#ifndef DEBUG_USED_CARD_SLOT_B_
	CARD_LockRom(s_CardLockID);
#else
	LockExCard(s_CardLockID);
#endif

    // カード電源リセット
#ifdef SDK_ARM7
	McPowerOff();
	McPowerOn();
#else // SDK_ARM9
	// ARM7にPXI経由でカード電源ONをお願い。ONになるまで待つ。
    
#endif
    
	// バッファを設定
    s_cbData.pBootSegBuf   = s_pBootSegBuffer;
    s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

    // ロード処理開始
	if(HOTSW_IsCardAccessible()){
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
			u8 i;
            u8 *romEmuInf = (u8 *)s_cbData.romEmuBuf;
            
			// バナーリードが完了して、フラグ処理が終わるまでARM9と排他制御する
            LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
			
	    	// Boot Segment読み込み
	    	state  = s_funcTable[s_cbData.cardType].ReadBootSegment_N(&s_cbData);
			retval = (retval == HOTSW_SUCCESS) ? state : retval;
            
            // Romエミュレーション情報を取得
			state  = ReadRomEmulationData(&s_cbData);
			retval = (retval == HOTSW_SUCCESS) ? state : retval;

            // 取得したRomエミュレーション情報を比較
            s_cbData.debuggerFlg = TRUE;
            for(i=0; i<4; i++){
                if ( rom_emu_info[i] != romEmuInf[i] ){
					s_cbData.debuggerFlg = FALSE;
                    break;
                }
            }
            if(s_cbData.debuggerFlg){
				OS_PutString("Read Debugger\n");
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
                // NTRカードの場合はRomHeaderバッファの1ページ目以降をクリアしておく。
                MI_CpuClearFast((void *)(SYSM_CARD_ROM_HEADER_BAK + PAGE_SIZE), SYSM_CARD_ROM_HEADER_SIZE - PAGE_SIZE);
            }

            // SecureコマンドのPNG_ONコマンドetc用のレイテンシを求める(Latency1とLatency2を足す)
			s_cbData.secureLatency = AddLatency2ToLatency1(s_cbData.pBootSegBuf->rh.s.secure_cmd_param);

	    	// Key Table初期化
	    	GCDm_MakeBlowfishTableDS(&s_cbData, 8);

			// コマンド認証値・コマンドカウンタ初期値・PNジェネレータ初期値の生成
            GenVA_VB_VD();

	    	// セキュアモードに移行
	    	state  = ChangeModeNormal(&s_cbData);
			retval = (retval == HOTSW_SUCCESS) ? state : retval;

	    	// ---------------------- Secure Mode ----------------------
			romMode = HOTSW_ROM_MODE_SECURE;

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
				goto end;
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

end:
	if( retval == HOTSW_SUCCESS )
	{
        // バッドブロックを置換
		state = HOTSWi_RefreshBadBlock(romMode);
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
	}

    // カードDMA終了確認
    while( MI_IsNDmaBusy(HOTSW_NDMA_NO) == TRUE ){}

    // カードアクセス終了確認
	while( reg_HOTSW_MCCNT1 & REG_MI_MCCNT1_START_MASK ){}

	// カードのロック開放(※ロックIDは開放せずに持ち続ける)
#ifndef DEBUG_USED_CARD_SLOT_B_
	CARD_UnlockRom(s_CardLockID);
#else
	UnlockExCard(s_CardLockID);
#endif

//	OS_TPrintf( "Load Card Time : %dms\n\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

    return retval;
}

/* -----------------------------------------------------------------
 * HOTSWi_RefreshBadBlock関数
 *
 * ノーマルモードまたはゲームモードでバッドブロックを置換
 * ----------------------------------------------------------------- */
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

/* -----------------------------------------------------------------
 * HOTSW_GetRomEmulationBuffer関数
 *
 * Romエミュレーション情報を格納しているバッファへのポインタを返す
 * ----------------------------------------------------------------- */
void* HOTSW_GetRomEmulationBuffer(void)
{
	return s_cbData.romEmuBuf;
}

/* -----------------------------------------------------------------
 * LoadBannerData関数
 *
 * バナーデータを読み込む
 * 
 * 注：一度カードブートしてゲームモードになってから呼び出してください
 * ----------------------------------------------------------------- */
static HotSwState LoadBannerData(void)
{
    BOOL state;
	HotSwState retval = HOTSW_SUCCESS;

	// バナーリード
	if( s_cbData.pBootSegBuf->rh.s.banner_offset ) {
        retval  = ReadPageGame(&s_cbData,	s_cbData.pBootSegBuf->rh.s.banner_offset,
											(u32 *)SYSM_CARD_BANNER_BUF,
	                                     	sizeof(TWLBannerFile) );

        // バナーリードが成功していたら各種フラグTRUE その他の場合はFALSE (この関数の外で排他制御されているからここでは排他制御しないでOK)
        state = (retval == HOTSW_SUCCESS) ? TRUE : FALSE;
        SYSMi_GetWork()->flags.hotsw.isValidCardBanner  = state;
        SYSMi_GetWork()->flags.hotsw.isExistCard 		= state;
	}
    else{
        // バナーデータが登録されていない場合 (この関数の外で排他制御されているからここでは排他制御しないでOK)
        SYSMi_GetWork()->flags.hotsw.isValidCardBanner  = FALSE;
        SYSMi_GetWork()->flags.hotsw.isExistCard 		= TRUE;
    }

    if ( SYSMi_GetWork()->flags.hotsw.isExistCard )
    {
        SYSMi_GetWork()->flags.hotsw.isInspectCard = s_cbData.pBootSegBuf->rh.s.inspect_card;
    }
    else
    {
        SYSMi_GetWork()->flags.hotsw.isInspectCard = FALSE;
    }

   	// デバッガ情報
	if ( ! SYSMi_GetWork()->flags.hotsw.is1stCardChecked && s_cbData.debuggerFlg )
	{
		MI_CpuCopy8( HOTSW_GetRomEmulationBuffer(), &SYSMi_GetWork()->romEmuInfo, ROM_EMULATION_DATA_SIZE );
		SYSMi_GetWork()->flags.hotsw.isOnDebugger = s_cbData.debuggerFlg;
	}

    SYSMi_GetWork()->flags.hotsw.isCardStateChanged = TRUE;
    SYSMi_GetWork()->flags.hotsw.is1stCardChecked   = TRUE;

	return retval;
}

/* -----------------------------------------------------------------
 * LoadStaticModule関数
 *
 * ARM7,9の常駐モジュールを展開する関数
 * 
 * 注：一度カードブートしてゲームモードになってから呼び出してください
 * ----------------------------------------------------------------- */
static HotSwState LoadStaticModule(void)
{
	HotSwState retval = HOTSW_SUCCESS;
    HotSwState state  = HOTSW_SUCCESS;
    
	s_cbData.arm9Stc = (u32)s_cbData.pBootSegBuf->rh.s.main_ram_address;
	// 配置先と再配置情報を取得 & Arm9の常駐モジュール残りを指定先に転送
    if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_STATIC, &s_cbData.arm9Stc, s_cbData.pBootSegBuf->rh.s.main_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC] , s_cbData.twlFlg)){
    	if(s_cbData.pBootSegBuf->rh.s.main_size > SECURE_SEGMENT_SIZE){
	   		retval = ReadPageGame(&s_cbData,		s_cbData.pBootSegBuf->rh.s.main_rom_offset + SECURE_SEGMENT_SIZE,
           		                      		(u32 *)(s_cbData.arm9Stc 						   + SECURE_SEGMENT_SIZE),
               		                	    	    s_cbData.pBootSegBuf->rh.s.main_size 	   - SECURE_SEGMENT_SIZE);
       	}
    }
    else{
		retval = HOTSW_BUFFER_OVERRUN_ERROR;
    }

    if(retval != HOTSW_SUCCESS){
		return retval;
    }
    
	s_cbData.arm7Stc = (u32)s_cbData.pBootSegBuf->rh.s.sub_ram_address;
    // 配置先と再配置情報を取得 & Arm7の常駐モジュールを指定先に転送
    if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_STATIC, &s_cbData.arm7Stc, s_cbData.pBootSegBuf->rh.s.sub_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_STATIC] , s_cbData.twlFlg)){
    	state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.sub_rom_offset, (u32 *)s_cbData.arm7Stc, s_cbData.pBootSegBuf->rh.s.sub_size);
    	retval = (retval == HOTSW_SUCCESS) ? state : retval;
    }
    else{
        retval = HOTSW_BUFFER_OVERRUN_ERROR;
    }
    
    if(retval != HOTSW_SUCCESS){
		return retval;
    }
    
	// TWLでのみロード
	if( s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL ) {
		u32 size = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ?
					 s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;
        s_cbData.arm9Ltd = (u32)s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address;
		// 配置先と再配置情報を取得 & Arm9の常駐モジュールを指定先に転送（※TWLカード対応していないので、注意！！）
        if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_LTD_STATIC, &s_cbData.arm9Ltd, s_cbData.pBootSegBuf->rh.s.main_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC] , TRUE)){
	    	state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset, (u32 *)SYSM_CARD_TWL_SECURE_BUF, size);
			retval = (retval == HOTSW_SUCCESS) ? state : retval;

			if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ) {
		    	state  = ReadPageGame(&s_cbData,		s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset + SECURE_SEGMENT_SIZE,
	         	                    		 	(u32 *)(s_cbData.arm9Ltd 							   + SECURE_SEGMENT_SIZE),
	            	                            		s_cbData.pBootSegBuf->rh.s.main_ltd_size 	   - size);
            	retval = (retval == HOTSW_SUCCESS) ? state : retval;
			}
        }
        else{
			retval = HOTSW_BUFFER_OVERRUN_ERROR;
        }

		if(retval != HOTSW_SUCCESS){
			return retval;
    	}
        
		s_cbData.arm7Ltd = (u32)s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address;
        // 配置先と再配置情報を取得 & Arm7の常駐モジュールを指定先に転送
        if(SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_LTD_STATIC, &s_cbData.arm7Ltd, s_cbData.pBootSegBuf->rh.s.sub_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_LTD_STATIC], TRUE)){
	    	state  = ReadPageGame(&s_cbData, s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset, (u32 *)s_cbData.arm7Ltd, s_cbData.pBootSegBuf->rh.s.sub_ltd_size);
        	retval = (retval == HOTSW_SUCCESS) ? state : retval;
        }
        else{
			retval = HOTSW_BUFFER_OVERRUN_ERROR;
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
        // Arm9常駐モジュール Hash値のチェック
        if(!CheckArm9HashValue()){
            state = HOTSW_HASH_CHECK_ERROR;
			OS_PutString("×Arm9 Static Module Hash Check Error...\n");
    	}
    	
    	// Arm7常駐モジュール Hash値のチェック
    	if(!CheckArm7HashValue()){
            state = HOTSW_HASH_CHECK_ERROR;
			OS_PutString("×Arm7 Static Module Hash Check Error...\n");
    	}
        
		// Arm9拡張常駐モジュール Hash値のチェック
        if(!CheckExtArm9HashValue()){
            state = HOTSW_HASH_CHECK_ERROR;
			OS_PutString("×Arm9 Ltd Static Module Hash Check Error...\n");
    	}
        
        // Arm7拡張常駐モジュール Hash値のチェック
    	if(!CheckExtArm7HashValue()){
            state = HOTSW_HASH_CHECK_ERROR;
			OS_PutString("×Arm7 Ltd Static Module Hash Check Error...\n");
    	}
        retval = (retval == HOTSW_SUCCESS) ? state : retval;
#endif
	}

    return retval;
}

/* -----------------------------------------------------------------
 * CheckCardAuthCode関数
 *
 * Rom Headerの認証コードアドレスを読んで、クローンブート対応か判定する
 *
 * 注：カードブート処理中は呼び出さないようにする
 * ----------------------------------------------------------------- */
static HotSwState CheckCardAuthCode(void)
{
	u32 authBuf[PAGE_SIZE/sizeof(u32)];
	u32 auth_offset   = s_cbData.pBootSegBuf->rh.s.rom_valid_size ? s_cbData.pBootSegBuf->rh.s.rom_valid_size : 0x01000000;
	u32 page_offset   = auth_offset & 0xFFFFFE00;
	HotSwState retval = HOTSW_SUCCESS;

    u8	*p = (u8 *)authBuf;
    
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

/* -----------------------------------------------------------------
 * HOTSW_SetBootSegmentBuffer関数
 *
 * Boot Segment バッファの指定
 *
 * 注：カードブート処理中は呼び出さないようにする
 * ----------------------------------------------------------------- */
void HOTSW_SetBootSegmentBuffer(void* buf, u32 size)
{
	SDK_ASSERT(size > BOOT_SEGMENT_SIZE);

    s_pBootSegBuffer = (BootSegmentData *)buf;
    s_BootSegBufSize = size;

    s_cbData.pBootSegBuf = s_pBootSegBuffer;

    // バッファの初期化
    MI_CpuClear8(s_pBootSegBuffer, size);
}

/* -----------------------------------------------------------------
 * HOTSW_SetSecureSegmentBuffer関数
 *
 * Secure Segment バッファの指定
 * 
 * 注：カードブート処理中は呼び出さないようにする
 * ----------------------------------------------------------------- */
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

/* -----------------------------------------------------------------
 * GenVA_VB_VD関数
 *
 * コマンド認証値・コマンドカウンタ・PNジェネレータ初期値の生成
 * ----------------------------------------------------------------- */
static void GenVA_VB_VD(void)
{
    u32 dummy = 0;
	MATHRandContext32	rnd;					
    
	// 乱数を初期化 VBlankカウンタ値を種とする。
    MATH_InitRand32(&rnd, (u64)OS_GetVBlankCount());
    
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

/* -----------------------------------------------------------------
 * DecryptObjectFile関数
 *
 * セキュア領域先頭2KBの暗号化領域を復号化
 *
 * 注：セキュアモード中、またはセキュアモード前にこの関数を呼ぶと、
 * 　　正常にコマンドの暗号化が行えなくなります。
 * ----------------------------------------------------------------- */
static u32 encDestBuf[ENCRYPT_DEF_SIZE/sizeof(u32)];

static HotSwState DecryptObjectFile(void)
{
    u8  i;
  	s32 restSize;
  	s32 size 				= (s32)s_cbData.pBootSegBuf->rh.s.main_size;
	u32 *pEncBuf			= encDestBuf;
    u32 *pEncDes 			= s_cbData.pSecureSegBuf;
    BLOWFISH_CTX *tableBufp = &s_cbData.keyTable;
  	BOOL exist 				= TRUE;
    HotSwState retval		= HOTSW_SUCCESS;

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
			bufp += 2;	// 復号処理
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

/* -----------------------------------------------------------------
 * LockHotSwRsc関数
 *
 * 共有ワークのリソースの排他制御用　lockを行う
 * ----------------------------------------------------------------- */
static void LockHotSwRsc(OSLockWord* word)
{
    while(OS_TryLockByWord( s_RscLockID, word, NULL ) != OS_LOCK_SUCCESS){
		OS_Sleep(1);
    }
}

/* -----------------------------------------------------------------
 * UnlockHotSwRsc関数
 *
 * 共有ワークのリソースの排他制御用　Unlockを行う
 * ----------------------------------------------------------------- */
static void UnlockHotSwRsc(OSLockWord* word)
{
	OS_UnlockByWord( s_RscLockID, word, NULL );
}

/* -----------------------------------------------------------------
 * HOTSW_IsCardExist関数
 *
 * カードの存在判定
 *
 * ※SCFG_MC1のCDETフラグを見ている
 * ----------------------------------------------------------------- */
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

/* -----------------------------------------------------------------
 * HOTSW_IsCardAccessible関数
 *
 * カードスロットにアクセスできる状態か判定する
 *
 * ※SCFG_MC1のCDETフラグとM(モード)を見ている
 * ----------------------------------------------------------------- */
BOOL HOTSW_IsCardAccessible(void)
{
    if( HOTSW_IsCardExist() && CmpMcSlotMode(SLOT_STATUS_MODE_10) == TRUE){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

/* -----------------------------------------------------------------
 * IsSwap関数
 *
 * カードのスワップ判定
 *
 * ※SCFG_MC1のSWPフラグを見ている
 * ----------------------------------------------------------------- */
static BOOL IsSwap(void)
{
    if( reg_MI_MC1 & REG_MI_MC1_SWP_MASK ){
        return TRUE;
    }
    else{
        return FALSE;
    }
}

/* -----------------------------------------------------------------
 * GetMcSlotShift関数
 *
 * カードスロットのシフトビット数の取得
 * ----------------------------------------------------------------- */
static u32 GetMcSlotShift(void)
{
	return (u32)(IsSwap() * REG_MI_MC_SL2_CDET_SHIFT);
}
/* -----------------------------------------------------------------
 * GetMcSlotMask関数
 *
 * カードスロットのシフトビット数の取得
 * ----------------------------------------------------------------- */
static u32 GetMcSlotMask(void)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    return (u32)(REG_MI_MC_SL1_MODE_MASK << GetMcSlotShift());
#else
    return (u32)(REG_MI_MC_SL2_MODE_MASK >> GetMcSlotShift());
#endif
}

/* -----------------------------------------------------------------
 * SetMcSlotMode関数
 *
 * カードスロットのモード設定
 * ----------------------------------------------------------------- */
static void SetMcSlotMode(u32 mode)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~GetMcSlotMask()) | (mode << GetMcSlotShift()));
#else
    reg_MI_MC1 = (u32)((reg_MI_MC1 & ~GetMcSlotMask()) | (mode >> GetMcSlotShift()));
#endif
}

/* -----------------------------------------------------------------
 * CmpMcSlotMode関数
 *
 * カードスロットのモード比較
 * ----------------------------------------------------------------- */
static BOOL CmpMcSlotMode(u32 mode)
{
#ifndef DEBUG_USED_CARD_SLOT_B_
    if((reg_MI_MC1 & GetMcSlotMask()) == (mode << GetMcSlotShift())){
#else
    if((reg_MI_MC1 & GetMcSlotMask()) == (mode >> GetMcSlotShift())){
#endif
		return TRUE;
    }
    else{
		return FALSE;
    }
}

/*---------------------------------------------------------------------------*
  Name:		   McPowerOn

  Description: スロット電源ON関数
 *---------------------------------------------------------------------------*/
static void McPowerOn(void)
{
    // Counter-Aカウンタ設定値到達まで待つ
	while(CmpMcSlotMode(SLOT_STATUS_MODE_11) == TRUE){
		OS_Sleep(1);
    }

    if(CmpMcSlotMode(SLOT_STATUS_MODE_00) == TRUE){
		// [TODO:]待ち時間は暫定値。金子さんに数値を測定してもらう。
        // VDDの安定期間待ち
        OS_Sleep(100);
        
    	// SCFG_MC1 の Slot Status の M1,M0 を 01 にする
    	SetMcSlotMode(SLOT_STATUS_MODE_01);
		// 1ms待ち
		OS_Sleep(1);

    	// SCFG_MC1 の Slot Status の M1,M0 を 10 にする
    	SetMcSlotMode(SLOT_STATUS_MODE_10);

        // [TODO:]待ち時間は暫定値。金子さんに数値を測定してもらう。
        // RESBを上げるまでの待ち時間
		OS_Sleep(1);

		// リセットをhighに (RESB = 1にする)
		reg_HOTSW_MCCNT1 = RESB_MASK;
    
		// [TODO:]待ち時間は暫定値。金子さんに数値を測定してもらう。
        // カードへ最初のコマンドを送るまでの待ち時間
		OS_Sleep(100);
    }
}

/*---------------------------------------------------------------------------*
  Name:		   McPowerOff

  Description: スロット電源OFF関数
 *---------------------------------------------------------------------------*/
static void McPowerOff(void)
{
    // Counter-Aカウンタ設定値到達まで待つ
	while(CmpMcSlotMode(SLOT_STATUS_MODE_11) == TRUE){
		OS_Sleep(1);
    }
    
    if(CmpMcSlotMode(SLOT_STATUS_MODE_10) == TRUE){
    	// SCFG_MC1 の Slot Status の M1,M0 を 11 にする
    	SetMcSlotMode(SLOT_STATUS_MODE_11);

        // SCFG_MC1 の Slot Status の M1,M0 が 00 になるまでポーリング
        while(CmpMcSlotMode(SLOT_STATUS_MODE_00) == FALSE){
			OS_Sleep(1);
        }
    }
}

/*---------------------------------------------------------------------------*
  Name:         SetMCSCR
  
  Description:  符号生成回路初期値設定レジスタを設定する

  ※注：この関数はセキュアモードで、
		sPNG_ONコマンドを実行してから呼び出してください。
 *---------------------------------------------------------------------------*/
static void SetMCSCR(void)
{
	u32 pna_l = (u32)(PNA_BASE_VALUE | (s_cbData.vd << 15));
    u32 pna_h = (u32)(s_cbData.vd >> 17);
    
    // SCR A
	reg_HOTSW_MCSCR0 = pna_l;

    // SCR B
	reg_HOTSW_MCSCR1 = PNB_L_VALUE;

    // [d0 -d6 ] -> SCR A
    // [d16-d22] -> SCR B
    reg_HOTSW_MCSCR2 = (u32)(pna_h | PNB_H_VALUE << 16);

	// MCCNT1 レジスタ設定 (SCR = 1に)
    reg_HOTSW_MCCNT1 = SCR_MASK;
}

/*---------------------------------------------------------------------------*
  Name:		   McThread

  Description: カード抜け・挿し処理スレッド

  [TODO:]挿抜のフロー・フラグケアetcの確認(今の所、抜き挿ししてもタイトルが更新されない)
 *---------------------------------------------------------------------------*/
static void McThread(void *arg)
{
	#pragma unused( arg )

    BOOL 			isPulledOut = TRUE;
    HotSwState 		retval;
    HotSwMessage 	*msg;
    
    while(1){
        OS_ReceiveMessage(&s_ctData.hotswQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);

        if( msg->ctrl == TRUE ) {
            // [TODO]とりあえず、ここでHOTSWを抑制した時点でisExistCardがFALSEなら、HOTSWのFinalizeをするようにする。
			SYSMi_GetWork()->flags.hotsw.isEnableHotSW = msg->value;
            // [TODO]カードがあるときとないときで場合分けしてFinalize処理を実装
            //		 PXIメッセージを「抑制」と「Finalize」で分けて処理
            //			→ １．全てのレジスタをクリアする			(カードがささっていない時)
            //			   ２．一度電源を落としてNomalモードにする	(NANDアプリ等を起動する場合)
            //			   ３．必要なレジスタを残して、後はクリア	(ささっているカードを起動する場合)
//			HOTSW_Finalize();
        }
        
        while(1){
			// 活線挿抜抑制フラグが立っていたら処理しない
			if( !SYSMi_GetWork()->flags.hotsw.isEnableHotSW ) {
				SYSMi_GetWork()->flags.hotsw.is1stCardChecked  = TRUE;
                OS_PutString("### HotSw is restrained...\n");
				break;
			}
            
            // カードが挿さってたら
            if(HOTSW_IsCardExist()){
                // 前の状態が挿し
                if(!isPulledOut){
                    // 抜きがなかったか判定
                    if(CmpMcSlotMode(SLOT_STATUS_MODE_10) == TRUE){
	               		// フラグケア
                        LockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
                        
						SYSMi_GetWork()->flags.hotsw.isExistCard 		 = TRUE;
                		SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
                        
                    	// 新しいカードのIDを入れる
                    	SYSMi_GetWork()->nCardID = s_cbData.id_gam;

            			// カードデータロード完了フラグ
            			SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = TRUE;

                        UnlockHotSwRsc(&SYSMi_GetWork()->lockCardRsc);
                        
                        OS_PutString("ok!\n");

						break;
                    }
                }

                // カード読み込み開始
       			retval = LoadCardData();

				// Debug表示
                DebugPrintErrorMessage(retval);
                
                // カード読みが失敗していたらカードがないことにする
                if(retval != HOTSW_SUCCESS){
                	LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
					SYSMi_GetWork()->flags.hotsw.isExistCard 		 = FALSE;
                	SYSMi_GetWork()->flags.hotsw.isValidCardBanner   = FALSE;
                	SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
                	UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
                
                	// カードブート用構造体の初期化
					MI_CpuClear32(&s_cbData, sizeof(CardBootData));

					// バッファのクリア
					MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
					MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);
                    MI_CpuClearFast((u32 *)SYSM_CARD_BANNER_BUF, sizeof(TWLBannerFile));
                
					break;
                }
                
				// 状態フラグを更新
                isPulledOut = FALSE;
            }
            
            // カードが抜けてたら
            else{
                LockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
				SYSMi_GetWork()->flags.hotsw.isExistCard 		 = FALSE;
                SYSMi_GetWork()->flags.hotsw.isValidCardBanner   = FALSE;
                SYSMi_GetWork()->flags.hotsw.isCardStateChanged  = TRUE;
                SYSMi_GetWork()->flags.hotsw.isCardLoadCompleted = FALSE;
                UnlockHotSwRsc(&SYSMi_GetWork()->lockHotSW);
                
                // カードブート用構造体の初期化
				MI_CpuClear32(&s_cbData, sizeof(CardBootData));

				// バッファのクリア
				MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
				MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);
                MI_CpuClearFast((u32 *)SYSM_CARD_BANNER_BUF, sizeof(TWLBannerFile));
                
                isPulledOut = TRUE;

				break;
            }
        }
		SYSMi_GetWork()->flags.hotsw.is1stCardChecked  = TRUE;
    } // while loop
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackCard

  Description: カードB抜け割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCard(void)
{
	s_ctData.hotswPulledOutMsg[s_ctData.idx_pulledOut].ctrl  = FALSE;
    s_ctData.hotswPulledOutMsg[s_ctData.idx_pulledOut].value = 0;
    s_ctData.hotswPulledOutMsg[s_ctData.idx_pulledOut].type  = HOTSW_PULLOUT;
    
	// メッセージ送信
    OS_SendMessage(&s_ctData.hotswQueue, (OSMessage *)&s_ctData.hotswPulledOutMsg[s_ctData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

    // メッセージインデックスをインクリメント
    s_ctData.idx_pulledOut = (s_ctData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

	OS_PutString("○\n");
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackCardDet

  Description:  カードB挿し割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCardDet(void)
{
	s_ctData.hotswInsertMsg[s_ctData.idx_insert].ctrl  = FALSE;
    s_ctData.hotswInsertMsg[s_ctData.idx_insert].value = 0;
    s_ctData.hotswInsertMsg[s_ctData.idx_insert].type  = HOTSW_INSERT;
    
	// メッセージ送信
    OS_SendMessage(&s_ctData.hotswQueue, (OSMessage *)&s_ctData.hotswInsertMsg[s_ctData.idx_insert], OS_MESSAGE_NOBLOCK);

	// メッセージインデックスをインクリメント
    s_ctData.idx_insert = (s_ctData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;

	OS_PutString("●\n");
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackNDma

  Description:  カードB データ転送終了割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackNDma(void)
{
	// メッセージ送信
//    OS_SendMessage(&s_ctData.hotswDmaQueue, (OSMessage *)&s_ctData.hotswDmaMsg[s_ctData.idx_dma], OS_MESSAGE_NOBLOCK);

	// メッセージインデックスをインクリメント
//    s_ctData.idx_dma = (s_ctData.idx_dma+1) % HOTSW_DMA_MSG_NUM;
    
    OS_PutString("▽\n");
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackPxi

  Description:  PXI割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackPxi(PXIFifoTag tag, u32 data, BOOL err)
{
#pragma unused(tag)
#pragma unused(err)
	HotSwPxiMessage d;

	d.data = data;
    
    s_ctData.hotswPxiMsg[s_ctData.idx_ctrl].ctrl  = (d.msg.ctrl) ? TRUE : FALSE;
    s_ctData.hotswPxiMsg[s_ctData.idx_ctrl].value = d.msg.value;
	s_ctData.hotswPxiMsg[s_ctData.idx_ctrl].type  = HOTSW_CONTROL;

	// メッセージ送信
    OS_SendMessage(&s_ctData.hotswQueue, (OSMessage *)&s_ctData.hotswPxiMsg[s_ctData.idx_ctrl], OS_MESSAGE_NOBLOCK);

	// メッセージインデックスをインクリメント
    s_ctData.idx_ctrl = (s_ctData.idx_ctrl+1) % HOTSW_CTRL_MSG_NUM;
}

/*---------------------------------------------------------------------------*
  Name:			AllocateExCardBus

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
  Name:			AllocateExCardBus

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
  Name:			FreeExCardBus

  Description:  スロットBへのアクセス権を設定する
 *---------------------------------------------------------------------------*/
static void FreeExCardBus(void)
{
#ifdef  SDK_ARM9
    SetExCardProcessor(MI_PROCESSOR_ARM7);    // Card for SUB
#endif
}

/*---------------------------------------------------------------------------*
  Name:			LockSlotB

  Description:  スロットBをロックする
 *---------------------------------------------------------------------------*/
static s32 LockExCard(u16 lockID)
{
    return OS_LockByWord(lockID, (OSLockWord *)SLOT_B_LOCK_BUF, AllocateExCardBus);
}

/*---------------------------------------------------------------------------*
  Name:			UnlockSlotB

  Description:  スロットBをロックする
 *---------------------------------------------------------------------------*/
static s32 UnlockExCard(u16 lockID)
{
    return OS_UnlockByWord(lockID, (OSLockWord *)SLOT_B_LOCK_BUF, FreeExCardBus);
}

/*---------------------------------------------------------------------------*
  Name:			SetInterruptCallback
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
  	SetInterruptCallback( OS_IE_NDMA2 		, InterruptCallbackNDma );
#else
	SetInterruptCallback( OS_IE_CARD_B_IREQ , InterruptCallbackCard );
	SetInterruptCallback( OS_IE_CARD_B_DET  , InterruptCallbackCardDet );
	SetInterruptCallback( OS_IE_NDMA2		, InterruptCallbackNDma );
#endif
}

/* -----------------------------------------------------------------
 * CheckHashValue関数
 *
 * 常駐モジュール・拡張常駐モジュールのハッシュを計算して、
 * カード内のハッシュ値と比べる。
 * ----------------------------------------------------------------- */
#include <twl/os/common/systemCall.h>

// ----------------------------------------------------------------------
// 		Arm7常駐モジュールのハッシュチェック
// ----------------------------------------------------------------------
static BOOL CheckArm7HashValue(void)
{
	u8		sha1data[DIGEST_SIZE_SHA1];
	BOOL	retval = TRUE;

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

// ----------------------------------------------------------------------
// 		Arm9常駐モジュールのハッシュチェック
//
//		※ 先頭2Kの復号化が行われる前のデータのハッシュを比べる
// ----------------------------------------------------------------------
static BOOL CheckArm9HashValue(void)
{
	u8		sha1data[DIGEST_SIZE_SHA1];
	BOOL	retval = TRUE;
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

// ----------------------------------------------------------------------
// 		Arm7拡張常駐モジュールのハッシュチェック
// ----------------------------------------------------------------------
static BOOL CheckExtArm7HashValue(void)
{
	u8		sha1data[DIGEST_SIZE_SHA1];
	BOOL	retval = TRUE;

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

// ----------------------------------------------------------------------
// 		Arm9拡張常駐モジュールのハッシュチェック
// ----------------------------------------------------------------------
static BOOL CheckExtArm9HashValue(void)
{
	u8		sha1data[DIGEST_SIZE_SHA1];
    u32 	size;
	BOOL	retval = TRUE;
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
//							   Debug用表示関数
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
    
    OS_TPrintf("title Name         : %s\n", 	s_pBootSegBuffer->rh.s.title_name);
    OS_TPrintf("initial Code       : %x\n\n", 	*(u32 *)s_pBootSegBuffer->rh.s.game_code);

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
    OS_TPrintf("拡張機能制御レジスタ		 (MC_B(d24))   : %08x\n", reg_SCFG_EXT);
    OS_TPrintf("MC I/F制御レジスタ１		 (slot status) : %08x\n", reg_MI_MC1);
    OS_TPrintf("MC I/F制御レジスタ２		 (Counter-A)   : %04x\n", reg_MI_MC2);
    OS_TPrintf("MC コントロールレジスタ0	 (SEL etc)     : %04x\n", reg_HOTSW_MCCNT0);
    OS_TPrintf("MC コントロールレジスタ1	 (START etc)   : %08x\n", reg_HOTSW_MCCNT1);
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