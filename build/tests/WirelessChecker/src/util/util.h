/*---------------------------------------------------------------------------*
  Project:  TwlSDK - Wireless Checher
  File:     util.h

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
#ifndef WIRELESS_CHECHER_UTIL_H_
#define WIRELESS_CHECHER_UTIL_H_

#include <twl.h>

namespace util
{

const int DEFAULT_COLOR_PALETTE = 0;
  
enum PaletteColor
{
    COLOR_NULL=0,
    COLOR_WHITE,
    COLOR_BLACK,

    COLOR_RED,
    COLOR_YELLOW,
    COLOR_LIMEGREEN,
    COLOR_TURQUOISE,
    COLOR_BLUE,
    COLOR_BROWN,
    COLOR_LIGHTGRAY,
    COLOR_DARKGRAY,

    COLOR_GUIDE_BG
};

struct GamePad
{
    u16     trigger;
    u16     release;
    u16     button;
    bool    repeatTrigger;
    u8      padding0;
    int     beginPress;
};

void Init(void);
void WaitVBlankIntr(void);
void dispOn(void);
void UpdateGamePad(void);
void UpdateDisplay(void);

class CCanvas;
CCanvas* GetMain1Canvas();
CCanvas* GetMain3Canvas();
CCanvas* GetSub1Canvas();
CCanvas* GetSub3Canvas();

inline u16
IsPadDown(u16 key)
{
    extern GamePad System_GamePadState;
    return static_cast<u16>(System_GamePadState.trigger & key);
}
inline u16
IsPadUp(u16 key)
{
    extern GamePad System_GamePadState;
    return static_cast<u16>(System_GamePadState.release & key);
}
inline u16
IsPadPress(u16 key)
{
    extern GamePad System_GamePadState;
    return static_cast<u16>(System_GamePadState.button & key);
}
inline u16
IsPadTrigger(u16 key)
{
    extern GamePad System_GamePadState;
    if( System_GamePadState.repeatTrigger )
    {
        return IsPadPress(key);
    }
    else
    {
        return IsPadDown(key);
    }
}

u16 GetPadTrigger();

} // namespace util

#endif  // WIRELESS_CHECHER_UTIL_H_
