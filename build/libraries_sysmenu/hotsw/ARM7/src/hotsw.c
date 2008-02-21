/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     hotsw.c

  Copyright 2007 Nintendo.  All rights reserved.

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
#include	<dsCardType1.h>
#include	<dsCardType2.h>
#include	<romEmulation.h>
#include	<customNDma.h>
#include 	<../build/libraries/mb/common/include/mb_fileinfo.h> // MB_AUTHCODE_SIZE

//#define HOTSW_FORCE_CARD_B

// define -------------------------------------------------------------------
#define		CHATTERING_COUNTER					0x100
#define		COUNTER_A							0x100

#define 	UNDEF_CODE							0xe7ffdeff	// 未定義コード
#define 	ENCRYPT_DEF_SIZE					0x800		// 2KB  ※ ARM9常駐モジュール先頭2KB

#define		DIGEST_HASH_BLOCK_SIZE_SHA1			(512/8)

#define		ROM_EMULATION_START_OFS				0x160
#define		ROM_EMULATION_END_OFS				0x180

#define		HOTSW_THREAD_STACK_SIZE				1024		// スタックサイズ
#define		HOTSW_THREAD_PRIO					11			// カード電源ON → ゲームモードのスレッド優先度
#define 	HOTSW_MSG_BUFFER_NUM				32			// 受信バッファの数
#define		HOTSW_INSERT_MSG_NUM				16			// 挿し割り込み送信メッセージの数
#define		HOTSW_PULLED_MSG_NUM				16			// 抜け割り込み送信メッセージの数

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

    
// スレッド・メッセージ関係をまとめた構造体
typedef struct CardThreadData{
    u64  				stack[HOTSW_THREAD_STACK_SIZE / sizeof(u64)];
	OSThread 			thread;

	u32 				idx_insert;
    u32					idx_pulledOut;
	
    OSMessage			hotswInsertMsg[HOTSW_INSERT_MSG_NUM];
    OSMessage			hotswPulledOutMsg[HOTSW_PULLED_MSG_NUM];
    OSMessageQueue   	hotswQueue;
	OSMessage			hotswMsgBuffer[HOTSW_MSG_BUFFER_NUM];
}
CardThreadData;

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
static void InterruptCallbackCardData(void);

static void McThread(void *arg);
static void McPowerOn(void);
static void McPowerOff(void);
static void SetMCSCR(void);

static void GenVA_VB_VD(void);
static void DecryptObjectFile(void);
static HotSwState LoadTable(void);
static HotSwState ReadRomEmulationData(void);
static HotSwState ReadIDNormal(void);
static HotSwState LoadStaticModule(void);
static HotSwState LoadCardData(void);
static HotSwState CheckCardAuthCode(void);

static void SetHotSwState(BOOL busy);

static s32 LockExCard(u16 lockID);
static s32 UnlockExCard(u16 lockID);

static BOOL CheckArm7HashValue(void);
static BOOL CheckArm9HashValue(void);
static BOOL CheckExtArm7HashValue(void);
static BOOL CheckExtArm9HashValue(void);

static void ShowRegisterData(void);
static void ShowRomHeaderData(void);

// Static Values ------------------------------------------------------------
static char 				*encrypt_object_key ATTRIBUTE_ALIGN(4) = "encryObj";
static char					*rom_emu_info ATTRIBUTE_ALIGN(4)	   = "TWLD";

static u32					s_SecureSegBufSize, s_BootSegBufSize;

static u32					*s_pSecureSegBuffer;	// カード抜けてもバッファの場所覚えとく
static BootSegmentData		*s_pBootSegBuffer;		// カード抜けてもバッファの場所覚えとく

