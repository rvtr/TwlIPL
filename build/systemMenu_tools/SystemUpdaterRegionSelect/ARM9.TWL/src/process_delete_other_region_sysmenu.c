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
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/
#define TITLE_ID_MUST_ERASE_NUM   3
#define TITLE_ID_LIST_NUM         4

/*---------------------------------------------------------------------------*
    �����ϐ���`
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
};

static const u64 sTitleIdListHNB[TITLE_ID_LIST_NUM] =
{
	0x00030015484e424A,	// Japan     HNBJ
	0x00030015484e4245,	// America   HNBE
	0x00030015484e4250,	// Europe    HNBP
	0x00030015484e4255,	// Australia HNBU
};

static const u64 sTitleIdListHNL[TITLE_ID_LIST_NUM] =
{
	0x0003000f484e4c4A,	// Japan     HNLJ
	0x0003000f484e4c45,	// America   HNLE
	0x0003000f484e4c50,	// Europe    HNLP
	0x0003000f484e4c55,	// Australia HNLU
};

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/
static BOOL DeleteTitle(u64 titleId);

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         ProcessDeleteOtherResionSysmenu

  Description:  �I�����[�W�����ȊO��SystemMenu����������B
                ���[�U�[�A�v���͖{�̏������̍ۂɏ��������B

  Arguments:    �Ȃ��B

  Returns:      �Ȃ��B
 *---------------------------------------------------------------------------*/
BOOL ProcessDeleteOtherResionSysmenu(void)
{
	s32  i;
	BOOL ret = TRUE;

	// �����`���[�Ɩ{�̐ݒ��SysMenu�o�[�W������ALL�ł͑S�ď�������
	for (i=0;i<TITLE_ID_MUST_ERASE_NUM;i++)
	{
		ret = DeleteTitle( sTitleIdListMust[i] );
	}

	// �I�����[�W�����ƈقȂ郉���`���[�͏�������
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNA[i] );
		}
	}

	// �I�����[�W�����ƈقȂ�{�̐ݒ�͏�������
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNB[i] );
		}
	}

	// �I�����[�W�����ƈقȂ�SysMenu�o�[�W�����͏�������
	for (i=0;i<TITLE_ID_LIST_NUM;i++)
	{
		if (i != gRegion)
		{
			ret = DeleteTitle( sTitleIdListHNL[i] );
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