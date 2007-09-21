/*---------------------------------------------------------------------------*
  Project:  TwlIPL - include - mi
  File:     mi_loader.h

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

#ifndef FIRM_MI_LOADER_H_
#define FIRM_MI_LOADER_H_

#include <twl/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Name:         MI_LoadHeader

  Description:  load header

  Arguments:    pool        pointer to the pool info for SVC_DecryptoSign
                rsa_key     key address

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadHeader( int* pool, const void* rsa_key );

/*---------------------------------------------------------------------------*
  Name:         MI_LoadMenu

  Description:  load menu program

  Arguments:    None

  Returns:      TRUE if success
 *---------------------------------------------------------------------------*/
BOOL MI_LoadMenu( void );

/*---------------------------------------------------------------------------*
  Name:         MI_BootMenu

  Description:  boot menu

  Arguments:    None

  Returns:      None
 *---------------------------------------------------------------------------*/
void MI_BootMenu( void );

#ifdef __cplusplus
} /* extern "C" */
#endif


/* FIRM_MI_LOADER_H_ */
#endif
