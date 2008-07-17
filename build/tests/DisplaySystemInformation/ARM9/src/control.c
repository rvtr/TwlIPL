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
		
	// �㉺�ō��ڕύX
	if( pad.trg & PAD_KEY_UP )
	{
		controlFlag = TRUE;
		
		if( --(*changeLine) < 0 )
		{
			// ���C�����f�N�������g�������ʃ}�C�i�X�ɂȂ������ԍŌ��
			*changeLine = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		controlFlag = TRUE;
		
		if( ++(*changeLine) >= linemax )
		{
			// ���C�����C���N�������g�������ʁAmaxline�𒴂�����ŏ���
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
				// �_���l�ł�int�ł��n���Ȃ��֐��͎c�O�ȑΉ�������
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

	// B�ŃL�����Z�����Ė߂�
	if( pad.trg & PAD_BUTTON_B )
	{
		controlFlag = TRUE;
		*changeMode = FALSE;
	}
	
	return controlFlag ? CHANGE_CONTROL : CHANGE_NOTHING ;
}


BOOL control( int *menu, int *line, int *changeLine, int *changeMode )
{
	int linemax = s_numMenu[*menu]; // �I�𒆃y�[�W�̍��ڐ�
	BOOL controlFlag = FALSE;				// �������삪��������TRUE�ɂȂ�

	// �㉺�ō��ڕύX
	if( pad.trg & PAD_KEY_UP )
	{
		controlFlag = TRUE;
		
		if( --(*line) < 0 )
		{
			// ���C�����f�N�������g�������ʃ}�C�i�X�ɂȂ������ԍŌ��
			*line = linemax - 1;
		}
	}
	else if( pad.trg & PAD_KEY_DOWN )
	{
		controlFlag = TRUE;
		
		if( ++(*line) >= linemax )
		{
			// ���C�����C���N�������g�������ʁAmaxline�𒴂�����ŏ���
			*line = 0;
		}
	}

	// ���E�Ńy�[�W����
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
			
			// ���̉�ʂ̑I���ʒu���L�^
			selectLine[ROOTMENU_SIZE] = *line;

			// ���̃��j���[��ʂ��J��
			*menu = *line;
			*line = selectLine[*menu];
		}
		else if( gAllInfo[*menu][*line].changable )
		{
			controlFlag = TRUE;

			// �ύX�\�ȍ��ڂ͕ύX��ʂ��J��
			*changeMode = TRUE;
			*changeLine = gAllInfo[*menu][*line].iValue;
		}
		
	}
	
	if( pad.trg & PAD_BUTTON_B )
	{
		if( *menu != MENU_ROOT )
		{
			controlFlag = TRUE;

			// �ݒ�l�\����ʂ̂Ƃ��̓��[�g�ɖ߂�
			selectLine[*menu] = *line;
			*menu = MENU_ROOT;
			*line = selectLine[ROOTMENU_SIZE];
		}
	}

	if( ( pad.trg & PAD_BUTTON_SELECT ) && *menu == MENU_SCFG_ARM7 )
	{
		controlFlag = TRUE;
		
		// ARM7SCFG�̕\���f�[�^��؂�ւ���
		switchViewMode();
	}
		
	return controlFlag;
}


int getMaxPage( int menu )
{
// �\�������j���[�̃y�[�W�����y�[�W����̂�
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
// �\�������j���[�ɂ����錻�݂̃y�[�W�������ڂ���̂�
	if( menu == MENU_ROOT) return ROOTMENU_SIZE;
	
	return s_pageOffset[menu][page+1] - s_pageOffset[menu][page];
}