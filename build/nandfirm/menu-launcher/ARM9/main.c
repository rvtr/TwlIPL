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

/* 鍵はどこへ？ */
#if 0
#define RSA_KEY_ADDR    OSi_GetFromFirmAddr()->rsa_pubkey[7]
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

#define MENU_TITLE_ID 0x0001000152434e4cULL

/*
    PROFILE_ENABLE を定義するとある程度のパフォーマンスチェックができます。
    利用するためには、main.cかどこかに、u32 profile[256]; u32 pf_cnt = 0; を
    定義する必要があります。
*/
#define PROFILE_ENABLE

#ifdef SDK_FINALROM // FINALROMで無効化
#undef PROFILE_ENABLE
#endif

#ifdef PROFILE_ENABLE
#define PROFILE_MAX  16
u32 profile[PROFILE_MAX];
u32 pf_cnt = 0;
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

    FS_LoadHeader前にかなり(数100msec)時間があるので、可能なら
    OS_Init後にいろいろ処理したい！
    メインメモリの初期化
***************************************************************/
static void PostInit(void)
{
    /*
     メインメモリ関連
    */
    // ARM9領域を全クリア
    if ( OS_GetResetParameter() )
    {
        MI_CpuClearFast( (void*)HW_FIRM_RESET_BUF_END, HW_TWL_MAIN_MEM_MAIN_END-HW_FIRM_RESET_BUF_END );
    }
    else
    {
        MI_CpuClearFast( (void*)HW_MAIN_MEM_MAIN, HW_MAIN_MEM_MAIN_SIZE );
    }

    DC_FlushAll();
}

/***************************************************************
    CheckHeader

    ヘッダがシステムメニューとして問題ないかチェック
    先頭32Bは固定値と思われ (リマスターバージョンは違うかな)
***************************************************************/
static BOOL CheckHeader(void)
{
    // TODO
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
#ifdef SDK_FINALROM
    MI_CpuClearFast( (void*)HW_TWL_ROM_HEADER_BUF, HW_TWL_ROM_HEADER_BUF_SIZE );
    OS_BootFromFIRM();
#endif
}

void TwlMain( void )
{
    PreInit();

#ifdef PROFILE_ENABLE
    // 0: before PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    OS_InitFIRM();
    OS_EnableIrq();
    OS_EnableInterrupts();

#ifdef PROFILE_ENABLE
    OS_InitTick();
    // 1: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    PostInit();

#ifdef PROFILE_ENABLE
    // 2: after PostInit
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    // RSA用ヒープ設定
    SVC_InitSignHeap( &acPool, acHeap, sizeof(acHeap) );
    // HMAC用鍵準備
    FS_SetDigestKey( NULL );

#ifdef PROFILE_ENABLE
    // 3: after SVC_InitSignHeap
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    FS_Init(FS_DMA_NOT_USE);

#ifdef PROFILE_ENABLE
    // 4: after FS_Init
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    if ( !FS_ResolveSrl( MENU_TITLE_ID ) )
    {
        OS_TPrintf("Failed to call FS_ResolveSrl( 0x%016llx ).\n", MENU_TITLE_ID);
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 5: after FS_ResolveSrl
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    PXI_NotifyID( FIRM_PXI_ID_SET_PATH );

#ifdef PROFILE_ENABLE
    // 6: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    if ( !FS_LoadHeader(&acPool, RSA_KEY_ADDR ) && CheckHeader() )
    {
        OS_TPrintf("Failed to call FS_LoadHeader() and/or CheckHeader().\n");
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 7: after FS_LoadHeader
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    PXI_NotifyID( FIRM_PXI_ID_DONE_HEADER );

#ifdef PROFILE_ENABLE
    // 8: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    if ( !FS_LoadStatic() )
    {
        OS_TPrintf("Failed to call FS_LoadStatic().\n");
        goto end;
    }

#ifdef PROFILE_ENABLE
    // 9: after FS_LoadStatic
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    PXI_NotifyID( FIRM_PXI_ID_DONE_STATIC );

#ifdef PROFILE_ENABLE
    // 15: after PXI
    pf_cnt = PROFILE_MAX-1;
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
    {
        int i;
        OS_TPrintf("\n[ARM9] Begin\n");
        for (i = 0; i < PROFILE_MAX; i++)
        {
            OS_TPrintf("0x%08X\n", profile[i]);
        }
        OS_TPrintf("\n[ARM9] End\n");
        PXI_NotifyID( FIRM_PXI_ID_NULL );
    }
#endif

    OS_BootFromFIRM();

end:
    EraseAll();

    // failed
    while (1)
    {
        PXI_NotifyID( FIRM_PXI_ID_ERR );
    }
}

