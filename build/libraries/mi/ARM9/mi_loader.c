/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - mi
  File:     mi_loader.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include <firm.h>
#include <firm/format/format_rom.h>

#define PXI_FIFO_TAG_DATA   PXI_FIFO_TAG_USER_0

static ROM_Header* const rh = (ROM_Header*)HW_TWL_ROM_HEADER_BUF;

#define HEADER_SIZE 0x1000
#define AUTH_SIZE   0xe00
#define RSA_BLOCK_SIZE  128

#define HASH_UNIT   0x800   // TODO: optimizing to maximize cache efficiency

/*
    SHA1
*/

typedef struct SHA1_CTX // 実際には、サイズが同じなら中身は何でも良い
{
    u32 h0,h1,h2,h3,h4;
    u32 Nl,Nh;
    u32 data[16];
    int num;
    void (*sha_block)(struct SHA1_CTX *c, const u8 *W, int num);
}
SHA1_CTX;

static inline void SHA1_Init(SHA1_CTX *ctx)
{
    if (ctx == NULL)
        return;

    MI_CpuClear8(ctx, sizeof(SHA1_CTX));
    SVC_SHA1Init(ctx);
}

static inline void SHA1_Update(SHA1_CTX *ctx, const void* data, u32 len)
{
    if (ctx == NULL)
        return;
    if (len > 0 && data == NULL)
        return;
    SVC_SHA1Update(ctx, data, len);
}

static inline void SHA1_GetHash(SHA1_CTX *ctx, u8* md)
{
    if (ctx == NULL)
        return;
    if (md == NULL)
        return;
    SVC_SHA1GetHash(md, ctx);
}

static inline void SHA1_Calc(u8* md, const void* data, u32 len)
{
    SVC_CalcSHA1(md, data, len);
}

/*
    HMAC (SHA1)
*/

#define DIGEST_HASH_BLOCK_SIZE_SHA1                 (512/8)
typedef struct HMAC_CTX
{
    SHA1_CTX    sha1_ctx;
    u8          key[DIGEST_HASH_BLOCK_SIZE_SHA1];
    u32         len;
} HMAC_CTX;

static inline void HMAC_Init( HMAC_CTX *ctx, const void *key, u32 len )
{
    u8  ipad[DIGEST_HASH_BLOCK_SIZE_SHA1];
    int i;

    if ( ctx == NULL )
        return;
    if ( len > 0 && key == NULL )
        return;

    /* 鍵がブロック長よりも長い場合、ハッシュ値を鍵とする. */
    if ( len > DIGEST_HASH_BLOCK_SIZE_SHA1 )
    {
        SHA1_Calc( ctx->key, key, len );
        ctx->len = DIGEST_SIZE_SHA1;
    }
    else
    {
        MI_CpuCopy8( key, ctx->key, len );
        ctx->len = len;
    }
    /* 鍵とipadのXOR */
    for ( i = 0; i < ctx->len; i++ )
    {
        ipad[i] = (u8)(ctx->key[i] ^ 0x36);
    }
    /* 鍵のパディング部分とipadのXOR */
    for ( ; i < DIGEST_HASH_BLOCK_SIZE_SHA1; i++ )
    {
        ipad[i] = 0x00 ^ 0x36;
    }

    /* メッセージとの結合とハッシュ値の計算 */
    SHA1_Init( &ctx->sha1_ctx );
    SHA1_Update( &ctx->sha1_ctx, ipad, DIGEST_HASH_BLOCK_SIZE_SHA1 );
}

static inline void HMAC_Update( HMAC_CTX *ctx, void *data, u32 len )
{
    if ( ctx == NULL )
        return;
    if ( len > 0 && data == NULL )
        return;
    /* メッセージとの結合とハッシュ値の計算 */
    SHA1_Update( &ctx->sha1_ctx, data, len );
}

