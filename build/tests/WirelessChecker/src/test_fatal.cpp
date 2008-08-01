/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checker
  File:     test_fatal.cpp

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

#include "util/util.h"
#include "util/canvas.h"
#include "test_fatal.h"
#include "nwm_arm9_private.h"
#pragma exceptions on

extern "C" {
extern NWMRetCode NWMi_WrackFirmware(NWMCallbackFunc callback);
}

namespace
{
  OSMessageQueue   m_AsyncMsgq;
  OSMessage        m_AsyncMsg[1];
  
  void m_WrackFirmCallback(void *arg)
    {
      NWMCallback *cb = (NWMCallback*)arg;

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->retcode == NWM_RETCODE_FATAL_ERROR ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }

    }
  
  void m_InitMessage(void)
    {
      OS_InitMessageQueue(&m_AsyncMsgq, m_AsyncMsg, sizeof(m_AsyncMsg)/sizeof(m_AsyncMsg[0]));
    }

  BOOL m_WaitCallback(void)
    {
      OSMessage msg;
      (void)OS_ReceiveMessage(&m_AsyncMsgq, &msg, OS_MESSAGE_BLOCK);
      return (BOOL)msg;
    }
}





CTestFatal::CTestFatal()
{
  m_InitMessage();
}

CTestFatal::~CTestFatal()
{

}

BOOL
CTestFatal::Wrack(void)
{
  try
  {
    if (m_state < TEST_STATE_INITIALIZED)
    {
      throw L"ƒXƒe[ƒg‚ª•s³‚Å‚·.\n";
    }

    if (NWM_RETCODE_OPERATING != NWMi_WrackFirmware(m_WrackFirmCallback))
    {
      throw L"WrackFirmware Ž¸”s.\n";
    }
    if (m_WaitCallback() == FALSE)
    {
      throw L"WrackFirmware ”ñ“¯Šúˆ—Ž¸”s.\n";
    }
    LogPrintf(m_pc, L"WrackFirmware ¬Œ÷.\n");

  }
  catch (const wchar_t* string)
  {
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }

  return TRUE;
}

BOOL
CTestFatal::WrackTest(void)
{
  try
  {
    if (FALSE == Init())
    {
      throw L"Init() Ž¸”s.\n";
    }

    if (FALSE == Wrack())
    {
      End();
      throw L"Wrack() Ž¸”s.\n";
    }

    if (FALSE == End())
    {
      throw L"End() Ž¸”s.\n";
    }
  }
  catch (const wchar_t* string)
  {
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }

  LogPrintf(m_pc, L"ƒeƒXƒg‚ªŠ®—¹‚µ‚Ü‚µ‚½.\n");

  return TRUE;
}
