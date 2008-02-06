/*---------------------------------------------------------------------------*
  Project:  TwlIPL - libraries - pm
  File:     pm_pmic_ex.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#include <firm/pm.h>
#include <pm_pmic.h>

//================================================================================
//        PMIC BIT CONTROL
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SetParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_SetParams( u8 reg, u8 setBits, u8 maskBits )
{
    u8      tmp;
    tmp = PMi_GetRegister( reg );
    tmp &= ~maskBits;
    setBits &= maskBits;
    tmp |= setBits;
    PMi_SetRegister( reg, tmp );
}

/*---------------------------------------------------------------------------*
  Name:         PMi_SetFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_SetFlags( u8 reg, u8 setBits )
{
    PMi_SetParams( reg, setBits, setBits );
}

/*---------------------------------------------------------------------------*
  Name:         PMi_ResetFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                clrBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_ResetFlags( u8 reg, u8 clrBits )
{
    PMi_SetParams( reg, 0, clrBits );
}

