/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     Card.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include 	<twl/os/common/format_rom.h>
//#include 	<istdbglib.h>

#include	<nitro/card/types.h>
#include	<sysmenu.h>

// define -------------------------------------------------------------------
#define		STACK_SIZE							1024		// スタックサイズ
#define		MC_THREAD_PRIO						11			// カード電源ON → ゲームモードのスレッド優先度
#define		ML_THREAD_PRIO						12			// Boot Segment読み込み終わったら起動する。カードブートスレッドより優先度低。

#define		DEBUG_CARD_TYPE						1			// DS Card Type1 = 0  DS Card Type2 = 1

// Function prototype -------------------------------------------------------
static BOOL IsCardExist(void);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCard(void);
static void InterruptCallbackCardDet(void);
static void InterruptCallbackCardData(void);

static void McThread(void *arg);
static void StaticModuleLoadThread(void *arg);
static void LoadStaticModule_Secure(void);
static void McPowerOn(void);
static void SetMCSCR(void);

static void LoadTable(void);
static void ReadIDNormal(void);
static void MIm_CardDmaCopy32(u32 dmaNo, const void *src, void *dest);

static void ShowRegisterData(void);
static void ShowRomHeaderData(void);

// Static Values ------------------------------------------------------------
static u64  				s_MCStack[STACK_SIZE / sizeof(u64)];
static u64  				s_MLStack[STACK_SIZE / sizeof(u64)];

static OSThread 			s_MCThread;
static OSThread 			s_MLThread;

static u32					s_SecureSegBufSize, s_BootSegBufSize;

static u32					*s_pSecureSegBuffer;	// カード抜けてもバッファの場所覚えとく
static BootSegmentData		*s_pBootSegBuffer;		// カード抜けてもバッファの場所覚えとく

static CardBootData			s_cbData;

// -------------------------------------------------------------------

static CardBootFunction  	s_funcTable[] = {
	// DS Card Type 1
    {					   ReadBootSegNormal_DSType1, ChangeModeNormal_DSType1,
     ReadIDSecure_DSType1, ReadSegSecure_DSType1, 	  SwitchONPNGSecure_DSType1, ChangeModeSecure_DSType1,
     ReadIDGame_DSType1,   ReadPageGame_DSType1},
	// DS Card Type 2
    {					   ReadBootSegNormal_DSType2, ChangeModeNormal_DSType1,
     ReadIDSecure_DSType2, ReadSegSecure_DSType2, 	  SwitchONPNGSecure_DSType2, ChangeModeSecure_DSType2,
     ReadIDGame_DSType1,   ReadPageGame_DSType1}
};


// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         Card_Init
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void Cardm_Init(void)
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
	reg_MI_MC1 = (u32)((reg_MI_MC1 & 0xffff) | 0xc80000);

	// Counter-Aの値を設定
    reg_MI_MC2 = 0xc8;
#endif

	// カードブート用スレッドの生成
	OS_CreateThread(&s_MCThread,
                    McThread,
                    NULL,
                    s_MCStack + STACK_SIZE / sizeof(u64),
                    STACK_SIZE,
                    MC_THREAD_PRIO
                    );

    // スレッド起動
    OS_WakeupThreadDirect(&s_MCThread);

    // Boot Segment バッファの設定
	Card_SetBootSegmentBuffer((void *)SYSM_CARD_ROM_HEADER_BAK, SYSM_CARD_ROM_HEADER_SIZE );

    // Secure Segment バッファの設定
    Card_SetSecureSegmentBuffer((void *)SYSM_CARD_NTR_SECURE_BUF, SECURE_AREA_SIZE );

	// モジュールロード用スレッドの生成
/*	OS_CreateThread(&s_MLThread,
                    StaticModuleLoadThread,
                    NULL,
                    s_MLStack + STACK_SIZE / sizeof(u64),
                    STACK_SIZE,
                    ML_THREAD_PRIO
                    );

    // モジュールロード用スレッド起動
    OS_WakeupThreadDirect(&s_MLThread);*/
    
	// カードブート用構造体の初期化
	MI_CpuClear32(&s_cbData, sizeof(CardBootData));

	OS_TPrintf("*** sizeof(ROM_Header) : 0x%08x\n", sizeof(ROM_Header));
}

