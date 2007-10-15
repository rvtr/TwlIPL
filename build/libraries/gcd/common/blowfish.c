/*---------------------------------------------------------------------------*
  Project:  TwlBrom - libraries - GCD
  File:     blowfish.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <firm/gcd/blowfish.h>

#define MAXKEYBYTES	56          /* 448 bits */
#define	N		16


static u32 F(const BLOWFISH_CTX *ctx, u32 x);

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


