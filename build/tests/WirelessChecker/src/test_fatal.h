/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checher
  File:     test_fatal.h

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
#ifndef WIRELESS_CHECHER_TEST_FATAL_H_
#define WIRELESS_CHECHER_TEST_FATAL_H_

#include <twl.h>
#include "test.h"
#include "test_nwm.h"
#include "util/canvas.h"

class CTestFatal
     : public CTestNwm
{
private:
public:
  CTestFatal();
  virtual ~CTestFatal();
  BOOL Wrack(void);
  BOOL WrackTest(void);
};


#endif  // WIRELESS_CHECHER_TEST_FATAL_H_
