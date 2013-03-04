/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     dsCardCommon.c

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/#include 	<twl.h>
#include 	<firm/os/common/system.h>
#include	<blowfish.h>
#include 	<dsCardCommon.h>
#include	<customNDma.h>

// extern -------------------------------------------------------------------
extern CardThreadData HotSwThreadData;

// define -------------------------------------------------------------------
#define		SECURE_SEGMENT_NUM					4
#define		ONE_SEGMENT_PAGE_NUM				8

#define		ROM_EMULATION_START_OFS				0x160
#define		ROM_EMULATION_END_OFS				0x180

#define 	RD_NORMAL_ID_LATENCY1				0x657

//マジコン実験、レーテンシを修正
//#define MGCN_TEST_DS_LATENCY 1


// static value -------------------------------------------------------------
static OSMessage	s_Msg;

// Function prototype -------------------------------------------------------
static HotSwState HOTSWi_ChangeModeNormal(CardBootData *cbd, u64 cmd);
static void PreSendSecureCommand(CardBootData *cbd, u32 *scrambleMask);

// ===========================================================================
// 	Function Describe
// ===========================================================================

/*---------------------------------------------------------------------------*
  Name:			HOTSWi_SetCommand

  Description:  引数で与えられたコマンドのエンディアンを変えてレジスタにセットする
 *---------------------------------------------------------------------------*/
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

    // MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = *(u32*)cndBE.b;
	reg_HOTSW_MCCMD1 = *(u32*)&cndBE.b[4];
}


// ■------------------------------------■
// ■       ノーマルモードのコマンド     ■
// ■------------------------------------■
/*---------------------------------------------------------------------------*
  Name:			ReadIDNormal

  Description:  ノーマルモード時のカードIDを読み込む関数
 *---------------------------------------------------------------------------*/
HotSwState ReadIDNormal(CardBootData *cbd)
{
	GCDCmd64 cndLE;

    // DMA転送の準備
#ifdef USE_NEW_DMA
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_nml, sizeof(cbd->id_nml) );
#else
    HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_nml, sizeof(cbd->id_nml) );
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
    
	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | HOTSW_PAGE_STAT | (RD_NORMAL_ID_LATENCY1 & LATENCY1_MASK);

    // メッセージ受信
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

	// 1us Wait
    OS_SpinWait( OS_USEC_TO_CPUCYC(1) );
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadBootSegNormal
  
  Description:  ノーマルモードのBoot Segment読み込み
 
  CT=240ns  Latency1=0x1fff  Latency2=0x3f  Pagecount=8page
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
            // DMA転送の準備
#ifdef USE_NEW_DMA
        	HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, dst + (u32)(PAGE_WORD_SIZE*i), size );
#else
        	HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, dst + (u32)(PAGE_WORD_SIZE*i), size );
#endif
        }
        else{
			// DMA転送（読み捨て）の準備
#ifdef USE_NEW_DMA
    		HOTSW_NDmaPipe_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &temp, size );
#else
            HOTSW_DmaPipe32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &temp, size );
#endif
        }
        
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
        
		// MCCNT1 レジスタ設定
		reg_HOTSW_MCCNT1 = START_MASK | CT_MASK | PC_MASK & (pc << PC_SHIFT) | LATENCY2_MASK | LATENCY1_MASK;

	    // メッセージ受信
		OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

        page++;

		// 10us Wait
    	OS_SpinWait( OS_USEC_TO_CPUCYC(10) );        
    }
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:			ReadStatusNormal

  Description:  ノーマルモードでステータスを読み込む
 *---------------------------------------------------------------------------*/
HotSwState ReadStatusNormal(CardBootData *cbd)
{
	GCDCmd64 cndLE;

	cbd->romStatus = HOTSW_ROMST_RFS_READY_MASK;

  	if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
   	}

    // DMA転送の準備
#ifdef USE_NEW_DMA
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->romStatus, sizeof(cbd->romStatus) );
#else
    HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->romStatus, sizeof(cbd->romStatus) );
#endif
    
   	// リトルエンディアンで作って
	cndLE.dw  = HSWOP_N_OP_RD_STAT;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif

	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~(SCRAMBLE_MASK | LATENCY1_MASK)) | START_MASK | HOTSW_PAGE_STAT;

    // メッセージ受信
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:			RefreshBadBlockNormal

  Description:  ノーマルモードでバッドブロックを置換
 *---------------------------------------------------------------------------*/
