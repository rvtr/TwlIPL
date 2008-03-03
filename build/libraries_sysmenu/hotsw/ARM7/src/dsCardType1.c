/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     dsCardType1.c
 *---------------------------------------------------------------------------*/

#include 	<twl.h>
#include	<blowfish.h>
#include 	<dsCardType1.h>
#include	<customNDma.h>

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
  Name:         ReadIDNormal_DSType1
  
  Description:  Type1のノーマルモードのID読み込み
 *---------------------------------------------------------------------------*/
// 共通

/*---------------------------------------------------------------------------*
 * Name:         ReadBootSegNormal_DSType1
 * 
 * Description:  Type1のノーマルモードのBoot Segment読み込み
 *
 * CT=240ns  Latency1=0x1fff  Latency2=0x3f  Pagecount=8page
 *---------------------------------------------------------------------------*/
HotSwState ReadBootSegNormal_DSType1(CardBootData *cbd)
{
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
	// NewDMA転送の準備
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->pBootSegBuf->word, BOOT_SEGMENT_SIZE );
    
	// MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x00000000;
	reg_HOTSW_MCCMD1 = 0x00000000;

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (0x4 << PC_SHIFT) | LATENCY2_MASK | LATENCY1_MASK;
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

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
 * Name:         ReadIDSecure_DSType1
 * 
 * Description:
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=Status
 *---------------------------------------------------------------------------*/
HotSwState ReadIDSecure_DSType1(CardBootData *cbd)
{
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
	// NewDMA転送の準備
    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_scr, sizeof(cbd->id_scr) );
    
    // コマンド作成・設定
	SetSecureCommand(S_RD_ID, cbd);

	// MCCNT1 レジスタ設定
    // (START = 1 TRM = 1 PC = 111(ステータスリード) 後はRomヘッダ情報にお任せ)
	reg_HOTSW_MCCNT1 = cbd->pBootSegBuf->rh.s.secure_cmd_param
        				| START_MASK | TRM_MASK | PC_MASK & (0x7 << PC_SHIFT) | SE_MASK | DS_MASK ;
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ReadSegSecure_DSType1
 * 
 * Description:
 *---------------------------------------------------------------------------*/
HotSwState ReadSegSecure_DSType1(CardBootData *cbd)
{
    u32			i,j=0;
	u64			segNum = 4;
    u64			vae	= cbd->vae;
    GCDCmd64 	cndLE, cndBE;

    for(i=0; i<4; i++){
		if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}
        
		// NewDMA転送の準備
	    HOTSW_NDmaCopy_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, (cbd->pSecureSegBuf + ONE_SEGMENT_WORD_SIZE*i), ONE_SEGMENT_SIZE );
        
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
    	// (START = 1 TRM = 1 PC = 100(8ページリード) 後はRomヘッダ情報にお任せ)
		reg_HOTSW_MCCNT1 = cbd->pBootSegBuf->rh.s.secure_cmd_param
        					| START_MASK | TRM_MASK | PC_MASK & (0x4 << PC_SHIFT) | SE_MASK | DS_MASK;
        
    	// カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    	OS_SleepThread(NULL);
        
        // 読み込みセグメント番号インクリメント
		segNum++;
        
    	// コマンドカウンタインクリメント
		cbd->vbi++;
    }

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         SwitchONPNGSecure_DSType1
 * 
 * Description:
 *---------------------------------------------------------------------------*/
HotSwState SwitchONPNGSecure_DSType1(CardBootData *cbd)
{
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // コマンド作成・設定
	SetSecureCommand(S_PNG_ON, cbd);

    // MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = (cbd->pBootSegBuf->rh.s.secure_cmd_param & CT_MASK) |
        				START_MASK | TRM_MASK | SE_MASK | DS_MASK | (cbd->secureLatency & LATENCY1_MASK);

    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         SwitchOFFPNGSecure_DSType1
 * 
 * Description:
 *---------------------------------------------------------------------------*/
HotSwState SwitchOFFPNGSecure_DSType1(CardBootData *cbd)
{
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // コマンド作成・設定
	SetSecureCommand(S_PNG_OFF, cbd);

    // MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = (cbd->pBootSegBuf->rh.s.secure_cmd_param & CT_MASK) |
        				START_MASK | TRM_MASK | SE_MASK | DS_MASK | (cbd->secureLatency & LATENCY1_MASK);
    
    // カードデータ転送終了割り込みが起こるまで寝る(割り込みハンドラの中で起こされる)
    OS_SleepThread(NULL);

    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}

/*---------------------------------------------------------------------------*
 * Name:         ChangeModeSecure_DSType1
 * 
 * Description:
 *
 * CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeSecure_DSType1(CardBootData *cbd)
{
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }
    
    // コマンド作成・設定
	SetSecureCommand(S_CHG_MODE, cbd);

    // MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = (cbd->pBootSegBuf->rh.s.secure_cmd_param & CT_MASK) |
        				START_MASK | TRM_MASK | SE_MASK | DS_MASK | (cbd->secureLatency & LATENCY1_MASK);

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
 * Name:         ReadIDGame_DSType1
 * 
 * Description:  ゲームモードでIDを読み込む
 *---------------------------------------------------------------------------*/
// 共通

/*---------------------------------------------------------------------------*
 * Name:         ReadPageGame_DSType1
 * 
 * Description:  ゲームモードで、指定されたページを指定バッファに指定サイズ分を読み込む
 *
 * CT=150ns  Pagecount=1page  Latency=RomHeaderで指定の値
 *---------------------------------------------------------------------------*/
// 共通

