/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fs
  File:     fs_firm.c

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

#define MODULE_ALIGNMENT    0x10    // 16�o�C�g�P�ʂœǂݍ���
//#define MODULE_ALIGNMENT  0x200   // 512�o�C�g�P�ʂœǂݍ���
#define RoundUpModuleSize(value)    (((value) + MODULE_ALIGNMENT - 1) & -MODULE_ALIGNMENT)

#define CONTENT_INDEX_SRL           0
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

static AESKey FSiAesKeySeed;

/*---------------------------------------------------------------------------*
  Name:         FS_InitFIRM

  Description:  initialize FS/FATFS for firm

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FS_InitFIRM( void )
{
    MI_CpuClearFast( (void*)HW_FIRM_FS_TEMP_BUFFER, HW_FIRM_FS_TEMP_BUFFER_SIZE );
    FSiTemporaryBuffer = (void*)HW_FIRM_FS_TEMP_BUFFER;
    FATFS_InitFIRM();
    FS_Init( FS_DMA_NOT_USE );
}

/*---------------------------------------------------------------------------*
  Name:         FS_GetAesKeySeed

  Description:  retreive aes key seed in the signature

  Arguments:    None

  Returns:      pointer to seed
 *---------------------------------------------------------------------------*/
AESKey* const FS_GetAesKeySeed( void )
{
    return &FSiAesKeySeed;
}

/*---------------------------------------------------------------------------*
  Name:         FS_DeleteAesKeySeed

  Description:  delete aes key seed in the signature

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FS_DeleteAesKeySeed( void )
{
    MI_CpuClear8( &FSiAesKeySeed, sizeof(FSiAesKeySeed) );
}

/*---------------------------------------------------------------------------*
  Name:         FS_ResolveSrl

  Description:  resolve srl filename and store to HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    titleId         title id for srl file

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_ResolveSrl( u64 titleId )
{
    if ( ES_ERR_OK != ES_InitLib() ||
         !FS_GetTitleBootContentPathFast((char*)HW_TWL_FS_BOOT_SRL_PATH_BUF, titleId) ||
         ES_ERR_OK != ES_CloseLib() )
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_SetDigestKey

  Description:  set specified key or default key for HMAC-SHA-1

  Arguments:    digestKey       pointer to key
                                if NULL, use default key

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
void FS_SetDigestKey( const u8* digestKey )
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

/*---------------------------------------------------------------------------*
  Name:         CheckRomCertificate

  Description:  check the certification in the ROM

                ROM�w�b�_�ɕt�����ꂽ�ؖ����̃`�F�b�N���s���܂��B
                makerom.TWL���̃R�[�h�Ɉˑ����܂��B

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

    // �ؖ����w�b�_�̃}�W�b�N�i���o�[�`�F�b�N
    if( pCert->header.magicNumber != TWL_ROM_CERT_MAGIC_NUMBER ||
        // �ؖ����w�b�_��ROM�w�b�_�̃Q�[���R�[�h��v�`�F�b�N
        pCert->header.gameCode != gameCode )
    {
        return FALSE;
    }
    // �ؖ��������`�F�b�N
    SVC_DecryptSign( pool, &digest, pCert->sign, pCAPubKey );

    // �_�C�W�F�X�g�̌v�Z
    SVC_CalcSHA1( md, pCert, ROM_CERT_SIGN_OFFSET );

    // ��r
    return CheckDigest(md, digest, TRUE, TRUE);
}

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
        if ( PXI_RecvID() != FIRM_PXI_ID_LOAD_PIRIOD )
        {
            return FALSE;
        }
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
                MI_CpuClearFast( s, u );    // OS_Boot�ł̃N���A�Ɣ�r����
            }
        }
        else
        {
            MI_CpuCopyFast( src, dest, unit );
            MI_CpuClearFast( src, unit );   // OS_Boot�ł̃N���A�Ɣ�r����
        }
        DC_FlushRange( src, unit );
        size -= unit;
        dest += unit;
        MIi_SetWramBankMaster_B( count, MI_WRAM_ARM7 );
        count = ( count + 1 ) % HW_FIRM_LOAD_BUFFER_UNIT_NUMS;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         GetTransferSize

  Description:  get size to transfer once

                ��x�Ɏ�M����T�C�Y��Ԃ��܂��B

                �]���͈͂�AES�̈���܂����ꍇ�́A���E�܂ł̃T�C�Y (�������
                �����ȃT�C�Y) ��Ԃ��܂��B
                makerom.TWL�܂���IPL�̎g�p�Ɉˑ����܂��B

  Arguments:    offset  offset of region from head of ROM_Header
                size    size of region

  Returns:      size to transfer once
 *---------------------------------------------------------------------------*/
