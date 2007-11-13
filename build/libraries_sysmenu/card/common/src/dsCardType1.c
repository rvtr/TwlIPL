/*---------------------------------------------------------------------------*
  Project:  TwlSDK
  File:     
 *---------------------------------------------------------------------------*/

#include 	<twl.h>

#include 	"blowfish.h"
#include 	"Card.h"
#include	"dsCardType1.h"

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
  
  Description:  DSカードType1のノーマルモードのID読み込み
 *---------------------------------------------------------------------------*/
void ReadIDNormal_DSType1(CardBootData *cbd)
{
	cbd->id_nml = 0;
    
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
		cbd->id_nml = reg_MCD1;
	}

	OS_TPrintf("Nml Card B ID : %08x\n", cbd->id_nml);
}

/*---------------------------------------------------------------------------*
  Name:         ReadBootSegNormal_DSType1
  
  Description:  DSカードType1のノーマルモードのBoot Segment読み込み
 *---------------------------------------------------------------------------*/
void ReadBootSegNormal_DSType1(CardBootData *cbd)
{
	#pragma unused( cbd )
    
	u32 i = 0;

	// MCCMD レジスタ設定
	reg_MCCMD0 = 0x00000000;
	reg_MCCMD1 = 0x00000000;

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1  W/R = 0  PC = 100 (8ページリード) に)
	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,     0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,4,  0,0,  0,  0,0,0,  2320));
    
	// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
        *(cbd->pBootSegBuf->word + i++) = reg_MCD1;
	}
    
    OS_TPrintf("GameTitle : %s\n", cbd->pBootSegBuf->rh.s.title_name);
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeNormal_DSType1
  
  Description:  DSカードType1のノーマルモードのモード変更
 *---------------------------------------------------------------------------*/
void ChangeModeNormal_DSType1(CardBootData *cbd)
{
	GCDCmd64 tempCnd, cnd;
    u64 vae64 = cbd->vae;

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
    reg_MCCMD0 = *(u32 *)cnd.b;
	reg_MCCMD1 = *(u32 *)&cnd.b[4];

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1  W/R = 1  PC = 000 に)
	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,     0)) |
        		             		 CNT1_FLD(1,1,0,0,  0,0,  0,0,  0,  0,0,0,  2320));
    
	// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
	}

    OS_TPrintf("Mode Changed\n");
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
	reg_MCCMD0 = *(u32*)cndBE.b;
	reg_MCCMD1 = *(u32*)&cndBE.b[4];
}


/*---------------------------------------------------------------------------*
  Name:         ReadIDSecure_DSType1
  
  Description:  
 *---------------------------------------------------------------------------*/
void ReadIDSecure_DSType1(CardBootData *cbd)
{
    // コマンド作成・設定
	SetSecureCommand(S_RD_ID, cbd);
    
	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定
    // (START = 1 W/R = 0 TRM = 1 PC = 111(ステータスリード) CS = 1 SE = 1 DS = 1 Latency1 = 2320(0x910)に)
	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,  1,  0,0,0,     0)) |
        		             		 CNT1_FLD(1,0,0,1,  1,7,  0,0,  0,  0,1,1,  2320));
    
	// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
		cbd->id_scr = reg_MCD1;
	}

    // コマンドカウンタインクリメント
	cbd->vbi++;
    
	OS_TPrintf("Scr Card B ID : %08x\n", cbd->id_scr);
}

/*---------------------------------------------------------------------------*
  Name:         ReadSegSecure_DSType1
  
  Description:  
 *---------------------------------------------------------------------------*/
void ReadSegSecure_DSType1(CardBootData *cbd)
{
    u32			i,j;
	u64			segNum = 4;
    u64			vae	= cbd->vae;
    GCDCmd64 	cndLE, cndBE;
    OSTick		start;

    for(i=0; i<4; i++){
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
		reg_MCCMD0 = *(u32*)cndBE.b;
		reg_MCCMD1 = *(u32*)&cndBE.b[4];

		// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
		reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

		// MCCNT1 レジスタ設定
    	// (START = 1 W/R = 0 TRM = 1 PC = 100(8ページリード) CS = 1 Latency2 = 24(0x18) SE = 1 DS = 1 Latency1 = 2296(0x8f8)に)
		reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,1,   0,  0,0,0,     0)) |
        			             		 CNT1_FLD(1,0,0,1,  1,4,  0,0,  24,  0,1,1,  2296));
    
		// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
		j = (u32)(1024*(segNum - 4));
    	while(reg_MCCNT1 & START_FLG_MASK){
			while(!(reg_MCCNT1 & READY_FLG_MASK)){}
        	*(cbd->pSecureSegBuf + j++) = reg_MCD1;
		}
        
        // 読み込みセグメント番号インクリメント
		segNum++;
        
    	// コマンドカウンタインクリメント
		cbd->vbi++;

		// 100ms待ち
    	start = OS_GetTick();
    	while(OS_TicksToMilliSeconds(OS_GetTick()-start) < 100){}
        
    }

	OS_TPrintf("Read Scr Segment 16Kbyte\n");
}

