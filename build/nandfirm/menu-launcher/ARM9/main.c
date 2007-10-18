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
static int acPool[3];

/*
    Profile
*/
#ifndef SDK_FINALROM
#define PRFILE_MAX  128
u32 profile[PRFILE_MAX];
u32 pf_cnt = 0;
#endif

/***************************************************************
    PreInit

    FromBootの対応をまとめる＆メインメモリの初期化
    OS_Init前なので注意
***************************************************************/
static void PreInit(void)
{
    /*
     メインメモリ関連
    */

    // SHARED領域クリア (IS-TWL-DEBUGGERの更新待ち)
#ifdef SDK_FINALROM
    MIi_CpuClearFast( 0, (void*)HW_MAIN_MEM_SHARED, HW_MAIN_MEM_SHARED_END-HW_MAIN_MEM_SHARED );
#endif

    /*
        FromBrom関連
    */

    /* 鍵はどこへ？ */

    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );
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

#ifndef SDK_FINALROM
    // 0: before PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    OS_InitFIRM();
#ifndef SDK_FINALROM
    OS_InitTick();
    // 1: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    SVC_InitSignHeap( acPool, acHeap, sizeof(acHeap) );

    // load menu
    if ( MI_LoadHeader( acPool, RSA_KEY_ADDR ) && CheckHeader() && MI_LoadMenu() )
    {
#ifndef SDK_FINALROM
        // 127: before BootMenu
        pf_cnt = PRFILE_MAX-1;
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

        MI_BootMenu();
    }

    EraseAll();

    // failed
    while (1)
    {
        PXI_NotifyID( FIRM_PXI_ID_NULL );
    }
}

