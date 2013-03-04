/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checher
  File:     test.h

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
#ifndef WIRELESS_CHECHER_TEST_H_
#define WIRELESS_CHECHER_TEST_H_

#include <twl.h>
#include "util/util.h"
#include "util/canvas.h"
#include <nnsys/g2d/g2d_Font.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/g2d_TextCanvas.h>

class CTest

{
private:
  u8 m_line;
  static const int LOCAL_BUFFER_LEN = 1024;
public:
  enum {
    TEST_STATE_NONE,
    TEST_STATE_INITIALIZED,
    TEST_STATE_ENABLED,
    TEST_STATE_NUM
  };
  u8 m_state;
  util::CCanvas *m_pc;
  CTest();
  virtual ~CTest();
  virtual BOOL Init(void);
  virtual BOOL Enable(void);
  virtual BOOL ScanTest(void);
  virtual BOOL Disable(void);
  virtual BOOL End(void);
  u8 GetState(void);
  void LogClear(util::CCanvas* pc);
  void LogPrintf(util::CCanvas* pc, const NNSG2dChar* text, ...);
};

#endif  // WIRELESS_CHECHER_TEST_H_
