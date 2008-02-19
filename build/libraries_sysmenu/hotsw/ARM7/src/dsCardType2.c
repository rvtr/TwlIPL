/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     dsCardType2.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<dsCardType2.h>
#include	<customNDma.h>

// Define Data --------------------------------------------------------------
#define		SECURE_SEGMENT_NUM					4
#define		ONE_SEGMENT_PAGE_NUM				8
#define		COMMAND_DECRYPTION_WAIT				25 		// 25ms

// Function prototype -------------------------------------------------------
static void SetSecureCommand(SecureCommandType type, CardBootData *cbd);
static void SetMCSCR(void);


// ===========================================================================
// 	Function Describe
// ===========================================================================

// ■--------------------------------------■
// ■       ノーマルモードのコマンド       ■
// ■--------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         ReadIDNormal_DSType2
  
  Description:  Type2のノーマルモードのID読み込み
 *---------------------------------------------------------------------------*/
// 共通

/*---------------------------------------------------------------------------*
 * Name:         ReadBootSegNormal_DSType2
 * 
 * Description:  Type2のノーマルモードのBoot Segment読み込み
 * 
 * CT=240ns  Latency1=0x1fff  Latency2=0x3f  Pagecount=8page
 *---------------------------------------------------------------------------*/
HotSwState ReadBootSegNormal_DSType2(CardBootData *cbd)
{
	u32 		i = 0;
    u32			loop = ONE_SEGMENT_PAGE_NUM;
    u32 		*dst = cbd->pBootSegBuf->word;
    u64 		page = 0;
    GCDCmd64 	cndLE, cndBE;
    
    for(i=0; i<ONE_SEGMENT_PAGE_NUM; i++){
    	if(!HOTSW_IsCardExist()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
		// NewDMA転送の準備
        HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, cbd->pBootSegBuf->word + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
        
        // ゼロクリア
		MI_CpuClear8(&cndLE, sizeof(GCDCmd64));
        
    	// リトルエンディアンで作って
		cndLE.dw  = 0x0  << 24;
		cndLE.dw |= page << 33;

    	// ビックエンディアンにする
		cndBE.b[0] = cndLE.b[7];
		cndBE.b[1] = cndLE.b[6];
		cndBE.b[2] = cndLE.b[5];
		cndBE.b[3] = cndLE.b[4];
		cndBE.b[4] = cndLE.b[3];
		cndBE.b[5] = cndLE.b[2];
		cndBE.b[6] = cndLE.b[1];
		cndBE.b[7] = cndLE.b[0];
    
		// MCCMD レジスタ設定
    	reg_HOTSW_MCCMD0 = *(u32 *)cndBE.b;
		reg_HOTSW_MCCMD1 = *(u32 *)&cndBE.b[4];

		// MCCNT1 レジスタ設定
		reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x1 << PC_SHIFT) | LATENCY2_MASK | LATENCY1_MASK;

		// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
		OS_SleepThread(NULL);

        page++;
    }

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ChangeModeNormal_DSType2
 * 
 * Description:  Type2のノーマルモードのモード変更
 * 
 * CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
// Type1と同じ


// ■--------------------------------------■
// ■       セキュアモードのコマンド       ■
// ■--------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         SetSecureCommand
  
  Description:  
 *---------------------------------------------------------------------------*/
static void SetSecureCommand(SecureCommandType type, CardBootData *cbd)
{
	GCDCmd64 cndLE, cndBE;
    u64 data;

    // ゼロクリア
	MI_CpuClear8(&cndLE, sizeof(GCDCmd64));
	data = (type == S_PNG_ON) ? (u64)cbd->vd : (u64)cbd->vae;

    cndLE.dw  = cbd->vbi;
    cndLE.dw |= data << 20;
    
    // comannd0部分
	switch(type){
      case S_RD_ID:
        cndLE.dw |= 0x1000000000000000;
        break;
        
      case S_PNG_ON:
        cndLE.dw |= 0x4000000000000000;
        break;

      case S_PNG_OFF:
        cndLE.dw |= 0x6000000000000000;
        break;

      case S_CHG_MODE:
        cndLE.dw |= 0xa000000000000000;
        break;
    }

    if(!cbd->debuggerFlg){
    	// コマンドの暗号化
		EncryptByBlowfish( &cbd->keyTable, (u32*)&cndLE.b[4], (u32*)cndLE.b );
    }

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
}


/*---------------------------------------------------------------------------*
 * Name:         ReadIDSecure_DSType2
 * 
 * Description:  デバッガを読み込んだ場合はSCRAMBLE_MASK -> CS SE DS をマスク
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=Status
 *---------------------------------------------------------------------------*/
HotSwState ReadIDSecure_DSType2(CardBootData *cbd)
{
	u32 scrambleMask;

    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // スクランブルの設定
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
	// NewDMA転送の準備
	HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
    
    // コマンド作成・設定
	SetSecureCommand(S_RD_ID, cbd);
    
	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
	// 25ms待ち
    OS_Sleep(COMMAND_DECRYPTION_WAIT);

    // MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x7 << PC_SHIFT) | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
	// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
	OS_SleepThread(NULL);
    
    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ReadSegSecure_DSType2
 * 
 * Description:  Secure領域を読み込む関数
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=1page
 *---------------------------------------------------------------------------*/
HotSwState ReadSegSecure_DSType2(CardBootData *cbd)
{
    u32			i,j=0,k;
	u64			segNum = 4;
    u64			vae	= cbd->vae;
    GCDCmd64 	cndLE, cndBE;

    // スクランブルの設定
    u32 scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    for(i=0; i<SECURE_SEGMENT_NUM; i++){
	    if(!HOTSW_IsCardExist()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
		// ゼロクリア
		MI_CpuClear8(&cndLE, sizeof(GCDCmd64));
        
	    cndLE.dw  = cbd->vbi;
	    cndLE.dw |= vae << 20;
		cndLE.dw |= segNum << 44;
	    cndLE.dw |= 0x2000000000000000;
        
	    // コマンドの暗号化
		EncryptByBlowfish( &cbd->keyTable, (u32*)&cndLE.b[4], (u32*)cndLE.b );

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

		// MCCNT1 レジスタ設定
		reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
        
	    // 25ms待ち
    	OS_Sleep(COMMAND_DECRYPTION_WAIT);
        
		for(k=0; k<ONE_SEGMENT_PAGE_NUM; k++){
			// NewDMA転送の準備
			HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, cbd->pSecureSegBuf + (PAGE_WORD_SIZE * j), PAGE_SIZE );

    		// MCCMD レジスタ設定
			reg_HOTSW_MCCMD0 = 0x0;
			reg_HOTSW_MCCMD1 = 0x0;
    		
			// MCCNT1 レジスタ設定
			reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (0x1 << PC_SHIFT) | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
            
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
 * Name:         SwitchONPNGSecure_DSType2
 * 
 * Description:  PNジェネレータをONにする
 * 
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState SwitchONPNGSecure_DSType2(CardBootData *cbd)
{
	u32 scrambleMask;

    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // スクランブルの設定
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_PNG_ON, cbd);
    
	// MCCNT1 レジスタ設定 (START = 1 SE = 1 DS = 1 Latency1 = 0 に)
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
    // 25ms待ち
	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    
    // MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;

	// MCCNT1 レジスタ設定 (START = 1 SE = 1 DS = 1 Latency1 = 0 に)
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

	// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
	OS_SleepThread(NULL);

    // コマンドカウンタインクリメント
    cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         SwitchOFFPNGSecure_DSType2
 * 
 * Description:  PNジェネレータをOFFする
 * 
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState SwitchOFFPNGSecure_DSType2(CardBootData *cbd)
{
	u32 scrambleMask;

    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // スクランブルの設定
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_PNG_OFF, cbd);
    
	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

    // 25ms待ち
	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    
    // MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

	// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
	OS_SleepThread(NULL);

    // コマンドカウンタインクリメント
    cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ChangeModeSecure_DSType2
 * 
 * Description:  Gameモードに移行する
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeSecure_DSType2(CardBootData *cbd)
{
	u32 scrambleMask;

    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // スクランブルの設定
    scrambleMask = cbd->debuggerFlg ? 0 : (u32)(SECURE_COMMAND_SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_CHG_MODE, cbd);
    
	// MCCNT1 レジスタ設定 (START = 1 SE = 1 DS = 1 Latency1 = 0 に)
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
    // 25ms待ち
	OS_Sleep(COMMAND_DECRYPTION_WAIT);
    
    // MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x0;
	reg_HOTSW_MCCMD1 = 0x0;
    
	// MCCNT1 レジスタ設定 (START = 1 SE = 1 DS = 1 Latency1 = 0 に)
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    
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
  Name:         ReadIDGame_DSType2
  
  Description:  ゲームモードでIDを読み込む
 *---------------------------------------------------------------------------*/
// Type1と同じ


/*---------------------------------------------------------------------------*
 * Name:         ReadPageGame_DSType1
 * 
 * Description:  ゲームモードで、指定されたページを指定バッファに指定サイズ分を読み込む
 *
 * CT=150ns  Latency1=0x657  Latency2=0x1  Pagecount=1page
 *---------------------------------------------------------------------------*/
HotSwState ReadPageGame_DSType2(CardBootData *cbd, u32 start_addr, void* buf, u32 size)
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
	    if(!HOTSW_IsCardExist()){
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
		reg_HOTSW_MCCNT1 = cbd->pBootSegBuf->rh.s.game_cmd_param |
            				START_MASK | (PC_MASK & (0x1 << PC_SHIFT));
        
		// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
		OS_SleepThread(NULL);
    }

    return HOTSW_SUCCESS;
}