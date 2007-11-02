/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - fatfs
  File:     fatfs_loader.c

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

#include <symbols.h>

#include <firm.h>
#include <firm/format/format_rom.h>
#include <rtfs.h>
#include <devices/sdif_reg.h>

/*
    PROFILE_ENABLE ���`����Ƃ�����x�̃p�t�H�[�}���X�`�F�b�N���ł��܂��B
    ���p���邽�߂ɂ́Amain.c���ǂ����ɁAu32 profile[256]; u32 pf_cnt; ��
    ��`����K�v������܂��B
*/
//#define PROFILE_ENABLE

#define MODULE_ALIGNMENT    0x10    // 16�o�C�g�P�ʂœǂݍ���
//#define MODULE_ALIGNMENT  0x200   // 512�o�C�g�P�ʂœǂݍ���
#define RoundUpModuleSize(value)    (((value) + MODULE_ALIGNMENT - 1) & -MODULE_ALIGNMENT)

#ifdef SDK_FINALROM // FINALROM�Ŗ�����
#undef PROFILE_ENABLE
#endif

#ifdef PROFILE_ENABLE
#define PROFILE_PXI_SEND    0x10000000
#define PROFILE_PXI_RECV    0x20000000
extern u32 profile[];
extern u32 pf_cnt;
#endif


#define PXI_FIFO_TAG_DATA   PXI_FIFO_TAG_USER_0

