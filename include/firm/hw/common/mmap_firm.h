/*---------------------------------------------------------------------------*
  Project:  TwlFirm - HW - include
  File:     mmap_firm.h

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
#ifndef FIRM_HW_COMMON_MMAP_FIRM_H_
#define FIRM_HW_COMMON_MMAP_FIRM_H_

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------- *_LoadBuffer
#define HW_FIRM_LOAD_BUFFER_BASE        MI_GetWramMapStart_B()
#define HW_FIRM_LOAD_BUFFER_UNIT_SIZE   0x8000
#define HW_FIRM_LOAD_BUFFER_UNIT_NUMS   8
#define HW_FIRM_LOAD_BUFFER_SIZE        (HW_FIRM_LOAD_BUFFER_UNIT_SIZE * HW_FIRM_LOAD_BUFFER_UNIT_NUMS)
#define HW_FIRM_LOAD_BUFFER_END         (HW_FIRM_LOAD_BUFFER_BASE + HW_FIRM_LOAD_BUFFER_SIZE)


#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_HW_COMMON_MMAP_FIRM_H_ */
#endif
