/*---------------------------------------------------------------------------*
  Project:  TwlFirm - gcdfirm - sdmc-launcher
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
#define RSA_KEY_ADDR    OSi_GetFromFirmAddr()->rsa_pubkey[2]    // 鍵管理.xls参照
#else
#define RSA_KEY_ADDR    rsa_key
static const u8 rsa_key[128] =
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
#endif

#define RSA_HEAP_SIZE   (4*1024)    // RSA用ヒープサイズ (サイズ調整必要)

static u8 acHeap[RSA_HEAP_SIZE] __attribute__ ((aligned (32)));
static SVCSignHeapContext acPool;

#define MENU_FILE       "sdmc:/menu.srl"

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
#define PRINT_MEMORY_ADDR       0x02000200

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
    static const OSMountInfo    firmSettings[] =
    {
        { 'A', OS_MOUNT_DEVICE_SD, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "sdmc",    "/" },
        { 0 }
    };
    /*
     メインメモリ関連
    */
    // SHARED領域クリア
    MI_CpuClearFast((void *)HW_WRAM_EX_LOCK_BUF,        (HW_WRAM_EX_LOCK_BUF_END - HW_WRAM_EX_LOCK_BUF));
    MI_CpuClearFast((void *)HW_BIOS_EXCP_STACK_MAIN,    (HW_REAL_TIME_CLOCK_BUF - HW_BIOS_EXCP_STACK_MAIN));
    MI_CpuClearFast((void *)HW_PXI_SIGNAL_PARAM_ARM9,   (HW_MMEMCHECKER_MAIN - HW_PXI_SIGNAL_PARAM_ARM9));
    MI_CpuClearFast((void*)HW_ROM_HEADER_BUF,           (HW_ROM_HEADER_BUF_END-HW_ROM_HEADER_BUF));

    // FS_MOUNT領域の初期化
    MI_CpuCopy8(firmSettings, (char*)HW_TWL_FS_MOUNT_INFO_BUF, sizeof(firmSettings));

    /*
        FromBrom関連
    */
    if ( !OSi_FromBromToMenu() )
    {
        OS_Terminate();
    }
}

/***************************************************************
    PostInit

    各種初期化
***************************************************************/
static void PostInit(void)
{
    // RSA用ヒープ設定
    SVC_InitSignHeap( &acPool, acHeap, sizeof(acHeap) );
    // HMAC用鍵準備
    FS_SetDigestKey( NULL );
    // FS/FATFS初期化
    FS_InitFIRM();
}

/***************************************************************
    CheckHeader

    ヘッダがシステムメニューとして問題ないかチェック
***************************************************************/
static BOOL CheckHeader(void)
{
    static ROM_Header_Short* const rhs = (ROM_Header_Short*)HW_TWL_ROM_HEADER_BUF;
    // イニシャルコードなど
    OS_TPrintf("Initial Code        : %08X\n", *(u32*)rhs->game_code);
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
         HW_TWL_MAIN_MEM_END <= (u32)rhs->main_ram_address + rhs->main_size ||
         HW_TWL_MAIN_MEM_END <= (u32)rhs->sub_ram_address + rhs->sub_size ||
         HW_TWL_MAIN_MEM_END <= (u32)rhs->main_ltd_ram_address + rhs->main_ltd_size ||
         HW_TWL_MAIN_MEM_END <= (u32)rhs->sub_ltd_ram_address + rhs->sub_ltd_size ||
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
    PreInit();

    // 0: before PXI
    PUSH_PROFILE();

    OS_InitFIRM();
    OS_EnableIrq();
    OS_EnableInterrupts();

#ifdef PROFILE_ENABLE
    OS_InitTick();
#endif
    // 1: after PXI
    PUSH_PROFILE();

    PostInit();

    // 2: after PostInit
    PUSH_PROFILE();

    STD_CopyString((char*)HW_TWL_FS_BOOT_SRL_PATH_BUF, MENU_FILE);

    // 3: after FS_ResolveSrl
    PUSH_PROFILE();

    PXI_NotifyID( FIRM_PXI_ID_SET_PATH );

    // 4: after PXI
    PUSH_PROFILE();

    if ( !FS_LoadHeader(&acPool, RSA_KEY_ADDR ) || !CheckHeader() )
    {
        OS_TPrintf("Failed to call FS_LoadHeader() and/or CheckHeader().\n");
        goto end;
    }

    // 5: after FS_LoadHeader
    PUSH_PROFILE();

    PXI_NotifyID( FIRM_PXI_ID_DONE_HEADER );

    // 6: after PXI
    PUSH_PROFILE();

    AESi_SendSeed( FS_GetAesKeySeed() );
    FS_DeleteAesKeySeed();

    // 7: after AESi_SendSeed
    PUSH_PROFILE();

    if ( !FS_LoadStatic() )
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