static ROM_Header* const rh= (ROM_Header*)HW_TWL_ROM_HEADER_BUF;
static int menu_fd = -1;

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenRecentMenu

  Description:  open recent menu file
                �V�X�e�����j���[�̃t�@�C������肵�A�I�[�v�����A�t�@�C��ID��
                menu_fd�ɃZ�b�g���܂��B
                �ŏI�I�ɂ́A�Œ�̃^�C�g�����^�f�[�^��ǂݍ��݁AeTicket�̏���
                �����āA�V�X�e�����j���[�̃t�@�C������肷�邱�ƂɂȂ�\��B

  Arguments:    driveno     drive number ('A' is 0)

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenRecentMenu( int driveno )
{
    char *menufile = (char*)L"A:\\ipl\\menu.srl";
    if (driveno < 0 || driveno >= 26)
    {
        return FALSE;
    }
    menufile[0] += (char)driveno;
    menu_fd = po_open((u8*)menufile, PO_BINARY, 0);
    if (menu_fd < 0)
    {
        return FALSE;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenSpecifiedSrl

  Description:  open specified menu file

                �C�ӂ̃t�@�C�����I�[�v�����A�t�@�C��ID��menu_fd�ɃZ�b�g���܂��B

  Arguments:    menufile    target filename

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenSpecifiedSrl( const char* menufile )
{
    menu_fd = po_open((u8*)menufile, PO_BINARY, 0);
    if (menu_fd < 0)
    {
        return FALSE;
    }
    return TRUE;
}

#define HEADER_SIZE 0x1000
#define AUTH_SIZE   ROM_HEADER_SIGN_TARGET_SIZE

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadBuffer

  Description:  load data and pass to ARM9 via WRAM[B]

                LoadBuffer���J�j�Y���ŁAFAT���̃t�@�C���̓��e��ARM9�ɓ]�����܂��B

                [LoadBuffer���J�j�Y��]
                WRAM[B]�𗘗p���āAARM7,ARM9�Ԃ̃f�[�^�]�����s���܂��B
                WRAM[B]�̊e�X���b�g���o�P�c�����[�����œn���܂��B
                1�X���b�g���̃f�[�^�܂��͑S�f�[�^���i�[�ł����Ƃ��AARM9��
                FIRM_PXI_ID_LOAD_PIRIOD�𑗐M���܂��B
                �f�[�^�c������ꍇ�͎��̃X���b�g�̏����Ɉڂ�܂��B
                2��ڈȍ~�̌Ăяo���ł́A�O��Ō�̃X���b�g�̑�������g�p���܂��B
                �g�p�������X���b�g��ARM9���Ɋ��蓖�Ă��Ă���Ƃ��́AARM7����
                �Ȃ�܂ŃX�g�[�����܂��B

                [�g�p����]
                WRAM[B]�����b�N�����A������ԂƂ���ARM7���ɓ|���Ă������ƁB

                [���ӓ_]
                offset��size��ARM9�ɒʒm����܂���B�ʂ̌o�H�œ���������Ă��������B
                SRL�t�@�C����ǂݍ��ޏꍇ�́A�݂���ROM�w�b�_���Q�Ƃł���Ώ\���ł��B
                (ROM�w�b�_�����͌�����m���Ă���͂�)

                �⑫:
                �����ł́A���郉�C�u��������ARM7/ARM9���ŕ��������킹���邱�Ƃ�
                �O��ɂ��Ă��邪�A�ėp�I�ɂ���ɂ�(�Ɨ����C�u����������Ȃ�)�A
                ����M�ŃX���b�g�𔼕����Ƃ��A���ꂼ��Ɏ�M����PXI�R�[���o�b�N
                ���X���b�h��p�ӂ��A���M��API���f�[�^��WRAM�Ɋi�[������A������
                dest��size��ʒm����Ƃ����`��OK�ł͂Ȃ����H
                (�Ŋ���������Ԏ���Ԃ�)

  Arguments:    offset      offset of the file to load (512 bytes alignment)
                size        size to load

  Returns:      None
 *---------------------------------------------------------------------------*/
static BOOL FATFS_LoadBuffer(u32 offset, u32 size)
{
    u8* base = (u8*)HW_FIRM_LOAD_BUFFER_BASE;
    static int count = 0;

    // seek first
//    OS_TPrintf("po_lseek(offset=%X);\n", offset);
    if (po_lseek(menu_fd, (s32)offset, PSEEK_SET) < 0)
    {
        return FALSE;
    }
#ifdef PROFILE_ENABLE
    // x2: after Seek
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
    // loading loop
    while (size > 0)
    {
        u8* dest = base + count * HW_FIRM_LOAD_BUFFER_UNIT_SIZE;    // target buffer address
        u32 unit = size < HW_FIRM_LOAD_BUFFER_UNIT_SIZE ? size : HW_FIRM_LOAD_BUFFER_UNIT_SIZE; // size
        while (MI_GetWramBankMaster_B(count) != MI_WRAM_ARM7)       // waiting to be master
        {
        }
#ifdef PROFILE_ENABLE
        // x3...: after to wait ARM9
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
//        OS_TPrintf("po_read(dest=%X, unit=%X);\n", dest, unit);
        if (po_read(menu_fd, (u8*)dest, (int)unit) < 0)            // reading
        {
            return FALSE;
        }
#ifdef PROFILE_ENABLE
        // x4...: before PXI
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
        profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_PIRIOD;    // checkpoint
#endif
        PXI_NotifyID( FIRM_PXI_ID_LOAD_PIRIOD );
        count = (count + 1) % HW_FIRM_LOAD_BUFFER_UNIT_NUMS;
        size -= unit;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadHeader

  Description:  load header

                SRL��ROM�w�b�_������ǂݍ��݁AARM9�ɓn���܂��B
                ���M�O�ɁAARM9�� FIRM_PXI_ID_LOAD_HEADER �𑗐M���܂��B
                ���M��AARM9���� FIRM_PXI_ID_AUTH_HEADER ����M���܂��B
                ���̎��_�ŁA���C���������̏���̈ʒu��ROM�w�b�_���i�[���ꂽ��
                �z�肵�܂��B
                ���Ȃ���΁Aseed�f�[�^��16�o�C�g��M���܂��B
                �󂯎����seed��SeedA��KeyC�ɐݒ肳��܂��B
                makerom.TWL�܂���IPL�̎d�l�Ɉˑ����܂��B

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadHeader( void )
{
    // open the file in FATFS_InitFIRM()
    if (menu_fd < 0)
    {
        return FALSE;
    }

#ifdef PROFILE_ENABLE
    // 10: before PXI
    pf_cnt = 0x10;
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
    profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_HEADER;    // checkpoint
#endif
    // load header without AES
    PXI_NotifyID( FIRM_PXI_ID_LOAD_HEADER );
    FATFS_DisableAES();
    if (!FATFS_LoadBuffer(0, AUTH_SIZE) ||
#ifdef PROFILE_ENABLE
        // 12: after to load half
        ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
        !FATFS_LoadBuffer(AUTH_SIZE, HEADER_SIZE - AUTH_SIZE) ||
#ifdef PROFILE_ENABLE
        // 1x: after to load remain
        ((profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick())), FALSE) ||
#endif
        PXI_RecvID() != FIRM_PXI_ID_AUTH_HEADER )
    {
        return FALSE;
    }
#ifdef PROFILE_ENABLE
    // 1x: after PXI
    profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_HEADER;    // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    // set id depends on game_code and seed to use (or all?)
    {
        AESKeySeed seed;
        AESi_InitGameKeys((u8*)rh->s.game_code);
        PXI_RecvDataByFifo( PXI_FIFO_TAG_DATA, &seed, AES_BLOCK_SIZE );
        AESi_WaitKey();
        AESi_SetKeySeedA(&seed);    // APP
        //AESi_WaitKey();
        //AESi_SetKeySeedB(&seed);    // APP & HARD
        //AESi_WaitKey();
        //AESi_SetKeySeedC(&seed);    //
        //AESi_WaitKey();
        //AESi_SetKeySeedD(&seed);    // HARD
        AESi_WaitKey();
        AESi_SetKeyC(&seed);        // Direct
    }

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFSi_GetCounter

  Description:  get counter

                offset�ɑΉ�����AES�̃J�E���^�l���v�Z���܂��B
                makerom.TWL���̃R�[�h�Ɉˑ����܂��B

  Arguments:    offset  offset from head of ROM_Header

  Returns:      counter
 *---------------------------------------------------------------------------*/
static AESCounter* FATFSi_GetCounter( u32 offset )
{
    static AESCounter counter;

    MI_CpuCopy8( rh->s.main_static_digest, &counter, 16 );
    AESi_AddCounter( &counter, offset - rh->s.aes_target_rom_offset );
    return &counter;
}

/*---------------------------------------------------------------------------*
  Name:         FATFSi_SetupAES

  Description:  setup whiere to use AES

                AES�Í������ꂽ�f�[�^��ǂݍ��ނ��߂̃Z�b�g�A�b�v���s���܂��B
                fatfs_sdmc.c�̃h���C�o���g�p���Ă��邱�Ƃ������ƂȂ�܂��B
                (TwlSDK�W���ōs���ꍇ�́A���̎d�l�ɍ��킹�ďC�����K�v�I)

                ����API���Ăяo���O�ɁA���C���������̏���̈ʒu��ROM�w�b�_��
                �i�[����Ă���K�v������܂��B

                ���̑I�����s���Ă��܂����A���̐ݒ�͕ʂ̏ꏊ�ōs���Ă���
                �K�v������܂��B

                �]���͈͂�AES�̈���܂����ꍇ�́A���E�܂ł̃T�C�Y (�������
                �����ȃT�C�Y) ��Ԃ��܂��B
                makerom.TWL�܂���IPL�̎g�p�Ɉˑ����܂��B

  Arguments:    offset  offset of region from head of ROM_Header
                size    size of region

  Returns:      size to transfer once
 *---------------------------------------------------------------------------*/
static u32 FATFSi_SetupAES( u32 offset, u32 size )
{
    u32 aes_offset = rh->s.aes_target_rom_offset;
    u32 aes_end = aes_offset + RoundUpModuleSize(rh->s.aes_target_size);
    u32 end = offset + RoundUpModuleSize(size);
    if ( rh->s.enable_aes )
    {
        if ( offset >= aes_offset && offset < aes_end )
        {
            if ( end > aes_end )
            {
                size = aes_end - offset;
            }
            AESi_WaitKey();
            if (rh->s.developer_encrypt)
            {
                AESi_LoadKey( AES_KEY_SLOT_C );
            }
            else
            {
                AESi_LoadKey( AES_KEY_SLOT_A );
            }
            FATFS_EnableAES( FATFSi_GetCounter( offset ) );
        }
        else
        {
            if ( offset < aes_offset && offset + size > aes_offset )
            {
                size = aes_offset - offset;
            }
            FATFS_DisableAES();
        }
    }
    else
    {
        FATFS_DisableAES();
    }
    return size;
}

/*---------------------------------------------------------------------------*
  Name:         FATFSi_LoadModule

  Description:  transfer module to ARM9 via WRAM[B]

                FATFSi_LoadBuffer�̏��API�ł��B

                AES���E���܂����Ƃ���2��ɕ����邾���ł��B

  Arguments:    offset      offset from head of ROM_Header
                size        size to load

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
static /*inline*/ BOOL FATFSi_LoadModule(u32 offset, u32 size)
{
    size = RoundUpModuleSize( size );   // �A���C�������g����
    while ( size > 0 )
    {
        u32 unit = FATFSi_SetupAES( offset, size ); // ��x�̓]���T�C�Y
        if ( !FATFS_LoadBuffer( offset, unit ) )
        {
            return FALSE;
        }
        offset += unit;
        size -= unit;
    }
    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadStatic

  Description:  load static binary

                ARM9/ARM7��Static�����LTD Static��ǂݍ��݂܂��B
                ���M�O�ɁAARM9��FIRM_PXI_ID_LOAD_*_STATIC�𑗐M���܂��B
                ���M��́AARM9����FIRM_PXI_ID_AUTH_*_STATIC����M���܂��B
                �T�C�Y��0�̏ꍇ�́A���̃p�[�g��PXI�ʐM����s���܂���B

                ����API���Ăяo���O�ɁA���C���������̏���̈ʒu��ROM�w�b�_��
                �i�[����Ă���K�v������܂��B

                ARM9���ƈقȂ�A�f�o�C�X�ˑ��̃A���C�����g�C�����s���Ă��܂��B
                (�T�C�Y�̂�)

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadStatic( void )
{
#ifdef PROFILE_ENABLE
    // 30: LoadStatic
    pf_cnt = 0x30;
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
    profile[pf_cnt++] = (u32)PROFILE_PXI_SEND | FIRM_PXI_ID_LOAD_STATIC;    // checkpoint
#endif
    PXI_NotifyID( FIRM_PXI_ID_LOAD_STATIC );

    // load ARM9 static region without AES
    if ( rh->s.main_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 31: before PXI
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !FATFSi_LoadModule( rh->s.main_rom_offset, rh->s.main_size ) )
        {
            return FALSE;
        }
    }
    // load ARM7 static region without AES
    if ( rh->s.sub_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 50: before PXI
        pf_cnt = 0x50;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !FATFSi_LoadModule( rh->s.sub_rom_offset, rh->s.sub_size ) )
        {
            return FALSE;
        }
    }
    // load ARM9 extended static region with AES
    if ( rh->s.main_ltd_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 70: before PXI
        pf_cnt = 0x70;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !FATFSi_LoadModule( rh->s.main_ltd_rom_offset, rh->s.main_ltd_size ) )
        {
            return FALSE;
        }
    }
    // load ARM7 extended static region with AES
    if ( rh->s.sub_ltd_size > 0 )
    {
#ifdef PROFILE_ENABLE
        // 90: before PXI
        pf_cnt = 0x90;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        if ( !FATFSi_LoadModule( rh->s.sub_ltd_rom_offset, rh->s.sub_ltd_size ) )
        {
            return FALSE;
        }
    }

    // waiting result
    if ( PXI_RecvID() != FIRM_PXI_ID_AUTH_STATIC )
    {
        return FALSE;
    }
#ifdef PROFILE_ENABLE
    // 9x: after PXI
    profile[pf_cnt++] = (u32)PROFILE_PXI_RECV | FIRM_PXI_ID_AUTH_STATIC;    // checkpoint
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    return TRUE;
}

/*---------------------------------------------------------------------------*
  Name:         FATFS_Boot

  Description:  boot

                ROM�w�b�_�̏��������ɁAOSi_Boot���Ăяo�������ł��B

                ����API���Ăяo���O�ɁA���C���������̏���̈ʒu��ROM�w�b�_��
                �i�[����Ă���K�v������܂��B

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_Boot( void )
{
    OSi_Boot( rh->s.sub_entry_address, (MIHeader_WramRegs*)rh->s.main_wram_config_data );
}
