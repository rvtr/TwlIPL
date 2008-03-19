/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - sdmc-launcher
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

#ifndef FIRM_USE_TWLSDK_KEYS
static const u8* rsa_key_user = NULL;   // not acceptable
static const u8* rsa_key_sys = NULL;    // not acceptable
static const u8 rsa_key_secure[128] =
{
    0xC7, 0x94, 0x50, 0x00, 0x3A, 0xE1, 0x0E, 0x6C, 0xA8, 0xD1, 0xC0, 0x2D, 0x77, 0xB7, 0x6D, 0xBC,
    0x31, 0xDB, 0x12, 0x08, 0x09, 0x0D, 0x2A, 0xE8, 0xC9, 0x1A, 0x2B, 0x6E, 0x6C, 0x85, 0x78, 0xD7,
    0x46, 0x50, 0x05, 0xB5, 0xCC, 0x3B, 0xEC, 0xBA, 0xF4, 0xDE, 0xC2, 0x13, 0x13, 0xBE, 0x67, 0xEE,
    0x85, 0x19, 0xEB, 0x62, 0xB3, 0x5C, 0x09, 0xA8, 0x54, 0x44, 0x26, 0x85, 0x25, 0xEA, 0xE5, 0x85,
    0xD1, 0xB5, 0xCE, 0xA0, 0xFF, 0x6B, 0x61, 0xCA, 0x94, 0xC1, 0x67, 0xBE, 0xC0, 0x7E, 0x3B, 0xFF,
    0x12, 0x9B, 0x79, 0xDB, 0xAC, 0xD3, 0x5A, 0x3F, 0x14, 0x37, 0x49, 0xA8, 0x7C, 0x2F, 0x07, 0xF4,
    0x8B, 0xA9, 0x8B, 0x8D, 0xB2, 0x60, 0xA5, 0xD5, 0x64, 0xEE, 0xCF, 0x3F, 0x32, 0xEE, 0x77, 0xAC,
    0x27, 0x75, 0x2B, 0x04, 0xD7, 0x26, 0xA8, 0x8A, 0x55, 0x2A, 0x76, 0xE5, 0x68, 0x80, 0x57, 0x85
};
#else
static const u8 rsa_key_user[128] =
{
    0xAC, 0x93, 0xBB,
    0x3C, 0x15, 0x5C, 0x5F, 0x25, 0xB0, 0x4C, 0x37, 0xA4, 0x2D, 0x85, 0x29, 0x1D, 0x7A, 0x9D, 0x2D,
    0xD5, 0x79, 0xB5, 0x5D, 0xB1, 0x08, 0x20, 0x9C, 0xF0, 0x4C, 0x56, 0x27, 0x97, 0xF8, 0x7E, 0x3E,
    0xCB, 0x94, 0x06, 0x05, 0x94, 0x00, 0x92, 0x9B, 0xB0, 0x5B, 0x06, 0xF6, 0xAF, 0xAA, 0x9C, 0xA5,
    0xF0, 0x11, 0xA7, 0x8A, 0xCB, 0x0C, 0x11, 0xD6, 0x0C, 0x3D, 0x30, 0xAC, 0x51, 0x79, 0x5A, 0xB5,
    0x7F, 0x11, 0x92, 0x74, 0x48, 0x82, 0x81, 0xBF, 0x3B, 0xFA, 0x93, 0xBF, 0x6B, 0x5B, 0x3F, 0x86,
    0x96, 0x4F, 0xCC, 0x90, 0x12, 0xB2, 0x39, 0x8D, 0x68, 0x16, 0x7B, 0xC6, 0x87, 0xF1, 0xF5, 0x60,
    0x62, 0x39, 0xFB, 0x10, 0x7E, 0x48, 0x7F, 0xDD, 0x82, 0x38, 0x38, 0x76, 0xB5, 0xCE, 0x21, 0x4B,
    0xC9, 0x6F, 0x31, 0x8D, 0x23, 0x57, 0x3D, 0xB6, 0x6C, 0xEE, 0xC2, 0x0D, 0x11
};
static const u8 rsa_key_sys[128] =
{
        0xe9, 0x9e, 0xa7, 0x9f, 0x59, 0x4d, 0xf4, 0xa7, 0x60, 0x04, 0xbd, 0x47, 0xf2, 0xb3, 0x64, 0xcd,
        0x16, 0x79, 0xc1, 0x47, 0x39, 0xf6, 0xa9, 0xf8, 0xee, 0x1a, 0xd0, 0x72, 0xcf, 0x43, 0x97, 0x0c,
        0x93, 0xa1, 0x38, 0x4e, 0x13, 0x40, 0x6c, 0x10, 0x59, 0x43, 0xe2, 0x71, 0x29, 0x54, 0x14, 0x2c,
        0xc5, 0xda, 0x59, 0x4d, 0xb4, 0x6a, 0xef, 0x85, 0x61, 0x6f, 0x7f, 0x1c, 0x59, 0x34, 0x2c, 0xc6,
        0x24, 0xf3, 0x7b, 0xc3, 0xb7, 0x40, 0xd1, 0x46, 0xf8, 0x90, 0xb7, 0xc2, 0x98, 0x50, 0xaf, 0x95,
        0x52, 0x42, 0xdb, 0xac, 0xd6, 0x7e, 0xa9, 0xc3, 0x3d, 0x1b, 0x51, 0x56, 0x07, 0x06, 0xd0, 0x0b,
        0x01, 0xbb, 0x58, 0x93, 0xea, 0xa0, 0x2c, 0xc7, 0x7d, 0x6a, 0x31, 0x7e, 0xc9, 0xe2, 0xda, 0xfe,
        0x1f, 0x2e, 0x9d, 0xa7, 0x54, 0x84, 0xdc, 0x28, 0xb9, 0x18, 0xea, 0x16, 0xf2, 0x95, 0x55, 0x6d,
};
static const u8 rsa_key_secure[128] =
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