static CardBootData			s_cbData;
static CardThreadData		s_ctData;

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
    {					   ReadBootSegNormal_DSType1, ChangeModeNormal_DSType1,								// Normalモード関数
     ReadIDSecure_DSType1, ReadSegSecure_DSType1, 	  SwitchONPNGSecure_DSType1, ChangeModeSecure_DSType1,	// Secureモード関数
     ReadIDGame_DSType1,   ReadPageGame_DSType1},															// Game  モード関数
	// DS Card Type 2
    {					   ReadBootSegNormal_DSType2, ChangeModeNormal_DSType2,								// Normalモード関数
     ReadIDSecure_DSType2, ReadSegSecure_DSType2, 	  SwitchONPNGSecure_DSType2, ChangeModeSecure_DSType2,	// Secureモード関数
     ReadIDGame_DSType2,   ReadPageGame_DSType2},															// Game  モード関数
	// TWL Card Type 1
    {					   ReadBootSegNormal_DSType2, ChangeModeNormal_DSType2,								// Normalモード関数
     ReadIDSecure_DSType2, ReadSegSecure_DSType2, 	  SwitchONPNGSecure_DSType2, ChangeModeSecure_DSType2,	// Secureモード関数
     ReadIDGame_DSType2,   ReadPageGame_DSType2},															// Game  モード関数
	// RomEmulation
    {					   ReadBootSegNormal_ROMEMU,  ChangeModeNormal_ROMEMU,								// Normalモード関数
     ReadIDSecure_ROMEMU,  ReadSegSecure_ROMEMU, 	  SwitchONPNGSecure_ROMEMU,  ChangeModeSecure_ROMEMU,	// Secureモード関数
     ReadIDGame_ROMEMU,    ReadPageGame_ROMEMU}																// Game  モード関数
};


// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         HOTSW_Init
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void HOTSW_Init(void)
{
	OS_InitTick();
    OS_InitThread();
    
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
    
	// カードブート用スレッドの生成
	OS_CreateThread(&s_ctData.thread,
                    McThread,
                    NULL,
                    s_ctData.stack + HOTSW_THREAD_STACK_SIZE / sizeof(u64),
                    HOTSW_THREAD_STACK_SIZE,
                    HOTSW_THREAD_PRIO
                    );

    // メッセージキューの初期化
	OS_InitMessageQueue( &s_ctData.hotswQueue, &s_ctData.hotswMsgBuffer[0], HOTSW_MSG_BUFFER_NUM );
    
    // スレッド起動
    OS_WakeupThreadDirect(&s_ctData.thread);

    // Boot Segment バッファの設定
	HOTSW_SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );

    // Secure Segment バッファの設定
    HOTSW_SetSecureSegmentBuffer((void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );

    // カードが挿さってあったらスレッドを起動する
	if(HOTSW_IsCardExist()){
//		OS_PutString("Card Boot Start\n");

		// メッセージ送信
    	OS_SendMessage(&s_ctData.hotswQueue, (OSMessage)&s_ctData.hotswInsertMsg[s_ctData.idx_insert], OS_MESSAGE_NOBLOCK);

        // メッセージインデックスをインクリメント
        s_ctData.idx_insert = (s_ctData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;
	}
    else{
//		OS_PutString("No Card...\n");
//#ifdef DEBUG_USED_CARD_SLOT_B_
		SYSMi_GetWork()->flags.common.is1stCardChecked  = TRUE;
//#endif
    }
}

/* -----------------------------------------------------------------
 * LoadCardData関数
 *
 * カードからデータをロードする
 *
 * ※BootSegmentBuffer SecureSegmentBufferの設定を行ってから
 *   この関数を呼んでください。
 *
 * [TODO:]カードのロックを見直し。InitでロックIDをがめておいて使うのでもOK
 * ----------------------------------------------------------------- */
static HotSwState LoadCardData(void)
{
	OSTick start;
	s32 tempLockID;
	HotSwState retval = HOTSW_SUCCESS;

    start = OS_GetTick();

	// カードのロックIDを取得
	tempLockID = OS_GetLockID();
    if(tempLockID == OS_LOCK_ID_ERROR){
		return HOTSW_CARD_LOCK_ERROR;
    }
    else{
    	s_cbData.lockID = (u16)tempLockID;
    }

	// カードのロック
#ifndef DEBUG_USED_CARD_SLOT_B_
	CARD_LockRom(s_cbData.lockID);
#else
	LockExCard(s_cbData.lockID);
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

	// バッファのクリア
    MI_CpuClearFast(s_pBootSegBuffer, s_BootSegBufSize);
	MI_CpuClearFast(s_pSecureSegBuffer, s_SecureSegBufSize);

    // ブート処理開始
	if(HOTSW_IsCardExist()){
		// カード側でKey Tableをロードする
        retval = LoadTable();
        
    	// ---------------------- Normal Mode ----------------------
    	// カードID読み込み
		retval = ReadIDNormal();
        
		// カードタイプを判別をして、使う関数を切替える IDの最上位ビットが1なら3DM
        if(s_cbData.id_nml & 0x80000000){
			s_cbData.cardType = DS_CARD_TYPE_2;
//			OS_TPrintf("Card Type2\n");
        }
        else{
			s_cbData.cardType = DS_CARD_TYPE_1;
//			OS_TPrintf("Card Type1\n");
        }
		
		{
			u8 i;
            u8 *romEmuInf = (u8 *)s_cbData.romEmuBuf;
            
			// ※最低限ARM9と排他制御しないといけない範囲はこれだけ
			u16 id = (u16)OS_GetLockID();
			(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );			// ARM9と排他制御する
			
	    	// Boot Segment読み込み
	    	retval = s_funcTable[s_cbData.cardType].ReadBootSegment_N(&s_cbData);

            // Romエミュレーション情報を取得
			retval = ReadRomEmulationData();

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
                SYSMi_GetWork()->gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param & ~SCRAMBLE_MASK;
                OS_TPrintf("SYSMi_GetWork()->gameCommondParam : 0x%08x\n",SYSMi_GetWork()->gameCommondParam);
            }
            else{
				SYSMi_GetWork()->gameCommondParam = s_cbData.pBootSegBuf->rh.s.game_cmd_param;
            }
            
			// ROMヘッダCRCを算出してチェック。NintendoロゴCRCも確認。
			SYSMi_GetWork()->cardHeaderCrc16_bak = SVC_GetCRC16( 65535, s_cbData.pBootSegBuf, 0x015e );
			OS_TPrintf( "RomHeaderCRC16 : calc = %04x  romh = %04x\n",
						SYSMi_GetWork()->cardHeaderCrc16_bak, s_cbData.pBootSegBuf->rh.s.header_crc16 );
			
			if( ( SYSMi_GetWork()->cardHeaderCrc16_bak != s_cbData.pBootSegBuf->rh.s.header_crc16 ) ||
				( 0xcf56 != s_cbData.pBootSegBuf->rh.s.nintendo_logo_crc16 ) ){
				retval = HOTSW_CRC_CHECK_ERROR;
			}
			
			(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );		// ARM9と排他制御する
			OS_ReleaseLockID( id );
		}
		
		if( retval == HOTSW_SUCCESS ) {
	        // NTRカードかTWLカードか
	        if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
//				OS_TPrintf("TWL Card.\n");
	            s_cbData.twlFlg = TRUE;
	        }
            else{
                // NTRカードの場合はRomHeaderバッファの1ページ目以降をクリアしておく。
                MI_CpuClearFast((void *)(SYSM_CARD_ROM_HEADER_BAK + PAGE_SIZE), SYSM_CARD_ROM_HEADER_SIZE - PAGE_SIZE);
            }

            // SecureコマンドのPNG_ONコマンドetc用のレイテンシを求める(Latency1とLatency2を足す)
			s_cbData.secureLatency = AddLatency2ToLatency1(s_cbData.pBootSegBuf->rh.s.secure_cmd_param);
            
	    	// Key Table初期化
	    	GCDm_MakeBlowfishTableDS(&s_cbData.keyTable, &s_pBootSegBuffer->rh.s, s_cbData.keyBuf, 8);

			// コマンド認証値・コマンドカウンタ初期値・PNジェネレータ初期値の生成
            GenVA_VB_VD();
            
	    	// セキュアモードに移行
	    	retval = s_funcTable[s_cbData.cardType].ChangeMode_N(&s_cbData);

	    	// ---------------------- Secure Mode ----------------------
			// PNG設定
			retval = s_funcTable[s_cbData.cardType].SetPNG_S(&s_cbData);
	
	        // DS側符号生成回路初期値設定 (レジスタ設定)
			SetMCSCR();
	
			// ID読み込み
	    	retval = s_funcTable[s_cbData.cardType].ReadID_S(&s_cbData);

            // カードIDの比較をして、一致しなければFALSEを返す
            if(s_cbData.id_nml != s_cbData.id_scr){
				retval = HOTSW_ID_CHECK_ERROR;
            }

            if(retval == HOTSW_SUCCESS){
		    	// Secure領域のSegment読み込み
		    	retval = s_funcTable[s_cbData.cardType].ReadSegment_S(&s_cbData);
            }

	    	// ゲームモードに移行
			retval = s_funcTable[s_cbData.cardType].ChangeMode_S(&s_cbData);

            if(retval != HOTSW_SUCCESS){
				goto error;
            }
            
	    	// ---------------------- Game Mode ----------------------
	    	// ID読み込み
			retval = s_funcTable[s_cbData.cardType].ReadID_G(&s_cbData);

            // カードIDの比較をして、一致しなければFALSEを返す
            if(s_cbData.id_scr != s_cbData.id_gam){
				retval = HOTSW_ID_CHECK_ERROR;
				goto error;
            }

			// 常駐モジュール残りを指定先に転送
			retval = LoadStaticModule();
            
            // ARM9常駐モジュールの先頭2KBの暗号化領域を複合化
			DecryptObjectFile();

			// 認証コード読み込み＆ワーク領域にコピー
			retval = CheckCardAuthCode();
		}
    }
    else{
		retval = HOTSW_PULLED_OUT_ERROR;
    }

	goto end;
