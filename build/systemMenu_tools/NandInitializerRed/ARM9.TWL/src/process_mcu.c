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
#include "common_utility.h"

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

static s8 sMenuSelectNo;
static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];
static u8 sFileNum;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL WriteMcuData(char* full_path);

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
    FSFile    dir;
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Write MCU Data");
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
	kamiFontFillChar( 0, BG_COLOR_GRAY, BG_COLOR_GRAY );
	kamiFontFillChar( 1, BG_COLOR_GRAY, BG_COLOR_GRAY );
	kamiFontFillChar( 2, BG_COLOR_GRAY, BG_COLOR_TRANS );

    // SD�J�[�h�̃��[�g�f�B���N�g��������
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
        OS_Printf("Error FS_OpenDirectory(sdmc:/)\n");
		kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "Error FS_OpenDirectory(sdmc:/)");
    }
    else
    {
        FSDirectoryEntryInfo   info[1];
        OS_Printf("[%s]:\n", "sdmc:/");

		kamiFontPrintfConsole(CONSOLE_ORANGE, "------ hex file list -----\n");

		// .hex ��T���ăt�@�C������ۑ����Ă���
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
					if (!STD_CompareString( pExtension, ".hex") || !STD_CompareString( pExtension, ".HEX"))
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
#ifndef NAND_INITIALIZER_LIMITED_MODE
	// �I�[�g���s�p
	if (gAutoFlag)
	{
		sMenuSelectNo = 0;
		return mcuProcess2;
	}
#endif

	// �I�����j���[�̕ύX
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNo < 0) sMenuSelectNo = (s8)sFileNum;
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
	char full_path[FS_ENTRY_LONGNAME_MAX+6];

	if (STD_GetStringLength(sFilePath[sMenuSelectNo]))
	{
		kamiFontPrintf((s16)25,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_BLACK, "WAIT");
		kamiFontLoadScreenData();

		// .hex�̃t���p�X���쐬
		MakeFullPathForSD(sFilePath[sMenuSelectNo], full_path);
		result = WriteMcuData(full_path);
	}
	else
	{
		if (gAutoFlag)	{ FADE_OUT_RETURN( AutoProcess1 ); 		}
		else 			{ FADE_OUT_RETURN( TopmenuProcess0 );	}
	}

	// ����̌��ʂ�\��
	if ( result == TRUE )
	{
		kamiFontPrintf((s16)25,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_GREEN, " OK ");
	}
	else
	{
		kamiFontPrintf((s16)25,  (s16)(5+sMenuSelectNo*CHAR_OF_MENU_SPACE), FONT_COLOR_RED, " NG ");
	}

#ifndef NAND_INITIALIZER_LIMITED_MODE
	// Auto�p
	if (gAutoFlag)
	{
		if (result) 
		{ 
			gAutoProcessResult[AUTO_PROCESS_MENU_MCU] = AUTO_PROCESS_RESULT_SUCCESS; 
			FADE_OUT_RETURN( AutoProcess1 ); 
		}
		else 
		{ 
			gAutoProcessResult[AUTO_PROCESS_MENU_MCU] = AUTO_PROCESS_RESULT_FAILURE;  
			FADE_OUT_RETURN( AutoProcess2); 
		}
	}
#endif

	return mcuProcess1;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

static BOOL WriteMcuData(char* full_path)
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
    open_is_ok = FS_OpenFile(&file, full_path);
	if (!open_is_ok)
	{
    	OS_Printf("FS_OpenFile(\"%s\") ... ERROR!\n", full_path);
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
	    kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_ReadFile(\"%s\") ... ERROR!\n", full_path);
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

