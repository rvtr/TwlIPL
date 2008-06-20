/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     romEmulation.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<dsCardCommon.h>
#include 	<romEmulation.h>
#include	<customNDma.h>

// Define Data --------------------------------------------------------------
#define		BOOT_PAGE_NUM				8
#define		SECURE_PAGE_NUM				32

// static value -------------------------------------------------------------
static OSMessage	s_Msg;

// extern -------------------------------------------------------------------
extern CardThreadData HotSwThreadData;



// ===========================================================================
// 	Function Describe
// ===========================================================================

// ■--------------------------------------■
// ■       ノーマルモードのコマンド       ■
// ■--------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         ReadIDNormal_ROMEMU
  
  Description:  DSカードType1のノーマルモードのID読み込み
 *---------------------------------------------------------------------------*/
// 共通

/*---------------------------------------------------------------------------*
  Name:         ReadBootSegNormal_ROMEMU
  
  Description:  DSカードType1のノーマルモードのBoot Segment読み込み
 *---------------------------------------------------------------------------*/
// 共通

/*---------------------------------------------------------------------------*
  Name:         ChangeModeNormal_ROMEMU
  
  Description:  DSカードType1のノーマルモードのモード変更
 *---------------------------------------------------------------------------*/
// 共通


// ■--------------------------------------■
// ■       セキュアモードのコマンド       ■
// ■--------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         ReadIDSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState ReadIDSecure_ROMEMU(CardBootData *cbd)
{
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

#ifdef USE_NEW_DMA
	// カード割り込みによるDMAコピー
	HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
#else
    HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
#endif
    
  	// リトルエンディアンで作って
   	cndLE.dw  = HSWOP_N_OP_RD_ID;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif

	// MCCNT1 レジスタ設定 (START = 1 PC = 111(ステータスリード) latency1 = 1 に)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | (0x1 & LATENCY1_MASK);

    // メッセージ受信
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
    
    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         ReadSegSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState ReadSegSecure_ROMEMU(CardBootData *cbd)
{
	u32	*buf = (cbd->modeType == HOTSW_MODE1) ? cbd->pSecureSegBuf : cbd->pSecure2SegBuf;
	u32 i,j=0;
    u32 keyTable2Adr = (u32)cbd->pBootSegBuf->rh.s.twl_card_keytable_area_rom_offset * TWLCARD_BORDER_OFFSET;
    u32 Secure2Adr   = (cbd->modeType == HOTSW_MODE1) ? HOTSW_SECURE_AREA_OFS : (keyTable2Adr + HOTSW_SECURE2_AREA_OFS);
    u64 page = Secure2Adr/HOTSW_PAGE_SIZE;
	GCDCmd64 cndLE;
	u32 n = 0;
    
    for(i=0; i<SECURE_PAGE_NUM; i++){
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
#ifdef USE_NEW_DMA
		// NewDMA転送の準備
		HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
#else
		HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
#endif
        
    	// リトルエンディアンで作って
    	cndLE.dw  = HSWOP_N_OP_RD_PAGE;
    	cndLE.dw |= page << HSWOP_N_RD_PAGE_ADDR_SHIFT;

		// MCCMD レジスタ設定
		HOTSWi_SetCommand(&cndLE);

		// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif

		// MCCNT1 レジスタ設定 (START = 1 PC_MASK PC = 001(1ページリード)に latency1 = 0xd)
		reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x1 << PC_SHIFT) | (0xd & LATENCY1_MASK);

    	// メッセージ受信
		OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

        page++;
    }

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SwitchONPNGSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState SwitchONPNGSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         SwitchOFFPNGSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState SwitchOFFPNGSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
    
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // リトルエンディアンで作って
    cndLE.dw = HSWOP_S_OP_CHG_MODE;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 レジスタ設定 (START = 1 に)
	reg_HOTSW_MCCNT1 = START_MASK;
    
    HOTSW_WaitCardCtrl();

    return HOTSW_SUCCESS;
}

// ■------------------------------------■
// ■       ゲームモードのコマンド       ■
// ■------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         ReadIDGame_ROMEMU
  
  Description:  ゲームモードでIDを読み込む
 *---------------------------------------------------------------------------*/
// 共通

/*---------------------------------------------------------------------------*
  Name:         ReadPageGame_ROMEMU
  
  Description:  ゲームモードで、指定されたページを指定バッファに指定サイズ分を読み込む
 *---------------------------------------------------------------------------*/
// 共通
