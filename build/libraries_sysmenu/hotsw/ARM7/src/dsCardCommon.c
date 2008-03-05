/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     dsCardType1.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<dsCardCommon.h>
#include	<customNDma.h>

// define -------------------------------------------------------------------
#define		SECURE_SEGMENT_NUM					4
#define		ONE_SEGMENT_PAGE_NUM				8
#define		COMMAND_DECRYPTION_WAIT				25 		// 25ms

#define		ROM_EMULATION_START_OFS				0x160
#define		ROM_EMULATION_END_OFS				0x180

// Function prototype -------------------------------------------------------
static HotSwState HOTSWi_ChangeModeNormal(CardBootData *cbd, u64 cmd);
static void PreSendSecureCommand(CardBootData *cbd, u32 *scrambleMask);


// ===========================================================================
// 	Function Describe
// ===========================================================================

void HOTSWi_SetCommand(GCDCmd64 *cndLE)
{
	GCDCmd64 cndBE;

    // ビッグエンディアンに直す
	cndBE.b[7] = cndLE->b[0];
	cndBE.b[6] = cndLE->b[1];
    cndBE.b[5] = cndLE->b[2];
    cndBE.b[4] = cndLE->b[3];
    cndBE.b[3] = cndLE->b[4];
    cndBE.b[2] = cndLE->b[5];
    cndBE.b[1] = cndLE->b[6];
    cndBE.b[0] = cndLE->b[7];

	//---- confirm CARD free
	while( reg_HOTSW_MCCNT1 & REG_MI_MCCNT1_START_MASK ){}

    // MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = *(u32*)cndBE.b;
	reg_HOTSW_MCCMD1 = *(u32*)&cndBE.b[4];
}


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
	GCDCmd64 cndLE;

	// カード割り込みによるDMAコピー
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_nml, sizeof(cbd->id_nml) );

   	// リトルエンディアンで作って
	cndLE.dw  = HSWOP_N_OP_RD_ID;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 PC = 111(ステータスリード) latency1 = 1 に)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | (0x1 & LATENCY1_MASK);

    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
 * Name:         ReadBootSegNormal
 * 
 * Description:  ノーマルモードのBoot Segment読み込み
 *
 * CT=240ns  Latency1=0x1fff  Latency2=0x3f  Pagecount=8page
 *---------------------------------------------------------------------------*/
HotSwState ReadBootSegNormal(CardBootData *cbd)
{
	u32 		i, loop, pc, size;
    u32 		*dst = cbd->pBootSegBuf->word;
    u32			temp;
    u64 		page = 0;
	GCDCmd64 	cndLE;

    if(cbd->cardType == DS_CARD_TYPE_1){
    	loop = 0x1UL;
    	pc   = 0x4UL;
    	size = BOOT_SEGMENT_SIZE;
    }
    else{
    	loop = ONE_SEGMENT_PAGE_NUM;
    	pc   = 0x1UL;
    	size = PAGE_SIZE;
    }

    // secure2モード移行の為、Boot Segmentを1ページ分読み込む。データは捨てバッファに格納
    if(cbd->modeType == HOTSW_MODE2){
    	loop = 0x1UL;
    	pc   = 0x1UL;
    	size = PAGE_SIZE;
    }
    
    for(i=0; i<loop; i++){
    	if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}

        if(cbd->modeType == HOTSW_MODE1){
			// NewDMA転送の準備
        	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, dst + (u32)(PAGE_WORD_SIZE*i), size );
        }
        else{
			// NewDMA転送（読み捨て）の準備
			// Mode2のときは、データを捨てる。
    		HOTSW_NDmaPipe_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &temp, size );
        }

    	// リトルエンディアンで作って
		cndLE.dw  = HSWOP_N_OP_RD_PAGE;
		cndLE.dw |= page << HSWOP_N_RD_PAGE_ADDR_SHIFT;

		// MCCMD レジスタ設定
		HOTSWi_SetCommand(&cndLE);

		// MCCNT1 レジスタ設定
		reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (pc << PC_SHIFT) | LATENCY2_MASK | LATENCY1_MASK;

		// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
		OS_SleepThread(NULL);

        page++;
    }

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
 * Name:         ChangeModeNormal
 * 
 * Description:  ノーマルモードからセキュアモードへの変更
 * 
 * CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeNormal(CardBootData *cbd)
{
	return HOTSWi_ChangeModeNormal(cbd, HSWOP_N_OP_CHG_MODE);
}

