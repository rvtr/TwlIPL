/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checher
  File:     test_recovery.h

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
#ifndef WIRELESS_CHECHER_TEST_RECOVERY_H_
#define WIRELESS_CHECHER_TEST_RECOVERY_H_

#include <twl.h>
#include "test.h"
#include "test_fatal.h"
#include "util/canvas.h"

class CTestRecovery
     : public CTestFatal
{
private:
public:
  CTestRecovery();
  ~CTestRecovery();
  BOOL RecoveryTest(void);
  BOOL RecoveryTest2(void);
};


#endif  // WIRELESS_CHECHER_TEST_RECOVERY_H_
