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
#include "print.h"

#ifdef FIRM_USE_PRODUCT_KEYS
#define RSA_KEY_ADDR    OSi_GetFromFirmAddr()->rsa_pubkey[0]    // 鍵管理.xls参照
#else
#define RSA_KEY_ADDR    rsa_key_launcher
static const u8 rsa_key_launcher[128] =
{
        0xbc, 0xfd, 0xa1, 0xff, 0x1f, 0x66, 0xdf, 0xec, 0xb4, 0x69, 0xf8, 0xf7, 0x43, 0x0c, 0x5d, 0x0f,
        0x00, 0xd7, 0x20, 0x49, 0x42, 0x06, 0x03, 0x29, 0x85, 0x0b, 0x99, 0x59, 0x61, 0x98, 0x70, 0x6e,
        0xff, 0xf6, 0xb4, 0x70, 0x66, 0xf0, 0xdd, 0x8f, 0xdc, 0xe9, 0xf2, 0x0d, 0xd0, 0x21, 0x1d, 0x77,
        0xb8, 0x9c, 0x51, 0x87, 0xc0, 0xb1, 0x33, 0xab, 0x13, 0x96, 0x0b, 0x47, 0xb8, 0x42, 0x4a, 0x0d,
        0xc3, 0x77, 0xe1, 0x87, 0xb1, 0x6b, 0x24, 0x31, 0x10, 0x8a, 0x47, 0xf2, 0x32, 0xf4, 0xc9, 0x78,
        0x25, 0x13, 0xd4, 0x80, 0x10, 0x05, 0x52, 0xc3, 0xe7, 0x50, 0x7b, 0x29, 0x49, 0xce, 0x93, 0xd9,
        0x8f, 0x2a, 0xb5, 0x4d, 0xd1, 0xc1, 0x91, 0xd8, 0x07, 0x16, 0x10, 0x15, 0xff, 0xd6, 0x84, 0x8f,
        0x54, 0x3d, 0x91, 0x5b, 0x37, 0x45, 0x48, 0xe4, 0x6b, 0x62, 0xd2, 0x11, 0x9b, 0x0d, 0x71, 0x69,
};
#endif

static SVCSignHeapContext acPool;

#define MENU_TITLE_ID_HI    0x00030017ULL
#define MENU_TITLE_ID_LO    0x484e4141ULL
#define MENU_TITLE_ID       (MENU_TITLE_ID_HI << 32 | MENU_TITLE_ID_LO)

/*
    PROFILE_ENABLE を定義するとある程度のパフォーマンスチェックができます。
    利用するためには、main.cかどこかに、u32 profile[256]; u32 pf_cnt = 0; を
    定義する必要があります。
*/
//#define PROFILE_ENABLE

/*
    PRINT_MEMORY_ADDR を定義すると、そのアドレスからSPrintfを行います(このファイルのみ)
    FINALROM版でもコードが残るので注意してください。
*/
//#define PRINT_MEMORY_ADDR       0x02FFC2A0

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
extern const char *g_strIPLSvnRevision;
extern const char *g_strSDKSvnRevision;
static void PreInit(void)
{
    ROM_Header_Short* const rhs = (ROM_Header_Short*)HW_TWL_ROM_HEADER_BUF;
    static char buffer[4][8];   // バージョン情報
    /*
     メインメモリ関連
    */
    // SHARED領域はスタートアップ時でクリア
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
    // FS用アリーナ設定
    {
        static u32 arena[ 0x400 / sizeof(u32) ];
        OS_SetMainArenaLo( arena );
        OS_SetMainArenaHi( &arena[ 0x400 / sizeof(u32) ] );
    }
    // FS/FATFS初期化
    FS_InitFIRM();
}

