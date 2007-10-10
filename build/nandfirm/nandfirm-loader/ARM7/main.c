/*---------------------------------------------------------------------------*
  Project:  TwlFirm - nandfirm - nandrfirm-loader
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
#include <twl/os/ARM7/debugLED.h>

//#define BOOT_SECURE_SRL   // 本番SRLをブートするときにだけ定義する

#define FATFS_HEAP_SIZE     (64*1024)   // FATFS用ヒープ (サイズ調整必要)

#ifndef BOOT_SECURE_SRL
#define BOOT_DEVICE     FATFS_MEDIA_TYPE_SD
#define PARTITION_NO    0                       // 0固定
#define MENU_FILE       (char*)L"A:\\menu.srl"          // 対象ファイル(DRIVE_LETTERと合わせること)
#define MENU_FILE_A     (char*)L"A:\\menu_a.srl"        // 対象ファイル(DRIVE_LETTERと合わせること)
#define MENU_FILE_B     (char*)L"A:\\menu_b.srl"        // 対象ファイル(DRIVE_LETTERと合わせること)
#define MENU_FILE_L     (char*)L"A:\\menu_l.srl"        // 対象ファイル(DRIVE_LETTERと合わせること)
#define MENU_FILE_R     (char*)L"A:\\menu_r.srl"        // 対象ファイル(DRIVE_LETTERと合わせること)
#else
#define BOOT_DEVICE     FATFS_MEDIA_TYPE_NAND
#define PARTITION_NO    0                       // 対象パーティション
#endif

#define DRIVE_LETTER    'A'                     // マウント先ドライブ名
#define DRIVE_NO        (DRIVE_LETTER - 'A')    // マウント先ドライブ番号

static u8 fatfsHeap[FATFS_HEAP_SIZE] __attribute__ ((aligned (32)));

static SDPortContextData nandContext;   // 一時待避用 (次に渡すならSHAREDのどこかのアドレスにする)

#ifndef SDK_FINALROM
static u8 step = 0x80;
#endif

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
    OS_Init前なので注意 (ARM9によるメインメモリ初期化で消されないように注意)
***************************************************************/
static void PreInit(void)
{

    /*
        FromBrom関連
    */

#ifdef BOOT_SECURE_SRL
    /* 鍵はどこへ？ */
#endif
    // NANDパラメータの待避
    nandContext = OSi_GetFromBromAddr()->SDNandContext;

    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );
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

void TwlSpMain( void )
{
    // OS_InitDebugLED and OS_SetDebugLED are able to call after OS_Init
#ifndef SDK_FINALROM
    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x03, 0x00);
    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, ++step);
#endif

    PreInit();

#ifndef SDK_FINALROM
    I2Ci_WriteRegister(I2C_SLAVE_DEBUG_LED, 0x01, ++step);

    // 0: before PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    OS_InitFIRM();

#ifndef SDK_FINALROM
    // 1: after PXI
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    OS_SetDebugLED(++step);

    PM_InitFIRM();

#ifndef SDK_FINALROM
    // 2: after PM
    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif

    OS_SetDebugLED(++step);

