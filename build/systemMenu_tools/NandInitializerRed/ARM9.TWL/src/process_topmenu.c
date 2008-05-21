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
#include "process_nandfirm.h"
#include "process_norfirm.h"
#include "process_auto.h"
#include "process_fade.h"
#include "process_wireless_setting.h"
#include "process_font.h"
#include "cursor.h"
#include "keypad.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

enum {
	MENU_FORMAT = 0,
	MENU_HARDWARE_INFO,

#ifdef    USE_WIRELESS_FORCE_DISABLE_SETTING
	MENU_WIRELESS_SETTING,
#endif // USE_WIRELESS_FORCE_DISABLE_SETTING

#ifdef    USE_WRITE_FONT_DATA
	MENU_FONT_DATA,
#endif // USE_WRITE_FONT_DATA

#ifndef   NAND_INITIALIZER_LIMITED_MODE
	MENU_IMPORT_TAD,
	MENU_IMPORT_NANDFIRM,
	MENU_IMPORT_NORFIRM,
#endif // NAND_INITIALIZER_LIMITED_MODE
	NUM_OF_MENU_SELECT
};

typedef struct _MenuAndColor
{
	char* menu_name;
	u8    color;
} MenuAndColor;

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56
#define MENU_TOP_LINE         7
#define CHAR_OF_MENU_SPACE    2

static const MenuAndColor sMenuArray[] =
{
	{"    FORMAT NAND            ", BG_COLOR_BLUE   },
	{"    WRITE HARDWARE INFO    ", BG_COLOR_PURPLE },

#ifdef    USE_WIRELESS_FORCE_DISABLE_SETTING
	{"    WIRELESS FORCE SETTING ", BG_COLOR_YELLOW },
#endif // USE_WIRELESS_FORCE_DISABLE_SETTING

#ifdef    USE_WRITE_FONT_DATA
	{"    WRITE FONT DATA        ", BG_COLOR_BROWN  },
#endif // USE_WRITE_FONT_DATA

#ifndef   NAND_INITIALIZER_LIMITED_MODE
	{"    IMPORT TAD FROM SD     ", BG_COLOR_PINK   },
	{"    IMPORT NANDFIRM FROM SD", BG_COLOR_GREEN  },
	{"    IMPORT NORFIRM  FROM SD", BG_COLOR_VIOLET }
#endif // NAND_INITIALIZER_LIMITED_MODE
};

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

void* TopmenuProcess0(void)
{
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
#ifndef NAND_INITIALIZER_LIMITED_MODE
	kamiFontPrintf(7, 2, 0, "Nand Initializer RED");
#else
	kamiFontPrintf(8, 2, 0, "Nand Initializer");
#endif
	kamiFontPrintf(9, 4, 8, "<%s>", __DATE__);

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	for (i=0;i<NUM_OF_MENU_SELECT;i++)
	{
		// ���j���[������`��
		kamiFontPrintf(3,  (s16)(MENU_TOP_LINE+CHAR_OF_MENU_SPACE*i), FONT_COLOR_BLACK, sMenuArray[i].menu_name);
		// �w�i�ݒ�
		kamiFontFillChar( MENU_TOP_LINE+CHAR_OF_MENU_SPACE*i-1, BG_COLOR_NONE, sMenuArray[i].color );
		kamiFontFillChar( MENU_TOP_LINE+CHAR_OF_MENU_SPACE*i,   sMenuArray[i].color,  sMenuArray[i].color );
		kamiFontFillChar( MENU_TOP_LINE+CHAR_OF_MENU_SPACE*i+1, sMenuArray[i].color,  BG_COLOR_NONE );
	}

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

#ifdef   USE_FOR_NIGHTLY_AUTO_TEST
		FADE_OUT_RETURN( FormatProcess0 );
#endif //USE_FOR_NIGHTLY_AUTO_TEST

#ifndef NAND_INITIALIZER_LIMITED_MODE

#ifdef AUTO_FORMAT_MODE
		FADE_OUT_RETURN( AutoProcess0 );
#endif

	// L&R���������ŃI�[�g���s�I
    if (kamiPadIsPress(PAD_BUTTON_L) && kamiPadIsPress(PAD_BUTTON_R))
	{
		FADE_OUT_RETURN( AutoProcess0 );
	}
#endif // NAND_INITIALIZER_LIMITED_MODE

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
	case MENU_FORMAT:
		FADE_OUT_RETURN( FormatProcess0 );
	case MENU_HARDWARE_INFO:
		FADE_OUT_RETURN( HWInfoProcess0 );

#ifdef    USE_WIRELESS_FORCE_DISABLE_SETTING
	case MENU_WIRELESS_SETTING:
		FADE_OUT_RETURN( WirelessSettingProcess0 );
#endif // USE_WIRELESS_FORCE_DISABLE_SETTING

#ifdef    USE_WRITE_FONT_DATA
	case MENU_FONT_DATA:
		FADE_OUT_RETURN( fontProcess0 );
#endif // USE_WRITE_FONT_DATA

#ifndef   NAND_INITIALIZER_LIMITED_MODE
	case MENU_IMPORT_TAD:
		FADE_OUT_RETURN( ImportProcess0 );
	case MENU_IMPORT_NANDFIRM:
		FADE_OUT_RETURN( NandfirmProcess0 );
	case MENU_IMPORT_NORFIRM:
		FADE_OUT_RETURN( NorfirmProcess0 );
#endif // NAND_INITIALIZER_LIMITED_MODE
	}

	return TopmenuProcess1;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/
