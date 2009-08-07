 /*---------------------------------------------------------------------------*
  Project:  TwlSDK - WCM - demos - wcm-list-2
  File:     screen.h

  Copyright 2007-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-09-17#$
  $Rev: 8556 $
  $Author: okubata_ryoma $
 *---------------------------------------------------------------------------*/
#ifndef SCREEN_H_
#define SCREEN_H_

#ifdef __cplusplus

extern "C" {
#endif

/*===========================================================================*/
#include <nitro/types.h>

// フォントの色
#define COLOR_BLACK			0x10
#define COLOR_RED			0x11
#define COLOR_GREEN			0x12
#define COLOR_BLUE			0x13
#define COLOR_YELLOW		0x14
#define COLOR_PURPLE		0x15
#define COLOR_L_BLUE		0x16
#define COLOR_D_RED			0x17
#define COLOR_D_GREEN		0x18
#define COLOR_D_BLUE		0x19
#define COLOR_D_YELLOW		0x1a
#define COLOR_D_PURPLE		0x1b
#define COLOR_D_L_BLUE		0x1c
#define COLOR_GRAY			0x1d
#define COLOR_D_GRAY		0x1e
#define COLOR_WHITE			0x1f

/*---------------------------------------------------------------------------*
    関数 定義
 *---------------------------------------------------------------------------*/
void    InitScreen(void);
void    ClearScreen(void);
void    ClearMainScreen(void);
void    ClearSubScreen(void);
void    PutMainScreen(s32 x, s32 y, u8 palette, char* text, ...);
void    PutSubScreen(s32 x, s32 y, u8 palette, char* text, ...);
void    UpdateScreen(void);

/*===========================================================================*/
#ifdef __cplusplus

}       /* extern "C" */
#endif

#endif /* SCREEN_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
