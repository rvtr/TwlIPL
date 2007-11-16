/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     launcher.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/

#ifndef	__LAUNCHER_H__
#define	__LAUNCHER_H__

#include <twl.h>
#include <sysmenu.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data-------------------------------------------------------
#define SYSMENU_VER			0x071115

// global variables--------------------------------------------------

// function----------------------------------------------------------
void LauncherInit( TitleProperty *pTitleList );

void LauncherLoading( TitleProperty *pTitleList );
TitleProperty *LauncherMain( TitleProperty *pTitleList );


#ifdef __cplusplus
}
#endif

#endif  // __LAUNCHER_H__
