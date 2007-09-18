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
#include <twl/os/ARM7/debugLED.h>

#define FIRM_ENABLE_JTAG

#define DRIVE_LETTER    'A'                     // マウント先ドライブ名
#define DRIVE_NO        (DRIVE_LETTER - 'A')    // マウント先ドライブ番号
#define PARTITION_NO    0                       // 対象パーティション

void TwlSpMain( void )
{
    OS_TPrintf( "\nNAND Boot time is %d msec.\n", OS_TicksToMilliSecondsBROM32(OS_GetTick()));

//  for normal program
//    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );

//  required here?
//    reg_GX_VRAMCNT_C = REG_GX_VRAMCNT_C_FIELD( TRUE, 0, 0x2);

    OS_InitFIRM();

    OS_InitDebugLED();
    OS_SetDebugLED(0x01);

#ifdef FIRM_ENABLE_JTAG
    reg_SCFG_JTAG = REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK;
#endif // FIRM_ENABLE_JTAG

    PM_InitFIRM();

    OS_SetDebugLED(0x02);

    // load menu
    if ( FATFS_InitFIRM()  &&                           // ARM7側のみ＆NANDのみ
         FATFS_MountNandFirm(DRIVE_NO, PARTITION_NO) && // NAND[0]をX:ドライブにマウント
         FATFS_OpenRecentMenu(DRIVE_NO) )               // 目的のファイルをオープンしてしまう
    {

        OS_SetDebugLED(0x04);

        if ( FATFS_LoadHeader() && FATFS_LoadMenu() )
        {

            OS_SetDebugLED(0x08);

            FATFS_BootMenu();
        }
    }
    else
    {
        PXI_NotifyID( FIRM_PXI_ID_NULL );
    }

    OS_SetDebugLED(0xF0);

    OS_Terminate();
}

