/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checher
  File:     test_wm.h

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
#ifndef WIRELESS_CHECHER_TEST_WM_H_
#define WIRELESS_CHECHER_TEST_WM_H_

#include <twl.h>
#include "test.h"
#include "util/canvas.h"

class CTestWm
     : public CTest
{
private:
  static const u8 m_DmaNo = 3;
  u8 *m_pWmBuffer;
  u8 *m_pScanBuffer;
public:
  CTestWm();
  virtual ~CTestWm();
  BOOL Init(void);
  BOOL Enable(void);
  BOOL ScanTest(void);
  BOOL Disable(void);
  BOOL End(void);
  BOOL WirelessTest(void);
};


#endif  // WIRELESS_CHECHER_TEST_WM_H_
