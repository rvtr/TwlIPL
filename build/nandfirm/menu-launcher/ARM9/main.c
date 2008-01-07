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

#ifndef FIRM_USE_TWLSDK_KEYS
#define RSA_KEY_ADDR    OSi_GetFromFirmAddr()->rsa_pubkey[0]    // 鍵管理.xls参照
#else
#define RSA_KEY_ADDR    rsa_key
static const u8 rsa_key[128] =
{
    0xdf, 0x56, 0x30,
    0xc9, 0xae, 0x05, 0x55, 0xe8, 0xdf, 0xbe, 0xe6, 0xb9, 0x30, 0xb9, 0x76, 0x93, 0xb4, 0xc2, 0x20,
    0xe7, 0xae, 0x4c, 0x3e, 0xc3, 0xed, 0x27, 0xcf, 0x5d, 0x4f, 0xb5, 0x7d, 0xde, 0x38, 0xbc, 0xfe,
    0x25, 0x32, 0xd8, 0x23, 0x98, 0x52, 0xb5, 0xda, 0xf7, 0x39, 0xdc, 0xb3, 0x0a, 0x94, 0x7a, 0x2b,
    0x79, 0xe6, 0xe0, 0x4c, 0xbc, 0x21, 0xbd, 0x59, 0xb2, 0xc7, 0xf1, 0xc0, 0xf1, 0xfb, 0x29, 0x75,
    0xa1, 0x21, 0x93, 0x01, 0x29, 0x1c, 0x9a, 0xe1, 0x2d, 0x55, 0xfc, 0x7b, 0xb8, 0xcb, 0x07, 0x33,
    0xc5, 0x91, 0x0d, 0xc8, 0x45, 0x59, 0xef, 0xbe, 0x58, 0xc7, 0xc1, 0x1d, 0xd5, 0xf2, 0xcf, 0x1f,
    0xe0, 0x6d, 0x21, 0x00, 0xcd, 0x42, 0xd8, 0x84, 0x85, 0xe3, 0xb2, 0x02, 0x1a, 0xa5, 0x89, 0x02,
    0xa1, 0x96, 0xc6, 0xf7, 0x61, 0x68, 0x66, 0xe6, 0x65, 0x12, 0xb7, 0xf1, 0x49
};
#endif

#define RSA_HEAP_SIZE   (4*1024)    // RSA用ヒープサイズ (サイズ調整必要)

static u8 acHeap[RSA_HEAP_SIZE] __attribute__ ((aligned (32)));
static SVCSignHeapContext acPool;

#define MENU_TITLE_ID 0x000300074c4e4352ULL

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
        { 'F', OS_MOUNT_DEVICE_NAND, OS_MOUNT_TGT_ROOT, 0, OS_MOUNT_RSC_WRAM, (OS_MOUNT_USR_R|OS_MOUNT_USR_W), 0, 0, "nand",    "/" },
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
PXI_NotifyID( FIRM_PXI_ID_NULL );

    PostInit();

    // 2: after PostInit
    PUSH_PROFILE();
PXI_NotifyID( FIRM_PXI_ID_NULL );

    if ( !FS_ResolveSrl( MENU_TITLE_ID ) )
    {
        OS_TPrintf("Failed to call FS_ResolveSrl( 0x%016llx ).\n", MENU_TITLE_ID);
        goto end;
    }

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

