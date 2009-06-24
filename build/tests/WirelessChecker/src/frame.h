/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checher
  File:     frame.h

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
#ifndef WIRELESS_CHECHER_FRAME_H_
#define WIRELESS_CHECHER_FRAME_H_

#include <twl.h>
#include "util/util.h"
#include <vector>

class CFrame
{
public:
  enum {
    ITEM_TEST_WM,
    ITEM_TEST_NWM,
    ITEM_TEST_FATAL,
    ITEM_TEST_RECOVERY,
    ITEM_NUM
  };
private:
  u8 m_itemIndex;
  OSMessageQueue   m_EntrMsgq;
  OSMessage        m_EntrMsg[1];
  BOOL m_isEnter;
  
  static const u8 m_itemNum = ITEM_NUM;
  void incRound()
  {
    if (m_itemNum)
    {
      m_itemIndex = (u8)((m_itemIndex + 1)%m_itemNum);
    }
  }
  void decRound()
  {
    if (m_itemNum)
    {
      m_itemIndex = (u8)((m_itemNum + m_itemIndex - 1)%m_itemNum);
    }
  }
public:
  CFrame();
  CFrame(u8 item);
  virtual ~CFrame();
  void InitDisp();
  void CursorDisp(int xpos, int ystart, util::PaletteColor color);
  void HelpDisp(util::PaletteColor color);
  virtual void ProcessButton();
  u8 GetItemIndex()
  {
    return m_itemIndex;
  }
  BOOL IsEnter();
  BOOL AutoEnter();
  void DrawBorder(util::CCanvas* pCanvas, int px, int py, int pw, int ph, util::PaletteColor color);
};

#endif  // WIRELESS_CHECHER_TEST_NWM_H_
