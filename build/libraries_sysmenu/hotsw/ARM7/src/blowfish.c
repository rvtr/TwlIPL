/*---------------------------------------------------------------------------*
  Project:  TwlBrom - libraries - GCD
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

#define MAXKEYBYTES			56          /* 448 bits */
#define	N					16

// Function Prototype -------------------------------------------------------
static u32 F(const BLOWFISH_CTX *ctx, u32 x);
//static void GCDi_InitBlowfishKeyAndTableDS(BLOWFISH_CTX *ctx, u32 *keyBufp, s32 keyLen);

//*****************************************
//
// 	GCDm_MakeBlowfishTableDSä÷êî
//
//*****************************************
void GCDm_MakeBlowfishTableDS(BLOWFISH_CTX *tableBufp, ROM_Header_Short *rhs, u32 *keyBufp, s32 keyLen)
{
  const BLOWFISH_CTX *blowfishInitTablep = &GCDi_BlowfishInitTableDS;
  u32 blowfishedKey[2];

  MI_CpuCopy32((void *)blowfishInitTablep, (void *)tableBufp, sizeof(BLOWFISH_CTX));

  keyBufp[0] = *(u32 *)rhs->game_code;
  keyBufp[1] = *(u32 *)rhs->game_code >> 1;
  keyBufp[2] = *(u32 *)rhs->game_code << 1;

  InitBlowfishKeyAndTableDS(tableBufp, keyBufp, keyLen);

  blowfishedKey[0] = (u32)rhs->ctrl_reserved_B[0];
  blowfishedKey[1] = *(u32 *)&rhs->ctrl_reserved_B[4];

//  OS_TPrintf("Blowfish - key[0]:%d  key[1]:%d\n",blowfishedKey[0],blowfishedKey[1]);
    
  DecryptByBlowfish(tableBufp, &(blowfishedKey)[1], &(blowfishedKey)[0]);

  InitBlowfishKeyAndTableDS(tableBufp, keyBufp, keyLen);
}

//*****************************************
//
// 	InitBlowfishKeyAndTableDSä÷êî
//
//*****************************************
void InitBlowfishKeyAndTableDS(BLOWFISH_CTX *ctx, u32 *keyBufp, s32 keyLen)
{
  EncryptByBlowfish(ctx, &(keyBufp)[2], &(keyBufp)[1]);
  EncryptByBlowfish(ctx, &(keyBufp)[1], &(keyBufp)[0]);
  InitBlowfish(ctx, (u8 *)keyBufp, keyLen);
}

//*****************************************
//
// 	InitBlowfishä÷êî
//
//*****************************************
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

//*****************************************
//
// 	EncryptByBlowfishä÷êî
//
//*****************************************
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

//*****************************************
//
// 	DecryptByBlowfishä÷êî
//
//*****************************************
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

//*****************************************
//
// 	Fä÷êî
//
//*****************************************
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


