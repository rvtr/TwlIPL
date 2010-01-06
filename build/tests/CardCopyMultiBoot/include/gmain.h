/*---------------------------------------------------------------------------*
  Project:  TwlSDK - MB - demos - cloneboot
  File:     gmain.h

  Copyright 2006-2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
*---------------------------------------------------------------------------*/
#ifndef MB_DEMO_GMAIN_H_
#define MB_DEMO_GMAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <nitro.h>


typedef struct
{
    u32     count;                     // フレーム情報
    u16     key;                       // キー情報
    u16     padding;
}
GShareData;


//============================================================================
//  関数宣言
//============================================================================

void    GInitDataShare(void);
void    GStepDataShare(s32 frame);
void    GMain(void);


#ifdef __cplusplus
}/* extern "C" */
#endif

#endif // MB_DEMO_GMAIN_H_