/* -----------------------------------------------------------------
 * Card_Boot関数
 *
 * カード起動をスタート
 *
 * ※BootSegmentBuffer SecureSegmentBufferの設定を行ってから
 *   この関数を呼んでください。
 * ----------------------------------------------------------------- */
BOOL Card_Boot(void)
{
	s32 tempLockID;
	BOOL retval = TRUE;
    OSTick start = OS_GetTick();
	
	OS_TPrintf("---------------- Card Boot Start ---------------\n");
	// カード電源ON
	McPowerOn();
    
	// VAE・VBI・VD値の設定
    s_cbData.vae = VAE_VALUE;
    s_cbData.vbi = VBI_VALUE;
	s_cbData.vd  = VD_VALUE;

	// セキュア領域の読み込みセグメント先頭番号(Segment4 〜 Segment7)
    s_cbData.secureSegNum = 4;

	// バッファを設定
    s_cbData.pBootSegBuf   = s_pBootSegBuffer;
    s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

	// カードのロックIDを取得
	tempLockID = OS_GetLockID();
    if(tempLockID == OS_LOCK_ID_ERROR){
		retval = FALSE;
    }
    else{
    	s_cbData.lockID = (u16)tempLockID;
    }

    // ブート処理開始
	if(IsCardExist() && retval){
		// カード側でKey Tableをロードする
        LoadTable();
        
    	// ---------------------- Normal Mode ----------------------
    	// カードID読み込み
		ReadIDNormal();

		// カードタイプを判別をして、使う関数を切替える IDの最上位ビットが1なら3DM
        if(s_cbData.id_nml & 0x80000000){
			s_cbData.cardType = DS_CARD_TYPE_2;
            OS_TPrintf("Card Type2\n");
        }
        else{
			s_cbData.cardType = DS_CARD_TYPE_1;
            OS_TPrintf("Card Type1\n");
        }
		
		{
			// ※最低限ARM9と排他制御しないといけない範囲はこれだけ
			u16 id = (u16)OS_GetLockID();
			(void)OS_LockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );			// ARM9と排他制御する
			
	    	// Boot Segment読み込み
	    	s_funcTable[s_cbData.cardType].ReadBootSegment_N(&s_cbData);
			
			// ROMヘッダCRCを算出してチェック。NintendoロゴCRCも確認。
			SYSMi_GetWork()->cardHeaderCrc16_bak = SVC_GetCRC16( 65535, s_cbData.pBootSegBuf, 0x015e );
			OS_TPrintf( "RomHeaderCRC16 : calc = %04x  romh = %04x\n",
						SYSMi_GetWork()->cardHeaderCrc16_bak, s_cbData.pBootSegBuf->rh.s.header_crc16 );
			
			if( ( SYSMi_GetWork()->cardHeaderCrc16_bak != s_cbData.pBootSegBuf->rh.s.header_crc16 ) ||
				( 0xcf56 != s_cbData.pBootSegBuf->rh.s.nintendo_logo_crc16 ) ){
				retval = FALSE;
			}
			
			SYSMi_GetWork()->isExistCard = retval;
			SYSMi_GetWork()->isCardStateChanged = TRUE;	// 本当は挿抜単位でここを立てる。
			
			(void)OS_UnlockByWord( id, &SYSMi_GetWork()->lockCardRsc, NULL );		// ARM9と排他制御する
			OS_ReleaseLockID( id );
		}
		
		if( retval ) {
	        // NTRカードかTWLカードか
	        if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
				OS_TPrintf("TWL Card.\n");
	            s_cbData.twlFlg = TRUE;
	        }
	    	// Key Table初期化
	    	GCDm_MakeBlowfishTableDS(&s_cbData.keyTable, &s_pBootSegBuffer->rh.s, s_cbData.keyBuf, 8);
	
	    	// セキュアモードに移行
	    	s_funcTable[s_cbData.cardType].ChangeMode_N(&s_cbData);
	
	    	// ---------------------- Secure Mode ----------------------
			// PNG設定
			s_funcTable[s_cbData.cardType].SetPNG_S(&s_cbData);
	
	        // DS側符号生成回路初期値設定 (レジスタ設定)
			SetMCSCR();
	
			// ID読み込み
	    	s_funcTable[s_cbData.cardType].ReadID_S(&s_cbData);
	
	    	// Secure領域のSegment読み込み
	    	s_funcTable[s_cbData.cardType].ReadSegment_S(&s_cbData);
	
			// Arm9の常駐モジュールを指定先に転送
			LoadStaticModule_Secure();
	    	// ゲームモードに移行
			s_funcTable[s_cbData.cardType].ChangeMode_S(&s_cbData);
	
	    	// ---------------------- Game Mode ----------------------
	    	// ID読み込み
			s_funcTable[s_cbData.cardType].ReadID_G(&s_cbData);
			// 常駐モジュール残りを指定先に転送
			Card_LoadStaticModule();
	
			// デバッグ出力
			ShowRomHeaderData();
		}

		// ※最終的にはカードIDをHW_BOOT_CHECK_INFO_BUFに入れないと、アプリ起動後のカード抜け処理が上手く動作しないので注意。
		//   今はスロットBを使用しているので、ノーケアでOK.
