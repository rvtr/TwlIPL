/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Nmenu
  File:     Canvas.h

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
#ifndef TLIB_CONTROL_CANVAS_H_
#define TLIB_CONTROL_CANVAS_H_

#define NNS_G2D_UNICODE
#include <nnsys/g2d/g2di_Char.h>
#include <nnsys/g2d/g2d_TextCanvas.h>
#include <nnsys/gfd.h>
#include <string>
#include "util.h"

namespace util
{

class CCanvas
{
private:
    static const int CANVAS_WIDTH  = 32;
    static const int CANVAS_HEIGHT = 24;
    static const int CANVAS_LEFT   = 0;
    static const int CANVAS_TOP    = 0;
    static const int TEXT_HSPACE   = 1;
    static const int TEXT_VSPACE   = 1;
    static const int CANVAS_CHARACTER_OFFSET = 1;

    static const int LOCAL_BUFFER_LEN = 1024;

    static const u32 DRAWTEXT_FLAG_DEFAULT = ( NNS_G2D_VERTICALORIGIN_TOP
                                      | NNS_G2D_HORIZONTALORIGIN_LEFT
                                      | NNS_G2D_HORIZONTALALIGN_LEFT );

private:
    NNSG2dCharCanvas    m_cc;
    NNSG2dTextCanvas    m_txn;
    GXCharFmt16         m_offscreen[32 * 24];
    const NNSG2dFont*   m_pFont;
    NNS_GFD_DST_TYPE    m_transType;
    u32                 m_transOffset;
    bool                m_bTransferRegsitered;

public:
    CCanvas();
    ~CCanvas();

public:
    void Init(NNS_GFD_DST_TYPE type, u32 offset, void* pScrBase, const NNSG2dFont* pFont);
    void Clear (PaletteColor c);
    void Clear (int x, int y, int w, int h, PaletteColor c);
    void Print (int x, int y, PaletteColor c, const NNSG2dChar* text);
    void Printf(int x, int y, PaletteColor c, const NNSG2dChar* text, ...);
    std::wstring WrapText(std::wstring text, int width);

    void ResetTransferTask();

protected:
    void RegisterTransferTask();
};

}
// end of namespace tlib


#endif  // TLIB_CONTROL_CANVAS_H_
