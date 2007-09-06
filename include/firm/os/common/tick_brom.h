/*---------------------------------------------------------------------------*
  Project:  TwlBromSDK - OS - include
  File:     tick_addin.h

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

#ifndef FRIM_OS_TICK_BROM_H_
#define FRIM_OS_TICK_BROM_H_

#ifdef __cplusplus
extern "C" {
#endif

//---- sec to tick
#define  OS_MicroSecondsToTicksBROM(   usec ) ((OSTick)( ((OS_SYSTEM_CLOCK/1000) * (u64)(usec)) / 1024 / 1000 ))
#define  OS_MicroSecondsToTicksBROM32( usec ) ((OSTick)( ((OS_SYSTEM_CLOCK/1000) * (u32)(usec)) / 1024 / 1000 ))

#define  OS_MilliSecondsToTicksBROM(   msec ) ((OSTick)( ((OS_SYSTEM_CLOCK/1000) * (u64)(msec)) / 1024 ))
#define  OS_MilliSecondsToTicksBROM32( msec ) ((OSTick)( ((OS_SYSTEM_CLOCK/1000) * (u32)(msec)) / 1024 ))

#define  OS_SecondsToTicksBROM(         sec ) ((OSTick)( (OS_SYSTEM_CLOCK * (u64)(sec)) / 1024 ))
#define  OS_SecondsToTicksBROM32(       sec ) ((OSTick)( (OS_SYSTEM_CLOCK * (u32)(sec)) / 1024 ))

//---- tick to sec
#define  OS_TicksToMicroSecondsBROM(   tick ) ( ((u64)(tick) * 1024 * 1000) / (OS_SYSTEM_CLOCK/1000) )
#define  OS_TicksToMicroSecondsBROM32( tick ) ( ((u32)(tick) * 1024 * 1000) / (OS_SYSTEM_CLOCK/1000) )

#define  OS_TicksToMilliSecondsBROM(   tick ) ( ((u64)(tick) * 1024) / (OS_SYSTEM_CLOCK/1000) )
#define  OS_TicksToMilliSecondsBROM32( tick ) ( ((u32)(tick) * 1024) / (OS_SYSTEM_CLOCK/1000) )

#define  OS_TicksToSecondsBROM(        tick ) ( ((u64)(tick) * 1024) / OS_SYSTEM_CLOCK )
#define  OS_TicksToSecondsBROM32(      tick ) ( ((u32)(tick) * 1024) / OS_SYSTEM_CLOCK )


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FRIM_OS_TICK_BROM_H_ */
#endif
