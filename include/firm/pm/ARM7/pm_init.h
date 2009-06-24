/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - pm
  File:     pm_init.h

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

#ifndef FIRM_PM_ARM7_PM_INIT_H_
#define FIRM_PM_ARM7_PM_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         PM_InitFIRM

  Description:  power B/L on

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_InitFIRM( void );

/*---------------------------------------------------------------------------*
  Name:         PM_BackLightOn

  Description:  power B/L on if not set yet

  Arguments:    force       TRUE: wait until valid condition
                            FALSE not set unless valid condition

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_BackLightOn( BOOL force );

/*---------------------------------------------------------------------------*
  Name:         PM_Shutdown

  Description:  shutdown

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void PM_Shutdown( void );

#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_PM_ARM7_PM_INIT_H_ */
#endif