HotSwState RefreshBadBlockNormal(CardBootData *cbd)
{
	GCDCmd64 cndLE;

  	if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
   	}

   	// リトルエンディアンで作って
	cndLE.dw  = HSWOP_N_OP_RFS_BLK;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~(SCRAMBLE_MASK | LATENCY1_MASK)) | START_MASK | HOTSW_PAGE_0;

    // カードデータ転送終了まで待つ
    HOTSW_WaitCardCtrl();

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ChangeModeNormal
  
  Description:  ノーマルモードからセキュアモードへの変更
  
  CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeNormal(CardBootData *cbd)
{
	return HOTSWi_ChangeModeNormal(cbd, HSWOP_N_OP_CHG_MODE);
}


/*---------------------------------------------------------------------------*
  Name:         ChangeModeNorma2
  
  Description:  ノーマルモードからセキュア２モードへの変更
  
  CT=240ns  Latency1=0x18  Latency2=0  Pagecount=0page
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

	// MCCNT0 レジスタ設定
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~SCRAMBLE_MASK) | START_MASK | HOTSW_PAGE_0;

    // カードデータ転送終了まで待つ
	HOTSW_WaitCardCtrl();

	// 47us Wait
    OS_SpinWait( OS_USEC_TO_CPUCYC(47) );
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:			LoadTable

  Description:  カード側の Key Table をロードする関数

  ※この関数は開発カード用に発行しないといけない。
  　製品版カードの場合、このコマンドは無視される
 *---------------------------------------------------------------------------*/
HotSwState LoadTable(void)
{
	GCDCmd64 cndLE;
	u32 temp;

    // DMA転送の準備
#ifdef USE_NEW_DMA
	HOTSW_NDmaPipe_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &temp, HOTSW_LOAD_TABLE_SIZE );
#else
    HOTSW_DmaPipe32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &temp, HOTSW_LOAD_TABLE_SIZE );
#endif

    // リトルエンディアンで作って
    cndLE.dw  = HSWOP_N_OP_LD_TABLE;
    
	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif
    
    // MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | HOTSW_PAGE_16 | LATENCY2_MASK & (0x18 << LATENCY2_SHIFT);

    // メッセージ受信
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

	// 1us Wait
    OS_SpinWait( OS_USEC_TO_CPUCYC(1) );
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadRomEmulationInfo
  
  Description:  Romエミュレーション情報の読み込み
 *---------------------------------------------------------------------------*/
HotSwState ReadRomEmulationInfo(SYSMRomEmuInfo *info)
{
	u32 count=0;
    u32 temp;
    u32 *dst = (void*)info;

    MI_CpuClear8( info, sizeof(SYSMRomEmuInfo) );

    // 量産用CPUでは平文アクセス防止のためリードしない
    if ( ! (*(u8*)HWi_WSYS08_ADDR & HWi_WSYS08_OP_OPT_MASK) ||
         ! ((OS_GetRunningConsoleType() & OS_CONSOLE_SIZE_MASK) == OS_CONSOLE_SIZE_32MB)
       )
    {
        return HOTSW_SUCCESS;
    }
    
	// MCCMD レジスタ設定
	reg_HOTSW_MCCMD0 = 0x3e000000;
	reg_HOTSW_MCCMD1 = 0x0;

	// MCCNT0 レジスタ設定
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 レジスタ設定 (START = 1  PC = 001(1ページリード)に latency1 = 0x5fe)
	reg_HOTSW_MCCNT1 = START_MASK | HOTSW_PAGE_1 | (0x5fe & LATENCY1_MASK);

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

    return HOTSW_SUCCESS;
}


// ■--------------------------------------■
// ■       セキュアモードのコマンド       ■
// ■--------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         SetSecureCommand
  
  Description:  引数で与えられたコマンドをBlowfishで暗号化してレジスタにセット
 *---------------------------------------------------------------------------*/