#define RSA_HEAP_SIZE   (4*1024)    // RSA用ヒープサイズ (サイズ調整必要)

static u8 acHeap[RSA_HEAP_SIZE] __attribute__ ((aligned (32)));
static SVCSignHeapContext acPool;

#define MENU_FILE       "sdmc:/menu.srl"
#define MENU_FILE_A     "sdmc:/menu_a.srl"
#define MENU_FILE_B     "sdmc:/menu_b.srl"
#define MENU_FILE_L     "sdmc:/menu_l.srl"
#define MENU_FILE_R     "sdmc:/menu_r.srl"

/*
    PROFILE_ENABLE を定義するとある程度のパフォーマンスチェックができます。
    利用するためには、main.cかどこかに、u32 profile[256]; u32 pf_cnt = 0; を
    定義する必要があります。
*/
#define PROFILE_ENABLE

/*
    PRINT_MEMORY_ADDR を定義すると、そのアドレスからSPrintfを行います(このファイルのみ)
    FINALROM版でもコードが残るので注意してください。
*/
//#define PRINT_MEMORY_ADDR       0x02FFC000

//#ifdef SDK_FINALROM // FINALROMで無効化
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

    FromBootの対応＆OS_Init前に必要なメインメモリの初期化
***************************************************************/
static void PreInit(void)
{
    /*
     メインメモリ関連
    */
    // SHARED領域クリア
    MI_CpuClearFast((void *)HW_WRAM_EX_LOCK_BUF,        (HW_WRAM_EX_LOCK_BUF_END - HW_WRAM_EX_LOCK_BUF));
    MI_CpuClearFast((void *)HW_BIOS_EXCP_STACK_MAIN,    (HW_REAL_TIME_CLOCK_BUF - HW_BIOS_EXCP_STACK_MAIN));
    MI_CpuClearFast((void *)HW_PXI_SIGNAL_PARAM_ARM9,   (HW_MMEMCHECKER_MAIN - HW_PXI_SIGNAL_PARAM_ARM9));
    MI_CpuClearFast((void*)HW_ROM_HEADER_BUF,           (HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF));

    /*
        FromBrom関連
    */
    if ( !OSi_FromBromToMenu() )
    {
        OS_Terminate();
    }

    // ブートタイプの変更
    ( (OSBootInfo *)OS_GetBootInfo() )->boot_type = OS_BOOTTYPE_NAND;
}

/***************************************************************
    PostInit

    各種初期化
***************************************************************/
static void PostInit(void)
{
    // RSA用ヒープ設定
    SVC_InitSignHeap( &acPool, acHeap, sizeof(acHeap) );
    // FS/FATFS初期化
    FS_InitFIRM();
}

