/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     boot.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-8#$
  $Rev:  $
  $Author: yoshida_teruhisa $
 *---------------------------------------------------------------------------*/

#if !defined(_SYSMENU_BOOT_H_)
#define _SYSMENU_BOOT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Type definition
 *---------------------------------------------------------------------------*/
#define BOOTCORE_ARM7_ADDR			0x0380f100

/*---------------------------------------------------------------------------*
  functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         BOOT_Core

  Description:  ブートのコア関数

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BOOT_Core( void );

/*---------------------------------------------------------------------------*
  Name:         BOOT_Init

  Description:  ブートのための初期化

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BOOT_Init( void );

/*---------------------------------------------------------------------------*
  Name:         BOOT_WaitStart

  Description:  スタート待ち(ARM7)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL BOOT_WaitStart( void );

/*---------------------------------------------------------------------------*
  Name:         BOOT_WaitStart

  Description:  ブート開始及びARM7への通知及び待機(ARM9)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BOOT_Ready( void );

#ifdef __cplusplus
}
#endif

#endif /*	_SYSMENU_BOOT_H_	*/
