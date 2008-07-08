/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     blowfish.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.
 *---------------------------------------------------------------------------*/
#include <twl.h>
#include <blowfish.h>
#include <firm/format/from_firm.h>
#include <firm/hw/ARM7/mmap_firm.h>

// Function Prototype -------------------------------------------------------


/*---------------------------------------------------------------------------*
  Name:         MakeBlowfishTableDSForNAND
  
  Description:  NANDアプリ用KeyTableの生成
 *---------------------------------------------------------------------------*/
void HOTSWi_MakeBlowfishTableDSForNAND(void)
{
    CardBootData *cbd = HOTSWi_GetCardBootData();
    cbd->pBootSegBuf = (void*)SYSM_APP_ROM_HEADER_BUF;
    MakeBlowfishTableDS(cbd, 8);
}

/*---------------------------------------------------------------------------*
  Name:         MakeBlowfishTableDS
  
  Description:  KeyTableの生成
 *---------------------------------------------------------------------------*/
void MakeBlowfishTableDS(CardBootData *cbd, s32 keyLen)
{
	const BLOWFISH_CTX *initTable = &HotSwBlowfishInitTableBufDS;

    u32 		 *RomHeaderGameCode = (u32 *)cbd->pBootSegBuf->rh.s.game_code;
	u32			 *keyBuf			= cbd->keyBuf;
    BLOWFISH_CTX *ctx				= &cbd->keyTable;

	MI_CpuCopyFast((void *)initTable, (void *)ctx, sizeof(BLOWFISH_CTX));
    
  	keyBuf[0] = *RomHeaderGameCode;
  	keyBuf[1] = *RomHeaderGameCode >> 1;
  	keyBuf[2] = *RomHeaderGameCode << 1;

  	InitBlowfishKeyAndTableDS(ctx, keyBuf, keyLen);
  	InitBlowfishKeyAndTableDS(ctx, keyBuf, keyLen);
}


/*---------------------------------------------------------------------------*
  Name:         MakeBlowfishTableTWL
  
  Description:  KeyTable2の生成
 *---------------------------------------------------------------------------*/
void MakeBlowfishTableTWL(CardBootData *cbd, s32 keyLen, u16 bondingOp)
{
	u32 		 *RomHeaderGameCode = (u32 *)cbd->pBootSegBuf->rh.s.game_code;
	u32			 *keyBuf			= cbd->keyBuf2;
    BLOWFISH_CTX *ctx				= &cbd->keyTable2;
	void 		 *tempCTX;


    // 製品機の場合
	if(bondingOp == SCFG_OP_PRODUCT){
        MI_CpuCopyFast((void *)((OSFromFirm7Buf *)HW_FIRM_FROM_FIRM_BUF)->twl_blowfish, (void *)ctx, sizeof(BLOWFISH_CTX));
    }
    // 開発機の場合
    else{
		MI_CpuCopyFast(&HotSwBlowfishInitTableTWL_dev, (void *)ctx, sizeof(BLOWFISH_CTX));


		// スタック領域がオーバーフローするから、ヒープ領域から領域を確保する。
        tempCTX = OS_AllocFromSubPrivWram( sizeof(BLOWFISH_CTX) );

        if(tempCTX != NULL){
			ctx = tempCTX;
        }
        else{
			return;
        }
	}

	keyBuf[0] = *RomHeaderGameCode;
	keyBuf[1] = *RomHeaderGameCode >> 1;
	keyBuf[2] = *RomHeaderGameCode << 1;

	InitBlowfishKeyAndTableDS(ctx, keyBuf, keyLen);

    // Heapの開放
	if(bondingOp != SCFG_OP_PRODUCT){
		OS_FreeToSubPrivWram( tempCTX );
	}
}


/*---------------------------------------------------------------------------*
  Name:         InitBlowfishKeyAndTableDS
  
  Description:  
 *---------------------------------------------------------------------------*/
void InitBlowfishKeyAndTableDS(BLOWFISH_CTX *ctx, u32 *keyBufp, s32 keyLen)
{
	EncryptByBlowfish(ctx, (void*)&(keyBufp)[2], (void*)&(keyBufp)[1]);
	EncryptByBlowfish(ctx, (void*)&(keyBufp)[1], (void*)&(keyBufp)[0]);
	InitBlowfish(ctx, (u8 *)keyBufp, keyLen);
}

