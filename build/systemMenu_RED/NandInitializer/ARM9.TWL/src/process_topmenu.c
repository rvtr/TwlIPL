/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_topmenu.c

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

#ifndef NAND_FORMATTER_MODE
#define NUM_OF_MENU_SELECT    6
#else
#define NUM_OF_MENU_SELECT    2
#endif

#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;

#ifdef AUTO_FORMAT_MODE
static BOOL sAutoProcessFlag = TRUE;
#endif

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

void* TopmenuProcess0(void)
{
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
#ifndef NAND_FORMATTER_MODE
	kamiFontPrintf(8, 2, 0, "Nand Initializer");
#else
	kamiFontPrintf(9, 2, 0, "Nand Formatter");
#endif
	kamiFontPrintf(9, 4, 8, "<%s>", __DATE__);

	// ���j���[�ꗗ
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "    FORMAT NAND            ");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "    WRITE HARDWARE INFO    ");
#ifndef NAND_FORMATTER_MODE
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "    WRITE ETICKET SIGN     ");
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "    INPORT TAD FROM SD     ");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "    INPORT NANDFIRM FROM SD");
	kamiFontPrintf(3, 17, FONT_COLOR_BLACK, "    INPORT NORFIRM FROM SD");
#endif

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�ݒ�
	kamiFontFillChar( 6, BG_COLOR_TRANS, BG_COLOR_BLUE );
	kamiFontFillChar( 7, BG_COLOR_BLUE,  BG_COLOR_BLUE );
	kamiFontFillChar( 8, BG_COLOR_BLUE,  BG_COLOR_TRANS );

	kamiFontFillChar( 8, BG_COLOR_NONE,  BG_COLOR_PURPLE );
	kamiFontFillChar( 9, BG_COLOR_PURPLE, BG_COLOR_PURPLE );
	kamiFontFillChar(10, BG_COLOR_PURPLE, BG_COLOR_TRANS );
#ifndef NAND_FORMATTER_MODE
	kamiFontFillChar(10, BG_COLOR_NONE,  BG_COLOR_GRAY );
	kamiFontFillChar(11, BG_COLOR_GRAY, BG_COLOR_GRAY );
	kamiFontFillChar(12, BG_COLOR_GRAY, BG_COLOR_TRANS );

	kamiFontFillChar(12, BG_COLOR_NONE,  BG_COLOR_PINK );
	kamiFontFillChar(13, BG_COLOR_PINK, BG_COLOR_PINK );
	kamiFontFillChar(14, BG_COLOR_PINK, BG_COLOR_TRANS );

	kamiFontFillChar(14, BG_COLOR_NONE,  BG_COLOR_GREEN );
	kamiFontFillChar(15, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar(16, BG_COLOR_GREEN, BG_COLOR_TRANS );

	kamiFontFillChar(16, BG_COLOR_NONE,  BG_COLOR_VIOLET );
	kamiFontFillChar(17, BG_COLOR_VIOLET, BG_COLOR_VIOLET );
	kamiFontFillChar(18, BG_COLOR_VIOLET, BG_COLOR_TRANS );
#endif

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN(TopmenuProcess1);
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* TopmenuProcess1(void)
{
	// �I�����j���[�̕ύX
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = NUM_OF_MENU_SELECT -1;
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNo >= NUM_OF_MENU_SELECT) sMenuSelectNo = 0;
	}

	// �J�[�\���z�u
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// ����
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return TopmenuProcess2;
	}


#ifndef NAND_FORMATTER_MODE

#ifdef AUTO_FORMAT_MODE
	// sAutoProcessFlag �ŃI�[�g���s
	if (sAutoProcessFlag)
	{
		sAutoProcessFlag = FALSE;
		FADE_OUT_RETURN( AutoProcess0 );
	}
#endif

	// L&R���������ŃI�[�g���s�I
    if (kamiPadIsPress(PAD_BUTTON_L) && kamiPadIsPress(PAD_BUTTON_R))
	{
		FADE_OUT_RETURN( AutoProcess0 );
	}
#endif

	return TopmenuProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         Top Menu �v���Z�X�Q

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* TopmenuProcess2(void)
{
	switch ( sMenuSelectNo )
	{
	case 0:
		FADE_OUT_RETURN( FormatProcess0 );
	case 1:
		FADE_OUT_RETURN( HWInfoProcess0 );
#ifndef NAND_FORMATTER_MODE
	case 2:
		FADE_OUT_RETURN( eTicketProcess0 );
	case 3:
		FADE_OUT_RETURN( ImportProcess0 );
	case 4:
		FADE_OUT_RETURN( NandfirmProcess0 );
	case 5:
		FADE_OUT_RETURN( NorfirmProcess0 );
#endif
	}

	return TopmenuProcess1;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

