/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     cursor.c

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

#include <twl.h>
#include "graphics.h"
#include "cursor.h"

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    関数定義
 *---------------------------------------------------------------------------*/
void SetCursorPos(u16 x, u16 y)
{
	GXOamAttr oam;

/*  [] 
	G2_SetOBJAttr( 
		&oam, 
		x, y, 
		0, 
		GX_OAM_MODE_NORMAL, 
		FALSE, 
  		GX_OAM_EFFECT_FLIP_H, 
		GX_OAM_SHAPE_8x8, 
		GX_OAM_COLORMODE_16, 
		0xf6,	// charNo 
		0, 
		0 
	);

	DC_FlushRange( &oam, sizeof(GXOamAttr) );
	GXS_LoadOAM( &oam, 8, sizeof(GXOamAttr) );

	G2_SetOBJAttr( 
		&oam, 
		x+31, y, 
		0, 
		GX_OAM_MODE_NORMAL, 
		FALSE, 
  		GX_OAM_EFFECT_NONE, 
		GX_OAM_SHAPE_8x8, 
		GX_OAM_COLORMODE_16, 
		0xf6,	// charNo 
		0, 
		0 
	);

	DC_FlushRange( &oam, sizeof(GXOamAttr) );
	GXS_LoadOAM( &oam, 16, sizeof(GXOamAttr) );
*/

	G2_SetOBJAttr( 
		&oam, 
		x, y, 
		0, 
		GX_OAM_MODE_NORMAL, 
		FALSE, 
  		GX_OAM_EFFECT_NONE, 
		GX_OAM_SHAPE_8x8, 
		GX_OAM_COLORMODE_16, 
		0xff,	// charNo 
		0, 
		0 
	);
	DC_FlushRange( &oam, sizeof(GXOamAttr) );
	GXS_LoadOAM( &oam, 24, sizeof(GXOamAttr) );
}

