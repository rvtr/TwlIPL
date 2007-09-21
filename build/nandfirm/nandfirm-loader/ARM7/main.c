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
#include <firm/fatfs.h>
#include <twl/fatfs.h>
#include <twl/os/ARM7/debugLED.h>

//#define BOOT_SECURE_SRL   // 本番SRLをブートするときにだけ定義する
#define FIRM_ENABLE_JTAG

#define FATFS_HEAP_SIZE     (64*1024)

#ifndef BOOT_SECURE_SRL
#define BOOT_DEVICE     FATFS_MEDIA_TYPE_SD
#define PARTITION_NO    0                       // 0固定
#define MENU_FILE       (char*)L"A:\\menu.srl"          // 対象ファイル(DRIVE_LETTERと合わせること)
#define MENU_FILE_A     "A:\\menu_a.srl"        // 対象ファイル(DRIVE_LETTERと合わせること)
#define MENU_FILE_B     "A:\\menu_b.srl"        // 対象ファイル(DRIVE_LETTERと合わせること)
#define MENU_FILE_L     "A:\\menu_l.srl"        // 対象ファイル(DRIVE_LETTERと合わせること)
#define MENU_FILE_R     "A:\\menu_r.srl"        // 対象ファイル(DRIVE_LETTERと合わせること)
#else
#define BOOT_DEVICE     FATFS_MEDIA_TYPE_NAND
#define PARTITION_NO    0                       // 対象パーティション
#endif

#define DRIVE_LETTER    'A'                     // マウント先ドライブ名
#define DRIVE_NO        (DRIVE_LETTER - 'A')    // マウント先ドライブ番号

static u64 fatfs_arena[FATFS_HEAP_SIZE/sizeof(u64)];

#ifndef SDK_FINALROM
static u8 step = 0;
#endif

void TwlSpMain( void )
{
//    OS_TPrintf( "\nNAND Boot time is %d msec.\n", OS_TicksToMilliSecondsBROM32(OS_GetTick()));

#ifndef BOOT_SECURE_SRL
    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );
#endif

#ifdef FIRM_ENABLE_JTAG
    reg_SCFG_JTAG = REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK;
#endif // FIRM_ENABLE_JTAG

    OS_InitFIRM();

    OS_InitDebugLED();
    OS_SetDebugLED(++step);

    PM_InitFIRM();

    OS_SetDebugLED(++step);

    /* FATFSライブラリ用にカレントヒープに設定 */
    {
        OSHeapHandle hh;
        u8     *lo = (u8*)fatfs_arena;
        u8     *hi = (u8*)fatfs_arena + FATFS_HEAP_SIZE;
        OS_SetSubPrivArenaLo(lo);
        OS_SetSubPrivArenaHi(hi);
        OS_SetSubPrivArenaLo(lo = OS_InitAlloc(OS_ARENA_MAIN_SUBPRIV, lo, hi, 1));
        hh = OS_CreateHeap(OS_ARENA_MAIN_SUBPRIV, lo + 32, hi - 32);
        OS_SetCurrentHeap(OS_ARENA_MAIN_SUBPRIV, hh);
    }

    OS_SetDebugLED(++step);

    if ( FATFS_InitFIRM() )
    {
        OS_SetDebugLED(++step);

        if ( FATFS_MountDriveFirm( DRIVE_NO, BOOT_DEVICE, PARTITION_NO ) )
        {
            BOOL result;
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
                OS_SetDebugLED(++step);

                if ( FATFS_LoadHeader() && FATFS_LoadMenu() )
                {
                    OS_SetDebugLED(++step);

                    FATFS_BootMenu();
                }
            }
        }
    }

    // failed
    PXI_NotifyID( FIRM_PXI_ID_NULL );

    OS_SetDebugLED( (u8)(0xF0 | ++step));

    OS_Terminate();
}