/***************************************************************
    TryResolveSrl

    リストからランチャーSRLを解決する
***************************************************************/
//#define DEBUG_CRYPTO_ALLOCATOR
#ifdef DEBUG_CRYPTO_ALLOCATOR
#include <nitro/crypto.h>
static int allocated=0;
static void* myAlloc(size_t size)
{
    void* ptr = OS_Alloc(size);
    if (ptr)
    {
        OS_TPrintf("Alloc [%d] %d bytes.\n", ++allocated, size);
    }
    else
    {
        OS_TPrintf("Failed to allocate %d bytes.\n", size);
    }
    return ptr;
}
static void myFree(void* ptr)
{
    OS_TPrintf("Free [%d]\n", --allocated);
    OS_Free(ptr);
}
#endif
static BOOL TryResolveSrl(void)
{
    OSTitleId titleIdList[] =
    {
        MENU_TITLE_ID_HI << 32, // titleId_Lo is resolved by HWSecureInfo
        MENU_TITLE_ID
    };
    int num;

    // CRYPTO用ヒープ設定 (ESライブラリしか使わないはず)
    void* lo = OS_InitAlloc( OS_ARENA_MAIN, (void*)HW_FIRM_RSA_BUF, (void*)HW_FIRM_RSA_BUF_END, 1);
    void* hi = (void*)HW_FIRM_RSA_BUF_END;
    OSHeapHandle hh = OS_CreateHeap( OS_ARENA_MAIN, lo, hi );
    if ( hh < 0 )
    {
        OS_TPrintf("Failed to allocate heap.\n");
        return FALSE;
    }
    OS_SetCurrentHeap( OS_ARENA_MAIN, hh );

#ifdef DEBUG_CRYPTO_ALLOCATOR
    CRYPTO_SetAllocator(myAlloc, myFree);
#endif

    if ( !LCFG_ReadHWSecureInfo() )
    {
        OS_TPrintf("Failed to load HWSecureInfo.\n");
        // 4: after LCFG_ReadHWSecureInfo
        PUSH_PROFILE();

        if ( FS_ResolveSrlList( &titleIdList[1], 1 ) < 0 )  // one title ID only
        {
            OS_TPrintf("Failed to call FS_ResolveSrlList().\n");
            return FALSE;
        }
        OS_TPrintf("Launcher Title ID: 0x%016llx\n", titleIdList[1]);
    }
    else
    {
        LCFG_THW_GetLauncherTitleID_Lo( (u8*)&titleIdList[0] );
        // 4: after LCFG_ReadHWSecureInfo
        PUSH_PROFILE();

        num = FS_ResolveSrlList( titleIdList, sizeof(titleIdList)/sizeof(titleIdList[0]) );
        if ( num < 0 )
        {
            OS_TPrintf("Failed to call FS_ResolveSrlList().\n");
            return FALSE;
        }
        OS_TPrintf("Launcher Title ID: 0x%016llx\n", titleIdList[num]);
    }

    OS_DestroyHeap( OS_ARENA_MAIN, hh );

    return TRUE;
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
    OS_TPrintf("Codec Mode          : %s\n", rhs->exFlags.codec_mode ? "TWL" : "NITRO");
    OS_TPrintf("Sigunature          : %s\n", rhs->enable_signature ? "AVAILABLE" : "NOT AVAILABLE");
    OS_TPrintf("AES Encryption      : %s\n", rhs->enable_aes ? "AVAILABLE" : "NOT AVAILABLE");
    if ( rhs->enable_aes )
    {
        OS_TPrintf("AES Key Type        : %s\n", ( rhs->developer_encrypt_old || rhs->exFlags.developer_encrypt ) ? "FOR DEVELOPMENT" : "FOR PRODUCT");
        if ( rhs->aes_target_size )
        {
            OS_TPrintf("AES address         : %08X\n", rhs->aes_target_rom_offset);
            OS_TPrintf("AES size            : %08X\n", rhs->aes_target_size);
        }
        if ( rhs->aes_target2_size )
        {
            OS_TPrintf("AES2 address        : %08X\n", rhs->aes_target2_rom_offset);
            OS_TPrintf("AES2 size           : %08X\n", rhs->aes_target2_size);
        }
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
    if ( rhs->platform_code != PLATFORM_CODE_TWL_LIMITED ||     // TWL Limited only
         !rhs->exFlags.codec_mode ||                            // TWL mode only
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
    int point = 1;
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

    if ( !TryResolveSrl() )
    {
        goto end;
    }
    point++;    // 2
    // 5: after FS_ResolveSrl
    PUSH_PROFILE();

    PXI_NotifyID( FIRM_PXI_ID_SET_PATH );
    // 6: after PXI
    PUSH_PROFILE();

    /* ES (CRYPTO) ライブラリはここまで */
    /* SVN_RSA はここから*/
    // RSA用ヒープ設定
    SVC_InitSignHeap( &acPool, (void*)HW_FIRM_RSA_BUF, HW_FIRM_RSA_BUF_SIZE );

    if ( !FS_LoadHeader( &acPool, NULL, NULL, RSA_KEY_ADDR ) || !CheckHeader() )
    {
        OS_TPrintf("Failed to call FS_LoadHeader() and/or CheckHeader().\n");
        goto end;
    }
    point++;    // 3
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
    point++;    // 4
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
    PrintError("Error: %d-%s-%s", point, g_strIPLSvnRevision, g_strSDKSvnRevision);
    EraseAll();

    // failed
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    PXI_NotifyID( FIRM_PXI_ID_ERR );
    OS_Terminate();
}

