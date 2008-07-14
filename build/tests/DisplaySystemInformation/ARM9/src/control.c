/*---------------------------------------------------------------------------*
  Project:  TwlIPL - tests - DisplaySystemInformation
  File:     control.c

  Copyright **** Nintendo.  All rights reserved.

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
#include "misc.h"
#include "drawFunc.h"
#include "control.h"
#include "strResource.h"

static int selectLine[ROOTMENU_SIZE+1];


BOOL control( int *menu, int *line )
{
	int linemax = s_numMenu[*menu]; // 選択中ページの項目数
	BOOL controlFlag = FALSE;				// 何か操作があったらTRUEになる

	// 上下で項目変更								
	if( pad.trg & PAD_KEY_UP )
	{
		controlFlag = TRUE;
		
		if( --(*line) < 0 )
		{
			// ラインをデクリメントした結果マイナスになったら一番最後へ
			*line = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		controlFlag = TRUE;
		
		if( ++(*line) >= linemax )
		{
			// ラインをインクリメントした結果、maxlineを超えたら最初へ
			*line = 0;
		}
	}

	// 左右でページ送り
	if( pad.trg & PAD_KEY_RIGHT )
	{
		controlFlag = TRUE;
		*line += DISP_NUM_LINES - 2;
		
		if( *line >= linemax )
		{
			*line = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_LEFT )
	{
		controlFlag = TRUE;
		*line -= DISP_NUM_LINES - 2;
		
		if( *line < 0 )
		{
			*line = 0;
		}
	}


	if( pad.trg & PAD_BUTTON_A )
	{
		controlFlag = TRUE;
		
		if(*menu == MENU_ROOT)
		{
			// 今の画面の選択位置を記録
			selectLine[ROOTMENU_SIZE] = *line;

			// 次のメニュー画面を開く
			*menu = *line;
			*line = selectLine[*menu];
		}
		else
		{
			// !!! 設定可能な項目だったら設定変更画面
		}
		
	}
	
	if( pad.trg & PAD_BUTTON_B )
	{
		if( *menu != MENU_ROOT )
		{
			// !!! とりあえず今はルートに戻る
			// 値設定画面の時はキャンセルするだけにする
			controlFlag = TRUE;
			
			selectLine[*menu] = *line;
			*menu = MENU_ROOT;
			*line = selectLine[ROOTMENU_SIZE];
		}
	}
		
	return controlFlag;
}


int getMaxPage( int menu )
{
// 表示中メニューのページが何ページあるのか
	int i;
	
	if( menu == MENU_ROOT ) return 0;
	
	for(i=0; i<MAXPAGE; i++ )
	{
		if( s_pageOffset[menu][i] == s_numMenu[menu] )
		{
			return i;
		}
	}
	
	return 0;
}

int getMaxLine( int menu , int page )
{
// 表示中メニューにおける現在のページが何項目あるのか
	if( menu == MENU_ROOT) return ROOTMENU_SIZE;
	
	return s_pageOffset[menu][page+1] - s_pageOffset[menu][page];
}