/*---------------------------------------------------------------------------*
  Name:         SwitchONPNGSecure_DSType1
  
  Description:  
 *---------------------------------------------------------------------------*/
void SwitchONPNGSecure_DSType1(CardBootData *cbd)
{
    // コマンド作成・設定
	SetSecureCommand(S_PNG_ON, cbd);
    
	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 W/R = 1 TRM = 1 PC = 000 Latency1 = 2320(0x910) に)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,     0)) |
        		             		 CNT1_FLD(1,1,0,1,  0,0,  0,0,  0,  0,1,1,  2320));
    
	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
	}

    // コマンドカウンタインクリメント
	cbd->vbi++;

   	OS_TPrintf("PN Generator ON\n");
}

/*---------------------------------------------------------------------------*
  Name:         SwitchOFFPNGSecure_DSType1
  
  Description:  
 *---------------------------------------------------------------------------*/
void SwitchOFFPNGSecure_DSType1(CardBootData *cbd)
{
    // コマンド作成・設定
	SetSecureCommand(S_PNG_OFF, cbd);
    
	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 W/R = 1 TRM = 1 PC = 000 Latency1 = 2320(0x910) に)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,     0)) |
        		             		 CNT1_FLD(1,1,0,1,  0,0,  0,0,  0,  0,1,1,  2320));
    
	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
	}

    // コマンドカウンタインクリメント
	cbd->vbi++;

   	OS_TPrintf("PN Generator OFF\n");
}

/*---------------------------------------------------------------------------*
  Name:         ChangeModeSecure_DSType1
  
  Description:  
 *---------------------------------------------------------------------------*/
void ChangeModeSecure_DSType1(CardBootData *cbd)
{
    // コマンド作成・設定
	SetSecureCommand(S_CHG_MODE, cbd);
    
	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 W/R = 1 TRM = 1 PC = 000 Latency1 = 2320(0x910) に)
    reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  1,0,0,     0)) |
        		             		 CNT1_FLD(1,1,0,1,  0,0,  0,0,  0,  0,1,1,  2320));
    
	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
	}

    // コマンドカウンタインクリメント
	cbd->vbi++;

   	OS_TPrintf("Change Mode\n");
}


// ■------------------------------------■
// ■       ゲームモードのコマンド       ■
// ■------------------------------------■
/*---------------------------------------------------------------------------*
  Name:         ReadIDGame_DSType1
  
  Description:  ゲームモードでIDを読み込む
 *---------------------------------------------------------------------------*/
void ReadIDGame_DSType1(CardBootData *cbd)
{
	// MCCMD レジスタ設定
	reg_MCCMD0 = 0x000000B8;
	reg_MCCMD1 = 0x00000000;

	// MCCNT0 レジスタ設定 (E = 1  I = 1  SEL = 0に)
	reg_MCCNT0 = (u16)((reg_MCCNT0 & 0x0fff) | 0xc000);

	// MCCNT1 レジスタ設定 (START = 1 W/R = 0 PC = 111(ステータスリード) CS = 1 SE = 1 DS = 1 latency1 = 2320(必要ないけど) に)
	reg_MCCNT1 = (u32)((reg_MCCNT1 & CNT1_MSK(0,0,1,0,  0,0,  1,0,  0,  0,0,0,     0)) |
        		             		 CNT1_FLD(1,0,0,0,  0,7,  0,1,  0,  0,1,1,  2320));
    
	// MCCNTレジスタのRDYフラグをポーリングして、フラグが立ったらデータをMCD1レジスタに再度セット。スタートフラグが0になるまでループ。
	while(reg_MCCNT1 & START_FLG_MASK){
		while(!(reg_MCCNT1 & READY_FLG_MASK)){}
		cbd->id_gam = reg_MCD1;
	}

	OS_TPrintf("Gme Card B ID : %08x\n", cbd->id_gam);
}