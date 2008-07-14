/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     scanWDS.h

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

#ifndef	__SCAN_WDS_H__
#define	__SCAN_WDS_H__

#include <twl.h>
#include <sysmenu.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data-------------------------------------------------------
//#define ENABLE_WDS_PRINTF	1

// global variables--------------------------------------------------

// function----------------------------------------------------------
void InitializeWDS( void );
BOOL IsClearnupWDSWrapper( void );
void Callback_WDSWrapper( void *ptr );
void Callback_WDSPreSleep( void *ptr );
void Callback_WDSPostSleep( void *ptr );

#ifdef ENABLE_WDS_PRINTF
#define WDS_PRINTF				OS_TPrintf
#else
#define WDS_PRINTF(...)			((void) 0)
#endif

#ifdef __cplusplus
}
#endif

#endif  // __SCAN_WDS_H__
