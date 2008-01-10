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
void ReadBootSegNormal_ROMEMU(CardBootData *cbd)
{
	u32 i,j=0;
    u64 page = 0;
	GCDCmd64 tempCnd, cnd;
//	u32 n = 0;
    
    for(i=0; i<BOOT_PAGE_NUM; i++){
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
        	*(cbd->pBootSegBuf->word + j++) = reg_HOTSW_MCD1;
		}

        page++;
    }

/*
	// NewDMA転送の準備
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->pBootSegBuf->word, BOOT_SEGMENT_SIZE );
    
	// MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x00000000;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 PC_MASK PC = 100(8ページリード)に latency1 = 0xd)
	reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x4 << PC_SHIFT) | (0xd & LATENCY1_MASK);
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);
*/
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeNormal_ROMEMU
  
  Description:  DSカードType1のノーマルモードのモード変更
 *---------------------------------------------------------------------------*/
void ChangeModeNormal_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
    
	GCDCmd64 tempCnd, cnd;

    // ゼロクリア
	MI_CpuClear8(&tempCnd, sizeof(GCDCmd64));
    
    // リトルエンディアンで作って
    tempCnd.dw = 0x3c00000000000000;

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
}


// ■--------------------------------------■
// ■       セキュアモードのコマンド       ■
// ■--------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         ReadIDSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void ReadIDSecure_ROMEMU(CardBootData *cbd)
{
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
}

/*---------------------------------------------------------------------------*
  Name:         ReadSegSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void ReadSegSecure_ROMEMU(CardBootData *cbd)
{
	u32 i,j=0;
    u64 page = 0x20;
	GCDCmd64 tempCnd, cnd;
	u32 n = 0;
    
    for(i=0; i<SECURE_PAGE_NUM; i++){
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
//			OS_TPrintf("Secure Data Address : %08x\n", (cbd->pSecureSegBuf + j));
/*			OS_TPrintf("%02x ",reg_HOTSW_MCD1);
            if(!(n++ % 0xf)){
				OS_PutString("\n");
            }*/
		}
        page++;
    }

/*	u64			segNum = 4;
    u64			vae	= cbd->vae;
    GCDCmd64 	cndLE, cndBE;

    // NewDMA転送の準備
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, cbd->pSecureSegBuf, SECURE_SEGMENT_SIZE );

    // コマンド構造体初期化
	MI_CpuClear8(&cndLE, sizeof(GCDCmd64));

    // コマンド作成
	cndLE.dw = 0x20 << 33;

    // ビッグエンディアンに直す(暗号化後)
	cndBE.b[7] = cndLE.b[0];
	cndBE.b[6] = cndLE.b[1];
    cndBE.b[5] = cndLE.b[2];
    cndBE.b[4] = cndLE.b[3];
    cndBE.b[3] = cndLE.b[4];
    cndBE.b[2] = cndLE.b[5];
    cndBE.b[1] = cndLE.b[6];
    cndBE.b[0] = cndLE.b[7];

    // MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = *(u32*)cndBE.b;
	reg_HOTSW_MCCMD1 = *(u32*)&cndBE.b[4];

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 PC_MASK PC = 110(32ページリード)に latency1 = 0xd)
	reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x6 << PC_SHIFT) | (0xd & LATENCY1_MASK);
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);
*/
}

/*---------------------------------------------------------------------------*
  Name:         SwitchONPNGSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void SwitchONPNGSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
}

/*---------------------------------------------------------------------------*
  Name:         SwitchOFFPNGSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void SwitchOFFPNGSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeSecure_ROMEMU
  
  Description:  
 *---------------------------------------------------------------------------*/
void ChangeModeSecure_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
    
	GCDCmd64 tempCnd, cnd;

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
}


// ■------------------------------------■
// ■       ゲームモードのコマンド       ■
// ■------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         ReadIDGame_ROMEMU
  
  Description:  ゲームモードでIDを読み込む
 *---------------------------------------------------------------------------*/
void ReadIDGame_ROMEMU(CardBootData *cbd)
{
	#pragma unused( cbd )
    
	// カード割り込みによるDMAコピー
	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_gam, sizeof(cbd->id_gam) );
    
	// MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x000000B8;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 PC = 111(ステータスリード) latency1 = 1 に)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | (0x1 & LATENCY1_MASK);

    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);
}

/*---------------------------------------------------------------------------*
  Name:         ReadPageGame_ROMEMU
  
  Description:  ゲームモードで、指定されたページを指定バッファに指定サイズ分を読み込む
 *---------------------------------------------------------------------------*/
void ReadPageGame_ROMEMU(CardBootData *cbd, u32 start_addr, void* buf, u32 size)
{
	#pragma unused( cbd )
    
    u32 		loop, counter=0;
	u64			i, page;
    GCDCmd64 	cndLE, cndBE;

    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

    OS_TPrintf("Src Addr : 0x%08x  Dst Addr : 0x%08x\n", start_addr, buf);
    OS_TPrintf("Read Game Segment  Page Count : %d   size : %x\n", loop, size);

    for(i=0; i<loop; i++){
		// ゼロクリア
		MI_CpuClear8(&cndLE, sizeof(GCDCmd64));

        // コマンド作成
		cndLE.dw  = (page + i) << 33;
		cndLE.dw |= 0xB700000000000000;
        
        // ビッグエンディアンに直す(暗号化後)
		cndBE.b[7] = cndLE.b[0];
		cndBE.b[6] = cndLE.b[1];
    	cndBE.b[5] = cndLE.b[2];
    	cndBE.b[4] = cndLE.b[3];
    	cndBE.b[3] = cndLE.b[4];
   		cndBE.b[2] = cndLE.b[5];
    	cndBE.b[1] = cndLE.b[6];
	    cndBE.b[0] = cndLE.b[7];

    	// MCCMD レジスタ設定
		reg_HOTSW_MCCMD0 = *(u32*)cndBE.b;
		reg_HOTSW_MCCMD1 = *(u32*)&cndBE.b[4];
        
		// MCCNT1 レジスタ設定 (START = 1 PC = 111(ステータスリード) latency1 = d に)
		reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x1 << PC_SHIFT) | (0xd & LATENCY1_MASK);
        
		// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
		while(reg_HOTSW_MCCNT1 & START_FLG_MASK){
			while(!(reg_HOTSW_MCCNT1 & READY_FLG_MASK)){}
            *((u32 *)buf + counter++) = reg_HOTSW_MCD1;
		}
    }
}
