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
#ifndef SYSM_MCU_ARM9_H_
#define SYSM_MCU_ARM9_H_

#include <twl/mcu.h>
#include <twl/mcu/ARM7/mcu_reg.h>
#include "../common/fifo.h"

/*---------------------------------------------------------------------------*
    íËêîíËã`
 *---------------------------------------------------------------------------*/

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum SYSMMcuResult
{
    MCU_RESULT_SUCCESS = 0,
    MCU_RESULT_SUCCESS_TRUE = 0,
    MCU_RESULT_SUCCESS_FALSE,
    MCU_RESULT_BUSY,
    MCU_RESULT_ILLEGAL_PARAMETER,
    MCU_RESULT_SEND_ERROR,
    MCU_RESULT_INVALID_COMMAND,
    MCU_RESULT_ILLEGAL_STATUS,
    MCU_RESULT_FATAL_ERROR,
    MCU_RESULT_MAX
}
SYSMMcuResult;

typedef void (*SYSMMcuCallback)(SYSMMcuResult result, void *arg);

/*===========================================================================*/

void SYSM_InitMcuPxi( void );
u8 SYSMi_GetMcuVersion( void );

SYSMMcuResult SYSM_ReadMcuRegisterAsync( u8 addr, u8* pValue, SYSMMcuCallback callback, void* arg );
SYSMMcuResult SYSM_WriteMcuRegisterAsync( u8 addr, u8 value, SYSMMcuCallback callback, void* arg );
void SYSM_SetMcuInterruptHandler( SYSMMcuCallback handler );

/*===========================================================================*/


#ifdef  __cplusplus
}          /* extern "C" */
#endif

#endif /* SYSM_MCU_ARM9_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
