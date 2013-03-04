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

#include "util/util.h"
#include "util/canvas.h"
#include "frame.h"

namespace
{

}

CFrame::CFrame() : m_itemIndex(ITEM_TEST_WM)
{
  OS_InitMessageQueue(&m_EntrMsgq, m_EntrMsg, sizeof(m_EntrMsg)/sizeof(m_EntrMsg[0]));

}

CFrame::CFrame(u8 item)
{
  m_itemIndex = item;
  OS_InitMessageQueue(&m_EntrMsgq, m_EntrMsg, sizeof(m_EntrMsg)/sizeof(m_EntrMsg[0]));
}

CFrame::~CFrame()
{

}

void
CFrame::InitDisp()
{
    util::CCanvas* pcm = util::GetMain1Canvas();
    util::CCanvas* pcs = util::GetSub1Canvas();
  
    pcm->Clear(util::COLOR_BLACK);
    pcs->Clear(util::COLOR_BLACK);
    pcm->Printf(16*5, 0, util::COLOR_WHITE, L"�����@�\�m�F�c�[��");
    pcm->Printf(96, 16*1, util::COLOR_WHITE, L"BUILD TIME: %s %s", L""__DATE__, L""__TIME__);

    DrawBorder(pcm, 4, 16*2 + 4, 256 - 8, 192 - (16*2 + 4), util::COLOR_WHITE);
    pcm->Printf(16*1,  16*3, util::COLOR_WHITE, L"����ON/OFF�ݒ�[ %s ]", TRUE == OS_IsAvailableWireless() ? L"ON ":L"OFF");
    pcm->Printf(16*8,  16*3, util::COLOR_WHITE, L"��������OFF�{��[ %s ]", TRUE == OS_IsForceDisableWireless() ? L"Yes":L"No ");

    pcm->Printf(16*2,  16*5, util::COLOR_WHITE, L"DS�����̏������e�X�g");
    pcm->Printf(16*2,  16*6, util::COLOR_WHITE, L"TWL�����̏������e�X�g");
    pcm->Printf(16*2,  16*7, util::COLOR_WHITE, L"FATAL�G���[�G�~�����[�V����");
    pcm->Printf(16*2,  16*8, util::COLOR_WHITE, L"FATAL�G���[���J�o���e�X�g");

    DrawBorder(pcs, 4, 16*9 + 4, 256 - 8, 192 - (16*9 + 4), util::COLOR_WHITE);
}

void
CFrame::CursorDisp(int xpos, int ystart, util::PaletteColor color)
{
  util::CCanvas* pcm = util::GetMain1Canvas();
  pcm->Clear(xpos,  ystart, 16, 16*ITEM_NUM, util::COLOR_BLACK);
  pcm->Print(xpos,  ystart + 16*GetItemIndex(), color, L"��");
}

void
CFrame::HelpDisp(util::PaletteColor color)
{
  util::CCanvas* pcs = util::GetSub1Canvas();
  pcs->Clear(8, 16*9 + 8, 256 - 16, 16*2, util::COLOR_BLACK);

  wchar_t *pHelpString1, *pHelpString2;

  switch (GetItemIndex())
  {
    case ITEM_TEST_WM:
    pHelpString1 = L"DS�����̏������e�X�g���s���܂��B";
    pHelpString2 = L"LED�̓_�ł��m�F���Ă��������B";
    break;
    case ITEM_TEST_NWM:
    pHelpString1 = L"TWL�����̏������e�X�g���s���܂��B";
    pHelpString2 = L"LED�̓_�ł��m�F���Ă��������B";
    break;
    case ITEM_TEST_FATAL:
    pHelpString1 = L"FATAL�G���[�𔭐������ATWL�������g���Ȃ����܂�";
    pHelpString2 = L"HW���Z�b�g�ŉ񕜂��܂��B";
    break;
    case ITEM_TEST_RECOVERY:
    pHelpString1 = L"FATAL�G���[�̃��J�o���[���e�X�g���܂��B";
    pHelpString2 = L"�ċN�����TWL�������������m�F���Ă��������B";
    break;
  }
  pcs->Print(8, 16*9 + 8, color, pHelpString1);
  pcs->Print(8, 16*10 + 8, color, pHelpString2);
}

BOOL
CFrame::AutoEnter()
{
    return OS_SendMessage(&m_EntrMsgq, (OSMessage)TRUE, OS_MESSAGE_NOBLOCK);
}

void
CFrame::ProcessButton()
{
  if (util::IsPadTrigger(PAD_KEY_UP))
  {
    decRound();
  } else
  if (util::IsPadTrigger(PAD_KEY_DOWN))
  {
    incRound();
  }

  if (util::IsPadTrigger(PAD_BUTTON_A))
  {
    if (FALSE == OS_SendMessage(&m_EntrMsgq, (OSMessage)TRUE, OS_MESSAGE_NOBLOCK))
    {
      // already entered
    }
  }
}

BOOL
CFrame::IsEnter()
{
  OSMessage msg;
  return OS_ReceiveMessage(&m_EntrMsgq, &msg, OS_MESSAGE_NOBLOCK);
  
}

void
CFrame::DrawBorder(util::CCanvas* pCanvas, int px, int py, int pw, int ph, util::PaletteColor color)
{
    pCanvas->Clear(px,          py,          pw, 1,      color);
    pCanvas->Clear(px,          py + ph - 1, pw, 1,      color);
    pCanvas->Clear(px,          py + 1,      1,  ph - 2, color);
    pCanvas->Clear(px + pw - 1, py + 1,      1,  ph - 2, color);
}
