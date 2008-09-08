/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_nandfirm.c

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
#include "kami_font.h"
#include "kami_pxi.h"
#include "process_topmenu.h"
#include "process_nandfirm.h"
#include "process_import.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "kami_write_nandfirm.h"
#include "common_utility.h"

#include "TWLHWInfo_api.h"
#include <firm/format/firm_common.h>
#include <../build/libraries/spi/ARM9/include/spi.h>

/*---------------------------------------------------------------------------*
    �}�N����`
 *---------------------------------------------------------------------------*/

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE		8
#define CHAR_OF_MENU_SPACE		1
#define CURSOR_ORIGIN_X			32
#define CURSOR_ORIGIN_Y			40

#define FILE_NUM_MAX			16

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s32 sMenuSelectNo;
static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
static u8 sFileNum;

/*---------------------------------------------------------------------------*
    �֐��錾
 *---------------------------------------------------------------------------*/

void MakeFullPathForSD(char* file_name, char* full_path);

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         Import �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess0(void)
{
    FSFile    dir;
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Import NandFirm from SD");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// �z��N���A
	MI_CpuClear8( sFilePath, sizeof(sFilePath) );

	// �t�@�C����������
	sFileNum = 0;

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 1, BG_COLOR_GREEN, BG_COLOR_GREEN );
	kamiFontFillChar( 2, BG_COLOR_GREEN, BG_COLOR_TRANS );

    // SD�J�[�h�̃��[�g�f�B���N�g��������
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
		kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory(sdmc:/)");
    }
    else
    {
        FSDirectoryEntryInfo   info[1];
        OS_Printf("[%s]:\n", "sdmc:/");

		kamiFontPrintfConsole(CONSOLE_ORANGE, "------ nand file list -----\n");

		// .nand ��T���ăt�@�C������ۑ����Ă���
        while (FS_ReadDirectory(&dir, info))
        {
            OS_Printf("  %s", info->longname);
            if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) != 0)
            {
                OS_Printf("/\n");
            }
            else
            {
				char* pExtension;
              OS_Printf(" (%d BYTEs)\n", info->filesize);

				// �g���q�̃`�F�b�N
				pExtension = STD_SearchCharReverse( info->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".nand") || !STD_CompareString( pExtension, ".NAND"))
					{
						STD_CopyString( sFilePath[sFileNum], info->longname );
						kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", sFileNum, info->longname);

						// �ő�16�ŏI��
						if (++sFileNum >= FILE_NUM_MAX)
						{
							break;
						}
					}
				}
            }
        }
        (void)FS_CloseDirectory(&dir);

		kamiFontPrintfConsole(CONSOLE_ORANGE, "--------------------------\n");
    }

	// ���j���[�ꗗ
	kamiFontPrintf((s16)3, (s16)4, FONT_COLOR_BLACK, "+--------------------+----+");
	kamiFontPrintf((s16)3, (s16)(5+sFileNum+1), FONT_COLOR_BLACK, "+--------------------+----+");

	// tad �t�@�C�����X�g��\��
	for (i=0;i<sFileNum; i++)
	{
		// �t�@�C�����ǉ�
		kamiFontPrintf((s16)3,  (s16)(5+CHAR_OF_MENU_SPACE*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sFilePath[i]);
	}

	// �Ō�Ƀ��^�[����ǉ�
	kamiFontPrintf((s16)3,  (s16)(5+CHAR_OF_MENU_SPACE*sFileNum), FONT_COLOR_BLACK, "l   RETURN           l    l");

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( NandfirmProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess1(void)
{
	// �I�[�g���s�p
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return NandfirmProcess2;
	}

	// �I�����j���[�̕ύX
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = sFileNum;
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNo > sFileNum) sMenuSelectNo = 0;
	}

	// �J�[�\���z�u
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// ����
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return NandfirmProcess2;
	}
	// �g�b�v���j���[�֖߂�
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	return NandfirmProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         �v���Z�X2

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* NandfirmProcess2(void)
{
	BOOL ret;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];

	if (STD_GetStringLength(sFilePath[sMenuSelectNo]))
	{
		// .nand�̃t���p�X���쐬
		MakeFullPathForSD(sFilePath[sMenuSelectNo], full_path);
		ret = kamiWriteNandfirm(full_path, OS_AllocFromMain, OS_FreeToMain);
	}
	else
	{
		if (gAutoFlag)	{ FADE_OUT_RETURN( AutoProcess2 ); 		}
		else 			{ FADE_OUT_RETURN( TopmenuProcess0 );	}
	}

	// ����̌��ʂ�\��
	if ( ret == TRUE )
	{
		kamiFontPrintf((s16)26,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, "OK");
	}
	else
	{
		kamiFontPrintf((s16)26,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_RED, "NG");
	}

	// Auto�p
	if (gAutoFlag)
	{
		if (ret) 
		{
			gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_NANDFIRM] = AUTO_PROCESS_RESULT_SUCCESS;  
			FADE_OUT_RETURN( AutoProcess1 ); 
		}
		else 
		{ 
			gAutoProcessResult[AUTO_PROCESS_MENU_IMPORT_NANDFIRM] = AUTO_PROCESS_RESULT_FAILURE;
			FADE_OUT_RETURN( AutoProcess2 ); 
		}
	}

	return NandfirmProcess1;
}
