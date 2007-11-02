/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fatfs
  File:     fatfs.h

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
#include <twl/aes/ARM7/lo.h>
#include <twl/fatfs.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FATFS_EnableAES

  Description:  enable AES data path

  Arguments:    counter     initial counter value

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_EnableAES( const AESCounter* pCounter );

/*---------------------------------------------------------------------------*
  Name:         FATFS_DisableAES

  Description:  bypass AES

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_DisableAES( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_InitFIRM

  Description:  init file system

  Arguments:    nandContext : nand context to omit initialization (if any)

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_InitFIRM( void* nandContext );

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountDriveFIRM

  Description:  mount nand partition

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_MountDriveFIRM( int driveno, FATFSMediaType media, int partition_no );

#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FATFS_FATFS_FIRM_H_ */
#endif
