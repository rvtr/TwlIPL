/*---------------------------------------------------------------------------*
  Project:  TwlBrom - libraries - GCD
  File:     ds_blowfish.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <firm/gcd.h>
#include <firm/gcd/blowfish.h>


#ifdef PRINT_DEBUG
#define DBG_PRINTF vlink_dos_printf
#define DBG_CHAR vlink_dos_put_console
#else
#define DBG_PRINTF( ... )  ((void)0)
#define DBG_CHAR( c )      ((void)0)
#endif

static void my_memcopy(void *src, void *dst, int size);


// DS

static char *normal_mode_key ATTRIBUTE_ALIGN(4) = "NmMdOnly";
static char *png_off_key ATTRIBUTE_ALIGN(4) = "enPngOFF";
static char *encrypt_object_key ATTRIBUTE_ALIGN(4) = "encryObj";

extern GCDSharedWork GCDi_SharedWork[2];
extern GCDSecureWork GCDi_SecureWork[2];

static void GCDi_InitBlowfishKeyAndTableDS(BLOWFISH_CTX *ctx, u32 *keyBufp, s32 keyLen)
{
  EncryptByBlowfish(ctx, &(keyBufp)[2], &(keyBufp)[1]);
  EncryptByBlowfish(ctx, &(keyBufp)[1], &(keyBufp)[0]);
  InitBlowfish(ctx, (u8 *)keyBufp, keyLen);
}

static void GCDi_MakeBlowfishTableDS(GCDSlot slot, BLOWFISH_CTX *tableBufp, const u32 *keyp, u32 *keyBufp, s32 keyLen)
{
  const BLOWFISH_CTX *blowfishInitTablep = &GCDi_BlowfishInitTableDS;
  GCDRomHeader   *rh = GCDi_GetRomHeaderAddr( slot );
  GCDSecureWork* sec = &GCDi_SecureWork[slot];
  u32 *blowfishedKeyp;

  MI_CpuCopy32((void *)blowfishInitTablep, (void *)tableBufp, sizeof(BLOWFISH_CTX));

  keyBufp[0] = *keyp;
  keyBufp[1] = (*keyp >> 1);
  keyBufp[2] = (*keyp << 1);

  GCDi_InitBlowfishKeyAndTableDS(tableBufp, keyBufp, keyLen);

  *(u64 *)sec->cardNormalModeKey = *(u64 *)rh->l.romNormalModeKey;
  blowfishedKeyp = sec->cardNormalModeKey;

  DecryptByBlowfish(tableBufp, &(blowfishedKeyp)[1], &(blowfishedKeyp)[0]);

  if (keyLen > 8) {
    sec->isGenUnScrambleKey = 1;
  }
  GCDi_InitBlowfishKeyAndTableDS(tableBufp, keyBufp, keyLen);
}

void GCDi_InitCardOpBlowfishDS( GCDSlot slot )
{
  GCDRomHeader  *rh  = GCDi_GetRomHeaderAddr( slot );
  GCDSecureWork* sec = &GCDi_SecureWork[slot];
  GCDi_MakeBlowfishTableDS(slot, &sec->blowfishCardTable, &rh->l.initialCode, sec->cardKeyBuf, 8);
}

static void GCDi_ChangeObjectTableDS(GCDSlot slot, s32 keyLen)
{
  GCDSecureWork* sec = &GCDi_SecureWork[slot];
  BLOWFISH_CTX *tableBufp = &sec->blowfishCardTable;
  u32 *keyBufp = sec->cardKeyBuf;

  keyBufp[1] = (keyBufp[1] << 1);
  keyBufp[2] = (keyBufp[2] >> 1);
  GCDi_InitBlowfishKeyAndTableDS(tableBufp, keyBufp, keyLen);
}


#define UNDEF_CODE			0xe7ffdeff
#define ENCRYPT_DEF_SIZE	0x800
/* あとで名前変える */
#define ENC_DESTINATION_BUF_SIZE 0x4000
static u32 encDestBuf[ENC_DESTINATION_BUF_SIZE/sizeof(u32)];

void GCDi_DecryptObjectFileDS( GCDSlot slot )
{
  GCDRomHeader  *rh = GCDi_GetRomHeaderAddr( slot );
  GCDSecureWork* sec = &GCDi_SecureWork[slot];
  GCDSharedWork* sh = &GCDi_SharedWork[slot];
  BLOWFISH_CTX *tableBufp = &sec->blowfishCardTable;
  u32 *encBufp  = (u32 *)&sec->blowfishFlashTable;
  u32 *encDestp = (u32 *)rh->l.arm9.ramAddr;
  s32 size = sec->secureSize;
  s32 restSize;
  BOOL exist = TRUE;
  int i;

    sec->enableReadSecure = 1; // SECURE領域リード・イネーブル

  encDestp = GCDi_SecureAreaBuf[slot];
  encBufp = encDestBuf;

  if (size > ENCRYPT_DEF_SIZE) {
    size = ENCRYPT_DEF_SIZE;
  }
  restSize = size;

  if (sec->enableReadSecure) {
    MI_CpuCopy32(encDestp, encBufp, (u32)size);
    DBG_PRINTF("%s %d\n",__FUNCTION__,__LINE__);
    DecryptByBlowfish(tableBufp, &(encBufp)[1], &(encBufp)[0]);
  }

  GCDi_ChangeObjectTableDS( slot, 8 );

  if (sec->enableReadSecure) {
    DecryptByBlowfish(tableBufp, &(encBufp)[1], &(encBufp)[0]);
    DBG_PRINTF("%s %d\n",__FUNCTION__,__LINE__);

#if 1
        for ( i=0; i<8; i++ )
        {
            if ( encrypt_object_key[i] != ((char*)encBufp)[i] )
            {
                exist = FALSE;
                break;
            }
        }

    // 暗号化オブジェクト有効時
    if ( exist )
    {
#else
    if ((encBufp[0] == encrypt_object_key[0])	// 暗号化オブジェクト有効時
	&& (encBufp[1] == encrypt_object_key[1])) {
#endif
      u32 *bufp  = encBufp;

      bufp[0] = UNDEF_CODE;
      bufp[1] = UNDEF_CODE;
      while ((restSize -= 8) > 0) {
	bufp += 2;	// 復号処理
	DecryptByBlowfish(tableBufp, &(bufp)[1], &(bufp)[0]);
      }
      DBG_PRINTF("SUCCESS %s %d\n",__FUNCTION__,__LINE__);
    } 
    else  {
	DBG_PRINTF("ERROR %s %d\n",__FUNCTION__,__LINE__);
	// CpuClearFast32(UNDEF_CODE, encBufp, size); // 未定義コードでクリア
	sh->disableEncryptedCardData = 1;	// 暗号化オブジェクト無効 通知
    }
    MI_CpuCopy32(encBufp, encDestp, (u32)size);
    DBG_PRINTF("%s %d\n",__FUNCTION__,__LINE__);
  }
}


/***************************************************

 utility functions

 ***************************************************/
static void my_memcopy(void *src, void *dst, int size)
{
  u8 *s,*d;
  int i;
  s = (u8 *)src;
  d = (u8 *)dst;
  for( i = 0 ; i < size ; i++ ) {
    *d++ = *s++;
  }
}

