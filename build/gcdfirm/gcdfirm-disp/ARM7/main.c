/*---------------------------------------------------------------------------*
  Project:  TwlFirm - tools - firm_writer_gcd
  File:     main.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <firm.h>
#include <twl/mcu.h>

//#define PRINT_DEBUG

#ifndef PRINT_DEBUG
#undef  OS_TPrintf
#undef  OS_PutChar
#define OS_TPrintf(...) ((void)0)
#define OS_PutChar(...) ((void)0)
#endif // PRINT_DEBUG

void TwlSpMain( void )
{
#ifdef PRINT_DEBUG
    reg_SCFG_JTAG = REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK;
#endif // PRINT_DEBUG

    MIi_CpuClearFast( 0, (void*)OSi_GetFromBromAddr(), sizeof(OSFromBromBuf) );

    OS_InitFIRM();

    OS_TPrintf( "\nARM7 starts.\n" );

    OS_EnableInterrupts();
    OS_EnableIrq();

    PM_InitFIRM();
#if SDK_TS_VERSION < 300
    PMi_SetParams( REG_PMIC_BL_BRT_A_ADDR, PMIC_BACKLIGHT_BRIGHT_DEFAULT, PMIC_BL_BRT_A_MASK );
    PMi_SetParams( REG_PMIC_BL_BRT_B_ADDR, PMIC_BACKLIGHT_BRIGHT_DEFAULT, PMIC_BL_BRT_B_MASK );
#else
    MCUi_WriteRegister( MCU_REG_BL_ADDR, MCU_REG_BL_BRIGHTNESS_MASK );
#endif
    PM_BackLightOn( TRUE );

    // ƒ{ƒ^ƒ“‚ª‰Ÿ‚³‚ê‚é‚Ü‚Å‘Ò‚Â
    OS_TPrintf( "\nPress A button.\n");
    while ( !(PAD_Read() & PAD_BUTTON_A) )
    {
    }

    OS_TPrintf( "\nARM7 ends.\n" );
    (void)OS_DisableIrq();
//    OSi_Finalize();
#ifdef PRINT_DEBUG
    reg_SCFG_JTAG = REG_SCFG_JTAG_CPUJE_MASK | REG_SCFG_JTAG_ARM7SEL_MASK;
#endif // PRINT_DEBUG
    while (1)
    {
    }
//    OS_Terminate();
}