static void SetSecureCommand(SecureCommandType type, CardBootData *cbd)
{
	GCDCmd64 		cndLE;
    u64 			data;
    BLOWFISH_CTX	*ctx;

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

	// コマンド作成
	data = (type == S_PNG_ON) ? (u64)cbd->vd : (u64)cbd->vae;
    cndLE.dw |= cbd->vbi;
    cndLE.dw |= data << HSWOP_S_VA_SHIFT;
    
    if(!HOTSWi_IsRomEmulation()){
		ctx = (cbd->modeType == HOTSW_MODE1) ? &cbd->keyTable : &cbd->keyTable2;
        
    	// コマンドの暗号化
		EncryptByBlowfish( ctx, (u32*)&cndLE.b[4], (u32*)cndLE.b );
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
		u32 latency = (u32)cbd->pBootSegBuf->rh.s.secure_cmd_latency * 0x100;

		// MCCNT0 レジスタ設定
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
        
		// MCCNT1 レジスタ設定
		reg_HOTSW_MCCNT1 = START_MASK | *scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

		// セキュアコマンド間レイテンシ待ち
    	OS_Sleep( OS_CPUCYC_TO_MSEC(latency) );
    }
    // ★ TWL-XtraROM＆NTR-MROM対応
    else{
		*scrambleMask |= TRM_MASK;
    }
}


/*---------------------------------------------------------------------------*
  Name:         ReadIDSecure
  
  Description:
 
  CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=Status
 *---------------------------------------------------------------------------*/
HotSwState ReadIDSecure(CardBootData *cbd)
{
	u32 scrambleMask;
	u32	*buf = (cbd->modeType == HOTSW_MODE1) ? &cbd->id_scr : &cbd->id_scr2;
    
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // スクランブルの設定
    scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_RD_ID, cbd);

	// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
	PreSendSecureCommand(cbd, &scrambleMask);

    // DMA転送の準備
#ifdef USE_NEW_DMA
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, buf, sizeof(buf) );
#else
    HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, buf, sizeof(buf) );
#endif

	// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif
    
	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = START_MASK | HOTSW_PAGE_STAT | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

    // メッセージ受信
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadSegSecure
  
  Description:
 *---------------------------------------------------------------------------*/
HotSwState ReadSegSecure(CardBootData *cbd)
{
    u32 			scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SCRAMBLE_MASK & ~CS_MASK);
	u32				*buf = (cbd->modeType == HOTSW_MODE1) ? cbd->pSecureSegBuf : cbd->pSecure2SegBuf;
    u32				loop, pc, size, interval, i, j=0, k;
	u64				segNum = 4;
	GCDCmd64 		cndLE;
    BLOWFISH_CTX	*ctx = (cbd->modeType == HOTSW_MODE1) ? &cbd->keyTable : &cbd->keyTable2;

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
		EncryptByBlowfish( ctx, (u32*)&cndLE.b[4], (u32*)cndLE.b );

		// MCCMD レジスタ設定
		HOTSWi_SetCommand(&cndLE);

		// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
		PreSendSecureCommand(cbd, &scrambleMask);

        for(k=0; k<loop; k++){
            // DMA転送の準備
#ifdef USE_NEW_DMA
		    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, buf + (interval*j), size );
#else
            HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, buf + (interval*j), size );
#endif
            
			// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
			reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
			reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif

			// MCCNT1 レジスタ設定
			reg_HOTSW_MCCNT1 = START_MASK | PC_MASK & (pc << PC_SHIFT) | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;

    		// メッセージ受信
			OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

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
  Name:         SwitchONPNGSecure
  
  Description:
 *---------------------------------------------------------------------------*/
HotSwState SwitchONPNGSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // スクランブルの設定
    scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_PNG_ON, cbd);

	// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
	PreSendSecureCommand(cbd, &scrambleMask);



	// MCCNT0 レジスタ設定
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 レジスタ設定
#ifdef MGCN_TEST_DS_LATENCY
    if (cbd->cardType == DS_CARD_TYPE_1) //マジコン調査: TWLのLatencyバグ修正(=DS?)
	    reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    else 
	    reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);
#else
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);
#endif


    // カードデータ転送終了まで待つ
	HOTSW_WaitCardCtrl();
    
    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         SwitchOFFPNGSecure
  
  Description:
 *---------------------------------------------------------------------------*/
HotSwState SwitchOFFPNGSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardExist()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // スクランブルの設定
    scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_PNG_OFF, cbd);

	// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
	PreSendSecureCommand(cbd, &scrambleMask);

	// MCCNT0 レジスタ設定
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 レジスタ設定
#ifdef MGCN_TEST_DS_LATENCY
    if (cbd->cardType == DS_CARD_TYPE_1) //マジコン調査: TWLのLatencyバグ修正(=DS?)
	    reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    else 
	    reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);
#else
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);
#endif



    // カードデータ転送終了まで待つ
	HOTSW_WaitCardCtrl();
    
    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ChangeModeSecure
  
  Description:
 
  CT=240ns  Latency1=0x8f8+0x18  Latency2=0  Pagecount=0page
 *---------------------------------------------------------------------------*/
