/*---------------------------------------------------------------------------*
  Project:  SystemUpdater
  File:     process_delete_tad.c

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
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include <twl/nam.h>
#include <twl/lcfg.h>
#include <sysmenu/namut.h>
#include "kami_font.h"
#include "hw_info.h"
#include "TWLHWInfo_api.h"
#include "graphics.h"
#include "kami_global.h"
#include "font.h"

/*---------------------------------------------------------------------------*
    型定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    定数定義
 *---------------------------------------------------------------------------*/
#define TITLE_ID_MUST_ERASE_NUM   3
#define TITLE_ID_LIST_NUM         OS_TWL_REGION_MAX

/*---------------------------------------------------------------------------*
    内部変数定義
 *---------------------------------------------------------------------------*/

static const u64 sTitleIdListMust[TITLE_ID_MUST_ERASE_NUM] =
{
	0x00030017484e4141,	// ALL       HNAA
	0x00030015484e4241,	// ALL       HNBA
	0x0003000f484e4c41,	// ALL       HNLA
};

static const u64 sTitleIdListHNA[TITLE_ID_LIST_NUM] =
{
	0x00030017484e414A,	// Japan     HNAJ
	0x00030017484e4145,	// America   HNAE
	0x00030017484e4150,	// Europe    HNAP
	0x00030017484e4155,	// Australia HNAU
	0x00030017484e4143,	// China     HNAC
	0x00030017484e414B,	// Korea     HNAK

};

static const u64 sTitleIdListHNB[TITLE_ID_LIST_NUM] =
{
	0x00030015484e424A,	// Japan     HNBJ
	0x00030015484e4245,	// America   HNBE
	0x00030015484e4250,	// Europe    HNBP
	0x00030015484e4255,	// Australia HNBU
	0x00030015484e4243,	// China     HNBC
	0x00030015484e424B,	// Korea     HNBK
};

static const u64 sTitleIdListHNL[TITLE_ID_LIST_NUM] =
{
	0x0003000f484e4c4A,	// Japan     HNLJ
	0x0003000f484e4c45,	// America   HNLE
	0x0003000f484e4c50,	// Europe    HNLP
	0x0003000f484e4c55,	// Australia HNLU
	0x0003000f484e4c43,	// China     HNLC
	0x0003000f484e4c4B,	// Korea     HNLK
};

static const u64 sTitleIdListHNI[TITLE_ID_LIST_NUM] =
{
	0x00030005484e494A,	// Japan     HNIJ
	0x00030005484e4945,	// America   HNIE
	0x00030005484e4950,	// Europe    HNIP
	0x00030005484e4955,	// Australia HNIU
	0x00030005484e4943,	// China     HNIC
	0x00030005484e494B,	// Korea     HNIK
};

static const u64 sTitleIdListHNK[TITLE_ID_LIST_NUM] =
{
	0x00030005484e4B4A,	// Japan     HNKJ
	0x00030005484e4B45,	// America   HNKE
	0x00030005484e4B50,	// Europe    HNKP
	0x00030005484e4B55,	// Australia HNKU
	0x00030005484e4B43,	// China     HNKC
	0x00030005484e4B4B,	// Korea     HNKK
};


static const u64 sTitleIdListHNF[TITLE_ID_LIST_NUM] =
{
	0x00030015484E464A,	// Japan     HNFJ
	0x00030015484E4645,	// America   HNFE
	0x00030015484E4650,	// Europe    HNFP
	0x00030015484E4655,	// Australia HNFU
	0x00030015484E4643,	// China     HNFC
	0x00030015484E464B,	// Korea     HNFK
};

static const u64 sTitleIdListHNJ[TITLE_ID_LIST_NUM] =
{
	0x00030005484E4A4A,	// Japan     HNJJ
	0x00030005484E4A45,	// America   HNJE
	0x00030005484E4A50,	// Europe    HNJP
	0x00030005484E4A55,	// Australia HNJU
	0x00030005484E4A43,	// China     HNJC
	0x00030005484E4A4B,	// Korea     HNJK
};


/*---------------------------------------------------------------------------*
    内部関数宣言
 *---------------------------------------------------------------------------*/
static BOOL DeleteTitle(u64 titleId);

/*---------------------------------------------------------------------------*
    処理関数定義
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessDeleteOtherResionSysmenu

  Description:  選択リージョン以外のSystemMenuを消去する。
                ユーザーアプリは本体初期化の際に消去される。

  Arguments:    なし。

  Returns:      なし。
 *---------------------------------------------------------------------------*/
BOOL ProcessDeleteOtherResionSysmenu(void)
{
	s32  i;
	BOOL ret = TRUE;

	// ランチャーと本体設定とSysMenuバージョンのALL版は全て消去する
	for (i=0;i<TITLE_ID_MUST_ERASE_NUM;i++)
	{
		ret = DeleteTitle( sTitleIdListMust[i] );
	}

	// 選択リージョンと異なるランチャーは消去する
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNA[i] );
		}
	}

	// 選択リージョンと異なる本体設定は消去する
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNB[i] );
		}
	}

	// 選択リージョンと異なるSysMenuバージョンは消去する
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNL[i] );
		}
	}

	// 選択リージョンと異なる写真帳は消去する
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNI[i] );
		}
	}

	// 選択リージョンと異なるDSiサウンドは消去する
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNK[i] );
		}
	}

	// 選択リージョンと異なるショップは消去する
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNF[i] );
		}
	}

	// 選択リージョンと異なるNintendoゾーンは消去する
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNJ[i] );
		}
	}

	return ret;
}

static BOOL DeleteTitle(u64 titleId)
{
	s32  nam_result = NAM_DeleteTitleCompletely( titleId );
	if ( nam_result != NAM_OK )
	{
		kamiFontPrintfConsole(CONSOLE_RED, "tad delete Fail! RetCode=%x\n", nam_result);
		return FALSE;
	}
	return TRUE;
} 
