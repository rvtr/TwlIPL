/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     dsCardType1.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<dsCardCommon.h>
#include	<customNDma.h>

// define -------------------------------------------------------------------
#define		ROM_EMULATION_START_OFS				0x160
#define		ROM_EMULATION_END_OFS				0x180

// Function prototype -------------------------------------------------------


// ===========================================================================
// 	Function Describe
// ===========================================================================

// ■------------------------------------■
// ■       ノーマルモードのコマンド     ■
// ■------------------------------------■
/* -----------------------------------------------------------------
 * ReadIDNormal関数
 *
 * ノーマルモード時のカードIDを読み込む関数
 * ----------------------------------------------------------------- */
HotSwState ReadIDNormal(CardBootData *cbd)
{
	// カード割り込みによるDMAコピー
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_nml, sizeof(cbd->id_nml) );

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
 * Name:         ChangeModeNormal
 * 
 * Description:  Type1のノーマルモードのモード変更
 * 
 * CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeNormal(CardBootData *cbd)
{
	GCDCmd64 tempCnd, cnd;
    u64 vae64 = cbd->vae;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // ゼロクリア
	MI_CpuClear8(&tempCnd, sizeof(GCDCmd64));
    
    // リトルエンディアンで作って
	tempCnd.dw  = cbd->vbi << 8;
	tempCnd.dw |= vae64 << 32;
    tempCnd.dw |= 0x3c00000000000000;

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

	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~SCRAMBLE_MASK) |
        				START_MASK | (PC_MASK & (0x0 << PC_SHIFT));
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    return HOTSW_SUCCESS;
}

/* -----------------------------------------------------------------
 * LoadTable関数
 *
 * カード側の Key Table をロードする関数。
 *
 * ※この関数は開発カード用に発行しないといけない。
 *   製品版カードの場合、このコマンドは無視される
 * ----------------------------------------------------------------- */
HotSwState LoadTable(void)
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
HotSwState ReadRomEmulationData(CardBootData *cbd)
{
	u32 count=0;
    u32 temp;
    u32 *dst = cbd->romEmuBuf;

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

   	MI_CpuCopyFast(cbd->romEmuBuf, (void*)HW_ISD_RESERVED, 32);

    return HOTSW_SUCCESS;
}

// ■------------------------------------■
// ■       ゲームモードのコマンド       ■
// ■------------------------------------■
/*---------------------------------------------------------------------------*
 * Name:         ReadIDGame
 * 
 * Description:  ゲームモードでIDを読み込む
 *---------------------------------------------------------------------------*/
HotSwState ReadIDGame(CardBootData *cbd)
{
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
	// NewDMA転送の準備
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_gam, sizeof(cbd->id_gam) );

	// MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x000000B8;
	reg_HOTSW_MCCMD1 = 0x00000000;

   	// MCCNT1 レジスタ設定 (START = 1 W/R = 0 PC = 111(ステータスリード) その他Romヘッダの情報におまかせ)
	reg_HOTSW_MCCNT1 = cbd->gameCommondParam |
        				START_MASK | (PC_MASK & (0x7 << PC_SHIFT));
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ReadPageGame
 * 
 * Description:  ゲームモードで、指定されたページを指定バッファに指定サイズ分を読み込む
 *
 * CT=150ns  Pagecount=1page  Latency=RomHeaderで指定の値
 *---------------------------------------------------------------------------*/
HotSwState ReadPageGame(CardBootData *cbd, u32 start_addr, void* buf, u32 size)
{
    u32 		loop, counter=0;
	u64			i, page;
    GCDCmd64 	cndLE, cndBE;

    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

//	OS_TPrintf("Src Addr : 0x%08x  Dst Addr : 0x%08x\n", start_addr, buf);
//	OS_TPrintf("Read Game Segment  Page Count : %d   size : %x\n", loop, size);
    
    for(i=0; i<loop; i++){
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
		// NewDMA転送の準備
		HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, (u32 *)buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );

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
        
   		// MCCNT1 レジスタ設定 (START = 1 W/R = 0 PC = 001(1ページリード) その他Romヘッダの情報におまかせ)
		reg_HOTSW_MCCNT1 = cbd->gameCommondParam |
            				START_MASK | (PC_MASK & (0x1 << PC_SHIFT));
        
		// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
		OS_SleepThread(NULL);
    }

    return HOTSW_SUCCESS;
}