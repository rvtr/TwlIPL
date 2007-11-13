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
#define RSA_KEY_ADDR    OSi_GetFromBromAddr()->rsa_pubkey[7]

#define RSA_HEAP_SIZE   (4*1024)    // RSA用ヒープサイズ (サイズ調整必要)

static u8 acHeap[RSA_HEAP_SIZE] __attribute__ ((aligned (32)));
static SVCSignHeapContext acPool;

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
#define PRFILE_MAX  128
u32 profile[PRFILE_MAX];
u32 pf_cnt = 0;
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
    // SHARED領域クリア (ここだけでOK?)
    MIi_CpuClearFast( 0, (void*)HW_PXI_SIGNAL_PARAM_ARM9, HW_MAIN_MEM_SHARED_END-HW_PXI_SIGNAL_PARAM_ARM9);

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

    MI_LoadHeader前にかなり(数100msec)時間があるので、可能なら
    OS_Init後にいろいろ処理したい！
    メインメモリの初期化
***************************************************************/
extern u32 SDK_SECTION_ARENA_DTCM_START;
static void PostInit(void)
{
    /*
     メインメモリ関連
    */
    // (DTCMの手前までの領域を全クリア)
    //MI_CpuClearFast( (void*)HW_DELIVER_ARG_BUF_END, SDK_SECTION_ARENA_DTCM_START-HW_DELIVER_ARG_BUF_END );
    // (ARM9領域を全クリア)
    MI_CpuClearFast( (void*)HW_DELIVER_ARG_BUF_END, HW_MAIN_MEM_MAIN_END-HW_DELIVER_ARG_BUF_END );

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
    // TODO
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
#ifdef PROFILE_ENABLE
    OS_InitTick();
    // 1: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    SVC_InitSignHeap( &acPool, acHeap, sizeof(acHeap) );

    PostInit();

    // load menu
    if ( MI_LoadHeader( &acPool, RSA_KEY_ADDR ) && CheckHeader() && MI_LoadStatic() )
    {
#ifdef PROFILE_ENABLE
        // 127: before Boot
        pf_cnt = PRFILE_MAX-1;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

        MI_Boot();
    }

    EraseAll();

    // failed
    while (1)
    {
        PXI_NotifyID( FIRM_PXI_ID_NULL );
    }
}