static inline void HMAC_GetHash( HMAC_CTX *ctx, u8* md )
{
    u8 opad[DIGEST_HASH_BLOCK_SIZE_SHA1];
    u8 temp[20];
    int i;

    if ( ctx == NULL )
        return;
    if ( md == NULL )
        return;

    /* メッセージとの結合とハッシュ値の計算 */
    SHA1_GetHash( &ctx->sha1_ctx, temp );

    /* 鍵とopadのXOR */
    for ( i = 0; i < ctx->len; i++ )
    {
        opad[i] = (u8)(ctx->key[i] ^ 0x5c);
    }
    /* 鍵のパディング部分とopadのXOR */
    for ( ; i < DIGEST_HASH_BLOCK_SIZE_SHA1; i++ )
    {
        opad[i] = 0x00 ^ 0x5c;
    }
    /* ハッシュ値との結合とハッシュ値の計算 */
    SHA1_Init( &ctx->sha1_ctx );
    SHA1_Update( &ctx->sha1_ctx, opad, DIGEST_HASH_BLOCK_SIZE_SHA1 );
    SHA1_Update( &ctx->sha1_ctx, temp, DIGEST_SIZE_SHA1 );
    SHA1_GetHash( &ctx->sha1_ctx, md );
}

static const u8 s_digestDefaultKey[ DIGEST_HASH_BLOCK_SIZE_SHA1 ] = {
    0x21, 0x06, 0xc0, 0xde,
    0xba, 0x98, 0xce, 0x3f,
    0xa6, 0x92, 0xe3, 0x9d,
    0x46, 0xf2, 0xed, 0x01,

    0x76, 0xe3, 0xcc, 0x08,
    0x56, 0x23, 0x63, 0xfa,
    0xca, 0xd4, 0xec, 0xdf,
    0x9a, 0x62, 0x78, 0x34,

    0x8f, 0x6d, 0x63, 0x3c,
    0xfe, 0x22, 0xca, 0x92,
    0x20, 0x88, 0x97, 0x23,
    0xd2, 0xcf, 0xae, 0xc2,

    0x32, 0x67, 0x8d, 0xfe,
    0xca, 0x83, 0x64, 0x98,
    0xac, 0xfd, 0x3e, 0x37,
    0x87, 0x46, 0x58, 0x24,
};

static BOOL CheckRomCertificate( int* pool, const RomCertificate *pCert, const void* pCAPubKey, u32 gameCode )
{
    u8 digest[DIGEST_SIZE_SHA1];
    u8 md[DIGEST_SIZE_SHA1];
    int i;
    BOOL result = TRUE;

    // 証明書ヘッダのマジックナンバーチェック
    if( pCert->header.magicNumber != TWL_ROM_CERT_MAGIC_NUMBER ||
        // 証明書ヘッダとROMヘッダのゲームコード一致チェック
        pCert->header.gameCode != gameCode )
    {
        result = FALSE;
    }
    // 証明書署名チェック
    SVC_DecryptoSign( pool, &digest, pCert->sign, pCAPubKey );

    // ダイジェストの計算
    SHA1_Calc( md, pCert, ROM_CERT_SIGN_OFFSET );

    // 比較
    for (i = 0; i < DIGEST_SIZE_SHA1; i++)
    {
        if ( md[i] != digest[i] )
        {
            result = FALSE;
        }
    }

    return result;
}

#ifndef SDK_FINALROM
#define PROFILE_PXI_SEND    1000000000
#define PROFILE_PXI_RECV    2000000000
extern u32 profile[];
extern u32 pf_cnt;
#endif

