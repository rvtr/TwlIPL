/*---------------------------------------------------------------------------*
  Project:  TwlFirm - OS - include 
  File:     os_cache_tag.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef TWL_OS_CACHE_TAG_H_
#define TWL_OS_CACHE_TAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <twl/misc.h>
#include <twl/types.h>

//===========================================================================
//              DATA CACHE (for specified range)
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         DC_ClearTagAll

  Description:  clear tag in data cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_ClearTagAll( void );

/*---------------------------------------------------------------------------*
  Name:         DC_ClearDataAll

  Description:  clear data in data cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_ClearDataAll( void );

/*---------------------------------------------------------------------------*
  Name:         DC_FillTagAll

  Description:  clear tag in data cache

  Arguments:    data :  fill data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_FillTagAll( u32 data );

/*---------------------------------------------------------------------------*
  Name:         DC_FillDataAll

  Description:  fill data in data cache

  Arguments:    data :  fill data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_FillDataAll( u32 data );

/*---------------------------------------------------------------------------*
  Name:         DC_GetTagAndDataAll

  Description:  get tag and data in data cache

  Arguments:    tag           tag address
                data          data address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    DC_GetTagAndDataAll( void* tag, void* data );

//===========================================================================
//              INSTRUCTION CACHE
//===========================================================================
/*---------------------------------------------------------------------------*
  Name:         IC_ClearTagAll

  Description:  clear tag in instruction cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_ClearTagAll( void );

/*---------------------------------------------------------------------------*
  Name:         IC_ClearInstructionAll

  Description:  clear instruction in instruction cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_ClearInstructionAll( void );

/*---------------------------------------------------------------------------*
  Name:         IC_FillTagAll

  Description:  fill tag in instruction cache

  Arguments:    None

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_FillTagAll( u32 data );

/*---------------------------------------------------------------------------*
  Name:         IC_FillInstructionAll

  Description:  fill instruction in instruction cache

  Arguments:    data :  fill data

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_FillInstructionAll( u32 data );

/*---------------------------------------------------------------------------*
  Name:         IC_GetTagAndInstructionAll

  Description:  get tag and instruction in instruction cache

  Arguments:    tag           tag address
                inst          instruction address

  Returns:      None.
 *---------------------------------------------------------------------------*/
void    IC_GetTagAndInstructionAll( void* tag, void* inst );

#ifdef __cplusplus
} /* extern "C" */
#endif

/* TWL_OS_CACHE_TAG_H_ */
#endif