/*---------------------------------------------------------------------------*
 * Name:         ChangeModeNorma2
 * 
 * Description:  ノーマルモードからセキュア２モードへの変更
 * 
 * CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeNormal2(CardBootData *cbd)
{
	return HOTSWi_ChangeModeNormal(cbd, HSWOP_N_OP_CHG2_MODE);
}


static HotSwState HOTSWi_ChangeModeNormal(CardBootData *cbd, u64 cmd)
{
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // リトルエンディアンで作って
    cndLE.dw  = cmd;
	cndLE.dw |= cbd->vbi << HSWOP_N_VBI_SHIFT;
	cndLE.dw |= (u64)cbd->vae << HSWOP_N_VAE_SHIFT;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

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
	GCDCmd64 cndLE;
	u32 temp;
    
	// NewDMA転送（読み捨て）の準備
	HOTSW_NDmaPipe_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &temp, HOTSW_LOAD_TABLE_SIZE );

    // リトルエンディアンで作って
    cndLE.dw  = HSWOP_N_OP_LD_TABLE;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

    // MCCNT1 レジスタ設定 (START = 1 W/R = 0 PC = 101(16ページ) latency1 = 0(必要ないけど) に)
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x5 << PC_SHIFT);

    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

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


// ■--------------------------------------■
// ■       セキュアモードのコマンド       ■
// ■--------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         SetSecureCommand
  
  Description:  
 *---------------------------------------------------------------------------*/
static void SetSecureCommand(SecureCommandType type, CardBootData *cbd)
{
	GCDCmd64 cndLE;
    u64 data;

    // comannd0部分
	switch(type){
      case S_RD_ID:
        cndLE.dw = HSWOP_S_OP_RD_ID;
        break;
        
      case S_PNG_ON:
        cndLE.dw = HSWOP_S_OP_PNG_ON;
        break;

      case S_PNG_OFF:
        cndLE.dw = HSWOP_S_OP_PNG_OFF;
        break;

      case S_CHG_MODE:
        cndLE.dw = HSWOP_S_OP_CHG_MODE;
        break;
    }

	data = (type == S_PNG_ON) ? (u64)cbd->vd : (u64)cbd->vae;
    
    cndLE.dw |= cbd->vbi;
    cndLE.dw |= data << HSWOP_S_VA_SHIFT;
    
    if(!cbd->debuggerFlg){
    	// コマンドの暗号化
		EncryptByBlowfish( &cbd->keyTable, (u32*)&cndLE.b[4], (u32*)cndLE.b );
    }

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);
}


/*---------------------------------------------------------------------------*
  Name:         PreSendSecureCommand
  
  Description:  
 *---------------------------------------------------------------------------*/
static void PreSendSecureCommand(CardBootData *cbd, u32 *scrambleMask)
{
    // ★ TWL-ROM＆NTR-3DM対応
    if(cbd->cardType == DS_CARD_TYPE_2){
		// MCCNT1 レジスタ設定
		reg_HOTSW_MCCNT1 = START_MASK | *scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

		// 25ms待ち
    	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    }
    // ★ NTR-MROM対応
    else{
		*scrambleMask |= TRM_MASK;
    }
}


/*---------------------------------------------------------------------------*
 * Name:         ReadIDSecure
 * 
 * Description:
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=Status
 *---------------------------------------------------------------------------*/
HotSwState ReadIDSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // スクランブルの設定
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_RD_ID, cbd);

	// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
	PreSendSecureCommand(cbd, &scrambleMask);

	// NewDMA転送の準備
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
    
	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ReadSegSecure
 * 
 * Description:
 *---------------------------------------------------------------------------*/
