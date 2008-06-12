/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_mcu.c

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
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_mcu.h"
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

#define NUM_OF_MENU_SELECT    2
#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL WriteMcuData(void);

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         mcu �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* mcuProcess0(void)
{
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Write MCU Data");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// ���j���[�ꗗ
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   WRITE MCU DATA  l     l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l   RETURN          l     l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+-------------------+-----+");

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_GRAY, BG_COLOR_GRAY );
	kamiFontFillChar( 1, BG_COLOR_GRAY, BG_COLOR_GRAY );
	kamiFontFillChar( 2, BG_COLOR_GRAY, BG_COLOR_TRANS );

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( mcuProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         mcu �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* mcuProcess1(void)
{
/*
#ifndef NAND_INITIALIZER_LIMITED_MODE
	// �I�[�g���s�p
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return mcuProcess2;
	}
#endif
*/

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
		return mcuProcess2;
	}
	// �g�b�v���j���[�֖߂�
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return mcuProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         mcu �v���Z�X�Q

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* mcuProcess2(void)
{
	BOOL result;

	switch( sMenuSelectNo )
	{
	case 0:
		result = WriteMcuData();
		if (result)
		{
			kamiFontPrintf(25, 7, FONT_COLOR_GREEN, "OK");
		}
		else
		{
			kamiFontPrintf(25, 7, FONT_COLOR_RED, "NG");
		}
		break;
	case 1:
		FADE_OUT_RETURN( TopmenuProcess0 );
	}
/*
#ifndef NAND_INITIALIZER_LIMITED_MODE
	// Auto�p
	if (gAutoFlag)
	{
		if (result) { FADE_OUT_RETURN( AutoProcess1 ); }
		else { FADE_OUT_RETURN( AutoProcess2); }
	}
#endif
*/
	return mcuProcess1;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

static BOOL WriteMcuData(void)
{
    FSFile  file;	
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	void* pTempBuf;
	u32 file_size;
	u32 alloc_size;
	BOOL result = TRUE;

	// ROM�t�@�C���I�[�v��
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, MCU_DATA_FILE_PATH_IN_ROM);
	if (!open_is_ok)
	{
    	OS_Printf("FS_OpenFile(\"%s\") ... ERROR!\n", MCU_DATA_FILE_PATH_IN_ROM);
		return FALSE;
	}

	// ROM�t�@�C�����[�h
	file_size  = FS_GetFileLength(&file) ;
	alloc_size = ROUND_UP(file_size, 32) ;
	pTempBuf = OS_Alloc( alloc_size );
	SDK_NULL_ASSERT(pTempBuf);
	DC_InvalidateRange(pTempBuf, alloc_size);
	read_is_ok = FS_ReadFile( &file, pTempBuf, (s32)file_size );
	if (!read_is_ok)
	{
	    kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_ReadFile(\"%s\") ... ERROR!\n", MCU_DATA_FILE_PATH_IN_ROM);
		FS_CloseFile(&file);
		OS_Free(pTempBuf);
		return FALSE;
	}

	// ROM�t�@�C���N���[�Y
	FS_CloseFile(&file);

	if (kamiMcuWriteFirm(pTempBuf) != KAMI_RESULT_SUCCESS)
	{
		result = FALSE;
	}

	OS_Free(pTempBuf);

	return result;
}

