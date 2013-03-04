/*---------------------------------------------------------------------------*
  Project:  TwlSDK - YUV2RGB
  File:     cpu_yuv2rgb.h

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
#ifndef YUV2RGB_H_
#define YUV2RGB_H_

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DSP_DMA_NOT_USE            0xFFFFFFFFUL

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

typedef struct YUYV
{
    u8 y1;
    u8 u;
    u8 y2;
    u8 v;
} YUYV;

typedef struct RGBX16
{
    u16 r:5;
    u16 g:5;
    u16 b:5;
    u16 x:1;
} RGBX16;

typedef void (*YUV2RGBCallback)(void);

typedef enum _MessageYuv2Rgb
{
    MESSAGE_YUV2RGB_CONVERT = 1
} MessageYuv2Rgb;

/*---------------------------------------------------------------------------*
  Name:         DSP_Yuv2RgbInit

  Description:  初期化関数

  Arguments:    ***

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL DSP_Yuv2RgbInit(u32 dmaNo);

/*---------------------------------------------------------------------------*
  Name:         DSP_Yuv2RgbConvertAsync

  Description:  YUV->RGB変換を行います。（非同期版）

  Arguments:    ***

  Returns:      None
 *---------------------------------------------------------------------------*/
BOOL DSP_Yuv2RgbConvertAsync(void* src, void* dest, u32 size, YUV2RGBCallback callback);

// CPU版
u32 CpuYuv2Rgb( const void* src, void* dest, u32 pixels );

#ifdef __cplusplus
} /* extern "C" */
#endif

/* YUV2RGB_H_ */
#endif
