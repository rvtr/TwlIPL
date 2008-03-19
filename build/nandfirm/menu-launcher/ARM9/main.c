/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - menu-launcher
  File:     main.c

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
#include <twl/lcfg.h>

#ifndef FIRM_USE_TWLSDK_KEYS
#define RSA_KEY_ADDR    OSi_GetFromFirmAddr()->rsa_pubkey[0]    // ���Ǘ�.xls�Q��
#else
#define RSA_KEY_ADDR    rsa_key
static const u8 rsa_key_launcher[128] =
{
        0xa7, 0x9f, 0x54, 0xa0, 0xc7, 0x45, 0xae, 0xf6, 0x63, 0xa7, 0x53, 0xb7, 0x0a, 0xcc, 0x0b, 0xcb,
        0x65, 0xe1, 0x11, 0xc6, 0x05, 0x15, 0xb5, 0x6e, 0xbd, 0xac, 0x0c, 0xca, 0xf4, 0x7c, 0x68, 0x7a,
        0xf9, 0x0e, 0x5d, 0x98, 0x5b, 0xc8, 0x4d, 0x22, 0x3b, 0xa3, 0xbe, 0x8b, 0x5b, 0x7f, 0x26, 0x44,
        0x9f, 0xc4, 0x48, 0x44, 0xb1, 0x32, 0xb7, 0xbe, 0x63, 0xba, 0xd6, 0xc1, 0x10, 0xce, 0xf6, 0xed,
        0x47, 0x8f, 0xe1, 0xff, 0x7f, 0x5a, 0xd5, 0x5d, 0x94, 0x38, 0x2f, 0xa1, 0xd4, 0xef, 0x82, 0xb1,
        0x0d, 0xc4, 0x43, 0xec, 0xbe, 0x77, 0xb6, 0x82, 0x9c, 0xfa, 0x17, 0x87, 0x84, 0x82, 0x25, 0x46,
        0xfb, 0xd6, 0x05, 0xc8, 0x9a, 0x7e, 0xad, 0x44, 0x40, 0x0d, 0x35, 0x9c, 0x45, 0x44, 0x64, 0x36,
        0x61, 0x4b, 0xf7, 0xe6, 0x31, 0x5c, 0x7d, 0x96, 0x73, 0xe8, 0xac, 0xb4, 0xe3, 0x5e, 0xd1, 0x9d,
};
#endif

#define RSA_HEAP_SIZE   (4*1024)    // RSA�p�q�[�v�T�C�Y (�T�C�Y�����K�v)

static u8 acHeap[RSA_HEAP_SIZE] __attribute__ ((aligned (32)));
static SVCSignHeapContext acPool;

#define MENU_TITLE_ID_HI    0x00030017ULL
#define MENU_TITLE_ID_LO    0x4c4e4352ULL
#define MENU_TITLE_ID       (MENU_TITLE_ID_HI << 32 | MENU_TITLE_ID_LO)

/*
    PROFILE_ENABLE ���`����Ƃ�����x�̃p�t�H�[�}���X�`�F�b�N���ł��܂��B
    ���p���邽�߂ɂ́Amain.c���ǂ����ɁAu32 profile[256]; u32 pf_cnt = 0; ��
    ��`����K�v������܂��B
*/
#define PROFILE_ENABLE

/*
    PRINT_MEMORY_ADDR ���`����ƁA���̃A�h���X����SPrintf���s���܂�(���̃t�@�C���̂�)
    FINALROM�łł��R�[�h���c��̂Œ��ӂ��Ă��������B
*/
//#define PRINT_MEMORY_ADDR       0x02FFC200

//#ifdef SDK_FINALROM // FINALROM�Ŗ�����
//#undef PROFILE_ENABLE
//#endif