HotSwState ChangeModeSecure(CardBootData *cbd)
{
	u32 scrambleMask;
    
    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // スクランブルの設定
    scrambleMask = HOTSWi_IsRomEmulation() ? 0 : (u32)(SCRAMBLE_MASK & ~CS_MASK);
    
    // コマンド作成・設定
	SetSecureCommand(S_CHG_MODE, cbd);

	// コマンド初回送信（NTR-MROMはレイテンシクロック設定変更のみ）
	PreSendSecureCommand(cbd, &scrambleMask);

	// MCCNT0 レジスタ設定
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
	// MCCNT1 レジスタ設定
#ifdef MGCN_TEST_DS_LATENCY
    if (cbd->cardType == DS_CARD_TYPE_1) //マジコン調査: TWLのLatencyバグ修正(=DS?)
	    reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param;
    else 
	    reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);
#else
	reg_HOTSW_MCCNT1 = START_MASK | scrambleMask | cbd->pBootSegBuf->rh.s.secure_cmd_param | (cbd->secureLatency & LATENCY1_MASK);
#endif

    // カードデータ転送終了まで待つ
	HOTSW_WaitCardCtrl();
    
    // コマンドカウンタインクリメント
	cbd->vbi++;

    return HOTSW_SUCCESS;
}


// ■------------------------------------■
// ■       ゲームモードのコマンド       ■
// ■------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         ReadIDGame
  
  Description:  ゲームモードでIDを読み込む
 *---------------------------------------------------------------------------*/
HotSwState ReadIDGame(CardBootData *cbd)
{
	GCDCmd64 cndLE;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // DMA転送の準備
#ifdef USE_NEW_DMA
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_gam, sizeof(cbd->id_gam) );
#else
    HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->id_gam, sizeof(cbd->id_gam) );
#endif
    
   	// リトルエンディアンで作って
	cndLE.dw  = HSWOP_G_OP_RD_ID;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif
    
	reg_HOTSW_MCCNT1 = cbd->gameCommondParam | START_MASK | HOTSW_PAGE_STAT;

    // メッセージ受信
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

	// 1us Wait
    OS_SpinWait( OS_USEC_TO_CPUCYC(1) );
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadPageGame
  
  Description:  ゲームモードで、指定されたページを指定バッファに指定サイズ分を読み込む
 
  CT=150ns  Pagecount=1page  Latency=RomHeaderで指定の値
 *---------------------------------------------------------------------------*/
HotSwState ReadPageGame(CardBootData *cbd, u32 start_addr, void* buf, u32 size)
{
    u32 		loop, counter=0;
	u64			i, page;
	GCDCmd64	cndLE;

    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

    for(i=0; i<loop; i++){
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}

        // DMA転送の準備
#ifdef USE_NEW_DMA
		HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, (u32 *)buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
#else
        HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, (u32 *)buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
#endif
        
        // コマンド作成
		cndLE.dw  = HSWOP_G_OP_RD_PAGE;
		cndLE.dw |= (page + i) << HSWOP_G_RD_PAGE_ADDR_SHIFT;

		// MCCMD レジスタ設定
		HOTSWi_SetCommand(&cndLE);

		// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif
        
   		// MCCNT1 レジスタ設定
		reg_HOTSW_MCCNT1 = cbd->gameCommondParam | START_MASK | HOTSW_PAGE_1;

    	// メッセージ受信
		OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
    }

	// 100ns Wait
    OS_SpinWait( OS_NSEC_TO_CPUCYC(100) );
    
    return HOTSW_SUCCESS;
}


/*---------------------------------------------------------------------------*
  Name:         ReadStatusGame
  
  Description:  ゲームモードでステータスを読み込む
 *---------------------------------------------------------------------------*/
HotSwState ReadStatusGame(CardBootData *cbd)
{
	GCDCmd64 cndLE;

	cbd->romStatus = HOTSW_ROMST_RFS_READY_MASK;

    if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
    }

    // DMA転送の準備
#ifdef USE_NEW_DMA
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, &cbd->romStatus, sizeof(cbd->romStatus) );
#else
    HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, &cbd->romStatus, sizeof(cbd->romStatus) );
#endif
    
   	// リトルエンディアンで作って
	cndLE.dw  = HSWOP_G_OP_RD_STAT;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif
    
   	// MCCNT1 レジスタ設定 (START = 1 W/R = 0 PC = 111(ステータスリード) その他Romヘッダの情報におまかせ)
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~LATENCY1_MASK) | START_MASK | HOTSW_PAGE_STAT;
    
    // メッセージ受信
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
    
    return HOTSW_SUCCESS;
}


