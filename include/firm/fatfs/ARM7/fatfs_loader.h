/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - fatfs
  File:     fatfs_loader.h

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

#ifndef FIRM_FATFS_FATFS_LOADER_H_
#define FIRM_FATFS_FATFS_LOADER_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenRecentMenu

  Description:  open recent menu file

  Arguments:    driveno     drive number ('A' is 0)

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenRecentMenu( int driveno );

/*---------------------------------------------------------------------------*
  Name:         FATFS_OpenSpecifiedMenu

  Description:  open specified menu file

  Arguments:    menufile    target filename

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL FATFS_OpenSpecifiedMenu( const char* menufile );

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadHeader

  Description:  load menu header

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadHeader( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_LoadMenu

  Description:  load menu binary

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL FATFS_LoadMenu( void );

/*---------------------------------------------------------------------------*
  Name:         FATFS_BootMenu

  Description:  boot menu

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void FATFS_BootMenu( void );


#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_FATFS_FATFS_LOADER_H_ */
#endif
