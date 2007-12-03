/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fatfs
  File:     fatfs_firm.h

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

#ifndef FIRM_FATFS_FATFS_FIRM_H_
#define FIRM_FATFS_FATFS_FIRM_H_

#include <twl/types.h>
#include <twl/fatfs/common/api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FIRM_FATFS_COMMAND_BUFFER       (HW_MAIN_MEM + 0x00ffc000)
#define FIRM_FATFS_COMMAND_BUFFER_SIZE  FATFS_COMMAND_BUFFER_MAX
#define FIRM_FATFS_COMMAND_BUFFER_END   (FIRM_FATFS_COMMAND_BUFFER + FIRM_FATFS_COMMAND_BUFFER_SIZE)

/*---------------------------------------------------------------------------*
  Name:         FATFS_InitFIRM

  Description:  initialize fatfs for firm

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_InitFIRM( void );

#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FATFS_FATFS_FIRM_H_ */
#endif
