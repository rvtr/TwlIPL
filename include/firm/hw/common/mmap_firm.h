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
#include <twl/fatfs/common/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HW_FIRM_ROUNDUP32(x)    ( ((x) + 31) & -32 )

//------------------------------------- *_LoadBuffer
#define HW_FIRM_LOAD_BUFFER_BASE        MI_GetWramMapStart_B()
#define HW_FIRM_LOAD_BUFFER_UNIT_SIZE   0x8000
#define HW_FIRM_LOAD_BUFFER_UNIT_NUMS   7
#define HW_FIRM_LOAD_BUFFER_SIZE        (HW_FIRM_LOAD_BUFFER_UNIT_SIZE * HW_FIRM_LOAD_BUFFER_UNIT_NUMS)
#define HW_FIRM_LOAD_BUFFER_END         (HW_FIRM_LOAD_BUFFER_BASE + HW_FIRM_LOAD_BUFFER_SIZE)

//------------------------------------- FS/FATFS
#define HW_FIRM_FS_MOUNT_INFO_BUF_SIZE      (HW_TWL_FS_BOOT_SRL_PATH_BUF - HW_TWL_FS_MOUNT_INFO_BUF)
#define HW_FIRM_FS_BOOT_SRL_PATH_BUF_SIZE   (HW_TWL_ROM_HEADER_BUF - HW_TWL_FS_BOOT_SRL_PATH_BUF)

#define HW_FIRM_FATFS_ARCHNAME_LIST         (HW_FIRM_FATFS_ARCHNAME_LIST_END - HW_FIRM_FATFS_ARCHNAME_LIST_SIZE)    // 0x02fee120
#define HW_FIRM_FATFS_ARCHNAME_LIST_SIZE    MATH_ROUNDUP(OS_MOUNT_ARCHIVE_NAME_LEN * OS_MOUNT_INFO_MAX + 1, 32) // 0xc0
#define HW_FIRM_FATFS_ARCHNAME_LIST_END     HW_FIRM_FATFS_COMMAND_BUFFER    // 0x2fee1e0

#define HW_FIRM_FATFS_COMMAND_BUFFER        (HW_FIRM_FATFS_COMMAND_BUFFER_END - HW_FIRM_FATFS_COMMAND_BUFFER_SIZE)
#define HW_FIRM_FATFS_COMMAND_BUFFER_SIZE   FATFS_COMMAND_BUFFER_MAX    // 0x1000
#define HW_FIRM_FATFS_COMMAND_BUFFER_END    HW_FIRM_FS_FATFS_DRIVE  // 0x2fef1e0

#define HW_FIRM_FS_FATFS_DRIVE              (HW_FIRM_FS_FATFS_DRIVE_END - HW_FIRM_FS_FATFS_DRIVE_SIZE)
#define HW_FIRM_FS_FATFS_DRIVE_SIZE         HW_FIRM_ROUNDUP32(sizeof(FSFATFSArchiveContext[FS_MOUNTDRIVE_MAX])) // 0x1cc0
#define HW_FIRM_FS_FATFS_DRIVE_END          HW_FIRM_FS_FATFS_ASYNC_REQUEST  // 0x02ff0ea0

#define HW_FIRM_FS_FATFS_ASYNC_REQUEST      (HW_FIRM_FS_FATFS_ASYNC_REQUEST_END - HW_FIRM_FS_FATFS_ASYNC_REQUEST_SIZE)
#define HW_FIRM_FS_FATFS_ASYNC_REQUEST_SIZE HW_FIRM_ROUNDUP32(sizeof(FATFSRequestBuffer[FS_MOUNTDRIVE_MAX]))    // 0x5960
#define HW_FIRM_FS_FATFS_ASYNC_REQUEST_END  HW_FIRM_FS_TEMP_BUFFER      // 0x02ff6800

#define HW_FIRM_FS_TEMP_BUFFER              (HW_FIRM_FS_TEMP_BUFFER_END - HW_FIRM_FS_TEMP_BUFFER_SIZE)
#define HW_FIRM_FS_TEMP_BUFFER_SIZE         FS_TEMPORARY_BUFFER_MAX // 0x5800
#define HW_FIRM_FS_TEMP_BUFFER_END          HW_TWL_MAIN_MEM_SHARED  // 0x02ffc000

#ifdef __cplusplus
} /* extern "C" */
#endif

/* FIRM_HW_COMMON_MMAP_FIRM_H_ */
#endif
