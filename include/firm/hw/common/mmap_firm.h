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

#include <nitro/fs/api.h>
#include <twl/fatfs/common/api.h>

#ifdef __cplusplus
extern "C" {
#endif

//------------------------------------- *_LoadBuffer
#define HW_FIRM_LOAD_BUFFER_BASE        MI_GetWramMapStart_B()
#define HW_FIRM_LOAD_BUFFER_UNIT_SIZE   0x8000
#define HW_FIRM_LOAD_BUFFER_UNIT_NUMS   8
#define HW_FIRM_LOAD_BUFFER_SIZE        (HW_FIRM_LOAD_BUFFER_UNIT_SIZE * HW_FIRM_LOAD_BUFFER_UNIT_NUMS)
#define HW_FIRM_LOAD_BUFFER_END         (HW_FIRM_LOAD_BUFFER_BASE + HW_FIRM_LOAD_BUFFER_SIZE)

//------------------------------------- FS/FATFS
#define HW_FIRM_FATFS_COMMAND_BUFFER        (HW_FIRM_FATFS_COMMAND_BUFFER_END - HW_FIRM_FATFS_COMMAND_BUFFER_SIZE)
#define HW_FIRM_FATFS_COMMAND_BUFFER_SIZE   FATFS_COMMAND_BUFFER_MAX    // 0x800
#define HW_FIRM_FATFS_COMMAND_BUFFER_END    HW_FIRM_FS_TWMP_BUFFER      // 0x02ff8000

#define HW_FIRM_FS_TWMP_BUFFER              (HW_FIRM_FS_TWMP_BUFFER_END - HW_FIRM_FS_TWMP_BUFFER_SIZE)
#define HW_FIRM_FS_TWMP_BUFFER_SIZE         FS_TEMPORARY_BUFFER_MAX // 0x4000
#define HW_FIRM_FS_TWMP_BUFFER_END          HW_TWL_MAIN_MEM_SHARED  // 0x02ffc000

#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_HW_COMMON_MMAP_FIRM_H_ */
#endif
