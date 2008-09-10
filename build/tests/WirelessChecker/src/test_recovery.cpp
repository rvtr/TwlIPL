/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checker
  File:     test_recovery.cpp

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
#include "test_recovery.h"
#include "nwm_arm9_private.h"
#pragma exceptions on


CTestRecovery::CTestRecovery()
{
}

CTestRecovery::~CTestRecovery()
{

}


BOOL
CTestRecovery::RecoveryTest(void)
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

    if (FALSE == Wrack())
    {
      Disable();
      End();
      throw L"Wrack() 失敗.\n";
    }

    LogPrintf(m_pc, L"FATALエラーを待っています.\n");

    if (TRUE == test_nwm::m_WaitCallback())
    {
      Disable();
      End();
      throw L"FATALエラーが起こりませんでした.\n";
    }

    LogPrintf(m_pc, L"FATALエラーが発生しました.\n");
  }
  catch (const wchar_t* string)
  {
    LogPrintf(m_pc, L"%s", string);
    return FALSE;
  }

  m_state = TEST_STATE_INITIALIZED;
  End();
  LogPrintf(m_pc, L"3秒後にHWリセットを行います.\n");
  OS_Sleep(3000);

  OSDeliverArgInfo argInfo;
  OS_InitDeliverArgInfo(&argInfo, 0);
  OSi_SetDeliverArgState( OS_DELIVER_ARG_BUF_ACCESSIBLE | OS_DELIVER_ARG_BUF_WRITABLE );
  OS_SetStringToDeliverArg( "fatal" );
  OS_EncodeDeliverArg();
  OS_DoApplicationJump(
                       *(const OSTitleId*)(HW_TWL_ROM_HEADER_BUF + 0x230),
                       OS_APP_JUMP_NORMAL);

  return TRUE;
}

BOOL
CTestRecovery::RecoveryTest2(void)
{
  return WirelessTest();
}

