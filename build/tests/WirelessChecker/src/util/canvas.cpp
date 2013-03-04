/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Nmenu
  File:     Canvas.cpp

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

namespace util
{

CCanvas::CCanvas()
: m_pFont(NULL)
{
}

CCanvas::~CCanvas()
{
}

void 
CCanvas::Init(NNS_GFD_DST_TYPE type, u32 offset, void* pScrBase, const NNSG2dFont* pFont)
{
    m_transType     = type;
    m_transOffset   = offset;
    m_pFont         = pFont;

    NNS_G2dCharCanvasInitForBG(
        &m_cc,
        m_offscreen,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        NNS_G2D_CHARA_COLORMODE_16 );

    NNS_G2dTextCanvasInit(
        &m_txn,
        &m_cc,
        m_pFont,
        TEXT_HSPACE,
        TEXT_VSPACE );

    NNS_G2dMapScrToCharText(
        pScrBase,
        CANVAS_WIDTH,
        CANVAS_HEIGHT,
        CANVAS_LEFT,
        CANVAS_TOP,
        NNS_G2D_TEXT_BG_WIDTH_256,
        CANVAS_CHARACTER_OFFSET,
        DEFAULT_COLOR_PALETTE );
}

void 
CCanvas::Clear(PaletteColor c)
{
    NNS_G2dCharCanvasClear(&m_cc, c);
    RegisterTransferTask();
}

void 
CCanvas::Clear(int x, int y, int w, int h, PaletteColor c)
{
    const int px = MATH_CLAMP(x, 0, HW_LCD_WIDTH);
    const int py = MATH_CLAMP(y, 0, HW_LCD_HEIGHT);
    const int pw = MATH_CLAMP(x + w, px, HW_LCD_WIDTH) - px;
    const int ph = MATH_CLAMP(y + h, py, HW_LCD_HEIGHT) - py;

    if( pw > 0 && ph > 0 )
    {
        NNS_G2dCharCanvasClearArea(&m_cc, c, px, py, pw, ph);
        RegisterTransferTask();
    }
}

void 
CCanvas::Print(int x, int y, PaletteColor c, const NNSG2dChar* text)
{
    NNS_G2dTextCanvasDrawText(
        &m_txn,
        x,
        y,
        c,
        DRAWTEXT_FLAG_DEFAULT,
        text );

    RegisterTransferTask();
}

void 
CCanvas::Printf(int x, int y, PaletteColor c, const NNSG2dChar* text, ...)
{
    std::auto_ptr<wchar_t> pLocalBuffer(new wchar_t[LOCAL_BUFFER_LEN]);
    va_list args;

    va_start(args, text);
    VSNWPrintf(pLocalBuffer.get(), LOCAL_BUFFER_LEN, text, args);
    va_end(args);

    Print(x, y, c, pLocalBuffer.get());
}


void
CCanvas::RegisterTransferTask()
{
    if( ! m_bTransferRegsitered )
    {
        NNS_GfdRegisterNewVramTransferTask(
            m_transType,
            m_transOffset,
            m_offscreen,
            sizeof(m_offscreen) );

        m_bTransferRegsitered = true;
    }
}

void
CCanvas::ResetTransferTask()
{
    m_bTransferRegsitered = false;
}


std::wstring
CCanvas::WrapText(std::wstring text, int width)
{
    int textWidth = 0;

    for( u32 i = 0; i < text.length(); ++i )
    {
        const int charWidth = NNS_G2dFontGetCharWidth(m_pFont, text[i]);

        if( text[i] == L'\n' )
        {
            textWidth = 0;
        }
        else if( (textWidth > 0) && ((textWidth + charWidth) > width) )
        {
            text.insert(i - 1, 1, L'\n');
            textWidth = 0;
        }
        else
        {
            textWidth += charWidth + TEXT_HSPACE;
        }
    }

    return text;
}



}
// end of namespace tlib
