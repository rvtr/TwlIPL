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

#include <iomanip>
#include <sstream>
#include <twl/os/common/sharedFont.h>
#include "util/util.h"
#include "util/canvas.h"
#include "frame.h"
#include "test_wm.h"
#include "test_nwm.h"
#include "test_fatal.h"
#include "test_recovery.h"
#include <nnsys/gfd.h>

namespace
{

}
//  end of anonymous namespace


void
TwlStartUp(void)
{

}

void
TwlMain(void)
{
    BOOL isFatalChecking = FALSE;
    util::Init();

    OSDeliverArgInfo argInfo;
#ifndef CHECK_ON_CTR
    if (TRUE == OS_IsValidDeliverArg())
    {
      OS_InitDeliverArgInfo(&argInfo, 0);
      OS_DecodeDeliverArg();
      if( (OS_GetDeliverArgc() > 0) && ( STD_CompareNString( (const char *)OS_GetDeliverArgv(1), "fatal", 3 ) == 0 ) )
      {
        isFatalChecking = TRUE;
      }
    }
#endif
    // display on
    util::WaitVBlankIntr();
    util::dispOn();

    CFrame menu(isFatalChecking == TRUE ? CFrame::ITEM_TEST_RECOVERY:CFrame::ITEM_TEST_WM);
    if (isFatalChecking == TRUE)
    {
      menu.AutoEnter();
    }
    CTestWm wm;
    CTestNwm nwm;
    CTestFatal fatal;
    CTestRecovery rcv;
    menu.InitDisp();
  
    for (;;)
    {
        util::UpdateGamePad();
        menu.ProcessButton();
        menu.CursorDisp(16*1, 16*5, util::COLOR_LIMEGREEN);
        menu.HelpDisp(util::COLOR_WHITE);

        if (menu.IsEnter())
        {
          BOOL result;
          u8   itemidx = menu.GetItemIndex();
          util::CCanvas* pcm = util::GetMain1Canvas();

          switch (itemidx)
          {
          case CFrame::ITEM_TEST_WM:
            pcm->Clear(16*12,  16*5, 16*3, 16, util::COLOR_BLACK);
            pcm->Print(16*12,  16*5, util::COLOR_YELLOW, L"TESTING");
            util::WaitVBlankIntr();
            util::UpdateDisplay();
            result = wm.WirelessTest();
            pcm->Clear(16*12,  16*5, 16*3, 16, util::COLOR_BLACK);
            pcm->Print(16*12,  16*5, result == TRUE ? util::COLOR_TURQUOISE:util::COLOR_RED, result == TRUE ? L"SUCCESS":L"FAILURE");
            break;
          case CFrame::ITEM_TEST_NWM:
            pcm->Clear(16*12,  16*6, 16*3, 16, util::COLOR_BLACK);
            pcm->Print(16*12,  16*6, util::COLOR_YELLOW, L"TESTING");
            util::WaitVBlankIntr();
            util::UpdateDisplay();
            result = nwm.WirelessTest();
            pcm->Clear(16*12,  16*6, 16*3, 16, util::COLOR_BLACK);
            pcm->Print(16*12,  16*6, result == TRUE ? util::COLOR_TURQUOISE:util::COLOR_RED, result == TRUE ? L"SUCCESS":L"FAILURE");
            break;
          case CFrame::ITEM_TEST_FATAL:
            pcm->Clear(16*12,  16*7, 16*3, 16, util::COLOR_BLACK);
            pcm->Print(16*12,  16*7, util::COLOR_YELLOW, L"TESTING");
            result = fatal.WrackTest();
            pcm->Clear(16*12,  16*7, 16*3, 16, util::COLOR_BLACK);
            pcm->Print(16*12,  16*7, result == TRUE ? util::COLOR_TURQUOISE:util::COLOR_RED, result == TRUE ? L"SUCCESS":L"FAILURE");
            break;
          case CFrame::ITEM_TEST_RECOVERY:
            pcm->Clear(16*12,  16*8, 16*3, 16, util::COLOR_BLACK);
            pcm->Print(16*12,  16*8, util::COLOR_YELLOW, L"TESTING");
            if (isFatalChecking == FALSE)
            {
                result = rcv.RecoveryTest();
            } else {
                result = rcv.RecoveryTest2();
                isFatalChecking = FALSE;
            }
            pcm->Clear(16*12,  16*8, 16*3, 16, util::COLOR_BLACK);
            pcm->Print(16*12,  16*8, result == TRUE ? util::COLOR_TURQUOISE:util::COLOR_RED, result == TRUE ? L"SUCCESS":L"FAILURE");
            break;
          }
        }

        util::WaitVBlankIntr();
        util::UpdateDisplay();
    }

}
