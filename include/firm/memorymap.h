/*---------------------------------------------------------------------------*
  Project:  TwlFirm - include - HW
  File:     memorymap.h

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
#ifndef FIRM_MEMORYMAP_H_
#define FIRM_MEMORYMAP_H_

#include    <twl/memorymap.h>

#ifdef  SDK_ARM9
#include    <firm/hw/ARM9/mmap_firm.h>
#else  //SDK_ARM7
#include    <firm/hw/ARM7/mmap_firm.h>
#endif

/* FIRM_MEMORYMAP_H_ */
#endif
