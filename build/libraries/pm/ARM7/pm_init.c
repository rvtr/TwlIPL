/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - pm
  File:     pm_init.c

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

#include <firm/pm.h>
#include <twl/spi/common/pm_common.h>

#define  OS_MSEC_TO_CPUCYC( msec ) ((u32)( ((HW_CPU_CLOCK/1000) * (u32)(msec)) ))

/*---------------------------------------------------------------------------*
  Name:         PM_InitFIRM

  Description:  power B/L on

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_InitFIRM( void )
{
    // LED
    PMi_ResetFlags( REG_PMIC_LED_CTL_ADDR, PMIC_LED_CTL_AUTO_BLINK | PMIC_LED_CTL_BLINK_BY_SLEEP );
    PMi_SetParams( REG_PMIC_LVL4_BRT_ADDR,
                   PMIC_LED_1_BRT_LEVEL_4_100  | PMIC_LED_2_BRT_LEVEL_4_100,
                   PMIC_LVL4_BRT_LED_1_MASK | PMIC_LVL4_BRT_LED_2_MASK
                 );
    PMi_SetParams( REG_PMIC_LVL3_BRT_ADDR,
                   PMIC_LED_1_BRT_LEVEL_3_OFF  | PMIC_LED_2_BRT_LEVEL_3_100,
                   PMIC_LVL3_BRT_LED_1_MASK | PMIC_LVL3_BRT_LED_2_MASK
                 );
    PMi_SetParams( REG_PMIC_LVL2_BRT_ADDR,
                   PMIC_LED_1_BRT_LEVEL_2_100  | PMIC_LED_1_BRT_LEVEL_2_OFF,
                   PMIC_LVL2_BRT_LED_1_MASK | PMIC_LVL2_BRT_LED_2_MASK
                 );
    PMi_SetParams( REG_PMIC_LVL1_BRT_ADDR,
                   PMIC_LED_1_BRT_LEVEL_1_OFF  | PMIC_LED_2_BRT_LEVEL_1_OFF,
                   PMIC_LVL1_BRT_LED_1_MASK | PMIC_LVL1_BRT_LED_2_MASK
                 );

    // LCD ON
    PMi_SetFlags( REG_PMIC_CTL2_ADDR, PMIC_CTL2_VDD50 );

    // back light ON
    PMi_SetParams( REG_PMIC_BL_BRT_A_ADDR, PMIC_BACKLIGHT_BRIGHT_MAX, PMIC_BL_BRT_A_MASK ); // TODO: less brightness
    PMi_SetParams( REG_PMIC_BL_BRT_B_ADDR, PMIC_BACKLIGHT_BRIGHT_MAX, PMIC_BL_BRT_B_MASK ); // TODO: less brightness
    OS_SpinWaitCpuCycles( OS_MSEC_TO_CPUCYC( 17*4 ) );
    PMi_SetFlags( REG_PMIC_CTL2_ADDR, PMIC_CTL2_BACK_LIGHT_1 | PMIC_CTL2_BACK_LIGHT_2 );
}
