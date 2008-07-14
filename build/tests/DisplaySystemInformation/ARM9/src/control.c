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
		controlFlag = TRUE;
		
		if(*menu == MENU_ROOT)
		{
			// ���̉�ʂ̑I���ʒu���L�^
			selectLine[ROOTMENU_SIZE] = *line;

			// ���̃��j���[��ʂ��J��
			*menu = *line;
			*line = selectLine[*menu];
		}
		else
		{
			// !!! �ݒ�\�ȍ��ڂ�������ݒ�ύX���
		}
		
	}
	
	if( pad.trg & PAD_BUTTON_B )
	{
		if( *menu != MENU_ROOT )
		{
			// !!! �Ƃ肠�������̓��[�g�ɖ߂�
			// �l�ݒ��ʂ̎��̓L�����Z�����邾���ɂ���
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