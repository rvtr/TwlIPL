/*---------------------------------------------------------------------------*
  Project:  TwlFirm - NVRAM
  File:     nvram.h

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

#ifndef FIRM_NVRAM_H_
#define FIRM_NVRAM_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef  SDK_ARM9
#else  // SDK_ARM7

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/
void NVRAMi_Read(u32 address, void *buf, u32 size);
void NVRAMi_Write(u32 address, void *buf, u32 size);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_NVRAM_H_ */
#endif