/***************************************************************
    CheckHeader

    ヘッダがシステムメニューとして問題ないかチェック
***************************************************************/
static BOOL CheckHeader(void)
{
    ROM_Header_Short* const rhs = (ROM_Header_Short*)HW_TWL_ROM_HEADER_BUF;
    // イニシャルコードなど
    OS_TPrintf("Initial Code        : %08X (%.4s)\n", *(u32*)rhs->game_code, rhs->game_code);
    OS_TPrintf("Platform Code       : %02X\n", rhs->platform_code);
    OS_TPrintf("Codec Mode          : %s\n", rhs->codec_mode ? "TWL" : "NITRO");
    OS_TPrintf("Sigunature          : %s\n", rhs->enable_signature ? "AVAILABLE" : "NOT AVAILABLE");
    OS_TPrintf("AES Encryption      : %s\n", rhs->enable_aes ? "AVAILABLE" : "NOT AVAILABLE");
    if ( rhs->enable_aes )
    {
        OS_TPrintf("AES Key Type        : %s\n", rhs->developer_encrypt ? "FOR DEVELOPMENT" : "FOR PRODUCT");
    }
    // エントリポイント
    OS_TPrintf("ARM9 Entry point    : %08X\n", rhs->main_entry_address);
    OS_TPrintf("ARM7 Entry point    : %08X\n", rhs->sub_entry_address);
    // ロード範囲
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
    // 順序ほぼ最適化済み
#ifndef FIRM_USE_TWLSDK_KEYS
    if ( rhs->platform_code != PLATFORM_CODE_TWL_LIMITED ||     // TWL Limited only
         !rhs->enable_signature ||                              // Should be use ROM header signature
#else
    if ( // no check
#endif
         !rhs->codec_mode ||                                    // TWL mode only
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
        OS_TPrintf("Invalid ROM header for SDMC Launcher!\n");
        return FALSE;
    }
    return TRUE;
}

/***************************************************************
    EraseAll

    不正終了しました
    いろいろ消してください
    DSモードにして終わるのがよいか？
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
    // 1: before Init
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

    switch ( PAD_Read() & PAD_KEYPORT_MASK )
    {
    case 0:
        STD_CopyString((char*)HW_FIRM_TEMP_SRL_PATH_BUF, MENU_FILE);
        break;
    case PAD_BUTTON_A:
        STD_CopyString((char*)HW_FIRM_TEMP_SRL_PATH_BUF, MENU_FILE_A);
        break;
    case PAD_BUTTON_B:
        STD_CopyString((char*)HW_FIRM_TEMP_SRL_PATH_BUF, MENU_FILE_B);
        break;
    case PAD_BUTTON_L:
        STD_CopyString((char*)HW_FIRM_TEMP_SRL_PATH_BUF, MENU_FILE_L);
        break;
    case PAD_BUTTON_R:
        STD_CopyString((char*)HW_FIRM_TEMP_SRL_PATH_BUF, MENU_FILE_R);
        break;
    default:
        OS_TPrintf("Unknown pad pattern (%X).\n", PAD_Read() & PAD_KEYPORT_MASK);
        goto end;
    }
    // 4: after FS_ResolveSrl
    PUSH_PROFILE();

    PXI_NotifyID( FIRM_PXI_ID_SET_PATH );
    // 5: after PXI
    PUSH_PROFILE();

    if ( !FS_LoadHeader( &acPool, rsa_key_user, rsa_key_sys, rsa_key_secure ) || !CheckHeader() )
    {
        OS_TPrintf("Failed to call FS_LoadHeader() and/or CheckHeader().\n");
        goto end;
    }
    // 6: after FS_LoadHeader
    PUSH_PROFILE();

    PXI_NotifyID( FIRM_PXI_ID_DONE_HEADER );
    // 7: after PXI
    PUSH_PROFILE();

    if ( !FS_LoadStatic( NULL ) )
    {
        OS_TPrintf("Failed to call FS_LoadStatic().\n");
        goto end;
    }
    // 8: after FS_LoadStatic
    PUSH_PROFILE();

    PXI_NotifyID( FIRM_PXI_ID_DONE_STATIC );
    // 9: after PXI
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
    }
#endif

    MI_CpuClearFast( OSi_GetFromFirmAddr(), sizeof(OSFromFirmBuf) );
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

