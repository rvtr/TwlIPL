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
    �����ϐ���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
static void VBlankIntr(void);

/*---------------------------------------------------------------------------*
  Name:         SDEvents

  Description:  SD�J�[�h�̑}���C�x���g�Ď��R�[���o�b�N

  Arguments:    userdata : �C�ӂ̃��[�U��`����
                event    : �C�x���g���
                arg      : �C�x���g�ŗL�̈���

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

