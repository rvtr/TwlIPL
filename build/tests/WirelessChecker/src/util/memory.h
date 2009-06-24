/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Nmenu
  File:     Memory.h

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
#ifndef TLIB_SYSTEM_MEMORY_H_
#define TLIB_SYSTEM_MEMORY_H_

#include <new>
#include <nnsys/fnd.h>

namespace util
{

void InitMemory();

NNSFndAllocator* GetAllocator();
NNSFndHeapHandle GetHeapHandle();
void* Alloc(size_t size, int align=32);
void Free(void* ptr);

}
// end of namespace tlib



#endif  // TLIB_SYSTEM_MEMORY_H_