static BOOL MI_LoadBuffer(u8* dest, u32 size, SHA1_CTX *ctx)
{
    u8* base = (u8*)HW_FIRM_LOAD_BUFFER_BASE;
    static int count = 0;
    while (size > 0)
    {
        u8* src = base + count * HW_FIRM_LOAD_BUFFER_UNIT_SIZE;
        u32 unit = size < HW_FIRM_LOAD_BUFFER_UNIT_SIZE ? size : HW_FIRM_LOAD_BUFFER_UNIT_SIZE;
OS_TPrintf("%s: src=%X, unit=%X\n", __func__, src, unit);
        if ( PXI_RecvID() != FIRM_PXI_ID_LOAD_PIRIOD )
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // x2...: after PXI
        profile[pf_cnt++] = PROFILE_PXI_RECV | FIRM_PXI_ID_LOAD_PIRIOD;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        MIi_SetWramBankMaster_B(count, MI_WRAM_ARM9);
        if (ctx)
        {
            int done;
            for (done = 0; done < unit; done += HASH_UNIT)
            {
                u8* s = src + done;
                u8* d = dest + done;
                u32 u = unit < done + HASH_UNIT ? unit - done : HASH_UNIT;
                SHA1_Update( ctx, s, u );
                MI_CpuCopyFast( s, d, u );
            }
        }
        else
        {
            MI_CpuCopyFast( src, dest, unit );
        }
        MI_CpuClearFast( src, unit );
        DC_FlushRange( src, unit );
        MIi_SetWramBankMaster_B(count, MI_WRAM_ARM7);
        count = (count + 1) % HW_FIRM_LOAD_BUFFER_UNIT_NUMS;
        size -= unit;
        dest += unit;
    }
    return TRUE;
}

