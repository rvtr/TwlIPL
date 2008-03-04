/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     romEmulation.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<romEmulation.h>
#include	<customNDma.h>

// Define Data --------------------------------------------------------------
#define		BOOT_PAGE_NUM				8
#define		SECURE_PAGE_NUM				32



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
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
	// カード割り込みによるDMAコピー
	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
    
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

/*---------------------------------------------------------------------------*
  Name:         ReadSegSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
HotSwState ReadSegSecure_ROMEMU(CardBootData *cbd)
{
	u32 i,j=0;
    u64 page = 0x20;
	GCDCmd64 tempCnd, cnd;
	u32 n = 0;
    
    for(i=0; i<SECURE_PAGE_NUM; i++){
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
    	// ゼロクリア
		MI_CpuClear8(&tempCnd, sizeof(GCDCmd64));
    
    	// リトルエンディアンで作って
    	tempCnd.dw = page << 33;

    	// ビックエンディアンにする
		cnd.b[0] = tempCnd.b[7];
		cnd.b[1] = tempCnd.b[6];
		cnd.b[2] = tempCnd.b[5];
		cnd.b[3] = tempCnd.b[4];
		cnd.b[4] = tempCnd.b[3];
		cnd.b[5] = tempCnd.b[2];
		cnd.b[6] = tempCnd.b[1];
		cnd.b[7] = tempCnd.b[0];
    
		// MCCMD レジスタ設定
	    reg_HOTSW_MCCMD0 = *(u32 *)cnd.b;
		reg_HOTSW_MCCMD1 = *(u32 *)&cnd.b[4];

		// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

		// MCCNT1 レジスタ設定 (START = 1 PC_MASK PC = 001(1ページリード)に latency1 = 0xd)
		reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x1 << PC_SHIFT) | (0xd & LATENCY1_MASK);
    
		// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
		while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
			while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
        	*(cbd->pSecureSegBuf + j++) = reg_HOTSW_MCD1;
		}
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
    
	GCDCmd64 tempCnd, cnd;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // ゼロクリア
	MI_CpuClear8(&tempCnd, sizeof(GCDCmd64));
    
    // リトルエンディアンで作って
    tempCnd.dw = 0xa000000000000000;

    // ビックエンディアンにする
	cnd.b[0] = tempCnd.b[7];
	cnd.b[1] = tempCnd.b[6];
	cnd.b[2] = tempCnd.b[5];
	cnd.b[3] = tempCnd.b[4];
	cnd.b[4] = tempCnd.b[3];
	cnd.b[5] = tempCnd.b[2];
	cnd.b[6] = tempCnd.b[1];
	cnd.b[7] = tempCnd.b[0];
    
	// MCCMD レジスタ設定
    reg_HOTSW_MCCMD0 = *(u32 *)cnd.b;
	reg_HOTSW_MCCMD1 = *(u32 *)&cnd.b[4];

	// MCCNT1 レジスタ設定 (START = 1 に)
	reg_HOTSW_MCCNT1 = START_MASK;
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

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