/* -----------------------------------------------------------------
  RefreshBadBlockGame関数
 
  ゲームモードでバッドブロックを置換
 * ----------------------------------------------------------------- */
HotSwState RefreshBadBlockGame(CardBootData *cbd)
{
	GCDCmd64 cndLE;

  	if(!HOTSW_IsCardAccessible()){
		return HOTSW_PULLED_OUT_ERROR;
   	}

   	// リトルエンディアンで作って
	cndLE.dw  = HSWOP_G_OP_RFS_BLK;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK );
    
   	// MCCNT1 レジスタ設定
	reg_HOTSW_MCCNT1 = (cbd->gameCommondParam & ~LATENCY1_MASK) | START_MASK | HOTSW_PAGE_0;

    // カードデータ転送終了まで待つ
	HOTSW_WaitCardCtrl();

    return HOTSW_SUCCESS;
}


// ---- Check for マジコン
//---------------------------------------------------------------------------*
//カードにDSTWOコマンド送る
//---------------------------------------------------------------------------*
void SendDstComnd(u8 cmd,void* res,int res_size,u32 param){
	GCDCmd64 cndLE;

    // DMA転送の準備
#ifdef USE_NEW_DMA
    HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, res, res_size );
#else
    HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, res, res_size );
#endif

    cndLE.dw = (u64)cmd << 56;
    if(cmd == 0x16) cndLE.dw  |= 0x0012345678000000ULL;

	// MCCMD レジスタ設定
	HOTSWi_SetCommand(&cndLE);

	// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
	reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif
    
	// MCCNT1 レジスタ設定
	//reg_HOTSW_MCCNT1 = START_MASK | HOTSW_PAGE_STAT | (RD_NORMAL_ID_LATENCY1 & LATENCY1_MASK);
	reg_HOTSW_MCCNT1 = param | START_MASK | HOTSW_PAGE_STAT;
    
    // メッセージ受信
	OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);

	// 1us Wait
    OS_SpinWait( OS_USEC_TO_CPUCYC(1) );
    
}



//マジコン調査
//DsTwoカードの特殊コマンドでpageリード
HotSwState ReadDsTwo(u32 param, u32 start_addr, void* buf, u32 size)
{
    u32 		loop, counter=0;
	u64			i, page;
	GCDCmd64	cndLE;

    page = (u32)(start_addr / PAGE_SIZE);
	loop = (u32)(size / PAGE_SIZE);
    loop = (size % PAGE_SIZE) ? loop + 1 : loop;

    for(i=0; i<loop; i++){
	    if(!HOTSW_IsCardAccessible()){
			return HOTSW_PULLED_OUT_ERROR;
    	}

        // DMA転送の準備
#ifdef USE_NEW_DMA
		HOTSW_NDmaCopy_Card( HOTSW_NDMA_NO, (u32 *)HOTSW_MCD1, (u32 *)buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
#else
        HOTSW_DmaCopy32_Card( HOTSW_DMA_NO, (u32 *)HOTSW_MCD1, (u32 *)buf + (u32)(PAGE_WORD_SIZE*i), PAGE_SIZE );
#endif
        
        // コマンド作成
		cndLE.dw  = 0x1800000000000000;
		cndLE.dw |= (page + i) << HSWOP_G_RD_PAGE_ADDR_SHIFT;

		// MCCMD レジスタ設定
		HOTSWi_SetCommand(&cndLE);

		// MCCNT0 レジスタ設定
#ifdef USE_NEW_DMA
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK);
#else
		reg_HOTSW_MCCNT0 = (u16)((reg_HOTSW_MCCNT0 & HOTSW_E2PROM_CTRL_MASK) | REG_MI_MCCNT0_E_MASK | REG_MI_MCCNT0_I_MASK);
#endif
        
   		// MCCNT1 レジスタ設定
		reg_HOTSW_MCCNT1 = param | START_MASK | HOTSW_PAGE_1;

    	// メッセージ受信
		OS_ReceiveMessage(&HotSwThreadData.hotswDmaQueue, (OSMessage *)&s_Msg, OS_MESSAGE_BLOCK);
    }

	// 100ns Wait
    OS_SpinWait( OS_NSEC_TO_CPUCYC(100) );
    
    return HOTSW_SUCCESS;
}
