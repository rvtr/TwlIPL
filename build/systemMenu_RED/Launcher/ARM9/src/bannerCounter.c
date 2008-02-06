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
	// TWL�̂݃J�E���g�C���N�������g
	if( c->banner->h.platform == BANNER_PLATFORM_TWL )
	{
		if( c->banner->anime.control[0].frameCount == 0 )
		{
			// �A�j���ɏI�[�������݂��Ȃ�
			OS_TPrintf( "BNC_incrementCount:Only a Terminator!\n" );
			return;
		}
		
		c->count++;
		if( c->count >= c->banner->anime.control[c->control].frameCount )
		{
			// �J�E���g�l���R���g���[���̃t���[���J�E���g�𒴂����̂Ŏ��̃R���g���[����
			c->control++;
			c->count = 0;

			//���[�v�y�ђ�~�̏���
			if( c->control >= BANNER_ANIME_CONTROL_INFO_NUM )
			{
				// �R���g���[�������E�𒴂����疳�����Ń��[�v
				BNC_resetCount( c );
			}
			else if( c->banner->anime.control[c->control].frameCount == 0 )
			{
				// �R���g���[���̃t���[���J�E���g��0�Ȃ�I�[���B
				if( c->banner->anime.control[c->control].animeType == 0 )
				{
					// �A�j���^�C�v0�Ȃ烋�[�v
					BNC_resetCount( c );
				}
				else if( c->banner->anime.control[c->control].animeType == 1 )
				{
					// �A�j���^�C�v1�Ȃ��~�i��O�̃R���g���[���ɖ߂��j
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
			// �A�j���ɏI�[�������݂��Ȃ�
			OS_TPrintf( "BNC_getFAD:Only a Terminator!\n" );
			ret.image = c->banner->v1.image;
			ret.pltt = c->banner->v1.pltt;
			ret.hflip = FALSE;
			ret.vflip = FALSE;
			return ret;
		}
		// �R���g���[���f�[�^��ǂ�ŁA���݂̃t���[���ɊY������f�[�^��Ԃ�
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