//		*(u32 *)HW_BOOT_CHECK_INFO_BUF = s_cbData.id_gam;

        OS_TPrintf("-----------------------------------------------\n\n");
    }
    else{
		OS_TPrintf("Card Not Found\n");
		retval = FALSE;
    }

    // カードロックIDの開放
	OS_ReleaseLockID( s_cbData.lockID );

	OS_TPrintf( "Load Card Time : %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
	
#ifdef DEBUG_USED_CARD_SLOT_B_
	SYSMi_GetWork()->is1stCardChecked  = TRUE;
#endif
	
    return retval;
}

/* -----------------------------------------------------------------
 * Card_LoadStaticModuler関数
 *
 * ARM7,9の常駐モジュールを展開する関数
 * 
 * 注：一度カードブートしてゲームモードになってから呼び出してください
 * ----------------------------------------------------------------- */
void Card_LoadStaticModule(void)
{
#ifdef DEBUG_USED_CARD_SLOT_B_
	// バナーリード
	if( s_cbData.pBootSegBuf->rh.s.banner_offset ) {
	    OS_TPrintf("  - Banner Loading...\n");
	    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.banner_offset,
												  (u32 *)SYSM_CARD_BANNER_BUF,
	                                              sizeof(TWLBannerFile) );
		SYSMi_GetWork()->isValidCardBanner = TRUE;
		SYSMi_GetWork()->is1stCardChecked  = TRUE;
	}
#endif
	
    OS_TPrintf("  - Arm9 Static Module Loading...\n");
    // Arm9の常駐モジュール残りを指定先に転送
    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.main_rom_offset  + SECURE_SEGMENT_SIZE,
                                 (u32 *)((u32)s_cbData.pBootSegBuf->rh.s.main_ram_address + SECURE_SEGMENT_SIZE),
                                              s_cbData.pBootSegBuf->rh.s.main_size        - SECURE_SEGMENT_SIZE);

    OS_TPrintf("  - Arm7 Static Module Loading...\n");
    // Arm7の常駐モジュールを指定先に転送
    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.sub_rom_offset,
                                 (u32 *)((u32)s_cbData.pBootSegBuf->rh.s.sub_ram_address),
                                              s_cbData.pBootSegBuf->rh.s.sub_size);

	// TWLでのみロード
	if( s_cbData.pBootSegBuf->rh.s.platform_code & PLATFORM_CODE_FLAG_TWL ) {
		u32 size = ( s_cbData.pBootSegBuf->rh.s.main_ltd_size < SECURE_SEGMENT_SIZE ) ?
					 s_cbData.pBootSegBuf->rh.s.main_ltd_size : SECURE_SEGMENT_SIZE;
	    OS_TPrintf("  - Arm9 Ltd. Static Module Loading...\n");
	    // Arm9の常駐モジュールを指定先に転送（※TWLカード対応していないので、注意！！）
		
		s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset,
                                 		   (u32 *)SYSM_CARD_TWL_SECURE_BUF,
	                                          	  size);
		if( s_cbData.pBootSegBuf->rh.s.main_ltd_size > SECURE_SEGMENT_SIZE ) {
		    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.main_ltd_rom_offset,
	                             		 (u32 *)((u32)s_cbData.pBootSegBuf->rh.s.main_ltd_ram_address),
	                                          		  s_cbData.pBootSegBuf->rh.s.main_ltd_size);
		}

	    OS_TPrintf("  - Arm7 Ltd. Static Module Loading...\n");
	    // Arm7の常駐モジュールを指定先に転送
	    s_funcTable[s_cbData.cardType].ReadPage_G(s_cbData.pBootSegBuf->rh.s.sub_ltd_rom_offset,
	                             	 (u32 *)((u32)s_cbData.pBootSegBuf->rh.s.sub_ltd_ram_address),
	                                          	  s_cbData.pBootSegBuf->rh.s.sub_ltd_size);
	}
}


