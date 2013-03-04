/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     loadSharedFont.h

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

#ifndef	__LOAD_SHARED_FONT_H__
#define	__LOAD_SHARED_FONT_H__

#include <twl.h>
#include <twl/os/common/sharedFont.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data-------------------------------------------------------
typedef struct _STicks
{
    OSTick  all;    // ‘S•”‚ÌŽžŠÔ
    OSTick  comp;   // ‰ð“€‚ÌŽžŠÔ
} STicks;

// global variables--------------------------------------------------
extern BOOL g_isSucceededLoad[ OS_SHARED_FONT_MAX ];

// function----------------------------------------------------------
void   LoadSharedFont( STicks *pTicks );
void   WriteFontIntoSD( void );

#ifdef __cplusplus
}
#endif

#endif  // __LOAD_SHARED_FONT_H__
