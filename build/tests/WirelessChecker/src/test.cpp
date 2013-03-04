/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checker
  File:     main.cpp

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

#include <twl.h>
#include "util/canvas.h"
#include "util/util.h"
#include "util/wprintf.h"
#include <nnsys/g2d/g2d_Font.h>
#include <nnsys/g2d/g2d_CharCanvas.h>
#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/gfd.h>
//#include <wchar.h>
#include <memory>
#include "test.h"

CTest::CTest() : m_state(TEST_STATE_NONE)
{
  m_pc = util::GetSub1Canvas();
}

CTest::~CTest()
{

}

BOOL
CTest::Init(void)
{
  return TRUE;
}

BOOL
CTest::Enable(void)
{
  return TRUE;
}

BOOL
CTest::ScanTest(void)
{
  return TRUE;
}

BOOL
CTest::Disable(void)
{
  return TRUE;
}

BOOL
CTest::End(void)
{
  return TRUE;
}

u8
CTest::GetState(void)
{
  return m_state;
}

void CTest::LogClear(util::CCanvas* pc)
{
  pc->Clear(0, 0, 256, 16*9, util::COLOR_BLACK);
  m_line = 0;
  util::WaitVBlankIntr();
  util::UpdateDisplay();
}

void
CTest::LogPrintf(util::CCanvas* pc, const NNSG2dChar* text, ...)
{
  std::auto_ptr<wchar_t> pLocalBuffer(new wchar_t[LOCAL_BUFFER_LEN]);
  va_list args;

  va_start(args, text);
  util::VSNWPrintf(pLocalBuffer.get(), LOCAL_BUFFER_LEN, text, args);
  va_end(args);
  pc->Print(16*1, 16*m_line++, util::COLOR_WHITE, pLocalBuffer.get());
  util::WaitVBlankIntr();
  util::UpdateDisplay();
}
