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
#include "viewSystemInfo.h"

static int selectLine[ROOTMENU_SIZE+1];

ChangeCotnrolResult changeControl( int *menu, int *line, int *changeLine, int *changeMode )
{
	int linemax = gAllInfo[*menu][*line].numKindName;
	BOOL controlFlag = FALSE;

	if( !gAllInfo[*menu][*line].changable )
	{
		*changeMode = FALSE;
		return CHANGE_CONTROL;
	}
		
	// 上下で項目変更
	if( pad.trg & PAD_KEY_UP )
	{
		controlFlag = TRUE;
		
		if( --(*changeLine) < 0 )
		{
			// ラインをデクリメントした結果マイナスになったら一番最後へ
			*changeLine = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		controlFlag = TRUE;
		
		if( ++(*changeLine) >= linemax )
		{
			// ラインをインクリメントした結果、maxlineを超えたら最初へ
			*changeLine = 0;
		}
	}

	if( pad.trg & PAD_BUTTON_A )
	{
		switch( gAllInfo[*menu][*line].argType )
		{
			case ARG_INT:
				gAllInfo[*menu][*line].changeFunc.cInt(*changeLine);
				break;
			
			case ARG_BOOL:
				gAllInfo[*menu][*line].changeFunc.cBool(*changeLine);
				break;
				
			case ARG_OTHER:
				// 論理値でもintでも渡せない関数は残念な対応をする
				if( *menu == MENU_SCFG_ARM7 && *line == SCFG_ARM9_RST_DSP )
				{
					*changeLine == 0 ? SCFG_ReleaseResetDSP(): SCFG_ResetDSP();
				}
				else if( *menu == MENU_SCFG_ARM7 && *line == SCFG_ARM9_EXT_PS )
				{
					SCFGPsramBoundary idx = SCFG_PSRAM_BOUNDARY_4MB;
					
					switch(*changeLine)
					{
						case 0:
							idx = SCFG_PSRAM_BOUNDARY_4MB;
							break;
						case 1:
							idx = SCFG_PSRAM_BOUNDARY_16MB;
							break;
						case 2:
							idx = SCFG_PSRAM_BOUNDARY_32MB;
							break;
					}
					
					SCFG_SetPsramBoundary( idx );
					
				}
				
				else if( *menu == MENU_SCFG_ARM7 && *line == SCFG_ARM9_EXT_CFG )
				{
					if( *changeLine == 0 )
					{
						SCFG_SetConfigBlockInaccessible();
					}
				}
				
				break;
		}
		
		return CHANGE_VALUE_CHANGED;
	}

	// Bでキャンセルして戻る
	if( pad.trg & PAD_BUTTON_B )
	{
		controlFlag = TRUE;
		*changeMode = FALSE;
	}
	
	return controlFlag ? CHANGE_CONTROL : CHANGE_NOTHING ;
}


BOOL control( int *menu, int *line, int *changeLine, int *changeMode )
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
		if(*menu == MENU_ROOT)
		{
			controlFlag = TRUE;
			
			// 今の画面の選択位置を記録
			selectLine[ROOTMENU_SIZE] = *line;

			// 次のメニュー画面を開く
			*menu = *line;
			*line = selectLine[*menu];
		}
		else if( gAllInfo[*menu][*line].changable )
		{
			controlFlag = TRUE;

			// 変更可能な項目は変更画面を開く
			*changeMode = TRUE;
			*changeLine = gAllInfo[*menu][*line].iValue;
		}
		
	}
	
	if( pad.trg & PAD_BUTTON_B )
	{
		if( *menu != MENU_ROOT )
		{
			controlFlag = TRUE;

			// 設定値表示画面のときはルートに戻る
			selectLine[*menu] = *line;
			*menu = MENU_ROOT;
			*line = selectLine[ROOTMENU_SIZE];
		}
	}

	if( ( pad.trg & PAD_BUTTON_SELECT ) && *menu == MENU_SCFG_ARM7 )
	{
		controlFlag = TRUE;
		
		// ARM7SCFGの表示データを切り替える
		switchViewMode();
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