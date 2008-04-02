/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_auto.c

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
#include "kami_font.h"
#include "process_topmenu.h"
#include "process_format.h"
#include "process_hw_info.h"
#include "process_import.h"
#include "process_eticket.h"
#include "process_nandfirm.h"
#include "process_norfirm.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define NUM_OF_MENU_SELECT    6
#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56

/*---------------------------------------------------------------------------*
    �O���[�o���ϐ���`
 *---------------------------------------------------------------------------*/

BOOL gAutoFlag = FALSE;

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Top Menu �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* AutoProcess0(void)
{
	// �I�[�g�t���O�Z�b�g
	gAutoFlag = TRUE;

	// ���j���[������
	sMenuSelectNo = 0;

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	return AutoProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* AutoProcess1(void)
{
	switch ( sMenuSelectNo++ )
	{
	case 0:
		return FormatProcess0;
	case 1:
		return HWInfoProcess0;
		break;
	case 2:
		return eTicketProcess0;
	case 3:
		return ImportProcess0;
	case 4:
		return NandfirmProcess0;
	case 5:
		return AutoProcess2;
	}

	// never reach
	return TopmenuProcess0;
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu �v���Z�X2

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* AutoProcess2(void)
{
	int i;
	u8 bg_color;

	// ������S�N���A
	kamiFontClear();

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Auto Nand Initialization");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// ���j���[�ꗗ
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "    FORMAT NAND            ");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "    WRITE HARDWARE INFO    ");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "    WRITE ETICKET SIGN     ");
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "    INPORT TAD FROM SD     ");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "    INPORT NANDFIRM FROM SD");
#ifndef AUTO_FORMAT_MODE
	kamiFontPrintf(3, 22, FONT_COLOR_BLACK, " Button B : return to menu");
#endif

	for (i=0;i<sMenuSelectNo-1;i++)
	{
		kamiFontPrintf(3, (s16)(7+2*i), FONT_COLOR_GREEN, "OK");
	}

	// ���s����
	if (i<5)
	{
		kamiFontPrintf(3, (s16)(7+2*i), FONT_COLOR_RED, "NG");
		kamiFontPrintf(3, 18, FONT_COLOR_BLACK, "    Error Occured!");
		bg_color = BG_COLOR_RED;
	}
	// ���s�Ȃ�
	else
	{
		kamiFontPrintf(3, 18, FONT_COLOR_BLACK, "   Finished Successfully!");
		bg_color = BG_COLOR_GREEN;
	}

	// �w�i�㕔
	kamiFontFillChar( 0, bg_color, bg_color );
	kamiFontFillChar( 1, bg_color, bg_color );
	kamiFontFillChar( 2, bg_color, BG_COLOR_TRANS );

	// �w�i����
	kamiFontFillChar(17, BG_COLOR_TRANS, bg_color );
	kamiFontFillChar(18, bg_color, bg_color );
	kamiFontFillChar(19, bg_color, BG_COLOR_TRANS );

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	// �I�[�g�t���O�N���A
	gAutoFlag = FALSE;

	FADE_IN_RETURN( AutoProcess3 );
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu �v���Z�X3

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* AutoProcess3(void)
{
#ifdef AUTO_FORMAT_MODE
	// �����\�t�g�ł̓I�[�g���������������i�K��Terminate�����܂��B
	OS_Terminate();
#endif

    if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return AutoProcess3;
}


/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

