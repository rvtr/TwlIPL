/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Nmenu
  File:     Memory.cpp

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

#include "util/memory.h"


namespace util
{


namespace
{
    NNSFndHeapHandle    shHeap;
    NNSFndAllocator     sAllocator;
    OSMutex             sLock;
}


void
InitMemory()
{
    u32   arenaLow      = MATH_ROUNDUP  ((u32)OS_GetMainArenaLo(), 16);
    u32   arenaHigh     = MATH_ROUNDDOWN((u32)OS_GetMainArenaHi(), 16);
    u32   heapSize      = arenaHigh - arenaLow;
    void* heapMemory    = OS_AllocFromMainArenaLo(heapSize, 16);

    shHeap = NNS_FndCreateExpHeap(heapMemory, heapSize);
    SDK_ASSERT( shHeap != NNS_FND_HEAP_INVALID_HANDLE );

    NNS_FndInitAllocatorForExpHeap(&sAllocator, shHeap, 4);

    OS_InitMutex(&sLock);
}

NNSFndHeapHandle
GetHeapHandle()
{
    return shHeap;
}

NNSFndAllocator*
GetAllocator()
{
    return &sAllocator;
}

void*
Alloc(size_t size, int align)
{
    OS_LockMutex(&sLock);
    SDK_ASSERT(NNS_FndCheckExpHeap(shHeap, NNS_FND_HEAP_ERROR_PRINT));
    void* p = NNS_FndAllocFromExpHeapEx(shHeap, size, align);
    OS_UnlockMutex(&sLock);

    return p;
}

void
Free(void* ptr)
{
    OS_LockMutex(&sLock);
    SDK_ASSERT(NNS_FndCheckExpHeap(shHeap, NNS_FND_HEAP_ERROR_PRINT));
    SDK_ASSERT(NNS_FndCheckForMBlockExpHeap(ptr, shHeap, NNS_FND_HEAP_ERROR_PRINT));
    NNS_FndFreeToExpHeap(shHeap, ptr);
    OS_UnlockMutex(&sLock);
}


}
//  end of namespace util




//------------ global namespace ------------------------------------

void*
operator new(size_t size)
{
    // operator new[] で 32 byte アライメントを保証できないので
    // operator new でも 32 byte アライメントは行わない。
    return util::Alloc(size, 4);
}

void*
operator new[](size_t size)
{
    // operator new[] では 32 byteアライメントを保証できない。
    return util::Alloc(size, 4);
}

void
operator delete(void* ptr)
{
    if( ptr != NULL )
    {
        util::Free(ptr);
    }
}

void
operator delete[](void* ptr)
{
    if( ptr != NULL )
    {
        util::Free(ptr);
    }
}
