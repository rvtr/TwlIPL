/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     sd_event.h

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

#ifndef SD_EVENT_H_
#define SD_EVENT_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/

void SDEvents(void *userdata, FSEvent event, void *arg);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* SD_EVENT_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
