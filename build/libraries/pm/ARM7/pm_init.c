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
    PMi_ResetFlags( REG_PMIC_LED_CTL_ADDR, PMIC_LED_CTL_L12_AT_BLK | PMIC_LED_CTL_L12_BLK_BY_SLP );
    PMi_SetParams( REG_PMIC_LED12_B4_ADDR,
                   PMIC_LED12_B4_L1_100  | PMIC_LED12_B4_L2_100,
                   PMIC_LED12_B4_L1_MASK | PMIC_LED12_B4_L2_MASK
                 );
    PMi_SetParams( REG_PMIC_LED12_B3_ADDR,
                   PMIC_LED12_B3_L1_OFF  | PMIC_LED12_B3_L2_100,
                   PMIC_LED12_B3_L1_MASK | PMIC_LED12_B3_L2_MASK
                 );
    PMi_SetParams( REG_PMIC_LED12_B2_ADDR,
                   PMIC_LED12_B2_L1_100  | PMIC_LED12_B2_L2_OFF,
                   PMIC_LED12_B2_L1_MASK | PMIC_LED12_B2_L2_MASK
                 );
    PMi_SetParams( REG_PMIC_LED12_B1_ADDR,
                   PMIC_LED12_B1_L1_OFF  | PMIC_LED12_B1_L2_OFF,
                   PMIC_LED12_B1_L1_MASK | PMIC_LED12_B1_L2_MASK
                 );

    // LCD ON
    PMi_SetFlags( REG_PMIC_CTL2_ADDR, PMIC_CTL2_LCD_PWR );

    // back light ON
    PMi_SetParams( REG_PMIC_BL1_BRT_ADDR, PMIC_BL_BRT_MAX, PMIC_BL1_BRT_MASK );
    PMi_SetParams( REG_PMIC_BL2_BRT_ADDR, PMIC_BL_BRT_MAX, PMIC_BL2_BRT_MASK );
    OS_SpinWait( OS_MSEC_TO_CPUCYC( 17*2 ) );
    PMi_SetFlags( REG_PMIC_CTL2_ADDR, PMIC_CTL2_BKLT1 | PMIC_CTL2_BKLT2 );
}
