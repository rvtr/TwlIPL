/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     mainFunc.c

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

#ifndef CMN_H_
#define CMN_H_


#include <nitro/types.h>
#include <nitro/os/common/interrupt.h>
#include <nitro/os/common/arena.h>
#include <nitro/os/common/alloc.h>
#include <nnsys/fnd/allocator.h>
#include <nnsys/fnd/archive.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data---------------------------------------------
#define ARY_SIZEOF(ary)     ( sizeof(ary) / sizeof( (ary)[0] ) )
#define ROUNDUP_DIV(a, b)   (( (a) + ((b) - 1) ) / (b))


// function------------------------------------------------
void CMN_InitAllocator( NNSFndAllocator* pAllocator );
void CMN_InitFileSystem( NNSFndAllocator* pAllocator );
void CMN_ClearVram( void );
u32  CMN_LoadFile(void** ppFile, const char* fpath, NNSFndAllocator* pAlloc);
void CMN_UnloadFile(void* pFile, NNSFndAllocator* pAlloc);
NNSFndArchive* CMN_LoadArchive(const char* name, const char* path, NNSFndAllocator* pAllocator);
void CMN_RemoveArchive(NNSFndArchive* archive, NNSFndAllocator* pAllocator);


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // CMN_H_