/* -----------------------------------------------------------------
 * Card_SetBootSegmentBuffer関数
 *
 * Boot Segment バッファの指定
 *
 * 注：カードブート処理中は呼び出さないようにする
 * ----------------------------------------------------------------- */
void Card_SetBootSegmentBuffer(void* buf, u32 size)
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
 * Card_SetSecureSegmentBuffer関数
 *
 * Secure Segment バッファの指定
 * 
 * 注：カードブート処理中は呼び出さないようにする
 * ----------------------------------------------------------------- */
void Card_SetSecureSegmentBuffer(void* buf, u32 size)
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
 * LoadTable関数
 *
 * カード側の Key Table をロードする関数。
 *
 * ※この関数は開発カード用に発行しないといけない。
 *   製品版カードの場合、このコマンドは無視される設計
 * ----------------------------------------------------------------- */
static void LoadTable(void)
{
	u32 temp;
    
	// MCCMD レジスタ設定
	reg_MCCMD0 = 0x0000009f;
	reg_MCCMD1 = 0x00000000;

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 W/R = 0 PC = 101(16ページ) latency1 = 0(必要ないけど) に)
	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,  0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,5,  0,0,  0,  0,0,0,  0));
    
	// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
        temp = reg_MCD1;
	}

    OS_TPrintf("Load Table...\n");
}

/* -----------------------------------------------------------------
 * ReadIDNormal関数
 *
 * ノーマルモード時のカードIDを読み込む関数
 * ----------------------------------------------------------------- */
static void ReadIDNormal(void)
{
	// MCCMD レジスタ設定
	reg_MCCMD0 = 0x00000090;
	reg_MCCMD1 = 0x00000000;

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 W/R = 0 PC = 111(ステータスリード) latency1 = 2320(必要ないけど) に)
	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,     0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,7,  0,0,  0,  0,0,0,  2320));
    
	// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
		s_cbData.id_nml = reg_MCD1;
	}
}

/* -----------------------------------------------------------------
 * IsCardExist関数
 *
 * カードの存在判定
 *
 * ※SCFG_MC1のSlot モード選択フラグを見ている
 *
 * モード選択フラグが 10 (全ての端子から有効出力) の時ささっていると判定
   		Slot A の場合 if((reg_MI_MC1 & 0x0c) == 0x08)
		Slot B の場合 if((reg_MI_MC1 & 0xc0) == 0x80)
 * ----------------------------------------------------------------- */
static BOOL IsCardExist(void)
{
    if((reg_MI_MC1 & SLOT_STATUS_MODE_SELECT_MSK) == SLOT_STATUS_MODE_10){
		return TRUE;
    }
    else{
		return FALSE;
    }
}

/* -----------------------------------------------------------------
 * McThread B 関数
 * ----------------------------------------------------------------- */
