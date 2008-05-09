/*---------------------------------------------------------------------------*
  Project:  TwlSDK - OS - include
  File:     system.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Log: $
  $NoKeywords: $
 *---------------------------------------------------------------------------*/
#ifndef TWL_FIRM_OS_SYSTEM_H_
#define TWL_FIRM_OS_SYSTEM_H_

#include <twl/misc.h>
#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------

typedef u32 OSCpuCycle;

#define OS_CPU_CLOCK                   HW_CPU_CLOCK

//---- sec to cpu cycle
// 150nsec - 30sec
#define  OS_SEC_TO_CPUCYC(  sec  ) ((OSCpuCycle)( ( OS_CPU_CLOCK * (u32)(sec)) ))
#define  OS_MSEC_TO_CPUCYC( msec ) ((OSCpuCycle)( ((OS_CPU_CLOCK/1000) * (u32)(msec)) ))
#define  OS_USEC_TO_CPUCYC( usec ) ((OSCpuCycle)( ((OS_CPU_CLOCK/1000) * (u32)(usec)) / 1000 ))
#define  OS_NSEC_TO_CPUCYC( nsec ) ((OSCpuCycle)( ((OS_CPU_CLOCK/1000) * (u32)(nsec)) / (1000 * 1000) ))

//---- cpu cycle to sec
// 150nsec - 30sec
#define  OS_CPUCYC_TO_SEC(  cyc ) ( ((u32)(cyc) ) / OS_CPU_CLOCK )
#define  OS_CPUCYC_TO_MSEC( cyc ) ( ((u32)(cyc) ) / (OS_CPU_CLOCK/1000) )
#define  OS_CPUCYC_TO_USEC( cyc ) ( ((u32)(cyc) * 1000) / (OS_CPU_CLOCK/1000) )
#define  OS_CPUCYC_TO_NSEC( cyc ) ( ((u32)(cyc) * 1000 * 1000) / (OS_CPU_CLOCK/1000) )



#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_FIRM_OS_SYSTEM_H_ */
#endif
