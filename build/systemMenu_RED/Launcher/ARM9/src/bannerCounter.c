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

void BNC_incrementCount( BannerCounter *c )
{
	// TWLのみカウントインクリメント
	if( c->banner->h.platform == BANNER_PLATFORM_TWL )
	{
		if( c->banner->anime.control[0].frameCount == 0 )
		{
			// アニメに終端しか存在しない
			OS_TPrintf( "BNC_incrementCount:Only a Terminator!\n" );
			return;
		}
		
		c->count++;
		if( c->count >= c->banner->anime.control[c->control].frameCount )
		{
			// カウント値がコントロールのフレームカウントを超えたので次のコントロールへ
			c->control++;
			c->count = 0;

			//ループ及び停止の処理
			if( c->control >= BANNER_ANIME_CONTROL_INFO_NUM )
			{
				// コントロールが限界を超えたら無条件でループ
				BNC_resetCount( c );
			}
			else if( c->banner->anime.control[c->control].frameCount == 0 )
			{
				// コントロールのフレームカウントが0なら終端到達
				if( c->banner->anime.control[c->control].animeType == 0 )
				{
					// アニメタイプ0ならループ
					BNC_resetCount( c );
				}
				else if( c->banner->anime.control[c->control].animeType == 1 )
				{
					// アニメタイプ1なら停止（一つ前のコントロールに戻す）
					c->control--;
				}
			}
		}
	}
}

FrameAnimeData BNC_getFAD( BannerCounter *c )
{
	FrameAnimeData ret;
	if( c->banner->h.platform == BANNER_PLATFORM_NTR )
	{
		ret.image = c->banner->v1.image;
		ret.pltt = c->banner->v1.pltt;
		ret.hflip = FALSE;
		ret.vflip = FALSE;
	}
	else
	{
		if( c->banner->anime.control[0].frameCount == 0 )
		{
			// アニメに終端しか存在しない
			OS_TPrintf( "BNC_getFAD:Only a Terminator!\n" );
			ret.image = c->banner->v1.image;
			ret.pltt = c->banner->v1.pltt;
			ret.hflip = FALSE;
			ret.vflip = FALSE;
			return ret;
		}
		// コントロールデータを読んで、現在のフレームに該当するデータを返す
		ret.image = c->banner->anime.image[ c->banner->anime.control[c->control].normal.cellNo ];
		ret.pltt = c->banner->anime.pltt[ c->banner->anime.control[c->control].normal.plttNo ];
		ret.hflip = c->banner->anime.control[c->control].normal.flipType & 0x1;
		ret.vflip = (c->banner->anime.control[c->control].normal.flipType & 0x2) >> 1;
	}
	return ret;
}

FrameAnimeData BNC_getFADAndIncCount( BannerCounter *c )
{
	FrameAnimeData ret = BNC_getFAD( c );
	BNC_incrementCount( c );
	return ret;
}

