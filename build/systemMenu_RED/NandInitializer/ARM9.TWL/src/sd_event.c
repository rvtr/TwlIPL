/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     sd_event.c

  Copyright 2008 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev:$
  $Author:$
 *---------------------------------------------------------------------------*/

#include <twl.h>
#include "kami_font.h"
#include <twl/fatfs.h>
#include <nitro/card.h>
#include "sd_event.h"

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部関数定義
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);

/*---------------------------------------------------------------------------*
  Name:         SDEvents

  Description:  SDカードの挿抜イベント監視コールバック

  Arguments:    userdata : 任意のユーザ定義引数
                event    : イベント種別
                arg      : イベント固有の引数

  Returns:      None.
 *---------------------------------------------------------------------------*/
void SDEvents(void *userdata, FSEvent event, void *arg)
{
    (void)userdata;
    (void)arg;
    if (event == FS_EVENT_MEDIA_REMOVED)
    {
        OS_TPrintf("sdmc:removed!\n");
    }
    else if (event == FS_EVENT_MEDIA_INSERTED)
    {
        OS_TPrintf("sdmc:inserted!\n");
    }
}