#ifdef PROFILE_ENABLE
#define PROFILE_MAX  16
u32 profile[PROFILE_MAX];
u32 pf_cnt = 0;
#define PUSH_PROFILE()  (profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick()))
#else
#define PUSH_PROFILE()  ((void)0)
#endif

#ifdef PRINT_MEMORY_ADDR
static char* debugPtr = (char*)PRINT_MEMORY_ADDR;
#undef OS_TPrintf
//#define OS_TPrintf(...) (debugPtr = (char*)((u32)(debugPtr + STD_TSPrintf(debugPtr, __VA_ARGS__) + 0xf) & ~0xf))
#define OS_TPrintf(...) (debugPtr += STD_TSPrintf(debugPtr, __VA_ARGS__))
#endif

/***************************************************************
    PreInit

    FromBoot�̑Ή���OS_Init�O�ɕK�v�ȃ��C���������̏�����
***************************************************************/
static void PreInit(void)
{
    ROM_Header_Short* const rhs = (ROM_Header_Short*)HW_TWL_ROM_HEADER_BUF;
    /*
     ���C���������֘A
    */
    // SHARED�̈�̓X�^�[�g�A�b�v���ŃN���A

    /*
        FromBrom�֘A
    */
    if ( !OSi_FromBromToMenu() )
    {
        OS_Terminate();
    }

    // �u�[�g�^�C�v�̕ύX
    ( (OSBootInfo *)OS_GetBootInfo() )->boot_type = OS_BOOTTYPE_NAND;
}

/***************************************************************
    PostInit

    �e�평����
***************************************************************/
static void PostInit(void)
{
    // RSA�p�q�[�v�ݒ�
    SVC_InitSignHeap( &acPool, acHeap, sizeof(acHeap) );
    // FS/FATFS������
    FS_InitFIRM();
}

/***************************************************************
    TryResolveSrl

    NAND�Ɋi�[���ꂽ��񂩂烉���`���[SRL����������
***************************************************************/
static BOOL TryResolveSrl(void)
{
    OSTitleId titleId = MENU_TITLE_ID_HI << 32;
    if ( !LCFG_ReadHWSecureInfo() )
    {
        OS_TPrintf("Failed to load HWSecureInfo.\n");
        return FALSE;
    }
    LCFG_THW_GetLauncherTitleID_Lo( (u8*)&titleId );
    // 4: after LCFG_ReadHWSecureInfo
    PUSH_PROFILE();

    if ( !FS_ResolveSrl( titleId ) )
    {
        OS_TPrintf("Failed to call FS_ResolveSrl( 0x%016llx ).\n", titleId);
        return FALSE;
    }
    OS_TPrintf("Launcher Title ID: 0x%016llx\n", titleId);
    return TRUE;
}
/***************************************************************
    RetryResolveSrl

    �f�t�H���g�ݒ肩�烉���`���[SRL����������
***************************************************************/
static BOOL RetryResolveSrl(void)
{
    if ( !FS_ResolveSrl( MENU_TITLE_ID ) )
    {
        OS_TPrintf("Failed to call FS_ResolveSrl( 0x%016llx ).\n", MENU_TITLE_ID);
        return FALSE;
    }
    OS_TPrintf("Launcher Title ID: 0x%016llx\n", MENU_TITLE_ID);
    return TRUE;
}