static void McThread(void *arg)
{
	#pragma unused( arg )
    
    while(1){
		OS_SleepThread(NULL);

        // カードブート
        Card_Boot();
    }
}

/* -----------------------------------------------------------------
 * StaticModuleLoadThread 関数
 * ----------------------------------------------------------------- */
static void StaticModuleLoadThread(void *arg)
{
	#pragma unused( arg )
    
    while(1){
		OS_SleepThread(NULL);


    }
}

static void LoadStaticModule_Secure(void)
{
    if(s_cbData.pBootSegBuf->rh.s.main_size >= SECURE_SEGMENT_SIZE){
		MI_DmaCopy32(1, s_cbData.pSecureSegBuf, s_cbData.pBootSegBuf->rh.s.main_ram_address, SECURE_SEGMENT_SIZE);
    }
    else{
		MI_DmaCopy32(1, s_cbData.pSecureSegBuf, s_cbData.pBootSegBuf->rh.s.main_ram_address, s_cbData.pBootSegBuf->rh.s.main_size);
    }
}

/* -----------------------------------------------------------------
 * McPowerOn関数
 * ----------------------------------------------------------------- */
static void McPowerOn(void)
{
	OSTick start;

    // SCFG_MC1 の Slot Status の M1,M0 を 11 にする
    reg_MI_MC1  = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | 0xc0);
	// 10ms待ち
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    
    // SCFG_MC1 の Slot Status の M1,M0 を 00 にする
    reg_MI_MC1  = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | 0x00);
	// 10ms待ち
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    
    // SCFG_MC1 の Slot Status の M1,M0 を 01 にする
    reg_MI_MC1  = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_01);
	// 10ms待ち
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    
    // SCFG_MC1 の Slot Status の M1,M0 を 10 にする
	reg_MI_MC1 	= (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_10);
	// 10ms待ち
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    
    // リセットをhighに (RESB = 1にする)
	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(1,1,0,1,1,1,1,1,1,1,1,1,1)) |
                             		 CNT1_FLD(0,0,1,0,0,0,0,0,0,0,0,0,0));
	// 10ms待ち
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 10){}

    OS_TPrintf("MC Power ON\n");
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
	reg_MCSCR0 = pna_l;

    // SCR B
	reg_MCSCR1 = PNB_L_VALUE;

    // [d0 -d6 ] -> SCR A
    // [d16-d22] -> SCR B
    reg_MCSCR2 = (u32)(pna_h | PNB_H_VALUE << 16);

	// MCCNT1 レジスタ設定 (SCR = 1に)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(1,1,1,1,  1,1,  1,1,  1,  0,1,1,  1)) |
           		             		 CNT1_FLD(0,0,0,0,  0,0,  0,0,  0,  1,0,0,  0));
}


//--------------------------------------------------------------------
//		改造DMA用
//--------------------------------------------------------------------
/* ---------------------------------------------------------------------------------------------------------------------------------------- */
// DMAイネーブル・DSカード起動モード・転送ビット幅 32ビット・繰り返しモード・ソースアドレス固定・デスティネーションアドレス固定・ワードカウント=1
#define MI_CNT_CARDWRITE32    ( MI_DMA_ENABLE | MI_DMA_TIMING_CARD | MI_DMA_32BIT_BUS | MI_DMA_CONTINUOUS_ON | MI_DMA_SRC_FIX | MI_DMA_DEST_FIX | 1 )

/* ---------------------------------------------------------------------------------------------------------------------------------------- */
// NitroSDK/build/libraries/mi/common/include/mi_dma.hより抜粋
#define MIi_Wait_BeforeDMA( dmaCntp, dmaNo )                  \
    do {                                                      \
      dmaCntp = &((vu32*)REG_DMA0SAD_ADDR)[dmaNo * 3 + 2];    \
      while ( *dmaCntp & REG_MI_DMA0CNT_E_MASK ) {}           \
    }while(0)

#define MIi_Wait_AfterDMA( dmaCntp )                          \
    do {                                                      \
      while ( *dmaCntp & REG_MI_DMA0CNT_E_MASK ) {}           \
    }while(0)

