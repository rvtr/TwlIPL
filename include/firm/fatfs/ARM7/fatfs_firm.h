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

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FATFS_EnableAES

  Description:  enable AES data path

  Arguments:    slot        aes key slot number
                counter     initial counter value

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_EnableAES( AESKeySlot slot, const AESCounter* pCounter );

/*---------------------------------------------------------------------------*
  Name:         FATFS_DisableAES

  Description:  bypass AES

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_DisableAES( void );

/*---------------------------------------------------------------------------*
  Name:         nandRead

  Description:  normal read

  Arguments:    block: source sector number in NAND
                dest: dest address (4 bytes alignment)
                count: sectors to transfer

  Returns:      None
 *---------------------------------------------------------------------------*/
void nandRead(u32 block, void *dest, u16 count);

/*---------------------------------------------------------------------------*
  Name:         nandReadAES

  Description:  AES read

  Arguments:    block: source sector number in NAND
                dest: dest address (4 bytes alignment)
                count: sectors to transfer

  Returns:      None
 *---------------------------------------------------------------------------*/
void nandReadAES(u32 block, void *dest, u16 count);

/*---------------------------------------------------------------------------*
  Name:         FATFS_InitFIRM

  Description:  init file system

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_InitFIRM( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_MountNandFirm

  Description:  mount nand partition

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_MountNandFirm( int driveno, int partition_no );

#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FATFS_FATFS_FIRM_H_ */
#endif
