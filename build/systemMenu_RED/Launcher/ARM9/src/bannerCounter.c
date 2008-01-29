/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     launcher.c

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2008-01-29#$
  $Rev: 533 $
  $Author: yoshida_teruhisa $
 *---------------------------------------------------------------------------*/

#include "bannerCounter.h"

// define data------------------------------------------

// extern data------------------------------------------

// function's prototype declaration---------------------

// global variable -------------------------------------

// static variable -------------------------------------

// const data  -----------------------------------------

//===============================================
// bannerCounter.c
//===============================================

FrameAnimeData BNC_getFAD( BannerCounter *c )
{
	FrameAnimeData ret;
	if( c->banner->h.platform == BANNER_PLATFORM_NTR )
	{
		ret.image = c->banner->v1.image;
		ret.pltt = c->banner->v1.pltt;
		ret.vflip = FALSE;
		ret.hflip = FALSE;
	}
	else
	{
		//TODO:TWLアニメーションバナーのデータを読んでフレームのデータを返す
	}
	return ret;
}

FrameAnimeData BNC_getFADAndIncCount( BannerCounter *c )
{
	FrameAnimeData ret = BNC_getFAD( c );
	BNC_incrementCount( c );
	return ret;
}

