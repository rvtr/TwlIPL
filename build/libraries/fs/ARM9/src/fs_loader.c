/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fs
  File:     fs_loader.c

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

#include <firm.h>
#include <estypes.h>
#include <es.h>

#define FS_HEADER_AUTH_SIZE 0xe00

#define HASH_UNIT                   0x1000

static ROM_Header* const rh = (ROM_Header*)HW_TWL_ROM_HEADER_BUF;

static u8 currentKey[ SVC_SHA1_BLOCK_SIZE ];

static const u8 defaultKey[ SVC_SHA1_BLOCK_SIZE ] =
{
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


/*---------------------------------------------------------------------------*
  Name:         FS_SetDigestKey

  Description:  set specified key or default key for HMAC-SHA-1

  Arguments:    digestKey       pointer to key
                                if NULL, use default key

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline void FS_SetDigestKey( const u8* digestKey )
{
    if ( digestKey )
    {
        MI_CpuCopy8(digestKey, currentKey, SVC_SHA1_BLOCK_SIZE);
    }
    else
    {
        MI_CpuCopy8(defaultKey, currentKey, SVC_SHA1_BLOCK_SIZE);
    }
}

static inline BOOL CheckDigest( u8* a, u8* b, BOOL aClr, BOOL bClr )
{
    BOOL result = TRUE;
    int i;
    for ( i = 0; i < SVC_SHA1_DIGEST_SIZE; i++ )
    {
        if ( a[i] != b[i] )
        {
            result = FALSE;
        }
    }
    if ( aClr ) MI_CpuClear8(a, SVC_SHA1_DIGEST_SIZE);
    if ( bClr ) MI_CpuClear8(b, SVC_SHA1_DIGEST_SIZE);
    return result;
}
#ifdef SUPPORT_CERTIFICATION
/*---------------------------------------------------------------------------*
  Name:         CheckRomCertificate

  Description:  check the certification in the ROM

                ROMヘッダに付加された証明書のチェックを行います。
                makerom.TWL内のコードに依存します。

  Arguments:    pool        pointer to the SVCSignHeapContext
                pCert       pointer to the certification
                pCAPubKey   pointer to the public key for the certification
                gameCode    initial code

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static BOOL CheckRomCertificate( SVCSignHeapContext* pool, const RomCertificate *pCert, const void* pCAPubKey, u32 gameCode )
{
    u8 digest[SVC_SHA1_DIGEST_SIZE];
    u8 md[SVC_SHA1_DIGEST_SIZE];

    // 証明書ヘッダのマジックナンバーチェック
    if( pCert->header.magicNumber != TWL_ROM_CERT_MAGIC_NUMBER ||
        // 証明書ヘッダとROMヘッダのゲームコード一致チェック
        pCert->header.gameCode != gameCode )
    {
        return FALSE;
    }
    // 証明書署名チェック
    SVC_DecryptSign( pool, &digest, pCert->sign, pCAPubKey );

    // ダイジェストの計算
    SVC_CalcSHA1( md, pCert, ROM_CERT_SIGN_OFFSET );

    // 比較
    return CheckDigest(md, digest, TRUE, TRUE);
}
#endif
/*---------------------------------------------------------------------------*
  Name:         FS_LoadBuffer

  Description:  receive data from ARM7 via WRAM-B and store in destination address,
                calculate SHA1 in parallel if ctx is specified

  Arguments:    dest            destination address to read
                size            total length to read in bytes
                ctx             pointer to SHA1 context or NULL

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadBuffer( u8* dest, u32 size, SVCSHA1Context *ctx )
{
    static int count = 0;

    while ( size > 0 )
    {
        u8* src = (u8*)HW_FIRM_LOAD_BUFFER_BASE + count * HW_FIRM_LOAD_BUFFER_UNIT_SIZE;
        u32 unit = size < HW_FIRM_LOAD_BUFFER_UNIT_SIZE ? size : HW_FIRM_LOAD_BUFFER_UNIT_SIZE;
        PXI_AcquireLoadBufferSemaphore(); // wait to be ready
        MIi_SetWramBankMaster_B( count, MI_WRAM_ARM9 );
        if (ctx)
        {
            int done;
            for ( done = 0; done < unit; done += HASH_UNIT )
            {
                u8* s = src + done;
                u8* d = dest + done;
                u32 u = unit - done < HASH_UNIT ? unit - done : HASH_UNIT;
                SVC_SHA1Update( ctx, s, u );
                MI_CpuCopyFast( s, d, u );
            }
        }
        else
        {
            MI_CpuCopyFast( src, dest, unit );
        }
        DC_InvalidateRange( src, unit );
        size -= unit;
        dest += unit;
        MIi_SetWramBankMaster_B( count, MI_WRAM_ARM7 );
        PXI_ReleaseLoadBufferSemaphore();
        count = ( count + 1 ) % HW_FIRM_LOAD_BUFFER_UNIT_NUMS;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         GetTransferSize

  Description:  get size to transfer once

                一度に受信するサイズを返します。

                転送範囲がAES領域をまたぐ場合は、境界までのサイズ (引数より
                小さなサイズ) を返します。
                makerom.TWLまたはIPLの使用に依存します。

  Arguments:    offset  offset of region from head of ROM_Header
                size    size of region

  Returns:      size to transfer once
 *---------------------------------------------------------------------------*/
static u32 GetTransferSize( u32 offset, u32 size )
{
    if ( rh->s.enable_aes )
    {
        u32 end = offset + size;
        u32 aes_offset = rh->s.aes_target_rom_offset;
        u32 aes_end = aes_offset + rh->s.aes_target_size;
        u32 aes_offset2 = rh->s.aes_target2_rom_offset;
        u32 aes_end2 = aes_offset2 + rh->s.aes_target2_size;

        if ( offset >= aes_offset && offset < aes_end )
        {
            if ( end > aes_end )
            {
                size = aes_end - offset;
            }
        }
        else if ( offset >= aes_offset2 && offset < aes_end2 )
        {
            if ( end > aes_end2 )
            {
                size = aes_end2 - offset;
            }
        }
        else
        {
            if ( offset < aes_offset && offset + size > aes_offset )
            {
                size = aes_offset - offset;
            }
        }
    }
    return size;
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadModule

  Description:  receive data from ARM7 via WRAM-B and store in destination address
                in view of AES settings in the ROM header at HW_TWL_ROM_HEADER_BUF,
                then verify the digest

  Arguments:    dest            destination address to read
                offset          file offset to start to read in bytes
                size            total length to read in bytes
                digest          digest to verify

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadModule( u8* dest, u32 offset, u32 size, const u8 digest[SVC_SHA1_DIGEST_SIZE] )
{
#ifndef NO_SECURITY_CHECK
    SVCHMACSHA1Context ctx;
    u8 md[SVC_SHA1_DIGEST_SIZE];

    SVC_HMACSHA1Init(&ctx, currentKey, SVC_SHA1_BLOCK_SIZE );
    while ( size > 0 )
    {
        u32 unit = GetTransferSize( offset, size );
        if ( !FS_LoadBuffer( dest, unit, &ctx.sha1_ctx ) )
        {
            return FALSE;
        }
        dest += unit;
        offset += unit;
        size -= unit;
    }
    SVC_HMACSHA1GetHash(&ctx, md);
    return CheckDigest(md, (u8*)digest, TRUE, FALSE);
#else
    (void)digest;
    while ( size > 0 )
    {
        u32 unit = GetTransferSize( offset, size );
        if ( !FS_LoadBuffer( dest, unit, NULL ) )
        {
            return FALSE;
        }
        dest += unit;
        offset += unit;
        size -= unit;
    }
    return TRUE;
#endif
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadHeader

  Description:  receive ROM header, store to HW_TWL_ROM_HEADER_BUF,
                and verify signature

  Arguments:    pool            heap context to call SVC_DecryptSign
                rsa_key_user    public key to verify the signature for user application
                rsa_key_sys     public key to verify the signature for system application
                rsa_key_secure  public key to verify the signature for secure application

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadHeader( SVCSignHeapContext* pool, const void* rsa_key_user, const void* rsa_key_sys, const void* rsa_key_secure )
{
#ifndef NO_SECURITY_CHECK
    const void* rsa_key;
    SVCSHA1Context ctx;
    u8 md[SVC_SHA1_DIGEST_SIZE];
    SignatureData sd;

    SVC_SHA1Init( &ctx );
    if ( !FS_LoadBuffer( (u8*)rh, FS_HEADER_AUTH_SIZE, &ctx ) )
    {
        return FALSE;
    }
    SVC_SHA1GetHash( &ctx, md );
    if ( !FS_LoadBuffer( (u8*)rh + FS_HEADER_AUTH_SIZE, HW_TWL_ROM_HEADER_BUF_SIZE - FS_HEADER_AUTH_SIZE, NULL ) )
    {
        return FALSE;
    }

    // 鍵の確定
    rsa_key = (rh->s.titleID_Hi & TITLE_ID_HI_SECURE_FLAG_MASK)
            ? rsa_key_secure
            : ( (rh->s.titleID_Hi & TITLE_ID_HI_APP_TYPE_MASK) ? rsa_key_sys : rsa_key_user );
#ifdef SUPPORT_CERTIFICATION
    // コンテンツ証明書
    if ( CheckRomCertificate( pool, &rh->certificate, rsa_key, *(u32*)rh->s.game_code ) )
    {
        rsa_key = rh->certificate.pubKeyMod;   // ヘッダ用の鍵の取り出し
    }
    else
    {
        // とりあえずコンテンツ証明書用の鍵がそのまま使えると仮定
    }
#endif
    // ヘッダ署名チェック
    SVC_DecryptSign( pool, &sd, rh->signature, rsa_key );

    if ( !CheckDigest( md, sd.digest, TRUE, FALSE ) )
    {
        MI_CpuClear8( &sd, sizeof(sd) );    // 残り削除 (他に必要なものはない？)
        return FALSE;
    }

    // ダイジェスト以外のデータのチェックが必要！！

    MI_CpuClear8( &sd, sizeof(sd) );    // 残り削除 (他に必要なものはない？)
#else
    (void)pool;
    (void)rsa_key1;
    (void)rsa_key2;
    FS_LoadBuffer( (u8*)rh, FS_HEADER_AUTH_SIZE, NULL );
    FS_LoadBuffer( (u8*)rh + FS_HEADER_AUTH_SIZE, HW_TWL_ROM_HEADER_BUF_SIZE - FS_HEADER_AUTH_SIZE, NULL );
#endif
    // ROMヘッダのコピー
    MI_CpuCopyFast( rh, (void*)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadStatic

  Description:  receive static regions from ARM6 via WRAM-B and store them
                specified by ROM header at HW_TWL_ROM_HEADER_BUF

  Arguments:    digestKey       pointer to key for HMAC-SHA1
                                if NULL, use default key

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadStatic( const u8* digestKey )
{
    FS_SetDigestKey( digestKey );
    if ( rh->s.main_size > 0 )
    {
        if ( !FS_LoadModule( rh->s.main_ram_address, rh->s.main_rom_offset, rh->s.main_size, rh->s.main_static_digest ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.sub_size > 0 )
    {
        if ( !FS_LoadModule( rh->s.sub_ram_address, rh->s.sub_rom_offset, rh->s.sub_size, rh->s.sub_static_digest ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.main_ltd_size > 0 )
    {
        if ( !FS_LoadModule( rh->s.main_ltd_ram_address, rh->s.main_ltd_rom_offset, rh->s.main_ltd_size, rh->s.main_ltd_static_digest ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.sub_ltd_size > 0 )
    {
        if ( !FS_LoadModule( rh->s.sub_ltd_ram_address, rh->s.sub_ltd_rom_offset, rh->s.sub_ltd_size, rh->s.sub_ltd_static_digest ) )
        {
            return FALSE;
        }
    }
    return TRUE;
}
