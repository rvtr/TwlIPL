/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     boot_api.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::           $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#if !defined(_SYSMENU_BOOT_H_)
#define _SYSMENU_BOOT_API_H_

#include <sysmenu/memorymap.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*
  Type definition
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  functions
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         BOOT_Core

  Description:  �u�[�g�̃R�A�֐�

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BOOT_Core( void );

/*---------------------------------------------------------------------------*
  Name:         BOOT_Init

  Description:  �u�[�g�̂��߂̏�����

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BOOT_Init( void );

/*---------------------------------------------------------------------------*
  Name:         BOOT_WaitStart

  Description:  �X�^�[�g�҂�(ARM7)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
BOOL BOOT_WaitStart( void );

/*---------------------------------------------------------------------------*
  Name:         BOOT_WaitStart

  Description:  �u�[�g�J�n�y��ARM7�ւ̒ʒm�y�ёҋ@(ARM9)

  Arguments:    None.

  Returns:      None.
 *---------------------------------------------------------------------------*/
void BOOT_Ready( void );

#ifdef __cplusplus
}
#endif

#endif /*	_SYSMENU_BOOT_API_H_	*/
