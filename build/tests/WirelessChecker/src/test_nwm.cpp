/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checker
  File:     test_nwm.cpp

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
#include "test_nwm.h"
#pragma exceptions on

namespace test_nwm
{
  OSMessageQueue   m_AsyncMsgq;
  OSMessage        m_AsyncMsg[1];
  void m_InitMessage(void);
  
  void m_LoadDeviceCallback(void *arg)
    {
      NWMCallback *cb = (NWMCallback*)arg;

      // TODO: get FW ver and RegDomain?

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->retcode == NWM_RETCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }
    }

  void m_OpenCallback(void *arg)
    {
      NWMCallback *cb = (NWMCallback*)arg;

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->retcode == NWM_RETCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }

    }

  void m_UnloadDeviceCallback(void *arg)
    {
      NWMCallback *cb = (NWMCallback*)arg;

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->retcode == NWM_RETCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }
    }

  void m_CloseCallback(void *arg)
    {
      NWMCallback *cb = (NWMCallback*)arg;

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->retcode == NWM_RETCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }

    }
  
  void m_ScanCallback(void *arg)
    {
      NWMStartScanCallback *cb = (NWMStartScanCallback*)arg;

      OS_TPrintf("Number of BSS: %d\n", cb->bssDescCount);

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->retcode == NWM_RETCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
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





CTestNwm::CTestNwm() : m_pNwmBuffer(0), m_pScanBuffer(0)
{
  test_nwm::m_InitMessage();
}

CTestNwm::~CTestNwm()
{

}


BOOL
CTestNwm::Init(void)
{
  LogClear(m_pc);
  
  try
  {
    if (m_state != TEST_STATE_NONE)
    {
      throw L"ステートが不正です.\n";
    }
    if (m_pNwmBuffer)
    {
      delete [] m_pNwmBuffer;
      m_pNwmBuffer = 0;
      throw L"NWMバッファが不正です.\n";
    }

    m_pNwmBuffer = new u8 [NWM_SYSTEM_BUF_SIZE + 0x20];


    if (NWM_RETCODE_SUCCESS != NWM_Init((void*)MATH_ROUNDUP((u32)m_pNwmBuffer, 32), NWM_SYSTEM_BUF_SIZE, m_DmaNo))
    {
      throw L"NWM_Init 失敗.\n";
    }

    LogPrintf(m_pc, L"NWM_Init 成功.\n");
  }

  catch (const wchar_t* string)
  {
    delete [] m_pNwmBuffer;
    m_pNwmBuffer = 0;
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }
  
  m_state = TEST_STATE_INITIALIZED;

  return TRUE;
}

BOOL
CTestNwm::Enable(void)
{
  try {
    if (m_state != TEST_STATE_INITIALIZED)
    {
      throw L"ステートが不正です.\n";
    }
    
    if (NWM_RETCODE_OPERATING != NWM_LoadDevice(test_nwm::m_LoadDeviceCallback))
    {
      throw L"NWM_LoadDevice 失敗.\n";
    }

    if (test_nwm::m_WaitCallback() == FALSE)
    {
      throw L"NWM_LoadDevice 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"NWM_LoadDevice 成功.\n");
    
    if (NWM_RETCODE_OPERATING != NWM_Open(test_nwm::m_OpenCallback))
    {
      throw L"NWM_Open 失敗.\n";
    }
    if (test_nwm::m_WaitCallback() == FALSE)
    {
      throw L"NWM_Open 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"NWM_Open 成功.\n");
  }

  catch (const wchar_t* string)
  {
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }
  m_state = TEST_STATE_ENABLED;
  return TRUE;
}

BOOL
CTestNwm::ScanTest(void)
{
  try
  {

    if (m_state != TEST_STATE_ENABLED)
    {
      throw L"ステートが不正です.\n";
    }

    if (m_pScanBuffer)
    {
      throw L"Scanバッファが不正です.\n";
    }

    m_pScanBuffer = new u8 [NWM_SIZE_SCANBUF_MAX + 0x20];

    NWMScanParam param;

    MI_CpuClear8(m_pScanBuffer, NWM_SIZE_SCANBUF_MAX + 0x20);
    DC_StoreRange(m_pScanBuffer, NWM_SIZE_SCANBUF_MAX + 0x20);
    param.scanBuf = (NWMBssDesc*)MATH_ROUNDUP((u32)m_pScanBuffer, 32);
    param.scanBufSize = NWM_SIZE_SCANBUF_MAX;
    param.channelList = (u16)(NWM_GetAllowedChannel() >> 1);
    param.channelDwellTime = NWM_GetDispersionScanPeriod(NWM_SCANTYPE_ACTIVE);
    param.scanType = NWM_SCANTYPE_ACTIVE;
    param.ssidLength = 0;
    MI_CpuFill8(param.ssid, 0xFF, sizeof(param.ssid));

    if (NWM_RETCODE_OPERATING != NWM_StartScan(test_nwm::m_ScanCallback, &param))
    {
      throw L"NWM_StartScan 失敗.\n";
    }

    if (test_nwm::m_WaitCallback() == FALSE)
    {
      throw L"NWM_StartScan 非同期処理失敗.\n";
    }

    LogPrintf(m_pc, L"NWM_StartScan 成功.\n");

  }

  catch (const wchar_t* string)
  {
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }
  
  delete [] m_pScanBuffer;
  m_pScanBuffer = 0;

  return TRUE;
}

BOOL
CTestNwm::Disable(void)
{
  try {
    if (m_state != TEST_STATE_ENABLED)
    {
      throw L"ステートが不正です.\n";
    }
    
    if (NWM_RETCODE_OPERATING != NWM_Close(test_nwm::m_CloseCallback))
    {
      throw L"NWM_Close 失敗.\n";
    }
    if (test_nwm::m_WaitCallback() == FALSE)
    {
      throw L"NWM_Close 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"NWM_Close 成功.\n");
    
    if (NWM_RETCODE_OPERATING != NWM_UnloadDevice(test_nwm::m_UnloadDeviceCallback))
    {
      throw L"NWM_UnloadDevice 失敗.\n";
    }
    if (test_nwm::m_WaitCallback() == FALSE)
    {
      throw L"NWM_UnloadDevice 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"NWM_UnloadDevice 成功.\n");
  }

  catch (const wchar_t* string)
  {
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }
  m_state = TEST_STATE_INITIALIZED;
  return TRUE;

}

BOOL
CTestNwm::End(void)
{
  try
  {
    if (m_state != TEST_STATE_INITIALIZED)
    {
      throw L"ステートが不正です.\n";
    }
    if (NWM_RETCODE_SUCCESS != NWM_End())
    {
      throw L"NWM_End 失敗.\n";
    }

    LogPrintf(m_pc, L"NWM_End 成功.\n");
  }

  catch (const wchar_t* string)
  {
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }

  m_state = TEST_STATE_NONE;
  delete [] m_pNwmBuffer;
  m_pNwmBuffer = 0;

  return TRUE;
}

BOOL
CTestNwm::WirelessTest(void)
{
  try
  {
    if (FALSE == Init())
    {
      throw L"Init() 失敗.\n";
    }
    if (FALSE == Enable())
    {
      End();
      throw L"Enable() 失敗.\n";
    }
    if (FALSE == ScanTest())
    {
      Disable();
      End();
      throw L"ScanTest() 失敗.\n";
    }
    if (FALSE == Disable())
    {
      throw L"Disable() 失敗.\n";
    }
    if (FALSE == End())
    {
      throw L"End() 失敗.\n";
    }
  }
  catch (const wchar_t* string)
  {
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }

  LogPrintf(m_pc, L"テストが完了しました.\n");

  return TRUE;
}
