/*---------------------------------------------------------------------------*
  Project:  Horizon
  File:     lgy_SaveDataChecker.cpp

  Copyright (C)2009-2011 Nintendo Co., Ltd.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Rev$
 *---------------------------------------------------------------------------*/

#ifndef __LGY_SAVEDATACHECKER_CPP__
#define __LGY_SAVEDATACHECKER_CPP__

#include <twl.h>
#include <sysmenu.h>

BOOL       CheckBackupData( TitleProperty *rhs);

void InitializeBackup( void);
void FinalizeBackup( void);
BOOL readEEPROM( u32 src, void *dst, u32 len);
BOOL writeEEPROM( u32 dst, const void *src, u32 len);
BOOL writeAndVerifyEEPROM( u32 dst, const void *src, u32 len);

/*
// cardnor_NspiIfApi.cpp
namespace nn      {
namespace drivers {
namespace cardnor {
namespace CTR     {
namespace ARM946ES{

void       InitializeTwlBackup( void);    
nn::Result ReadTwlEEPROM(u32 address, u32* buffer, size_t size);
nn::Result WriteTwlEEPROM(u32 address, const u32* buffer, size_t size);
void       FinalizeTwlBackup( void);
    
} // end of namespace ARM946ES
} // end of namespace CTR
} // end of namespace cardnor
} // end of namespace drivers
} // end of namespace nn
*/

//nn::Result ReadTwlEEPROM(u32 address, u32* buffer, size_t size);
//nn::Result WriteTwlEEPROM(u32 address, const u32* buffer, size_t size);

#endif