error:
		OS_TPrintf("ng... retval : %d\n", retval);
end:
	// カードのロック開放
#ifndef DEBUG_USED_CARD_SLOT_B_
	CARD_UnlockRom(s_cbData.lockID);
#else
	UnlockExCard(s_cbData.lockID);
#endif
    
    // カードロックIDの開放
	OS_ReleaseLockID( s_cbData.lockID );

//	OS_TPrintf( "Load Card Time : %dms\n\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );

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
 * LoadStaticModule関数
 *
 * ARM7,9の常駐モジュールを展開する関数
 * 
 * 注：一度カードブートしてゲームモードになってから呼び出してください
 * ----------------------------------------------------------------- */
static HotSwState LoadStaticModule(void)
{
	HotSwState retval = HOTSW_SUCCESS;
    
//#ifdef DEBUG_USED_CARD_SLOT_B_
	// バナーリード
	if( s_cbData.pBootSegBuf->rh.s.banner_offset ) {
//		OS_TPrintf("  - Banner Loading...\n");
        retval = s_funcTable[s_cbData.cardType].ReadPage_G(
            									 &s_cbData,
            									  s_cbData.pBootSegBuf->rh.s.banner_offset,
												  (u32 *)SYSM_CARD_BANNER_BUF,
	                                              sizeof(TWLBannerFile) );
	}

    if(retval != HOTSW_SUCCESS){
		return retval;
    }
    
	if(HOTSW_IsCardExist())
	{
        SYSMi_GetWork()->flags.common.isValidCardBanner  = TRUE;
        SYSMi_GetWork()->flags.common.isCardStateChanged = TRUE;
        SYSMi_GetWork()->flags.common.isExistCard 		 = TRUE;
	}
	
//#endif
    
	s_cbData.arm9Stc = (u32)s_cbData.pBootSegBuf->rh.s.main_ram_address;
	// 配置先と再配置情報を取得 & Arm9の常駐モジュール残りを指定先に転送
	SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_STATIC, &s_cbData.arm9Stc, s_cbData.pBootSegBuf->rh.s.main_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_STATIC] , s_cbData.twlFlg);
    retval = s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
    													s_cbData.pBootSegBuf->rh.s.main_rom_offset + SECURE_SEGMENT_SIZE,
                                 	  			(u32 *)(s_cbData.arm9Stc 						   + SECURE_SEGMENT_SIZE),
                                              			s_cbData.pBootSegBuf->rh.s.main_size       - SECURE_SEGMENT_SIZE);

    if(retval != HOTSW_SUCCESS){
		return retval;
    }
    
	s_cbData.arm7Stc = (u32)s_cbData.pBootSegBuf->rh.s.sub_ram_address;
    // 配置先と再配置情報を取得 & Arm7の常駐モジュールを指定先に転送
	SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_STATIC, &s_cbData.arm7Stc, s_cbData.pBootSegBuf->rh.s.sub_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_STATIC] , s_cbData.twlFlg);
    retval = s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
                                              			s_cbData.pBootSegBuf->rh.s.sub_rom_offset,
                                 	   			 (u32 *)s_cbData.arm7Stc,
                                              			s_cbData.pBootSegBuf->rh.s.sub_size);

    if(retval != HOTSW_SUCCESS){
		return retval;
    }
    
	// TWLでのみロード
	if( s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL ) {
		u32 size = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ?
					 s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;
        s_cbData.arm9Ltd = (u32)s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address;
		// 配置先と再配置情報を取得 & Arm9の常駐モジュールを指定先に転送（※TWLカード対応していないので、注意！！）
		SYSM_CheckLoadRegionAndSetRelocateInfo( ARM9_LTD_STATIC, &s_cbData.arm9Ltd, s_cbData.pBootSegBuf->rh.s.main_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM9_LTD_STATIC] , TRUE);
	    retval = s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
                                                  			s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset,
                                 		   			 (u32 *)SYSM_CARD_TWL_SECURE_BUF,
	                                          	  			size);
		if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ) {
		    retval = s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
                                                      			s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset + SECURE_SEGMENT_SIZE,
	                             		 	  			(u32 *)(s_cbData.arm9Ltd 							   + SECURE_SEGMENT_SIZE),
	                                          		  			s_cbData.pBootSegBuf->rh.s.main_ltd_size 	   - size);
		}

		if(retval != HOTSW_SUCCESS){
			return retval;
    	}
        
		s_cbData.arm7Ltd = (u32)s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address;
        // 配置先と再配置情報を取得 & Arm7の常駐モジュールを指定先に転送
		SYSM_CheckLoadRegionAndSetRelocateInfo( ARM7_LTD_STATIC, &s_cbData.arm7Ltd, s_cbData.pBootSegBuf->rh.s.sub_ltd_size, &SYSMi_GetWork()->romRelocateInfo[ARM7_LTD_STATIC], TRUE);
	    retval = s_funcTable[s_cbData.cardType].ReadPage_G(&s_cbData,
        													s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset,
	                             	 	   			 (u32 *)s_cbData.arm7Ltd,
	                                          	  			s_cbData.pBootSegBuf->rh.s.sub_ltd_size);

	    if(retval != HOTSW_SUCCESS){
			return retval;
    	}
        
		// セキュア領域先頭2K分のハッシュ値を求めて、Work領域にコピー
        {
			u8		sha1data[DIGEST_SIZE_SHA1];
		    SVCHMACSHA1Context hash;

    		// クリア
			MI_CpuClear8(sha1data, sizeof(sha1data));
    
    		// ハッシュ初期化
			SVC_HMACSHA1Init( &hash, s_digestDefaultKey, sizeof(s_digestDefaultKey) );

    		// セキュア領域先頭2kb分UpDate
			SVC_HMACSHA1Update( &hash, s_cbData.pSecureSegBuf, ENCRYPT_DEF_SIZE );

			// ハッシュコンテキストをWork領域にコピー
		    MI_CpuCopy8( &hash, &SYSMi_GetWork2()->hmac_sha1_context, sizeof(SVCHMACSHA1Context) );
        }
        
        // Arm9常駐モジュール Hash値のチェック
        if(!CheckArm9HashValue()){
            retval = HOTSW_HASH_CHECK_ERROR;
			OS_PutString("×Arm9 Static Module Hash Check Error...\n");
    	}
    	
    	// Arm7常駐モジュール Hash値のチェック
    	if(!CheckArm7HashValue()){
            retval = HOTSW_HASH_CHECK_ERROR;
			OS_PutString("×Arm7 Static Module Hash Check Error...\n");
    	}
        
		// Arm9拡張常駐モジュール Hash値のチェック
        if(!CheckExtArm9HashValue()){
            retval = HOTSW_HASH_CHECK_ERROR;
			OS_PutString("×Arm9 Ltd Static Module Hash Check Error...\n");
    	}
        
        // Arm7拡張常駐モジュール Hash値のチェック
    	if(!CheckExtArm7HashValue()){
            retval = HOTSW_HASH_CHECK_ERROR;
			OS_PutString("×Arm7 Ltd Static Module Hash Check Error...\n");
    	}
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
    
    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    retval = s_funcTable[s_cbData.cardType].ReadPage_G( &s_cbData, page_offset, authBuf, MB_AUTHCODE_SIZE );

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

    OS_TPrintf("*** Boot Segm   Address : 0x%08x\n", s_pBootSegBuffer);
}

