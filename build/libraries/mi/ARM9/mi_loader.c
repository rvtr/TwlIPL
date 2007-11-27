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
#include <twl/os/common/format_rom.h>

/*
    PROFILE_ENABLE を定義するとある程度のパフォーマンスチェックができます。
    利用するためには、main.cかどこかに、u32 profile[256]; u32 pf_cnt; を
    定義する必要があります。
*/
//#define PROFILE_ENABLE

#ifdef SDK_FINALROM // FINALROMで無効化
#undef PROFILE_ENABLE
#endif

#ifdef PROFILE_ENABLE
#define PROFILE_PXI_SEND    0x10000000
#define PROFILE_PXI_RECV    0x20000000
#define PROFILE_SHA1        0xa0000000
#define PROFILE_RSA         0xb0000000
extern u32 profile[];
extern u32 pf_cnt;
#endif


#define PXI_FIFO_TAG_DATA   PXI_FIFO_TAG_USER_0

static ROM_Header* const rh = (ROM_Header*)HW_TWL_ROM_HEADER_BUF;

#define HEADER_SIZE 0x1000
#define AUTH_SIZE   0xe00
#define RSA_BLOCK_SIZE  128

#define HASH_UNIT   0x1000   // TODO: optimizing to maximize cache efficiency

static const u8 s_digestDefaultKey[ SVC_SHA1_BLOCK_SIZE ] = {
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
    SVC_DecryptSign( pool, &digest, pCert->sign, pCAPubKey );

    // ダイジェストの計算
    SVC_CalcSHA1( md, pCert, ROM_CERT_SIGN_OFFSET );

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

/*---------------------------------------------------------------------------*
  Name:         MI_LoadBuffer

  Description:  receive data from ARM7 and store(move) via WRAM[B]

                LoadBufferメカニズムで、ファイルの内容をARM7から受け取ります。
                引数でSVCSHA1Contextを指定していた場合、コピーのついでにSHA1の
                計算も行います。

                [LoadBufferメカニズム]
                WRAM[B]を利用して、ARM7,ARM9間のデータ転送を行います。
                WRAM[B]の各スロットをバケツリレー方式で渡します。
                1スロット分のデータまたは全データが格納されたとき、ARM7から
                FIRM_PXI_ID_LOAD_PIRIODを受信します。
                ARM9は受信後にそのスロットの使用権をARM9に変更してデータを
                取り出し、完了後にメモリをクリアして(セキュリティ)、使用権を
                ARM7に戻します。

                [使用条件]
                WRAM[B]をロックせず、初期状態としてARM7側に倒しておくこと。

                [注意点]
                offsetとsizeはARM7から通知されません。別の経路で同期を取ってください。
                SRLファイルを読み込む場合は、互いにROMヘッダを参照できれば十分です。
                (ROMヘッダ部分は元から知っているはず)

                補足:
                ここでは、あるライブラリ内でARM7/ARM9側で歩調を合わせられることを
                前提にしているが、汎用的にするには(独立ライブラリ化するなら)、
                送受信でスロットを半分ずつとし、それぞれに受信側のPXIコールバック
                ＆スレッドを用意し、送信側APIがデータをWRAMに格納した後、他方に
                destとsizeを通知するという形でOKではないか？
                (で完了したら返事を返す)

  Arguments:    dest        destination address for received data
                size        size to load
                ctx         context for SHA1 if execute SVC_SHA1Update

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadBuffer(u8* dest, u32 size, SVCSHA1Context *ctx)
{
    u8* base = (u8*)HW_FIRM_LOAD_BUFFER_BASE;
    static int count = 0;

    while (size > 0)
    {
        u8* src = base + count * HW_FIRM_LOAD_BUFFER_UNIT_SIZE;
        u32 unit = size < HW_FIRM_LOAD_BUFFER_UNIT_SIZE ? size : HW_FIRM_LOAD_BUFFER_UNIT_SIZE;
        //OS_TPrintf("%s: src=%X, unit=%X\n", __func__, src, unit);
        if ( PXI_RecvID() != FIRM_PXI_ID_LOAD_PIRIOD )
        {
            return FALSE;
        }
#ifdef PROFILE_ENABLE
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
                SVC_SHA1Update( ctx, s, u );
                MI_CpuCopyFast( s, d, u );
            }
        }
        else
        {
            MI_CpuCopyFast( src, dest, unit );
        }
        MI_CpuClearFast( src, unit );
        DC_FlushRange( src, unit );
#ifdef PROFILE_ENABLE
        // x2...: after copy & clear
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        MIi_SetWramBankMaster_B(count, MI_WRAM_ARM7);
        count = (count + 1) % HW_FIRM_LOAD_BUFFER_UNIT_NUMS;
        size -= unit;
        dest += unit;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MI_LoadHeader

  Description:  load header

                SRLのROMヘッダ部分をARM7から受け取り、認証します。
                受信前に、ARM7から FIRM_PXI_ID_LOAD_HEADER を受信します。
                受信後、認証が通ったならARM7へ FIRM_PXI_ID_AUTH_HEADER を送信
                します。それ以前に、メインメモリの所定の位置にROMヘッダが格納
                されていなければなりません。
                続けて、seedデータを16バイト送信します。
                makerom.TWLまたはIPLの仕様に依存します。

  Arguments:    pool        pointer to the pool info for SVCSignHeapContext
                rsa_key     key address

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadHeader( SVCSignHeapContext* pool, const void* rsa_key )
{
    SVCSHA1Context ctx;
    u8 md[DIGEST_SIZE_SHA1];
    SignatureData sd;
    int i;
    BOOL result = TRUE;

    SVC_SHA1Init(&ctx);

#ifdef PROFILE_ENABLE
    pf_cnt = 0x10;
#endif
    // load header (hash target)
    if ( PXI_RecvID() != FIRM_PXI_ID_LOAD_HEADER ||
#ifdef PROFILE_ENABLE
        // 10: after PXI
        ((profile[pf_cnt++] = PROFILE_PXI_RECV | FIRM_PXI_ID_LOAD_HEADER), FALSE) ||
        ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
         !MI_LoadBuffer( (u8*)rh, AUTH_SIZE, &ctx ) )
    {
        return FALSE;
    }
    SVC_SHA1GetHash(&ctx, md);
#ifdef PROFILE_ENABLE
        // 1x: after HMAC
        profile[pf_cnt++] = PROFILE_SHA1;    // checkpoint
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
    SVC_DecryptSign( pool, &sd, rh->signature, rsa_key );
    for (i = 0; i < DIGEST_SIZE_SHA1; i++)
    {
        if ( md[i] != sd.digest[i] )
        {
            result = FALSE;
        }
    }
#ifdef PROFILE_ENABLE
    // 1x: after RSA, before PXI
    profile[pf_cnt++] = PROFILE_RSA; // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
    profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_AUTH_HEADER;    // checkpoint
#endif
    if ( result )
    {
        DC_StoreRange( rh, HW_TWL_ROM_HEADER_BUF_SIZE );
        PXI_NotifyID( FIRM_PXI_ID_AUTH_HEADER );
        AESi_SendSeed( (AESKey*)sd.aes_key_seed );
        // DS互換ヘッダコピー
        MI_CpuCopyFast( rh, (void*)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF );
    }
    MI_CpuClear8(&sd, sizeof(sd));
    MI_CpuClear8(&md, sizeof(md));

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MIi_GetTransferSize

  Description:  get size to transfer once

                一度に受信するサイズを返します。

                転送範囲がAES領域をまたぐ場合は、境界までのサイズ (引数より
                小さなサイズ) を返します。
                makerom.TWLまたはIPLの使用に依存します。

  Arguments:    offset  offset of region from head of ROM_Header
                size    size of region

  Returns:      size to transfer once
 *---------------------------------------------------------------------------*/
static u32 MIi_GetTransferSize( u32 offset, u32 size )
{
    u32 aes_offset = rh->s.aes_target_rom_offset;
    u32 aes_end = aes_offset + rh->s.aes_target_size;
    u32 end = offset + size;
    if ( rh->s.enable_aes )
    {
        if ( offset >= aes_offset && offset < aes_end )
        {
            if ( end > aes_end )
            {
                size = aes_end - offset;
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
  Name:         MIi_LoadModule

  Description:  receive module from ARM7 and store(move) via WRAM[B]

                MI_LoadBufferの上位APIです。

                AES境界をまたぐ場合は、2回のLoadBufferに分割します。

                すでにハッシュ値が分かっていて、ちょうどSHA1の計算範囲全体を
                読み込む場合に便利です。

  Arguments:    dest        destination address for received data
                offset      offset from head of ROM_Header
                size        size to load
                digest      digest to compare

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static /*inline*/ BOOL MIi_LoadModule(void* dest, u32 offset, u32 size, const u8 digest[DIGEST_SIZE_SHA1])
{
    SVCHMACSHA1Context ctx;
    u8 md[DIGEST_SIZE_SHA1];
    int i;
    BOOL result = TRUE;

    SVC_HMACSHA1Init(&ctx, s_digestDefaultKey, SVC_SHA1_BLOCK_SIZE );
    while ( size > 0 )
    {
        u32 unit = MIi_GetTransferSize( offset, size );
        if ( !MI_LoadBuffer( dest, unit, &ctx.sha1_ctx ) ) // UpdateはSHA1と同じ処理
        {
            return FALSE;
        }
        dest = (u8*)dest + unit;
        offset += unit;
        size -= unit;
    }
    SVC_HMACSHA1GetHash(&ctx, md);
#ifdef PROFILE_ENABLE
    // 3x: after SHA1
    profile[pf_cnt++] = PROFILE_SHA1;  // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    for (i = 0; i < DIGEST_SIZE_SHA1; i++)
    {
        if (md[i] != digest[i])
        {
            result = FALSE;
        }
    }
    MI_CpuClear8(md, DIGEST_SIZE_SHA1);
    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MI_LoadStatic

  Description:  load static binary

                ARM9/ARM7のStaticおよびLTD Staticを受信します。
                受信前に、ARM7からFIRM_PXI_ID_LOAD_*_STATICを受信します。
                受信後、認証が通ったならARM7へFIRM_PXI_ID_AUTH_*_STATICを送信
                します。サイズが0の場合は、そのパートのPXI通信すら行いません。

                このAPIを呼び出す前に、メインメモリの所定の位置にROMヘッダが
                格納されている必要があります。

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadStatic( void )
{
    // load static
    if ( PXI_RecvID() != FIRM_PXI_ID_LOAD_STATIC )
    {
        return FALSE;
    }
#ifdef PROFILE_ENABLE
    // 30: after PXI
    pf_cnt = 0x30;
    profile[pf_cnt++] = PROFILE_PXI_RECV | FIRM_PXI_ID_LOAD_STATIC;
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    // load ARM9 static region
    if ( rh->s.main_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 31: before PXI
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !MIi_LoadModule(rh->s.main_ram_address, rh->s.main_rom_offset, rh->s.main_size, rh->s.main_static_digest) )
        {
            return FALSE;
        }
    }

    // load ARM7 static region
    if ( rh->s.sub_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 50: before PXI
        pf_cnt = 0x50;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !MIi_LoadModule( rh->s.sub_ram_address, rh->s.sub_rom_offset, rh->s.sub_size, rh->s.sub_static_digest ) )
        {
            return FALSE;
        }
    }

    // load ARM9 extended static region
    if ( rh->s.main_ltd_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 70: before PXI
        pf_cnt = 0x70;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !MIi_LoadModule( rh->s.main_ltd_ram_address, rh->s.main_ltd_rom_offset, rh->s.main_ltd_size, rh->s.main_ltd_static_digest ) )
        {
            return FALSE;
        }
    }
    // load ARM7 extended static region
    if ( rh->s.sub_ltd_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 90: before PXI
        pf_cnt = 0x90;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !MIi_LoadModule( rh->s.sub_ltd_ram_address, rh->s.sub_ltd_rom_offset, rh->s.sub_ltd_size, rh->s.sub_ltd_static_digest ) )
        {
            return FALSE;
        }
    }
#ifdef PROFILE_ENABLE
    // 9x: before PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
    profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_AUTH_STATIC;    // checkpoint
#endif
    PXI_NotifyID( FIRM_PXI_ID_AUTH_STATIC );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         MI_Boot

  Description:  boot

                ROMヘッダの情報を引数に、OSi_Bootを呼び出すだけです。

                このAPIを呼び出す前に、メインメモリの所定の位置にROMヘッダが
                格納されている必要があります。

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_Boot( void )
{
    OS_BootWithRomHeaderFromFIRM( rh );
}