HotSwState ReadSegSecure(CardBootData *cbd)
{
    u32 		scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
	u32			*buf = (cbd->modeType == HOTSW_MODE1) ? cbd->pSecureSegBuf : cbd->pSecure2SegBuf;
    u32			loop, pc, size, interval, i, j=0, k;
	u64			segNum = 4;
	GCDCmd64 	cndLE;

    if(cbd->cardType == DS_CARD_TYPE_1){
    	loop	 = 0x1UL;
    	pc   	 = 0x4UL;
    	size 	 = ONE_SEGMENT_SIZE;
        interval = ONE_SEGMENT_WORD_SIZE;
    }
    else{
    	loop 	 = ONE_SEGMENT_PAGE_NUM;
    	pc   	 = 0x1UL;
    	size 	 = PAGE_SIZE;
        interval = PAGE_WORD_SIZE;
    }
    
    for(i=0; i<SECURE_SEGMENT_NUM; i++){
		if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}

	    cndLE.dw  = HSWOP_S_OP_RD_SEG;
	    cndLE.dw |= cbd->vbi;
	    cndLE.dw |= (u64)cbd->vae << HSWOP_S_VA_SHIFT;
		cndLE.dw |= segNum << HSWOP_S_VC_SHIFT;
        
	    // コマンドの暗号化
		EncryptByBlowfish( &cbd->keyTable, (u32*)&cndLE.b[4], (u32*)cndLE.b );

		// MCCMD レジスタ設定
		HOTSWi_SetCommand(&cndLE);

		// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
		PreSendSecureCommand(cbd, &scrambleMask);

        for(k=0; k<loop; k++){
			// NewDMA転送の準備
		    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, buf + (interval*j), size );

			// MCCNT1 レジスタ設定
			reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (pc << PC_SHIFT) | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

    		// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    		OS_SleepThread(NULL);

            // 転送済みページ数
            j++;
        }
        
        // 読み込みセグメント番号インクリメント
		segNum++;
        
    	// コマンドカウンタインクリメント
		cbd->vbi++;
    }

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         SwitchONPNGSecure
 * 
 * Description:
 *---------------------------------------------------------------------------*/
HotSwState SwitchONPNGSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // スクランブルの設定
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_PNG_ON, cbd);

	// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
	PreSendSecureCommand(cbd, &scrambleMask);

	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         SwitchOFFPNGSecure
 * 
 * Description:
 *---------------------------------------------------------------------------*/
HotSwState SwitchOFFPNGSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // スクランブルの設定
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_PNG_OFF, cbd);

	// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
	PreSendSecureCommand(cbd, &scrambleMask);

	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ChangeModeSecure
 * 
 * Description:
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // スクランブルの設定
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_CHG_MODE, cbd);

	// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
	PreSendSecureCommand(cbd, &scrambleMask);

	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    // コマンドカウンタインクリメント
	cbd->vbi++;

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
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
	// NewDMA転送の準備
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_gam, sizeof(cbd->id_gam) );

   	// リトルエンディアンで作って
	cndLE.dw  = HSWOP_G_OP_RD_ID;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

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
	GCDCmd64	cndLE;

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

        // コマンド作成
		cndLE.dw  = HSWOP_G_OP_RD_PAGE;
		cndLE.dw |= (page + i) << HSWOP_G_RD_PAGE_ADDR_SHIFT;

		// MCCMD レジスタ設定
		HOTSWi_SetCommand(&cndLE);

   		// MCCNT1 レジスタ設定 (START = 1 W/R = 0 PC = 001(1ページリード) その他Romヘッダの情報におまかせ)
		reg_HOTSW_MCCNT1 = cbd->gameCommondParam |
            				START_MASK | (PC_MASK & (0x1 << PC_SHIFT));
        
		// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
		OS_SleepThread(NULL);
    }

    return HOTSW_SUCCESS;
}