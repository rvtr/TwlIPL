/*---------------------------------------------------------------------------*
  Project:  TwlIPL - MCU
  File:     mcu.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-09-06$
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef SYSM_MCU_H_
#define SYSM_MCU_H_

#include <twl/mcu.h>

#include <sysmenu/mcu/common/fifo.h>
#ifdef SDK_ARM9
#include <sysmenu/mcu/ARM9/mcu.h>
#else // SDK_ARM7
#include <sysmenu/mcu/ARM7/mcu.h>
#endif // SDK_ARM7

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* SYSM_MCU_H_ */
#endif