#if 0
    OS_TPrintf("OS_GetMainArenaHi()        = 0x%08X\n", OS_GetMainArenaHi());
    OS_TPrintf("OS_GetMainArenaLo()        = 0x%08X\n", OS_GetMainArenaLo());
    OS_TPrintf("OS_GetSubPrivArenaHi()     = 0x%08X\n", OS_GetSubPrivArenaHi());
    OS_TPrintf("OS_GetSubPrivArenaLo()     = 0x%08X\n", OS_GetSubPrivArenaLo());
    OS_TPrintf("OS_GetMainExArenaHi()      = 0x%08X\n", OS_GetMainExArenaHi());
    OS_TPrintf("OS_GetMainExArenaLo()      = 0x%08X\n", OS_GetMainExArenaLo());
    OS_TPrintf("OS_GetITCMArenaHi()        = 0x%08X\n", OS_GetITCMArenaHi());
    OS_TPrintf("OS_GetITCMArenaLo()        = 0x%08X\n", OS_GetITCMArenaLo());
    OS_TPrintf("OS_GetDTCMArenaHi()        = 0x%08X\n", OS_GetDTCMArenaHi());
    OS_TPrintf("OS_GetDTCMArenaLo()        = 0x%08X\n", OS_GetDTCMArenaLo());
    OS_TPrintf("OS_GetSharedArenaHi()      = 0x%08X\n", OS_GetSharedArenaHi());
    OS_TPrintf("OS_GetSharedArenaLo()      = 0x%08X\n", OS_GetSharedArenaLo());
    OS_TPrintf("OS_GetWramMainArenaHi()    = 0x%08X\n", OS_GetWramMainArenaHi());
    OS_TPrintf("OS_GetWramMainArenaLo()    = 0x%08X\n", OS_GetWramMainArenaLo());
    OS_TPrintf("OS_GetWramSubArenaHi()     = 0x%08X\n", OS_GetWramSubArenaHi());
    OS_TPrintf("OS_GetWramSubArenaLo()     = 0x%08X\n", OS_GetWramSubArenaLo());
    OS_TPrintf("OS_GetWramSubPrivArenaHi() = 0x%08X\n", OS_GetWramSubPrivArenaHi());
    OS_TPrintf("OS_GetWramSubPrivArenaLo() = 0x%08X\n", OS_GetWramSubPrivArenaLo());
#endif

    /* FATFSライブラリ用にカレントヒープに設定 */
    {
        OSHeapHandle hh;
        u8     *lo = (u8*)fatfsHeap;
        u8     *hi = (u8*)fatfsHeap + FATFS_HEAP_SIZE;
        lo = OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, lo, hi, 1);
        hh = OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, lo + 32, hi - 32);
        OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
    }

    OS_SetDebugLED(++step);

    if ( FATFS_InitFIRM( &nandContext ) )
    {
#ifndef SDK_FINALROM
        // 3: after FATFS
        profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
        OS_SetDebugLED(++step);

        if ( FATFS_MountDriveFirm( DRIVE_NO, BOOT_DEVICE, PARTITION_NO ) )
        {
            BOOL result;
#ifndef SDK_FINALROM
            // 4: after Mount
            profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
            OS_SetDebugLED(++step);

#ifdef BOOT_SECURE_SRL
            result = FATFS_OpenRecentMenu( DRIVE_NO );
#else
            switch ( PAD_Read() & PAD_KEYPORT_MASK )
            {
            case 0:
                result = FATFS_OpenSpecifiedMenu( MENU_FILE );
                break;
            case PAD_BUTTON_A:
                result = FATFS_OpenSpecifiedMenu( MENU_FILE_A );
                break;
            case PAD_BUTTON_B:
                result = FATFS_OpenSpecifiedMenu( MENU_FILE_B );
                break;
            case PAD_BUTTON_L:
                result = FATFS_OpenSpecifiedMenu( MENU_FILE_L );
                break;
            case PAD_BUTTON_R:
                result = FATFS_OpenSpecifiedMenu( MENU_FILE_R );
                break;
            default:
                OS_SetDebugLED( (u8)(PAD_Read() & PAD_KEYPORT_MASK) );
                OS_Terminate();
                break;
            }
#endif
            if ( result )
            {
#ifndef SDK_FINALROM
                // 5: after Open
                profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
                OS_SetDebugLED(++step);

                if ( FATFS_LoadHeader() && FATFS_LoadMenu() )
                {
#ifndef SDK_FINALROM
                    // 127: before BootMenu
                    pf_cnt = PRFILE_MAX-1;
                    profile[pf_cnt++] = (u32)OS_TicksToMicroSeconds(OS_GetTick());
#endif
                    OS_SetDebugLED(++step);

                    FATFS_BootMenu();
                }
            }
        }
    }

    OS_SetDebugLED( (u8)(0xF0 | step));

    EraseAll();

    // failed
    while (1)
    {
        PXI_NotifyID( FIRM_PXI_ID_NULL );
    }
}

