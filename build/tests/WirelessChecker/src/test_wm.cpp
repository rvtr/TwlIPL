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
#include "test_wm.h"
#pragma exceptions on

namespace
{
  OSMessageQueue   m_AsyncMsgq;
  OSMessage        m_AsyncMsg[1];
  
  void m_EnableCallback(void *arg)
    {
      WMCallback *cb = (WMCallback*)arg;

      // TODO: get FW ver and RegDomain?

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->errcode == WM_ERRCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }
    }

  void m_PowerOnCallback(void *arg)
    {
      WMCallback *cb = (WMCallback*)arg;

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->errcode == WM_ERRCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }

    }

  void m_DisableCallback(void *arg)
    {
      WMCallback *cb = (WMCallback*)arg;

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->errcode == WM_ERRCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }
    }

  void m_PowerOffCallback(void *arg)
    {
      WMCallback *cb = (WMCallback*)arg;

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->errcode == WM_ERRCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }

    }
  
  void m_ScanCallback(void *arg)
    {
      WMStartScanExCallback *cb = (WMStartScanExCallback*)arg;

      OS_TPrintf("Number of BSS: %d\n", cb->bssDescCount);

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->errcode == WM_ERRCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
        {
        }

    }

  void m_EndScanCallback(void *arg)
    {
      WMCallback *cb = (WMCallback*)arg;

      if (FALSE == OS_SendMessage(&m_AsyncMsgq, (OSMessage)(cb->errcode == WM_ERRCODE_SUCCESS ? TRUE:FALSE), OS_MESSAGE_NOBLOCK))
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





CTestWm::CTestWm() : m_pWmBuffer(0), m_pScanBuffer(0)
{
  m_InitMessage();
  m_pc = util::GetSub1Canvas();
}

CTestWm::~CTestWm()
{

}


BOOL
CTestWm::Init(void)
{
  LogClear(m_pc);
  
  try
  {
    if (m_state != TEST_STATE_NONE)
    {
      throw L"ステートが不正です.\n";
    }
    if (m_pWmBuffer)
    {
      delete [] m_pWmBuffer;
      m_pWmBuffer = 0;
      throw L"WM用バッファが不正です.\n";
    }

    m_pWmBuffer = new u8 [WM_SYSTEM_BUF_SIZE + 0x20];


    if (WM_ERRCODE_SUCCESS != WM_Init((void*)MATH_ROUNDUP((u32)m_pWmBuffer, 32), m_DmaNo))
    {
      throw L"WM_Init 失敗.\n";
    }

    LogPrintf(m_pc, L"WM_Init 成功.\n");

  }

  catch (const wchar_t* string)
  {
    delete [] m_pWmBuffer;
    m_pWmBuffer = 0;
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }
  
  m_state = TEST_STATE_INITIALIZED;

  return TRUE;
}

BOOL
CTestWm::Enable(void)
{
  try {

    if (m_state != TEST_STATE_INITIALIZED)
    {
      throw L"ステートが不正です.\n";
    }
    
    if (WM_ERRCODE_OPERATING != WM_Enable(m_EnableCallback))
    {
      throw L"WM_Enable 失敗.\n";
    }
    if (m_WaitCallback() == FALSE)
    {
      throw L"WM_Enable 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"WM_Enable 成功.\n");
    if (WM_ERRCODE_OPERATING != WM_PowerOn(m_PowerOnCallback))
    {
      throw L"WM_PowerOn 失敗.\n";
    }
    if (m_WaitCallback() == FALSE)
    {
      throw L"WM_PowerOn 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"WM_PowerOn 成功.\n");
  }

  catch (const wchar_t* string)
  {
    LogPrintf(m_pc,  L"%s", string);
    return FALSE;
  }
  m_state = TEST_STATE_ENABLED;
  return TRUE;
}

BOOL
CTestWm::ScanTest(void)
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

    m_pScanBuffer = new u8 [WM_SIZE_SCAN_EX_BUF + 0x20];

    WMScanExParam param;

    MI_CpuClear8(m_pScanBuffer, WM_SIZE_SCAN_EX_BUF + 0x20);
    DC_StoreRange(m_pScanBuffer, WM_SIZE_SCAN_EX_BUF + 0x20);
    param.scanBuf = (WMBssDesc*)MATH_ROUNDUP((u32)m_pScanBuffer, 32);
    param.scanBufSize = WM_SIZE_SCAN_EX_BUF;
    param.channelList = (u16)WM_GetAllowedChannel();
    param.maxChannelTime = WM_GetDispersionScanPeriod();
    param.scanType = WM_SCANTYPE_ACTIVE;
    param.ssidLength = 0;
    MI_CpuFill8(param.ssid, 0xFF, sizeof(param.ssid));
    param.ssidMatchLength = 0;

    if (WM_ERRCODE_OPERATING != WM_StartScanEx(m_ScanCallback, &param))
    {
      throw L"WM_StartScanEx 失敗.\n";
    }

    if (m_WaitCallback() == FALSE)
    {
      throw L"WM_StartScanEx 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"WM_StartScanEx 成功.\n");

    if (WM_ERRCODE_OPERATING != WM_EndScan(m_EndScanCallback))
    {
      throw L"WM_EndScan 失敗.\n";
    }

    if (m_WaitCallback() == FALSE)
    {
      throw L"WM_EndScan 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"WM_EndScan 成功.\n");

  }

  catch (const wchar_t* string)
  {
    LogPrintf(m_pc,  L"%s", string);
    return FALSE;
  }
  
  delete [] m_pScanBuffer;
  m_pScanBuffer = 0;

  return TRUE;
}

BOOL
CTestWm::Disable(void)
{
  try {
    if (m_state != TEST_STATE_ENABLED)
    {
      throw L"ステートが不正です.\n";
    }
    
    if (WM_ERRCODE_OPERATING != WM_PowerOff(m_PowerOffCallback))
    {
      throw L"WM_PowerOff 失敗.\n";
    }
    if (m_WaitCallback() == FALSE)
    {
      throw L"WM_PowerOff 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"WM_PowerOff 成功.\n");
    
    if (WM_ERRCODE_OPERATING != WM_Disable(m_DisableCallback))
    {
      throw L"WM_Disable 失敗.\n";
    }
    if (m_WaitCallback() == FALSE)
    {
      throw L"WM_Disable 非同期処理失敗.\n";
    }
    LogPrintf(m_pc, L"WM_Disable 成功.\n");
  }

  catch (const wchar_t* string)
  {
    LogPrintf(m_pc,  L"%s", string);
    return FALSE;
  }

  m_state = TEST_STATE_INITIALIZED;
  return TRUE;

}

BOOL
CTestWm::End(void)
{
  try
  {
    if (m_state != TEST_STATE_INITIALIZED)
    {
      throw L"ステートが不正です\n";
    }
    if (WM_ERRCODE_SUCCESS != WM_Finish())
    {
      throw L"WM_Finish 失敗.\n";
    }
    LogPrintf(m_pc, L"WM_Finish 成功.\n");
  }

  catch (const wchar_t* string)
  {
    LogPrintf(m_pc,  L"%s", string);
    return FALSE;
  }

  m_state = TEST_STATE_NONE;
  delete [] m_pWmBuffer;
  m_pWmBuffer = 0;

  return TRUE;
}

BOOL
CTestWm::WirelessTest(void)
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
    LogPrintf(m_pc,  L"%s", string);
    return FALSE;
  }

  LogPrintf(m_pc,  L"テストが完了しました.\n");
  
  return TRUE;
}
