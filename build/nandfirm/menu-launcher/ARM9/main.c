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
#define RSA_KEY_ADDR    OSi_GetFromFirmAddr()->rsa_pubkey[0]    // 鍵管理.xls参照
#else
#define RSA_KEY_ADDR    rsa_key
static const u8 rsa_key[128] =
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
#endif

#define RSA_HEAP_SIZE   (4*1024)    // RSA用ヒープサイズ (サイズ調整必要)

static u8 acHeap[RSA_HEAP_SIZE] __attribute__ ((aligned (32)));
static SVCSignHeapContext acPool;

#define MENU_TITLE_ID_HI    0x00030007ULL
#define MENU_TITLE_ID_LO    0x4c4e4352ULL
#define MENU_TITLE_ID       (MENU_TITLE_ID_HI << 32 | MENU_TITLE_ID_LO)

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
#define PRINT_MEMORY_ADDR       0x02FFC200

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
        { 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },
        { 0 }
    };
    /*
     メインメモリ関連
    */
    // SHARED領域はスタートアップ時でクリア

    // FS_MOUNT領域の初期化
    MI_CpuCopy8(firmSettings, (char*)HW_TWL_FS_MOUNT_INFO_BUF, sizeof(firmSettings));

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
    TryResolveSrl

    NANDに格納された情報からランチャーSRLを解決する
***************************************************************/
static BOOL TryResolveSrl(void)
{
    u64 titleId = MENU_TITLE_ID_HI << 32;
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

    デフォルト設定からランチャーSRLを解決する
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

    ヘッダがシステムメニューとして問題ないかチェック
***************************************************************/
static BOOL CheckHeader(void)
{
    static ROM_Header_Short* const rhs = (ROM_Header_Short*)HW_TWL_ROM_HEADER_BUF;
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
         HW_TWL_MAIN_MEM_END <= (u32)rhs->main_ram_address + rhs->main_size ||
         HW_TWL_MAIN_MEM_END <= (u32)rhs->sub_ram_address + rhs->sub_size ||
         HW_TWL_MAIN_MEM_END <= (u32)rhs->main_ltd_ram_address + rhs->main_ltd_size ||
         HW_TWL_MAIN_MEM_END <= (u32)rhs->sub_ltd_ram_address + rhs->sub_ltd_size ||
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

    if ( !FS_LoadHeader( &acPool, NULL, RSA_KEY_ADDR ) || !CheckHeader() )
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

