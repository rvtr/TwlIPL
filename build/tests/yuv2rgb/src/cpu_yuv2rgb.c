    // 中身をいじってある。
/*---------------------------------------------------------------------------*
  Project:  TwlSDK - YUV2RGB
  File:     cpu_yuv2rgb.c

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

#include <twl.h>
#include "yuv2rgb.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    関数宣言
 *---------------------------------------------------------------------------*/

u32 yuv2rgb16( const void* src, void* dest, u32 pixels );

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/

static inline int r_diff( u8 v )
{
//    return (1402 * ( v - 128 )) / 1000;
    return (11760828/*.416*/ * ( v - 128)) >> 23;
}

static inline int g_diff( u8 u, u8 v )
{
//    return (-344 * ( u - 128 ) -714 * ( v - 128 )) / 1000;
    return (-2885681/*.152*/ * ( u  - 128 ) -5989466/*.112*/ * ( v - 128 )) >> 23;
}
static inline int b_diff( u8 u )
{
//    return (1772 * ( u - 128 )) / 1000;
    return (14864613/*.376*/ * ( u - 128 )) >> 23;
}

static inline int trim( int e )
{
    return e < 0 ? 0 : ( e > 255 ? 255 : e);
}

/*---------------------------------------------------------------------------*
  Name:         CpuYuv2Rgb

  Description:  YUV->RGB変換を行います(CPUバージョン）

  Arguments:    src    : input data adress (YUV)
                dest   : output data adress (RGB)
                pixels : num of pixel

  Returns:      None
 *---------------------------------------------------------------------------*/
u32 CpuYuv2Rgb( const void* src, void* dest, u32 pixels )
{
    OSTick begin;
    u8* yuyv = (u8*)src;
    u32* rgb = (u32*)dest;

    u32 limit = pixels >> 1;
    u32 i;

begin = OS_GetTick();

    for (i = 0; i < limit; i++)
    {
        u8 y1 = *yuyv++;
        u8 u  = *yuyv++;
        u8 y2 = *yuyv++;
        u8 v  = *yuyv++;
        *rgb++ = (u32)(
            0xffffffff
            //0x83e083e0 // G only
                        & (
                            0x80008000 |
                        ( ( trim( y2 + b_diff( u    ) ) & 0xF8 ) << 23) |
                        ( ( trim( y2 + g_diff( u, v ) ) & 0xF8 ) << 18) |
                        ( ( trim( y2 + r_diff( v    ) ) & 0xF8 ) << 13) |
                        ( ( trim( y1 + b_diff( u    ) ) & 0xF8 ) <<  7) |
                        ( ( trim( y1 + g_diff( u, v ) ) & 0xF8 ) <<  2) |
                        ( ( trim( y1 + r_diff( v    ) )        ) >>  3) ));
    }

OS_Printf("total(555) = %d us\n", OS_TicksToMicroSeconds(OS_GetTick() - begin));

    return pixels * sizeof(RGBX16);
}