/***************************************************************
    CheckHeader

    �w�b�_���V�X�e�����j���[�Ƃ��Ė��Ȃ����`�F�b�N
***************************************************************/
static BOOL CheckHeader(void)
{
    ROM_Header_Short* const rhs = (ROM_Header_Short*)HW_TWL_ROM_HEADER_BUF;
    // �C�j�V�����R�[�h�Ȃ�
    OS_TPrintf("Initial Code        : %08X (%.4s)\n", *(u32*)rhs->game_code, rhs->game_code);
    OS_TPrintf("Platform Code       : %02X\n", rhs->platform_code);
    OS_TPrintf("Codec Mode          : %s\n", rhs->codec_mode ? "TWL" : "NITRO");
    OS_TPrintf("Sigunature          : %s\n", rhs->enable_signature ? "AVAILABLE" : "NOT AVAILABLE");
    OS_TPrintf("AES Encryption      : %s\n", rhs->enable_aes ? "AVAILABLE" : "NOT AVAILABLE");
    if ( rhs->enable_aes )
    {
        OS_TPrintf("AES Key Type        : %s\n", rhs->developer_encrypt ? "FOR DEVELOPMENT" : "FOR PRODUCT");
    }
    // �G���g���|�C���g
    OS_TPrintf("ARM9 Entry point    : %08X\n", rhs->main_entry_address);
    OS_TPrintf("ARM7 Entry point    : %08X\n", rhs->sub_entry_address);
    // ���[�h�͈�
    OS_TPrintf("ARM9 ROM address    : %08X\n", rhs->main_rom_offset);
    OS_TPrintf("ARM9 RAM address    : %08X\n", rhs->main_ram_address);
    OS_TPrintf("ARM9 size           : %08X\n", rhs->main_size);
    OS_TPrintf("ARM7 ROM address    : %08X\n", rhs->sub_rom_offset);
    OS_TPrintf("ARM7 RAM address    : %08X\n", rhs->sub_ram_address);
    OS_TPrintf("ARM7 size           : %08X\n", rhs->sub_size);
    OS_TPrintf("ARM9 LTD ROM address: %08X\n", rhs->main_ltd_rom_offset);
    OS_TPrintf("ARM9 LTD RAM address: %08X\n", rhs->main_ltd_ram_address);
    OS_TPrintf("ARM9 LTD size       : %08X\n", rhs->main_ltd_size);
    OS_TPrintf("ARM7 LTD ROM address: %08X\n", rhs->sub_ltd_rom_offset);
    OS_TPrintf("ARM7 LTD RAM address: %08X\n", rhs->sub_ltd_ram_address);
    OS_TPrintf("ARM7 LTD size       : %08X\n", rhs->sub_ltd_size);
    // �����قڍœK���ς�
    if ( rhs->platform_code != PLATFORM_CODE_TWL_LIMITED ||     // TWL Limited only
         !rhs->codec_mode ||                                    // TWL mode only
         !rhs->enable_signature ||                              // Should be use ROM header signature
         (rhs->titleID_Hi & 0x0005) != 0x0005 ||                // check only NAND/SYSTEM bits (need?)
        // should be in main memory
         HW_TWL_MAIN_MEM > (u32)rhs->main_ram_address ||
         HW_TWL_MAIN_MEM > (u32)rhs->sub_ram_address ||
         HW_TWL_MAIN_MEM > (u32)rhs->main_ltd_ram_address ||
         HW_TWL_MAIN_MEM > (u32)rhs->sub_ltd_ram_address ||
        // should be in static area without Limited region
         (u32)rhs->main_ram_address > (u32)rhs->main_entry_address ||
         (u32)rhs->sub_ram_address > (u32)rhs->sub_entry_address ||
        // should be in main memory (end address)
         HW_FIRM_FATFS_COMMAND_BUFFER <= (u32)rhs->main_ram_address + rhs->main_size ||
         HW_FIRM_FATFS_COMMAND_BUFFER <= (u32)rhs->sub_ram_address + rhs->sub_size ||
         HW_FIRM_FATFS_COMMAND_BUFFER <= (u32)rhs->main_ltd_ram_address + rhs->main_ltd_size ||
         HW_FIRM_FATFS_COMMAND_BUFFER <= (u32)rhs->sub_ltd_ram_address + rhs->sub_ltd_size ||
        // should be in static area without Limited region (end address)
         (u32)rhs->main_ram_address + rhs->main_size <= (u32)rhs->main_entry_address ||
         (u32)rhs->sub_ram_address + rhs->sub_size <= (u32)rhs->sub_entry_address ||
         0 )
    {
        OS_TPrintf("Invalid ROM header for MENU Launcher!\n");
        return FALSE;
    }
    return TRUE;
}