/* -----------------------------------------------------------------
 * HOTSW_SetSecureSegmentBuffer関数
 *
 * Secure Segment バッファの指定
 * 
 * 注：カードブート処理中は呼び出さないようにする
 * ----------------------------------------------------------------- */
void HOTSW_SetSecureSegmentBuffer(void* buf, u32 size)
{
    SDK_ASSERT(size > SECURE_SEGMENT_SIZE);
    
	s_pSecureSegBuffer = (u32 *)buf;
	s_SecureSegBufSize = size;

	s_cbData.pSecureSegBuf = s_pSecureSegBuffer;
    
    // バッファの初期化
    MI_CpuClear8(s_pSecureSegBuffer, size);

    OS_TPrintf("*** Scr Seg Buf Address : 0x%08x\n", s_pSecureSegBuffer);
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
 * LoadTable関数
 *
 * カード側の Key Table をロードする関数。
 *
 * ※この関数は開発カード用に発行しないといけない。
 *   製品版カードの場合、このコマンドは無視される設計
 * ----------------------------------------------------------------- */
static HotSwState LoadTable(void)
{
	u32 temp;
    
	// MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x0000009f;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

    // MCCNT1 レジスタ設定 (START = 1 W/R = 0 PC = 101(16ページ) latency1 = 0(必要ないけど) に)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x5 << PC_SHIFT);
    
	// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
	while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
		while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
        temp = reg_HOTSW_MCD1;
	}

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         ReadRomEmulationData
  
  Description:  Romエミュレーションデータの読み込み
 *---------------------------------------------------------------------------*/
static HotSwState ReadRomEmulationData(void)
{
	u32 count=0;
    u32 temp;
    u32 *dst = s_cbData.romEmuBuf;

    // 量産用CPUでは平文アクセス防止のためリードしない
    if ( ! (*(u8*)(OS_CHIPTYPE_DEBUGGER_ADDR) & OS_CHIPTYPE_DEBUGGER_MASK) )
    {
        return HOTSW_SUCCESS;
    }

	// MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x3e000000;
	reg_HOTSW_MCCMD1 = 0x0;

	// MCCNT1 レジスタ設定 (START = 1  PC = 001(1ページリード)に latency1 = 0x5fe)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x1 << PC_SHIFT) | (0x5fe & LATENCY1_MASK);
    
	// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
	while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
		while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
        if(count >= ROM_EMULATION_START_OFS && count < ROM_EMULATION_END_OFS){
        	*dst++ = reg_HOTSW_MCD1;
        }
        else{
			temp = reg_HOTSW_MCD1;
        }
        count+=4;
	}

   	MI_CpuCopyFast(s_cbData.romEmuBuf, (void*)HW_ISD_RESERVED, 32);

    return HOTSW_SUCCESS;
}

