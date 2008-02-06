/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - norfirm-print
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-11#$
  $Rev: 22 $
  $Author: yutaka $
 *---------------------------------------------------------------------------*/
#include <firm.h>
#include "reboot.h"

//#define PRINT_DEBUG

#ifndef PRINT_DEBUG
#undef  OS_TPrintf
#define OS_TPrintf(...) ((void)0)
#endif // PRINT_DEBUG

void TwlSpMain( void )
{
    OS_TPrintf( "\nNOR Boot time is %d msec.\n", OS_TicksToMilliSecondsBROM32(OS_GetTick()));

//    MIi_CpuClearFast( 0, (void*)HW_TWL_ROM_HEADER_BUF, HW_MAIN_MEM_SYSTEM_END - HW_TWL_ROM_HEADER_BUF );  // include HW_MAIN_MEM_SHARED
    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );

    OS_InitFIRM();

    OS_TPrintf( "\nARM7 starts.\n" );

    OS_EnableInterrupts();
    OS_EnableIrq();

    PM_InitFIRM();
    PMi_SetParams( REG_PMIC_BL_BRT_A_ADDR, PMIC_BACKLIGHT_BRIGHT_MAX, PMIC_BL_BRT_A_MASK );
    PMi_SetParams( REG_PMIC_BL_BRT_B_ADDR, PMIC_BACKLIGHT_BRIGHT_MAX, PMIC_BL_BRT_B_MASK );
    PM_BackLightOn( TRUE );

    OS_TPrintf( "\nARM7 ends.\n" );

    REBOOT_DisableInterruptsAndProtectionUnit();
    reg_SCFG_JTAG = REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK | REG_SCFG_JTAG_DSPJE_MASK;
    while (1)
    {
    }
//    OS_Terminate();
}

