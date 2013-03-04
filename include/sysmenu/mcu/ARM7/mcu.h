/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mcu.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef SYSM_MCU_ARM7_H_
#define SYSM_MCU_ARM7_H_

/*---------------------------------------------------------------------------*
    íËêîíËã`
 *---------------------------------------------------------------------------*/

#ifdef  __cplusplus
extern "C" {
#endif

/*===========================================================================*/

void SYSM_InitMcuPxi( u32 prio );
u8 SYSMi_GetMcuVersion( void );

/*===========================================================================*/


#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* SYSM_MCU_ARM7_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