/* -----------------------------------------------------------------
 * ReadIDNormal関数
 *
 * ノーマルモード時のカードIDを読み込む関数
 * ----------------------------------------------------------------- */
static HotSwState ReadIDNormal(void)
{
	// カード割り込みによるDMAコピー
	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &s_cbData.id_nml, sizeof(s_cbData.id_nml) );
    
    // MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x00000090;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 PC = 111(ステータスリード) latency1 = 1 に)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | (0x1 & LATENCY1_MASK);

    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    return HOTSW_SUCCESS;
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

static void DecryptObjectFile(void)
{
    u8  i;
  	s32 restSize;
  	s32 size 				= (s32)s_cbData.pBootSegBuf->rh.s.main_size;
	u32 *pEncBuf			= encDestBuf;
    u32 *pEncDes 			= s_cbData.pSecureSegBuf;
    BLOWFISH_CTX *tableBufp = &s_cbData.keyTable;
  	BOOL exist 				= TRUE;

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
		OS_PutString("▼ DecryptObjectFile : Error...\n");

        MI_NDmaFill( HOTSW_DMA_NO, pEncBuf, UNDEF_CODE, (u32)size ); // 未定義コードでクリア
   	}
   	MI_CpuCopy32(pEncBuf, pEncDes, (u32)size);
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

  Description: スロットB 電源ON関数
 *---------------------------------------------------------------------------*/
