/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_format.c

  Copyright 2008 Nintendo.  All rights reserved.

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
#include <nitro/snd.h>
#include <twl/fatfs.h>
#include <nitro/card.h>
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_format.h"
#include "process_auto.h"
#include "cursor.h"
#include "keypad.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define NUM_OF_MENU_SELECT    4
#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;
static u8 sLock;
static u8 sFormatResult;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static void FormatCallback(KAMIResult result, void* arg);

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Format �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* FormatProcess0(void)
{
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Format Nand Flash");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// ���j���[�ꗗ
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   QUICK FORMAT    l     l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l   FULL FORMAT     l     l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "l   CHECK DISK      l     l");
	kamiFontPrintf(3, 12, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "l   RETURN          l     l");
	kamiFontPrintf(3, 14, FONT_COLOR_BLACK, "+-------------------+-----+");

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_BLUE, BG_COLOR_BLUE );
	kamiFontFillChar( 1, BG_COLOR_BLUE, BG_COLOR_BLUE );
	kamiFontFillChar( 2, BG_COLOR_BLUE, BG_COLOR_TRANS );

	return FormatProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         Format �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* FormatProcess1(void)
{
	// �I�[�g���s�p
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return FormatProcess2;
	}

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
		return FormatProcess2;
	}
	// �g�b�v���j���[�֖߂�
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		return TopmenuProcess0;
	}

	return FormatProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         Format �v���Z�X�Q

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* FormatProcess2(void)
{
	if (sLock == FALSE)
	{
		switch( sMenuSelectNo )
		{
		case 0:
			sLock = TRUE;
			ExeFormatAsync(FORMAT_MODE_QUICK, FormatCallback);
			kamiFontPrintf(24,  7, FONT_COLOR_BLACK, "     ");
			return FormatProcess3;
		case 1:
			sLock = TRUE;
			ExeFormatAsync(FORMAT_MODE_FULL, FormatCallback);
			kamiFontPrintf(24,  9, FONT_COLOR_BLACK, "     ");
			return FormatProcess3;
		case 2:
			{
				FATFSDiskInfo info;
				BOOL result = FALSE;

				kamiFontPrintf(24,  11, FONT_COLOR_BLACK, "     ");
				kamiFontLoadScreenData();

				FATFS_UnmountDrive("F:");
				FATFS_UnmountDrive("G:");
	            // �w���NAND�p�[�e�B�V����0��F�h���C�u�Ƀ}�E���g
	            if (FATFS_MountDrive("F", FATFS_MEDIA_TYPE_NAND, 0))
				{
					// �`�F�b�N�f�B�X�N���s
					if (FATFS_CheckDisk("F:", &info, TRUE, TRUE, TRUE))
					{
			            // �w���NAND�p�[�e�B�V����1��G�h���C�u�Ƀ}�E���g
			            if (FATFS_MountDrive("G", FATFS_MEDIA_TYPE_NAND, 1))
						{
							// �`�F�b�N�f�B�X�N���s
							if (FATFS_CheckDisk("G:", &info, TRUE, TRUE, TRUE))
							{
								result = TRUE;
							}
						}
					}
				}

				// �f�t�H���g�}�E���g��Ԃɖ߂��Ă���
				FATFS_UnmountDrive("G:");
				FATFS_MountDrive("G", FATFS_MEDIA_TYPE_SD, 0);

				if (result == TRUE) { kamiFontPrintf(24,  11, FONT_COLOR_GREEN, " OK  "); }
				else                { kamiFontPrintf(24,  11, FONT_COLOR_RED,   " NG  "); }

				return FormatProcess1;
			}
		case 3:
			return TopmenuProcess0;
		}
	}

	return FormatProcess1;
}

static void FormatCallback(KAMIResult result, void* /*arg*/)
{
	s16 y_pos;

	if ( sMenuSelectNo == 0 ) y_pos = 7;
	else y_pos = 9;

	if ( result == KAMI_RESULT_SUCCESS_TRUE )
	{
		kamiFontPrintf(24,  y_pos, FONT_COLOR_GREEN, " OK  ");
		sFormatResult = TRUE;
	}
	else
	{
		kamiFontPrintf(24,  y_pos, FONT_COLOR_RED,   " NG  ");
		sFormatResult = FALSE;
	}

	// ���b�N����
	sLock = FALSE;
}

/*---------------------------------------------------------------------------*
  Name:         Format �v���Z�X�R

  Description:  �t�H�[�}�b�g�����҂��v���Z�X

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* FormatProcess3(void)
{
	static s32 progress;
	s16 y_pos;

	// �����I������
	if (sLock == FALSE)
	{
		progress = 0;
		
		// Auto�p
		if (gAutoFlag)
		{
			if (sFormatResult == TRUE) return AutoProcess1;
			else return AutoProcess2;
		}

		return FormatProcess1;
	}

	if ( sMenuSelectNo == 0 ) { y_pos = 7; }
	else { y_pos = 9; }

	// �i���\���X�V
	if ( ++progress >= 30*5 ) 
	{
		progress = 0;
		kamiFontPrintf(24, y_pos, FONT_COLOR_BLACK, "     ");
	}

	kamiFontPrintf((s16)(24 + (progress / 30)),  y_pos, FONT_COLOR_BLACK, "*");

	return FormatProcess3;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

