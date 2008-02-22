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
#include <twl/aes.h>
#include <twl/aes_private.h>

#define FS_HEADER_AUTH_SIZE 0xe00

#define MODULE_ALIGNMENT    0x10    // 16�o�C�g�P�ʂœǂݍ���
//#define MODULE_ALIGNMENT  0x200   // 512�o�C�g�P�ʂœǂݍ���
#define RoundUpModuleSize(value)    (((value) + MODULE_ALIGNMENT - 1) & -MODULE_ALIGNMENT)

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

static BOOL         aesFlag;
static AESCounter   aesCounter;
static u8* const    aesBuffer = (u8*)HW_FIRM_FS_AES_BUFFER; // 0x2ff3800


/*---------------------------------------------------------------------------*
  Name:         FS2_SetDigestKey

  Description:  set specified key or default key for HMAC-SHA-1

  Arguments:    digestKey       pointer to key
                                if NULL, use default key

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static inline void FS2_SetDigestKey( const u8* digestKey )
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

static void AesCallback(AESResult result, void* arg)
{
    volatile BOOL *pBusy = (BOOL*)arg;
    *pBusy = FALSE;
    if (result != AES_RESULT_SUCCESS)
    {
        OS_TPrintf("Failed to decrypt by AES (%d)\n", result);
    }
}

static void CopyWithAes( const void* src, void* dest, u32 size )
{
    volatile BOOL aesBusy = TRUE;

    AES_SetKeySlot( AES_KEY_SLOT_A );
    aesBusy = TRUE;
    if ( AES_RESULT_SUCCESS == AES_Ctr( &aesCounter, src, size, dest, AesCallback, (void*)&aesBusy ) )
    {
        while ( aesBusy )
        {
        }
    }
    AES_AddToCounter( &aesCounter, size / AES_BLOCK_SIZE );
}

static void EnableAes( u32 offset )
{
    aesFlag = TRUE;
    MI_CpuCopy8( rh->s.main_static_digest, &aesCounter, AES_BLOCK_SIZE );
    AES_AddToCounter( &aesCounter, (offset - rh->s.aes_target_rom_offset) / AES_BLOCK_SIZE );
}
static void DisableAes( void )
{
    aesFlag = FALSE;
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
    if ( rh->s.enable_aes )
    {
        u32 end = offset + RoundUpModuleSize(size);
        u32 aes_offset = rh->s.aes_target_rom_offset;
        u32 aes_end = aes_offset + RoundUpModuleSize(rh->s.aes_target_size);
        u32 aes_offset2 = rh->s.aes_target2_rom_offset;
        u32 aes_end2 = aes_offset2 + RoundUpModuleSize(rh->s.aes_target2_size);
        if ( offset >= aes_offset && offset < aes_end )
        {
            if ( end > aes_end )
            {
                size = aes_end - offset;
            }
            if ( size > HW_FIRM_FS_AES_BUFFER_SIZE )
            {
                size = HW_FIRM_FS_AES_BUFFER_SIZE;
            }
            EnableAes( offset );
        }
        else if ( offset >= aes_offset2 && offset < aes_end2 )
        {
            if ( end > aes_end2 )
            {
                size = aes_end2 - offset;
            }
            if ( size > HW_FIRM_FS_AES_BUFFER_SIZE )
            {
                size = HW_FIRM_FS_AES_BUFFER_SIZE;
            }
            EnableAes( offset );
        }
        else
        {
            if ( offset < aes_offset && offset + size > aes_offset )
            {
                size = aes_offset - offset;
            }
            DisableAes();
        }
    }
    else
    {
        DisableAes();
    }
    return size;
}

/*---------------------------------------------------------------------------*
  Name:         FS2_LoadModule

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
BOOL FS2_LoadModule( FSFile *pFile, u8* dest, u32 offset, u32 size, const u8 digest[SVC_SHA1_DIGEST_SIZE] )
{
    u8 md[SVC_SHA1_DIGEST_SIZE];
    u8* hmacDest = dest;
    u32 hmacSize = size;

    if ( !FS_SeekFile( pFile, (s32)offset, FS_SEEK_SET ) )
    {
        return FALSE;
    }
    size = RoundUpModuleSize( size );
    while ( size > 0 )
    {
        u32 unit = GetTransferSize( offset, size );
        if ( aesFlag )
        {
            if ( !FS_ReadFile( pFile, aesBuffer, (s32)unit ) )
            {
                return FALSE;
            }
            DC_FlushRange( aesBuffer, unit );
            CopyWithAes( aesBuffer, dest, unit );
        }
        else
        {
            if ( !FS_ReadFile( pFile, dest, (s32)unit ) )
            {
                return FALSE;
            }
        }
        dest += unit;
        offset += unit;
        size -= unit;
    }
    SVC_CalcHMACSHA1( md, hmacDest, hmacSize, currentKey, SVC_SHA1_BLOCK_SIZE );
    return CheckDigest(md, (u8*)digest, TRUE, FALSE);
}

/*---------------------------------------------------------------------------*
  Name:         FS2_OpenSrl

  Description:  open srl file named at HW_TWL_FS_BOOT_SRL_PATH_BUF

  Arguments:    pFile   pointer to FSFile streucture

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS2_OpenSrl( FSFile *pFile )
{
    return FS_OpenFileEx( pFile, (char*)HW_TWL_FS_BOOT_SRL_PATH_BUF, FS_FILEMODE_R );
}

/*---------------------------------------------------------------------------*
  Name:         FS2_LoadHeader

  Description:  load ROM header to HW_TWL_ROM_HEADER_BUF using normal FS,
                and verify signature

  Arguments:    pFile           pointer to FSFile streucture
                pool            heap context to call SVC_DecryptSign
                rsa_key1        public key to verify the signature
                rsa_key2        public key to verify the signature
                                for system applications

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS2_LoadHeader( FSFile *pFile, SVCSignHeapContext* pool, const void* rsa_key1, const void* rsa_key2 )
{
    const void* rsa_key;
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

    // ���̊m��
    rsa_key = (rh->s.titleID_Hi & 0x1) ? rsa_key2 : rsa_key1;

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

    MI_CpuClear8( &sd, sizeof(sd) );    // �c��폜 (���ɕK�v�Ȃ��̂͂Ȃ��H)

    // ROM�w�b�_�̃R�s�[
    MI_CpuCopyFast( rh, (void*)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF );
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FS2_LoadStatic

  Description:  receive static regions from ARM6 via WRAM-B and store them
                specified by ROM header at HW_TWL_ROM_HEADER_BUF

  Arguments:    pFile           pointer to FSFile streucture
                digestKey       pointer to key for HMAC-SHA1
                                if NULL, use default key

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS2_LoadStatic( FSFile *pFile, const u8* digestKey )
{
    FS2_SetDigestKey( digestKey );
    if ( rh->s.main_size > 0 )
    {
        if ( !FS2_LoadModule( pFile, rh->s.main_ram_address, rh->s.main_rom_offset, rh->s.main_size, rh->s.main_static_digest ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.sub_size > 0 )
    {
        if ( !FS2_LoadModule( pFile, rh->s.sub_ram_address, rh->s.sub_rom_offset, rh->s.sub_size, rh->s.sub_static_digest ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.main_ltd_size > 0 )
    {
        if ( !FS2_LoadModule( pFile, rh->s.main_ltd_ram_address, rh->s.main_ltd_rom_offset, rh->s.main_ltd_size, rh->s.main_ltd_static_digest ) )
        {
            return FALSE;
        }
    }
    if ( rh->s.sub_ltd_size > 0 )
    {
        if ( !FS2_LoadModule( pFile, rh->s.sub_ltd_ram_address, rh->s.sub_ltd_rom_offset, rh->s.sub_ltd_size, rh->s.sub_ltd_static_digest ) )
        {
            return FALSE;
        }
    }
    return TRUE;
}