static u32 GetTransferSize( u32 offset, u32 size )
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
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadHeader

  Description:  receive ROM header, store to HW_TWL_ROM_HEADER_BUF,
                and verify signature

  Arguments:    pool            heap context to call SVC_DecryptSign
                rsa_key         public key to verify the signature

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadHeader( SVCSignHeapContext* pool, const void* rsa_key )
{
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

    // �R���e���c�ؖ���
    if ( CheckRomCertificate( pool, &rh->certificate, rsa_key, *(u32*)rh->s.game_code ) )
    {
        rsa_key = rh->certificate.pubKeyMod;   // �w�b�_�p�̌��̎��o��
    }
    else
    {
        // �Ƃ肠�����R���e���c�ؖ����p�̌������̂܂܎g����Ɖ���
    }

    // �w�b�_�����`�F�b�N
    SVC_DecryptSign( pool, &sd, rh->signature, rsa_key );

    if ( !CheckDigest( md, sd.digest, TRUE, FALSE ) )
    {
        MI_CpuClear8( &sd, sizeof(sd) );    // �c��폜 (���ɕK�v�Ȃ��̂͂Ȃ��H)
        return FALSE;
    }

    // �_�C�W�F�X�g�ȊO�̃f�[�^�̃`�F�b�N���K�v�I�I

    // ���̕ۑ�
    MI_CpuCopy8( (AESKey*)sd.aes_key_seed, &FSiAesKeySeed, sizeof(FSiAesKeySeed) );

    MI_CpuClear8( &sd, sizeof(sd) );    // �c��폜 (���ɕK�v�Ȃ��̂͂Ȃ��H)

    // ROM�w�b�_�̃R�s�[
    MI_CpuCopyFast( rh, (void*)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadStatic

  Description:  receive static regions from ARM6 via WRAM-B and store them
                specified by ROM header at HW_TWL_ROM_HEADER_BUF

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadStatic( void )
{
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

/*
    �ȉ��ALoadBuffer���g��Ȃ��� (�ʏ�FS API���g�p����)
    AES��Ή��I�I
*/

/*---------------------------------------------------------------------------*
  Name:         FS_LoadSrlModule

  Description:  receive data from ARM7 via WRAM-B and store in destination address
                in view of AES settings in the ROM header at HW_TWL_ROM_HEADER_BUF,
                then verify the digest

  Arguments:    pFile           pointer to FSFile streucture
                dest            destination address to read
                offset          file offset to start to read in bytes
                size            total length to read in bytes
                digest          digest to verify

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadSrlModule( FSFile *pFile, u8* dest, u32 offset, u32 size, const u8 digest[SVC_SHA1_DIGEST_SIZE] )
{
    u8 md[SVC_SHA1_DIGEST_SIZE];
    u8* hmacDest = dest;
    u32 hmacSize = size;

    if ( !FS_SeekFile( pFile, (s32)offset, FS_SEEK_SET ) )
    {
        return FALSE;
    }
    while ( size > 0 )
    {
        u32 unit = GetTransferSize( offset, size ); // AES�ΏۂȂ炤�܂������Ȃ�
        if ( !FS_ReadFile( pFile, dest, (s32)unit ) )
        {
            return FALSE;
        }
        dest += unit;
        offset += unit;
        size -= unit;
    }
    SVC_CalcHMACSHA1( md, hmacDest, hmacSize, currentKey, SVC_SHA1_BLOCK_SIZE );
    return CheckDigest(md, (u8*)digest, TRUE, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         FS_OpenSrl

  Description:  open srl file named at HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    pFile   pointer to FSFile streucture

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_OpenSrl( FSFile *pFile )
{
    return FS_OpenFileEx( pFile, (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF, FS_FILEMODE_R );
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadSrlHeader

  Description:  load ROM header to HW_TWL_ROM_HEADER_BUF using normal FS,
                and verify signature

  Arguments:    pFile           pointer to FSFile streucture
                pool            heap context to call SVC_DecryptSign
                rsa_key         public key to verify the signature

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadSrlHeader( FSFile *pFile, SVCSignHeapContext* pool, const void* rsa_key )
{
    u8 md[SVC_SHA1_DIGEST_SIZE];
    SignatureData sd;

    if ( !FS_SeekFile( pFile, 0, FS_SEEK_SET ) )
    {
        return FALSE;
    }
    if ( !FS_ReadFile( pFile, rh, HW_TWL_ROM_HEADER_BUF_SIZE ) )
    {
        return FALSE;
    }
    SVC_CalcSHA1( md, rh, FS_HEADER_AUTH_SIZE );

    // �R���e���c�ؖ���
    if ( CheckRomCertificate( pool, &rh->certificate, rsa_key, *(u32*)rh->s.game_code ) )
    {
        rsa_key = rh->certificate.pubKeyMod;   // �w�b�_�p�̌��̎��o��
    }
    else
    {
        // �Ƃ肠�����R���e���c�ؖ����p�̌������̂܂܎g����Ɖ���
    }

    // �w�b�_�����`�F�b�N
    SVC_DecryptSign( pool, &sd, rh->signature, rsa_key );

    if ( !CheckDigest( md, sd.digest, TRUE, FALSE ) )
    {
        MI_CpuClear8( &sd, sizeof(sd) );    // �c��폜 (���ɕK�v�Ȃ��̂͂Ȃ��H)
        return FALSE;
    }

    // �_�C�W�F�X�g�ȊO�̃f�[�^�̃`�F�b�N���K�v�I�I

    // ���̕ۑ�
    MI_CpuCopy8( (AESKey*)sd.aes_key_seed, &FSiAesKeySeed, sizeof(FSiAesKeySeed) );

    MI_CpuClear8( &sd, sizeof(sd) );    // �c��폜 (���ɕK�v�Ȃ��̂͂Ȃ��H)

    // ROM�w�b�_�̃R�s�[
    MI_CpuCopyFast( rh, (void*)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS_LoadSrlStatic

  Description:  receive static regions from ARM6 via WRAM-B and store them
                specified by ROM header at HW_TWL_ROM_HEADER_BUF

  Arguments:    pFile           pointer to FSFile streucture

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadSrlStatic( FSFile *pFile )
{
    if ( rh->s.main_size > 0 )
    {
        if ( !FS_LoadSrlModule( pFile, rh->s.main_ram_address, rh->s.main_rom_offset, rh->s.main_size, rh->s.main_static_digest ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.sub_size > 0 )
    {
        if ( !FS_LoadSrlModule( pFile, rh->s.sub_ram_address, rh->s.sub_rom_offset, rh->s.sub_size, rh->s.sub_static_digest ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.main_ltd_size > 0 )
    {
        if ( !FS_LoadSrlModule( pFile, rh->s.main_ltd_ram_address, rh->s.main_ltd_rom_offset, rh->s.main_ltd_size, rh->s.main_ltd_static_digest ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.sub_ltd_size > 0 )
    {
        if ( !FS_LoadSrlModule( pFile, rh->s.sub_ltd_ram_address, rh->s.sub_ltd_rom_offset, rh->s.sub_ltd_size, rh->s.sub_ltd_static_digest ) )
        {
            return FALSE;
        }
    }
    return TRUE;
}