static /*inline*/ BOOL MI_LoadModule(void* dest, u32 size, const u8 digest[DIGEST_SIZE_SHA1])
{
    HMAC_CTX ctx;
    u8 md[DIGEST_SIZE_SHA1];
    int i;
    BOOL result = TRUE;

    HMAC_Init(&ctx, s_digestDefaultKey, DIGEST_HASH_BLOCK_SIZE_SHA1 );
    if ( !MI_LoadBuffer( dest, size, &ctx.sha1_ctx ) )  // UpdateはSHA1と同じ処理
    {
        return FALSE;
    }
    HMAC_GetHash(&ctx, md);
#ifndef SDK_FINALROM
    // xx: after SHA1
    profile[pf_cnt++] = (u32)20202020;  // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    for ( i = 0; i < DIGEST_SIZE_SHA1; i++ )
    {
        if ( md[i] != digest[i] )
        {
            result = FALSE;
        }
    }

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MI_LoadHeader

  Description:  load header

  Arguments:    pool        pointer to the pool info for SVC_DecryptoSign
                rsa_key     key address

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadHeader( int* pool, const void* rsa_key )
{
    SHA1_CTX ctx;
    u8 md[DIGEST_SIZE_SHA1];
    SignatureData sd;
    int i;
    BOOL result = TRUE;

    SHA1_Init(&ctx);

#ifndef SDK_FINALROM
    pf_cnt = 10;
#endif
    // load header (hash target)
    if ( PXI_RecvID() != FIRM_PXI_ID_LOAD_HEADER ||
#ifndef SDK_FINALROM
        // 10: after PXI
        ((profile[pf_cnt++] = PROFILE_PXI_RECV | FIRM_PXI_ID_LOAD_HEADER), FALSE) ||
        ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
         !MI_LoadBuffer( (u8*)rh, AUTH_SIZE, &ctx ) )
    {
        return FALSE;
    }
    SHA1_GetHash(&ctx, md);
#ifndef SDK_FINALROM
        // 1x: after HMAC
        profile[pf_cnt++] = (u32)2020202020;    // checkpoint
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    // load header (remain)
    if ( !MI_LoadBuffer( (u8*)rh + AUTH_SIZE, HEADER_SIZE - AUTH_SIZE, NULL ) )
    {
        return FALSE;
    }

    // コンテンツ証明書
    if ( CheckRomCertificate( pool, &rh->certificate, rsa_key, *(u32*)rh->s.game_code ) )
    {
        rsa_key = rh->certificate.pubKeyMod;   // ヘッダ用の鍵の取り出し
    }
    else
    {
        // とりあえずコンテンツ証明書用の鍵がそのまま使えると仮定
    }

    // ヘッダ署名チェック
    SVC_DecryptoSign( pool, &sd, rh->signature, rsa_key );
    for (i = 0; i < DIGEST_SIZE_SHA1; i++)
    {
        if ( md[i] != sd.digest[i] )
        {
            result = FALSE;
        }
    }
#ifndef SDK_FINALROM
    // 1x: after RSA, before PXI
    profile[pf_cnt++] = (u32)128128128; // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
    profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_AUTH_HEADER;    // checkpoint
#endif
    if ( result )
    {
        PXI_NotifyID( FIRM_PXI_ID_AUTH_HEADER );
        PXI_SendDataByFifo( PXI_FIFO_TAG_DATA, sd.aes_key_seed, AES_BLOCK_SIZE );
        // DS互換ヘッダコピー
        MI_CpuCopyFast( rh, (void*)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF );
    }
    MI_CpuClear8(&sd, sizeof(sd));
    MI_CpuClear8(&md, sizeof(md));

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MI_LoadMenu

  Description:  load menu program

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadMenu( void )
{
    // load ARM9 static region
    if ( rh->s.main_size > 0 )
    {
#ifndef SDK_FINALROM
        // 30: before PXI
        pf_cnt = 30;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( PXI_RecvID() !=  FIRM_PXI_ID_LOAD_ARM9_STATIC ||
#ifndef SDK_FINALROM
            // 31: after PXI
            ((profile[pf_cnt++] = PROFILE_PXI_RECV | FIRM_PXI_ID_LOAD_ARM9_STATIC), FALSE) ||
            ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
             !MI_LoadModule( rh->s.main_ram_address, rh->s.main_size, rh->s.main_static_digest ) )
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // 3x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_AUTH_ARM9_STATIC;   // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_AUTH_ARM9_STATIC );
    }

    // load ARM7 static region
    if ( rh->s.sub_size > 0 )
    {
#ifndef SDK_FINALROM
        // 50: before PXI
        pf_cnt = 50;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( PXI_RecvID() !=  FIRM_PXI_ID_LOAD_ARM7_STATIC ||
#ifndef SDK_FINALROM
            // 51: after PXI
            ((profile[pf_cnt++] = PROFILE_PXI_RECV | FIRM_PXI_ID_LOAD_ARM7_STATIC), FALSE) ||
            ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
             !MI_LoadModule( rh->s.sub_ram_address, rh->s.sub_size, rh->s.sub_static_digest ) )
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // 5x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_AUTH_ARM7_STATIC;   // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_AUTH_ARM7_STATIC );
    }

    // load ARM9 extended static region
    if ( rh->s.main_ltd_size > 0 )
    {
#ifndef SDK_FINALROM
        // 70: before PXI
        pf_cnt = 70;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( PXI_RecvID() !=  FIRM_PXI_ID_LOAD_ARM9_LTD_STATIC ||
#ifndef SDK_FINALROM
            // 71: after PXI
            ((profile[pf_cnt++] = PROFILE_PXI_RECV | FIRM_PXI_ID_LOAD_ARM9_LTD_STATIC), FALSE) ||
            ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
             !MI_LoadModule( rh->s.main_ltd_ram_address, rh->s.main_ltd_size, rh->s.main_ltd_static_digest ) )
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // 7x: after PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_AUTH_ARM9_LTD_STATIC;    // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_AUTH_ARM9_LTD_STATIC );
    }
    // load ARM7 extended static region
    if ( rh->s.sub_ltd_size > 0 )
    {
#ifndef SDK_FINALROM
        // 90: before PXI
        pf_cnt = 90;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( PXI_RecvID() !=  FIRM_PXI_ID_LOAD_ARM7_LTD_STATIC ||
#ifndef SDK_FINALROM
            // 91: after PXI
            ((profile[pf_cnt++] = PROFILE_PXI_RECV | FIRM_PXI_ID_LOAD_ARM7_LTD_STATIC), FALSE) ||
            ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
             !MI_LoadModule( rh->s.sub_ltd_ram_address, rh->s.sub_ltd_size, rh->s.sub_ltd_static_digest ) )
        {
            return FALSE;
        }
#ifndef SDK_FINALROM
        // 9x: before PXI
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_AUTH_ARM7_LTD_STATIC;    // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_AUTH_ARM7_LTD_STATIC );
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MI_BootMenu

  Description:  boot menu

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_BootMenu( void )
{
    OSi_Boot( rh->s.main_entry_address, (MIHeader_WramRegs*)rh->s.main_wram_config_data );
}
