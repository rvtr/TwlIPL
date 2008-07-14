/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInfo
  File:     viewSystemInfo.h

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

#ifndef	__LOAD_VIEW_INFO__
#define	__LOAD_VIEW_INFO__

#include <twl.h>
#include "drawFunc.h"

#ifdef __cplusplus
extern "C" {
#endif

////////////////////////////////

// �e��{�́A���[�U���
extern char** gAllInfo[ROOTMENU_KSIZE]; // ���ꂼ��̃��j���[���Ƃɍ��ڂ̒��g��"�������"���z��
extern u16 gUserName[OS_OWNERINFO_NICKNAME_MAX + 1];
extern u16 gUserComment[OS_OWNERINFO_COMMENT_MAX + 1];
extern u16 gSecretAnswer[OS_TWL_PCTL_SECRET_ANSWER_LENGTH_MAX + 1];

////////////////////////////////

void displayInfoMain( void );
void displayInfoInit( void );

////////////////////////////////




#ifdef __cplusplus
}
#endif

#endif  // __LOAD_VIEW_INFO__
