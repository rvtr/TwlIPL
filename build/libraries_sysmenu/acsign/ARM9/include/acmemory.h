/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     acmemory.h

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

#ifndef _ACMEMORY_H_
#define _ACMEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

//
void    ACMemory_Clear( );
void*   ACMemory_Alloc( u32 size );
void    ACMemory_Free( void* adrs );
void*   ACMemory_Memset( void* adrs, u32 val, u32 cnt );
void*   ACMemory_Memcpy( void* dst, void* src, u32 cnt );

#ifdef __cplusplus
}
#endif

#endif //_ACMEMORY_H_
