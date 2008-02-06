/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fs
  File:     fs.h

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

#ifndef FIRM_FS_FS_LOADER_H_
#define FIRM_FS_FS_LOADER_H_

#include <twl/types.h>
#include <twl/aes/ARM7/lo.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FS_LoadBuffer

  Description:  load data in file and pass to ARM9 via WRAM-B

  Arguments:    fd              file discriptor to read
                offset          offset to start to read in bytes
                size            total length to read in bytes

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadBuffer( int fd, u32 offset, u32 size );

/*---------------------------------------------------------------------------*
  Name:         FS_LoadModule

  Description:  load data in file and pass to ARM9 via WRAM-B in view of AES
                settings in the ROM header at HW_TWL_ROM_HEADER_BUF

  Arguments:    fd              file discriptor to read
                offset          offset to start to read in bytes
                size            total length to read in bytes

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadModule( int fd, u32 offset, u32 size );

/*---------------------------------------------------------------------------*
  Name:         FS_LoadHeader

  Description:  load ROM header in the head of file and pass to ARM9 via WRAM-B

  Arguments:    fd              file discriptor to read

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadHeader( int fd );

/*---------------------------------------------------------------------------*
  Name:         FS_LoadStatic

  Description:  load static regions in file and pass to ARM9 via WRAM-B
                specified by ROM header at HW_TWL_ROM_HEADER_BUF

  Arguments:    fd              file discriptor to read

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FS_LoadStatic( int fd );


#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FS_FS_LOADER_H_ */
#endif