inline void MIi_DmaSetParams_wait(u32 dmaNo, u32 src, u32 dest, u32 ctrl)
{
    OSIntrMode enabled = OS_DisableInterrupts();
    vu32   *p = (vu32 *)((u32)REG_DMA0SAD_ADDR + dmaNo * 12);
    *p = (vu32)src;
    *(p + 1) = (vu32)dest;
    *(p + 2) = (vu32)ctrl;

    // ARM7 must wait 2 cycle (load is 3 cycle)
    {
        u32     dummy = reg_MI_DMA0SAD;
    }

    (void)OS_RestoreInterrupts(enabled);
}

//------------------------------------------------------------------------------
//
// NitroSDK　DMA転送関数　改造版
//
//------------------------------------------------------------------------------
// NitroSDK/build/libraries/mi/common/src/mi_dma_card.cより抜粋
static void MIm_CardDmaCopy32(u32 dmaNo, const void *src, void *dest)
{
    vu32   *dmaCntp;

    MIi_Wait_BeforeDMA(dmaCntp, dmaNo);
    MIi_DmaSetParams_wait(dmaNo, (u32)src, (u32)dest,(u32)MI_CNT_CARDWRITE32);
    /*
     * ここでは自動起動が ON になっただけ.
     * CARD レジスタへコマンドを設定して初めて起動する.
     */
}





/*---------------------------------------------------------------------------*
  Name:         InterruptCallbackCard
				InterruptCallbackCardDet
  				InterruptCallbackCardData
  
  Description:  各種割り込みコールバック関数
 *---------------------------------------------------------------------------*/
// カードB抜け
static void InterruptCallbackCard(void)
{
    // Mを 10 から 11 に遷移
	reg_MI_MC1 = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_11);
    
    // カードブート用構造体の初期化
	MI_CpuClear32(&s_cbData, sizeof(CardBootData));

    // バッファの初期化
    MI_CpuClear8(s_pBootSegBuffer, s_BootSegBufSize);
    MI_CpuClear8(s_pSecureSegBuffer, s_SecureSegBufSize);

    // MC_CNT1を初期化
    reg_MCCNT1 = 0x0;

    // カードロックIDの開放
	OS_ReleaseLockID( s_cbData.lockID );
	
#ifdef USE_SLOT_A
	OS_SetIrqCheckFlagEx(OS_IE_CARD_A_IREQ);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_IREQ);
#endif
}

// カードB挿し
static void InterruptCallbackCardDet(void)
{
    // カードブートスレッドを起動する
	OS_WakeupThreadDirect(&s_MCThread);

#ifdef USE_SLOT_A
    OS_SetIrqCheckFlagEx(OS_IE_CARD_A_DET);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_DET);
#endif
}

// カードB データ転送終了
static void InterruptCallbackCardData(void)
{
#ifdef USE_SLOT_A
    OS_SetIrqCheckFlagEx(OS_IE_CARD_A_DATA);
#else
    OS_SetIrqCheckFlagEx(OS_IE_CARD_B_DATA);
#endif
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
#ifdef USE_SLOT_A
  	SetInterruptCallback( OS_IE_CARD_A_IREQ , InterruptCallbackCard );
  	SetInterruptCallback( OS_IE_CARD_A_DET  , InterruptCallbackCardDet );
  	SetInterruptCallback( OS_IE_CARD_A_DATA , InterruptCallbackCardData );
#else
  	SetInterruptCallback( OS_IE_CARD_B_IREQ , InterruptCallbackCard );
  	SetInterruptCallback( OS_IE_CARD_B_DET  , InterruptCallbackCardDet );
  	SetInterruptCallback( OS_IE_CARD_B_DATA , InterruptCallbackCardData );
#endif
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
	OS_TPrintf("Debug Data -------------------------------\n");
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
    OS_TPrintf("MC コントロールレジスタ0	 (SEL etc)     : %04x\n", reg_MCCNT0);
    OS_TPrintf("MC コントロールレジスタ1	 (START etc)   : %08x\n", reg_MCCNT1);
    OS_TPrintf("----------------------------------------------------------\n");
}
