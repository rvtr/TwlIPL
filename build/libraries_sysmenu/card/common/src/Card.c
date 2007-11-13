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

#include	<nitro/card/types.h>
#include 	"blowfish.h"
#include 	"Card.h"
#include	"dsCardType1.h"

// define -------------------------------------------------------------------
#define		STACK_SIZE							1024		// スタックサイズ
#define		MC_THREAD_PRIO						11			// カード電源ON → ゲームモードのスレッド優先度

// Function prototype -------------------------------------------------------
static BOOL IsCardExist(void);

static void SetInterruptCallback( OSIrqMask intr_bit, OSIrqFunction func );
static void SetInterruptCallbackEx( OSIrqMask intr_bit, void *func );
static void SetInterrupt(void);

static void InterruptCallbackCard(void);
static void InterruptCallbackCardDet(void);
static void InterruptCallbackCardData(void);

static void McThread(void *arg);
static void McPowerOn(void);
static void SetMCSCR(void);

static void ShowRegisterData(void);
static void ShowRomHeaderData(void);

// Static Values ------------------------------------------------------------
static u64  				s_MCStack[STACK_SIZE / sizeof(u64)];
static OSThread 			s_MCThread;

static u32					s_SecureSegBufSize, s_BootSegBufSize;

static u32					*s_pSecureSegBuffer;	// カード抜けてもバッファの場所覚えとく
static BootSegmentData		*s_pBootSegBuffer;		// カード抜けてもバッファの場所覚えとく

static CardBootData			s_cbData;

static CardBootFunction  	s_funcTable[] = {
    {ReadIDNormal_DSType1, ReadBootSegNormal_DSType1, ChangeModeNormal_DSType1,
     ReadIDSecure_DSType1, ReadSegSecure_DSType1, SwitchONPNGSecure_DSType1, ChangeModeSecure_DSType1,
     ReadIDGame_DSType1}
};

// ===========================================================================
// 	Function Describe
// ===========================================================================
/*---------------------------------------------------------------------------*
  Name:         Card_Init
  Arguments:    None.
  Returns:      None.
 *---------------------------------------------------------------------------*/
void Card_Init(void)
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
	reg_MI_MC1 = (u32)((reg_MI_MC1 & 0xffff) | 0xfff0000);

	// Counter-Aの値を設定
    reg_MI_MC2 = 0xfff;
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

	// カードブート用構造体の初期化
	MI_CpuClear32(&s_cbData, sizeof(CardBootData));
}

/* -----------------------------------------------------------------
 * Card_Boot関数
 *
 * カード起動をスタート
 *
 * ※BootSegmentBuffer SecureSegmentBufferの設定を行ってから
 *   この関数を呼んでください。
 * ----------------------------------------------------------------- */
void Card_Boot(void)
{
	OS_TPrintf("---------------- Card Boot Start ---------------\n");
    
	// カード電源ON
	McPowerOn();
    
	// VAE・VBI・VD値の設定
    s_cbData.vae = VAE_VALUE;
    s_cbData.vbi = VBI_VALUE;
	s_cbData.vd  = VD_VALUE;

	// セキュア領域の読み込みセグメント先頭番号(Segment4 ～ Segment 7)
    s_cbData.secureSegNum = 4;

	// バッファを設定
    s_cbData.pBootSegBuf   = s_pBootSegBuffer;
    s_cbData.pSecureSegBuf = s_pSecureSegBuffer;

	if(IsCardExist()){
    	// ---------------------- Normal Mode ----------------------
    	// カードID読み込み
		s_funcTable[0].ReadID_N(&s_cbData);
    	
    	// Boot Segment読み込み
    	s_funcTable[0].ReadBootSegment_N(&s_cbData);

		// TWLカードかNTRカードか判定 (Platform code : bit0 : not support NTR,  bit1 : support TWL)
        if(s_cbData.pBootSegBuf->rh.s.platform_code & 0x02){
			OS_TPrintf("TWL Card.\n");
            
            s_cbData.twlFlg = TRUE;
        }

    	// Key Table初期化
    	GCDm_MakeBlowfishTableDS(&s_cbData.keyTable, &s_pBootSegBuffer->rh.s, s_cbData.keyBuf, 8);

    	// セキュアモードに移行
    	s_funcTable[0].ChangeMode_N(&s_cbData);

    	// ---------------------- Secure Mode ----------------------
		// PNG設定
		s_funcTable[0].SetPNG_S(&s_cbData);
    
		// DS側符号生成回路初期値設定 (レジスタ設定)
		SetMCSCR();

		// ID読み込み
    	s_funcTable[0].ReadID_S(&s_cbData);

    	// Secure領域のSegment読み込み
    	s_funcTable[0].ReadSegment_S(&s_cbData);

    	// ゲームモードに移行
		s_funcTable[0].ChangeMode_S(&s_cbData);
    
    	// ---------------------- Game Mode ----------------------
    	// ID読み込み
		s_funcTable[0].ReadID_G(&s_cbData);

		OS_TPrintf("-----------------------------------------------\n\n");
    }
    else{
		OS_TPrintf("Card Not Found\n");
    }
}

/* -----------------------------------------------------------------
 * IsCardExist関数
 *
 * カードの存在判定
 *
 * ※SCFG_MC1のSlot B モード選択フラグを見ている
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
 * McPowerOn関数
 * ----------------------------------------------------------------- */
static void McPowerOn(void)
{
	OSTick start;

    // SCFG_MC1 の Slot2 Status の M1,M0 を 01 にする
    reg_MI_MC1  = (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_01);
    
	// 100ms待ち
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 100){}

    // SCFG_MC1 の Slot2 Status の M1,M0 を 10 にする
	reg_MI_MC1 	= (u32)((reg_MI_MC1 & (~SLOT_STATUS_MODE_SELECT_MSK)) | SLOT_STATUS_MODE_10);

    // リセットをhighに (RESB = 1にする)
	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(1,1,0,1,1,1,1,1,1,1,1,1,1)) |
                             		 CNT1_FLD(0,0,1,0,0,0,0,0,0,0,0,0,0));

	// 100ms待ち
    start = OS_GetTick();
    while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 100){}

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
	OS_TPrintf("Rom Header Data -------------------------------\n");
    OS_TPrintf("titleName           : %s\n", s_pBootSegBuffer->rh.s.title_name);
    OS_TPrintf("initialCode         : %x\n", *(u32 *)s_pBootSegBuffer->rh.s.game_code);
    OS_TPrintf("-----------------------------------------------\n");
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
