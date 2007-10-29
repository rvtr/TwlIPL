/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     logoDemo.h

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

#ifndef	_LOGO_DEMO_H
#define	_LOGO_DEMO_H

#include <twl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void LoadLogo( void );
extern int	LogoMain( void );
extern void SetLogoEnable( BOOL enable );
extern BOOL IsLogoEnable( void );

#ifdef __cplusplus
}
#endif

#endif	/* _LOGO_DEMO_H */
