/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     fade.c

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

#include <twl.h>
#include "kami_global.h"

/*---------------------------------------------------------------------------*
    ’è”’è‹`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    “à•”•Ï”’è‹`
 *---------------------------------------------------------------------------*/

static int brightness = 0;
static int master_brightness = -16;
static int counter;

/*---------------------------------------------------------------------------*
    “à•”ŠÖ”’è‹`
 *---------------------------------------------------------------------------*/
BOOL FadeOutTick(void)
{
	if ( brightness < 16)
	{
//		if (++counter == 2)
//		{
			counter = 0;
			G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1, ++brightness );
//		}
		return FALSE;
	}
	return TRUE;
}

BOOL FadeInTick(void)
{
	if ( brightness > 0)
	{
//		if (++counter == 2)
//		{
			counter = 0;
			G2_SetBlendBrightness( GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1, --brightness );
//		}
		return FALSE;
	}
	return TRUE;
}

BOOL FadeInMaster(void)
{
	if ( master_brightness < 0)
	{
		if (++counter == 2)
		{
			counter = 0;
			GX_SetMasterBrightness( ++master_brightness );
		}
		return FALSE;
	}
	return TRUE;
}