static void McPowerOn(void)
{
    if(CmpMcSlotMode(SLOT_STATUS_MODE_00) == TRUE){
    	// SCFG_MC1 の Slot Status の M1,M0 を 01 にする
    	SetMcSlotMode(SLOT_STATUS_MODE_01);
		// 1ms待ち
		OS_Sleep(1);

    	// SCFG_MC1 の Slot Status の M1,M0 を 10 にする
    	SetMcSlotMode(SLOT_STATUS_MODE_10);
		// 1ms待ち
		OS_Sleep(1);

		// リセットをhighに (RESB = 1にする)
		reg_HOTSW_MCCNT1 = RESB_MASK;
    
		// 100ms待ち
		OS_Sleep(100);
    }
}

/*---------------------------------------------------------------------------*
  Name:		   McPowerOff

  Description: スロットB 電源OFF関数
 *---------------------------------------------------------------------------*/
static void McPowerOff(void)
{
    if(CmpMcSlotMode(SLOT_STATUS_MODE_10) == TRUE){
    	// SCFG_MC1 の Slot Status の M1,M0 を 11 にする
    	SetMcSlotMode(SLOT_STATUS_MODE_11);

        // SCFG_MC1 の Slot Status の M1,M0 が 00 になるまでポーリング
        while(CmpMcSlotMode(SLOT_STATUS_MODE_00) == FALSE){}

		// 100ms待ち [TODO:]待ち時間は暫定値。金子さんに数値を測定してもらう。
		OS_Sleep(100);
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

    u32			hotswCount  = UNDEF_CODE;
    BOOL 		isPulledOut = TRUE;
    HotSwState 	retval;
    OSMessage 	*msg;
    
    while(1){
        OS_ReceiveMessage(&s_ctData.hotswQueue, (OSMessage *)&msg, OS_MESSAGE_BLOCK);

        // カードデータロード完了フラグを下ろす
		SYSMi_GetWork()->flags.common.isCardLoadCompleted = FALSE;
        
        while(1){
			// 活線挿抜抑制フラグが立っていたら処理しない
			if( !SYSMi_GetWork()->flags.common.isEnableHotSW ) {
//#ifdef DEBUG_USED_CARD_SLOT_B_
				SYSMi_GetWork()->flags.common.is1stCardChecked  = TRUE;
//#endif
				break;
			}
            
            // カードが挿さってたら
            if(HOTSW_IsCardExist()){
                // 前の状態が挿し
                if(!isPulledOut){
                    // 抜き差しがなかったか判定
                    if(CARDi_IsPulledOutEx(hotswCount)){
						u16 id = (u16)OS_GetLockID();
						(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
						if( SYSMi_GetWork()->flags.arm9.reqChangeHotSW ) {
							SYSMi_GetWork()->flags.common.isEnableHotSW  = SYSMi_GetWork()->flags.arm9.nextHotSWStatus;
							SYSMi_GetWork()->flags.arm9.reqChangeHotSW   = 0;
							SYSMi_GetWork()->flags.arm9.nextHotSWStatus  = 0;
//							HOTSW_Finalize();
						}
						(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
						OS_ReleaseLockID( id );

	               		// フラグケア
    	           		{
        	       			u16 id = (u16)OS_GetLockID();
							(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
							SYSMi_GetWork()->flags.common.isExistCard 		  = TRUE;
                			SYSMi_GetWork()->flags.common.isCardStateChanged  = TRUE;
							(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
							OS_ReleaseLockID( id );
						}
                        
                    	// 新しいカードのIDを入れる
                    	SYSMi_GetWork()->nCardID = s_cbData.id_gam;

                    	// デバッガ情報
						MI_CpuCopy8( HOTSW_GetRomEmulationBuffer(), &SYSMi_GetWork()->romEmuInfo, ROM_EMULATION_DATA_SIZE );
						SYSMi_GetWork()->flags.common.isOnDebugger = s_cbData.debuggerFlg;

            			// カードデータロード完了フラグ
            			SYSMi_GetWork()->flags.common.isCardLoadCompleted = TRUE;

                        OS_PutString("ok!\n");

						break;
                    }
                }

                // 挿し回数を取得
				hotswCount = CARDi_GetSlotResetCount();

                // HotSwをbusy状態にする 
				SetHotSwState(TRUE);

                // カード読み込み開始
       			retval = LoadCardData();

				// HotSwをfree状態にする
                SetHotSwState(FALSE);

				// 状態フラグを更新
                isPulledOut = (retval == HOTSW_SUCCESS) ? FALSE : TRUE;
            }
            
            // カードが抜けてたら
            else{
                if(isPulledOut){
               		// フラグケア
               		{
               			u16 id = (u16)OS_GetLockID();
						(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
						SYSMi_GetWork()->flags.common.isExistCard 		  = FALSE;
                		SYSMi_GetWork()->flags.common.isValidCardBanner   = FALSE;
                		SYSMi_GetWork()->flags.common.isCardStateChanged  = TRUE;
						(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
						OS_ReleaseLockID( id );
					}

                	// カードブート用構造体の初期化
					MI_CpuClear32(&s_cbData, sizeof(CardBootData));

					break;
                }

                isPulledOut = TRUE;
            }
        }
//#ifdef DEBUG_USED_CARD_SLOT_B_
		SYSMi_GetWork()->flags.common.is1stCardChecked  = TRUE;
//#endif
    }
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackCard

  Description: カードB抜け割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCard(void)
{
	// メッセージ送信
    OS_SendMessage(&s_ctData.hotswQueue, (OSMessage *)&s_ctData.hotswPulledOutMsg[s_ctData.idx_pulledOut], OS_MESSAGE_NOBLOCK);

    // メッセージインデックスをインクリメント
    s_ctData.idx_pulledOut = (s_ctData.idx_pulledOut+1) % HOTSW_PULLED_MSG_NUM;

	OS_TPrintf("○ - idx_pulledOut : %d\n", s_ctData.idx_pulledOut);
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackCardDet

  Description:  カードB挿し割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCardDet(void)
{
    // SDKのカード状態をリセットする
	CARDi_ResetSlotStatus();
    
	// メッセージ送信
    OS_SendMessage(&s_ctData.hotswQueue, (OSMessage *)&s_ctData.hotswInsertMsg[s_ctData.idx_insert], OS_MESSAGE_NOBLOCK);

	// メッセージインデックスをインクリメント
    s_ctData.idx_insert = (s_ctData.idx_insert+1) % HOTSW_INSERT_MSG_NUM;

	OS_TPrintf("● - idx_insert : %d\n", s_ctData.idx_insert);
}

/*---------------------------------------------------------------------------*
  Name:			InterruptCallbackCardData

  Description:  カードB データ転送終了割り込みハンドラ
 *---------------------------------------------------------------------------*/
static void InterruptCallbackCardData(void)
{
	// データ転送終了待ちまで寝ていたのを起こす
    OS_WakeupThreadDirect(&s_ctData.thread);
}

/*---------------------------------------------------------------------------*
  Name:			SetHotSwState

  Description:  HotSwの状態を設定する
 *---------------------------------------------------------------------------*/
static void SetHotSwState(BOOL busy)
{
	u16 id = (u16)OS_GetLockID();
	(void)OS_LockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );

    SYSMi_GetWork()->flags.common.isBusyHotSW = busy ? 1 : 0;

	(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockHotSW, NULL );
	OS_ReleaseLockID( id );
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
  	SetInterruptCallback( OS_IE_CARD_A_DATA , InterruptCallbackCardData );
#else
	SetInterruptCallback( OS_IE_CARD_B_IREQ , InterruptCallbackCard );
	SetInterruptCallback( OS_IE_CARD_B_DET  , InterruptCallbackCardDet );
	SetInterruptCallback( OS_IE_CARD_B_DATA , InterruptCallbackCardData );
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
