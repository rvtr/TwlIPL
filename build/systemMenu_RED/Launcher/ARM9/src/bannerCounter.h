/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     bannerCounter.h

  Copyright 2007 Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date:: 2007-11-20#$
  $Rev: 231 $
  $Author: yoshida_teruhisa $
 *---------------------------------------------------------------------------*/

#ifndef	__BANNERCOUNTER_H__
#define	__BANNERCOUNTER_H__

#include <twl.h>
#include <sysmenu.h>

#ifdef __cplusplus
extern "C" {
#endif

// define data-------------------------------------------------------

typedef struct BannerCounter
{
	u32 count;
	TWLBannerFile *banner;
}
BannerCounter;

typedef struct FrameAnimeData{
	u8 *image;
	u8 *pltt;
	BOOL vflip;
	BOOL hflip;
}
FrameAnimeData;

// global variables--------------------------------------------------

// function----------------------------------------------------------

static inline void BNC_resetCount( BannerCounter *c )
{
	c->count = 0;
}

static inline void BNC_initCounter( BannerCounter *c, TWLBannerFile *b)
{
	c->banner = b;
	c->count = 0;
}

static inline void BNC_setBanner( BannerCounter *c, TWLBannerFile *b)
{
	c->banner = b;
}

static inline TWLBannerFile* BNC_getBanner( BannerCounter *c )
{
	return c->banner;
}

static inline void BNC_incrementCount( BannerCounter *c )
{
	c->count++;
}

FrameAnimeData BNC_getFAD( BannerCounter *c );
FrameAnimeData BNC_getFADAndIncCount( BannerCounter *c );

#ifdef __cplusplus
}
#endif

#endif  // __BANNERCOUNTER_H__
