/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     keypad.h

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

#ifndef KAMI_KEY_PAD_H_
#define KAMI_KEY_PAD_H_

#ifdef	__cplusplus
extern "C" {
#endif

/*===========================================================================*/

#include <nitro.h>

/*---------------------------------------------------------------------------*
    ä÷êîíËã`
 *---------------------------------------------------------------------------*/

void kamiPadRead(void);
BOOL kamiPadIsTrigger(u16 key);
BOOL kamiPadIsRepeatTrigger(u16 key);
BOOL kamiPadIsPress(u16 key);

/*===========================================================================*/

#ifdef	__cplusplus
}          /* extern "C" */
#endif

#endif /* KAMI_KEY_PAD_H_ */

/*---------------------------------------------------------------------------*
  End of file
 *---------------------------------------------------------------------------*/
