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

#define MAXKEYBYTES			56          /* 448 bits */
#define	N					16

// Function Prototype -------------------------------------------------------
static u32 F(const BLOWFISH_CTX *ctx, u32 x);


/*---------------------------------------------------------------------------*
  Name:         GCDm_MakeBlowfishTableDS
  
  Description:  KeyTable‚Ì¶¬
 *---------------------------------------------------------------------------*/
void GCDm_MakeBlowfishTableDS(CardBootData *cbd, s32 keyLen)
{
	const BLOWFISH_CTX *initTable = &HotSwBlowfishInitTableBufDS;

	u32 blowfishedKey[2];

    u8 			 *RomHeaderCtrlRsvB = cbd->pBootSegBuf->rh.s.ctrl_reserved_B;
    u32 		 *RomHeaderGameCode = (u32 *)cbd->pBootSegBuf->rh.s.game_code;
	u32			 *keyBuf			= cbd->keyBuf;
    BLOWFISH_CTX *ctx				= &cbd->keyTable;

    if(cbd->modeType == HOTSW_MODE1){
		MI_CpuCopyFast((void *)initTable, (void *)ctx, sizeof(BLOWFISH_CTX));
    }
    else{
		keyBuf	= cbd->keyBuf2;

		MI_CpuCopyFast(&HotSwBlowfishInitTableTWL, (void *)ctx, sizeof(BLOWFISH_CTX));
//		MI_CpuCopyFast((void *)((OSFromFirm7Buf *)HW_FIRM_FROM_FIRM_BUF)->twl_blowfish, (void *)ctx, sizeof(BLOWFISH_CTX));

        return;
    }

  	keyBuf[0] = *RomHeaderGameCode;
  	keyBuf[1] = *RomHeaderGameCode >> 1;
  	keyBuf[2] = *RomHeaderGameCode << 1;

  	InitBlowfishKeyAndTableDS(ctx, keyBuf, keyLen);

  	blowfishedKey[0] = (u32)RomHeaderCtrlRsvB[0];
  	blowfishedKey[1] = *(u32 *)&RomHeaderCtrlRsvB[4];
    
  	DecryptByBlowfish(ctx, &(blowfishedKey)[1], &(blowfishedKey)[0]);

  	InitBlowfishKeyAndTableDS(ctx, keyBuf, keyLen);
}


/*---------------------------------------------------------------------------*
  Name:         InitBlowfishKeyAndTableDS
  
  Description:  
 *---------------------------------------------------------------------------*/
void InitBlowfishKeyAndTableDS(BLOWFISH_CTX *ctx, u32 *keyBufp, s32 keyLen)
{
  EncryptByBlowfish(ctx, &(keyBufp)[2], &(keyBufp)[1]);
  EncryptByBlowfish(ctx, &(keyBufp)[1], &(keyBufp)[0]);
  InitBlowfish(ctx, (u8 *)keyBufp, keyLen);
}


/*---------------------------------------------------------------------------*
  Name:         InitBlowfish
  
  Description:  
 *---------------------------------------------------------------------------*/
void InitBlowfish(BLOWFISH_CTX *ctx, const unsigned char *key, int keyLen)
{
  int i, j, k;
  u32 data, datal, datar;

  j = 0;
  for (i = 0; i < N + 2; ++i) {
    data = 0x00000000;
    for (k = 0; k < 4; ++k) {
      data = (data << 8) | key[j];
      j = j + 1;
      if (j >= keyLen)
        j = 0;
    }
    ctx->P[i] = ctx->P[i] ^ data;
  }

  datal = 0x00000000;
  datar = 0x00000000;

  for (i = 0; i < N + 2; i += 2) {
    EncryptByBlowfish(ctx, &datal, &datar);
    ctx->P[i] = datal;
    ctx->P[i + 1] = datar;
  }

  for (i = 0; i < 4; ++i) {
    for (j = 0; j < 256; j += 2) {
      EncryptByBlowfish(ctx, &datal, &datar);
      ctx->S[i][j] = datal;
      ctx->S[i][j + 1] = datar;
    }
  }

}


/*---------------------------------------------------------------------------*
  Name:         EncryptByBlowfish
  
  Description:  
 *---------------------------------------------------------------------------*/
void EncryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr)
{
  u32  Xl;
  u32  Xr;
  u32  temp;
  int       i;

  Xl = *xl;
  Xr = *xr;

  for (i = 0; i < N; ++i) {
    Xl = Xl ^ ctx->P[i];
    Xr = F(ctx, Xl) ^ Xr;

    temp = Xl;

    Xl = Xr;
    Xr = temp;
  }

  temp = Xl;
  Xl = Xr;
  Xr = temp;

  Xr = Xr ^ ctx->P[N];
  Xl = Xl ^ ctx->P[N + 1];
  
  *xl = Xl;
  *xr = Xr;
}


/*---------------------------------------------------------------------------*
  Name:         DecryptByBlowfish
  
  Description:  
 *---------------------------------------------------------------------------*/
void DecryptByBlowfish(const BLOWFISH_CTX *ctx, u32 *xl, u32 *xr)
{
  u32  Xl;
  u32  Xr;
  u32  temp;
  int       i;


  Xl = *xl;
  Xr = *xr;

  for (i = N + 1; i > 1; --i) {
    Xl = Xl ^ ctx->P[i];
    Xr = F(ctx, Xl) ^ Xr;

    /* Exchange Xl and Xr */
    temp = Xl;
    Xl = Xr;
    Xr = temp;
  }

  /* Exchange Xl and Xr */
  temp = Xl;
  Xl = Xr;
  Xr = temp;

  Xr = Xr ^ ctx->P[1];
  Xl = Xl ^ ctx->P[0];

  *xl = Xl;
  *xr = Xr;
}


/*---------------------------------------------------------------------------*
  Name:         F
  
  Description:  
 *---------------------------------------------------------------------------*/
static u32 F(const BLOWFISH_CTX *ctx, u32 x) {
   u32  a, b, c, d;
   u32  y;

   d = x & 0x00FF;
   x >>= 8;

   c = x & 0x00FF;
   x >>= 8;

   b = x & 0x00FF;
   x >>= 8;

   a = x & 0x00FF;

   y = ctx->S[0][a] + ctx->S[1][b];

   y = y ^ ctx->S[2][c];
   y = y + ctx->S[3][d];

   return y;
}
