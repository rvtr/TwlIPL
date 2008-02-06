/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - pm
  File:     pm_pmic_ex.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef FIRM_PM_PMIC_EX_H_
#define FIRM_PM_PMIC_EX_H_

#include <firm/misc.h>
#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

//================================================================================
//        PMIC ACCESS
//================================================================================
/*---------------------------------------------------------------------------*
  Name:         PMi_SetParams

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set
                maskBits : bits to mask

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_SetParams( u8 reg, u8 setBits, u8 maskBits );

/*---------------------------------------------------------------------------*
  Name:         PMi_SetFlags

  Description:  set control bit to device register

  Arguments:    reg      : device register
                setBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_SetFlags( u8 reg, u8 setBits );

/*---------------------------------------------------------------------------*
  Name:         PMi_ResetFlags

  Description:  clear control bit to device register

  Arguments:    reg      : device register
                clrBits  : bits to set

  Returns:      None
 *---------------------------------------------------------------------------*/
void PMi_ResetFlags( u8 reg, u8 clrBits );


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_PM_PMIC_EX_H_ */
#endif