/***************************************************************
    EraseAll

    �s���I�����܂���
    ���낢������Ă�������
    DS���[�h�ɂ��ďI���̂��悢���H
***************************************************************/
static void EraseAll(void)
{
    MI_CpuClearFast( OSi_GetFromFirmAddr(), sizeof(OSFromFirmBuf) );
#ifdef SDK_FINALROM
    MI_CpuClearFast( (void*)HW_TWL_ROM_HEADER_BUF, HW_TWL_ROM_HEADER_BUF_SIZE );
    MI_CpuClearFast( (void*)HW_ROM_HEADER_BUF, HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF );
    OS_BootFromFIRM();
#endif
}

void TwlMain( void )
{
#ifdef PROFILE_ENABLE
    // 0: bootrom
    profile[pf_cnt++] = OS_TicksToMicroSecondsBROM32(OS_GetTick());
#endif

    PreInit();
#ifdef PROFILE_ENABLE
    // 1: before OS_InitFIRM
    profile[pf_cnt++] = OS_TicksToMicroSecondsBROM32(OS_GetTick());
#endif
    OS_InitFIRM();
    OS_EnableIrq();
    OS_EnableInterrupts();
#ifdef PROFILE_ENABLE
    // 2: before OS_InitTick
    profile[pf_cnt++] = OS_TicksToMicroSecondsBROM32(OS_GetTick());

    OS_InitTick();
#endif

    PostInit();
    // 3: after PostInit
    PUSH_PROFILE();

    if ( !TryResolveSrl() && !RetryResolveSrl() )
    {
        goto end;
    }
    // 5: after FS_ResolveSrl
    PUSH_PROFILE();

    PXI_NotifyID( FIRM_PXI_ID_SET_PATH );
    // 6: after PXI
    PUSH_PROFILE();

    if ( !FS_LoadHeader( &acPool, NULL, NULL, rsa_key_launcher ) || !CheckHeader() )
    {
        OS_TPrintf("Failed to call FS_LoadHeader() and/or CheckHeader().\n");
        goto end;
    }
    // 7: after FS_LoadHeader
    PUSH_PROFILE();

    PXI_NotifyID( FIRM_PXI_ID_DONE_HEADER );
    // 8: after PXI
    PUSH_PROFILE();

    if ( !FS_LoadStatic( NULL ) )
    {
        OS_TPrintf("Failed to call FS_LoadStatic().\n");
        goto end;
    }
    // 9: after FS_LoadStatic
    PUSH_PROFILE();

    PXI_NotifyID( FIRM_PXI_ID_DONE_STATIC );
    // 10: after PXI
    PUSH_PROFILE();

#ifdef PROFILE_ENABLE
    {
        int i;
        OS_TPrintf("\n[ARM9] Begin\n");
        for (i = 0; i < PROFILE_MAX; i++)
        {
//            OS_TPrintf("0x%08X\n", profile[i]);
            if ( !profile[i] ) break;
            OS_TPrintf("%2d: %7d usec", i, profile[i]);
            if (i)
            {
                OS_TPrintf(" ( %7d usec )\n", profile[i]-profile[i-1]);
            }
            else
            {
                OS_TPrintf("\n");
            }
        }
        OS_TPrintf("\n[ARM9] End\n");
        PXI_NotifyID( FIRM_PXI_ID_NULL );
        reg_OS_TM3CNT_H = 0;
        reg_OS_TM3CNT_L = 0;
        reg_OS_TM3CNT_H = (u16)(REG_OS_TM0CNT_H_E_MASK | OS_TIMER_PRESCALER_1024);
    }
#endif

    OS_BootFromFIRM();

end:
    EraseAll();

    // failed
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    OS_Terminate();
}

