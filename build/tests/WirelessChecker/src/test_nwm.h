/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checher
  File:     test_nwm.h

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
#ifndef WIRELESS_CHECHER_TEST_NWM_H_
#define WIRELESS_CHECHER_TEST_NWM_H_

#include <twl.h>
#include "test.h"

namespace test_nwm
{
  void m_LoadDeviceCallback(void *arg);
  void m_OpenCallback(void *arg);
  void m_UnloadDeviceCallback(void *arg);
  void m_CloseCallback(void *arg);
  void m_ScanCallback(void *arg);
  BOOL m_WaitCallback(void);
}

class CTestNwm
     : public CTest
{
private:
  static const u8 m_DmaNo = 3;
  u8 *m_pNwmBuffer;
  u8 *m_pScanBuffer;
public:
  CTestNwm();
  virtual ~CTestNwm();
  BOOL Init(void);
  BOOL Enable(void);
  BOOL ScanTest(void);
  BOOL Disable(void);
  BOOL End(void);
  BOOL WirelessTest(void);
};


#endif  // WIRELESS_CHECHER_TEST_NWM_H_
