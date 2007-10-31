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

/*
    PROFILE_ENABLE ���`����Ƃ�����x�̃p�t�H�[�}���X�`�F�b�N���ł��܂��B
    ���p���邽�߂ɂ́Amain.c���ǂ����ɁAu32 profile[256]; u32 pf_cnt; ��
    ��`����K�v������܂��B
*/
//#define PROFILE_ENABLE

#ifdef SDK_FINALROM // FINALROM�Ŗ�����
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
    u8 digest[DIGEST_SIZE_SHA1];
    u8 md[DIGEST_SIZE_SHA1];
    int i;
    BOOL result = TRUE;

    // �ؖ����w�b�_�̃}�W�b�N�i���o�[�`�F�b�N
    if( pCert->header.magicNumber != TWL_ROM_CERT_MAGIC_NUMBER ||
        // �ؖ����w�b�_��ROM�w�b�_�̃Q�[���R�[�h��v�`�F�b�N
        pCert->header.gameCode != gameCode )
    {
        result = FALSE;
    }
    // �ؖ��������`�F�b�N
    SVC_DecryptSign( pool, &digest, pCert->sign, pCAPubKey );

    // �_�C�W�F�X�g�̌v�Z
    SVC_CalcSHA1( md, pCert, ROM_CERT_SIGN_OFFSET );

    // ��r
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
  Name:         MIi_LoadBuffer

  Description:  receive data from ARM7 and store(move) via WRAM[B]

                LoadBuffer���J�j�Y���ŁA�t�@�C���̓��e��ARM7����󂯎��܂��B
                ������SVCSHA1Context���w�肵�Ă����ꍇ�A�R�s�[�̂��ł�SHA1��
                �v�Z���s���܂��B

                [LoadBuffer���J�j�Y��]
                WRAM[B]�𗘗p���āAARM7,ARM9�Ԃ̃f�[�^�]�����s���܂��B
                WRAM[B]�̊e�X���b�g���o�P�c�����[�����œn���܂��B
                1�X���b�g���̃f�[�^�܂��͑S�f�[�^���i�[���ꂽ�Ƃ��AARM7����
                FIRM_PXI_ID_LOAD_PIRIOD����M���܂��B
                ARM9�͎�M��ɂ��̃X���b�g�̎g�p����ARM9�ɕύX���ăf�[�^��
                ���o���A������Ƀ��������N���A����(�Z�L�����e�B)�A�g�p����
                ARM7�ɖ߂��܂��B

                [�g�p����]
                WRAM[B]�����b�N�����A������ԂƂ���ARM7���ɓ|���Ă������ƁB

                [���ӓ_]
                offset��size��ARM7����ʒm����܂���B�ʂ̌o�H�œ���������Ă��������B
                SRL�t�@�C����ǂݍ��ޏꍇ�́A�݂���ROM�w�b�_���Q�Ƃł���Ώ\���ł��B
                (ROM�w�b�_�����͌�����m���Ă���͂�)

                �⑫:
                �����ł́A���郉�C�u��������ARM7/ARM9���ŕ��������킹���邱�Ƃ�
                �O��ɂ��Ă��邪�A�ėp�I�ɂ���ɂ�(�Ɨ����C�u����������Ȃ�)�A
                ����M�ŃX���b�g�𔼕����Ƃ��A���ꂼ��Ɏ�M����PXI�R�[���o�b�N
                ���X���b�h��p�ӂ��A���M��API���f�[�^��WRAM�Ɋi�[������A������
                dest��size��ʒm����Ƃ����`��OK�ł͂Ȃ����H
                (�Ŋ���������Ԏ���Ԃ�)

  Arguments:    dest        destination address for received data
                size        size to load
                ctx         context for SHA1 if execute SVC_SHA1Update

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static BOOL MIi_LoadBuffer(u8* dest, u32 size, SVCSHA1Context *ctx)
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
                MI_CpuCopyFast( s, d, u );
                SVC_SHA1Update( ctx, s, u );
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

                SRL��ROM�w�b�_������ARM7����󂯎��A�F�؂��܂��B
                ��M�O�ɁAARM7���� FIRM_PXI_ID_LOAD_HEADER ����M���܂��B
                ��M��A�F�؂��ʂ����Ȃ�ARM7�� FIRM_PXI_ID_AUTH_HEADER �𑗐M
                ���܂��B����ȑO�ɁA���C���������̏���̈ʒu��ROM�w�b�_���i�[
                ����Ă��Ȃ���΂Ȃ�܂���B
                �����āAseed�f�[�^��16�o�C�g���M���܂��B
                makerom.TWL�܂���IPL�̎d�l�Ɉˑ����܂��B

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
         !MIi_LoadBuffer( (u8*)rh, AUTH_SIZE, &ctx ) )
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
    if ( !MIi_LoadBuffer( (u8*)rh + AUTH_SIZE, HEADER_SIZE - AUTH_SIZE, NULL ) )
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
        PXI_NotifyID( FIRM_PXI_ID_AUTH_HEADER );
        PXI_SendDataByFifo( PXI_FIFO_TAG_DATA, sd.aes_key_seed, AES_BLOCK_SIZE );
        // DS�݊��w�b�_�R�s�[
        MI_CpuCopyFast( rh, (void*)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF );
    }
    MI_CpuClear8(&sd, sizeof(sd));
    MI_CpuClear8(&md, sizeof(md));

    return result;
}

/*---------------------------------------------------------------------------*
  Name:         MIi_GetTransferSize

  Description:  get size to transfer once

                ��x�Ɏ�M����T�C�Y��Ԃ��܂��B

                �]���͈͂�AES�̈���܂����ꍇ�́A���E�܂ł̃T�C�Y (�������
                �����ȃT�C�Y) ��Ԃ��܂��B
                makerom.TWL�܂���IPL�̎g�p�Ɉˑ����܂��B

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

                MIi_LoadBuffer�̏��API�ł��B

                AES���E���܂����ꍇ�́A2���LoadBuffer�ɕ������܂��B

                ���łɃn�b�V���l���������Ă��āA���傤��SHA1�̌v�Z�͈͑S�̂�
                �ǂݍ��ޏꍇ�ɕ֗��ł��B

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
        if ( !MIi_LoadBuffer( dest, unit, &ctx.sha1_ctx ) ) // Update��SHA1�Ɠ�������
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

                ARM9/ARM7��Static�����LTD Static����M���܂��B
                ��M�O�ɁAARM7����FIRM_PXI_ID_LOAD_*_STATIC����M���܂��B
                ��M��A�F�؂��ʂ����Ȃ�ARM7��FIRM_PXI_ID_AUTH_*_STATIC�𑗐M
                ���܂��B�T�C�Y��0�̏ꍇ�́A���̃p�[�g��PXI�ʐM����s���܂���B

                ����API���Ăяo���O�ɁA���C���������̏���̈ʒu��ROM�w�b�_��
                �i�[����Ă���K�v������܂��B

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

                ROM�w�b�_�̏��������ɁAOSi_Boot���Ăяo�������ł��B

                ����API���Ăяo���O�ɁA���C���������̏���̈ʒu��ROM�w�b�_��
                �i�[����Ă���K�v������܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_Boot( void )
{
    OSi_Boot( rh->s.main_entry_address, (MIHeader_WramRegs*)rh->s.main_wram_config_data );
}
