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

/*---------------------------------------------------------------------------*
  Name:         PM_InitFIRM

  Description:  set default parameters

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_InitFIRM( void )
{
#ifndef PMIC_FINAL
    // LED
    PMi_ResetFlags( REG_PMIC_LED_CTL_ADDR, PMIC_LED_CTL_AUTO_BLINK | PMIC_LED_CTL_BLINK_BY_SLEEP );
    PMi_SetParams( REG_PMIC_LVL4_BRT_ADDR,
                   PMIC_LED_1_BRT_LEVEL_4_OFF  | PMIC_LED_2_BRT_LEVEL_4_OFF,
                   PMIC_LVL4_BRT_LED_1_MASK | PMIC_LVL4_BRT_LED_2_MASK
                 );
    PMi_SetParams( REG_PMIC_LVL3_BRT_ADDR,
                   PMIC_LED_1_BRT_LEVEL_3_100  | PMIC_LED_2_BRT_LEVEL_3_OFF,
                   PMIC_LVL3_BRT_LED_1_MASK | PMIC_LVL3_BRT_LED_2_MASK
                 );
    PMi_SetParams( REG_PMIC_LVL2_BRT_ADDR,
                   PMIC_LED_1_BRT_LEVEL_2_OFF  | PMIC_LED_1_BRT_LEVEL_2_100,
                   PMIC_LVL2_BRT_LED_1_MASK | PMIC_LVL2_BRT_LED_2_MASK
                 );
    PMi_SetParams( REG_PMIC_LVL1_BRT_ADDR,
                   PMIC_LED_1_BRT_LEVEL_1_100  | PMIC_LED_2_BRT_LEVEL_1_100,
                   PMIC_LVL1_BRT_LED_1_MASK | PMIC_LVL1_BRT_LED_2_MASK
                 );

    // correct battery LED curve
    PMi_SetFlags( REG_PMIC_VLBAT_CTL_ADDR, PMIC_VLBAT_CTL_VLBAT_2_ACTIVE | PMIC_VLBAT_CTL_VLBAT_3_ACTIVE );

    // LCD ON
    PMi_SetFlags( REG_PMIC_CTL2_ADDR, PMIC_CTL2_VDD50 );

    // back light level does not set
#endif
}

/*---------------------------------------------------------------------------*
  Name:         PM_BackLightOn

  Description:  power B/L on if not set yet

  Arguments:    force       TRUE: wait until valid condition
                            FALSE not set unless valid condition

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_BackLightOn( BOOL force )
{
    static BOOL doneBackLight = FALSE;

    if ( doneBackLight )
    {
        return; // have already set
    }
    if ( force )
    {
        while ( (reg_GX_DISPSTAT & REG_GX_DISPSTAT_INI_MASK) == FALSE )
        {
        }
    }
    if ( reg_GX_DISPSTAT & REG_GX_DISPSTAT_INI_MASK )
    {
        PMi_SetFlags( REG_PMIC_CTL_ADDR, PMIC_CTL_BKLT1 | PMIC_CTL_BKLT2 );
        doneBackLight = TRUE;
    }
}
