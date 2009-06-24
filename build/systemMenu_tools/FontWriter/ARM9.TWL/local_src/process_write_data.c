/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     process_write_data.c

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
#include "process_write_data.h"
#include "process_auto.h"
#include "process_fade.h"
#include "cursor.h"
#include "keypad.h"
#include "common_utility.h"

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

enum {
	MENU_FONT=0,
	NUM_OF_MENU_SELECT
};

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56
#define CURSOR_ORIGIN_Y_INDIVIDUALLY      40
#define CHAR_OF_MENU_SPACE    2
#define CHAR_OF_MENU_SPACE_INDIVIDUALLY  1

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

#define DOT_OF_MENU_SPACE               16
#define DOT_OF_MENU_SPACE_INDIVIDUALLY   8

#define FILE_NUM_MAX         256
#define VIEW_LINES_MAX        16

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;
static s32 sMenuSelectNoIndividually;

static char sFilePath[FILE_NUM_MAX][FS_ENTRY_LONGNAME_MAX];

static u8 sFileNum;

static s32 sDatListViewOffset;

static s32 	sLines;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

static BOOL WriteFontData(void);
static BOOL WriteDummyData(const char* nandpath);
static BOOL WriteCertData(void);
static void ShowDatList(void);

void* WriteDataProcess0(void);
void* WriteDataProcess1(void);
void* WriteDataProcess2(void);
void* WriteDataProcess3(void);
void* WriteDataProcess4(void);
void* WriteDataProcess5(void);

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         WriteData �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess0(void)
{
	int i;

	// �z��N���A
	MI_CpuClear8( sFilePath, sizeof(sFilePath) );

	// �t�@�C����������
	sFileNum = 0;

	// �\���I�t�Z�b�g������
	sDatListViewOffset = 0;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "FONT WRITER");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// ���j���[�ꗗ
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   Serch SD Card   l     l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+-------------------+-----+");

	// �w�i�S�N���A
	for (i=0;i<24;i++)
	{
		kamiFontFillChar( i, BG_COLOR_TRANS, BG_COLOR_TRANS );
	}

	// �w�i�㕔
	kamiFontFillChar( 0, BG_COLOR_BROWN, BG_COLOR_BROWN );
	kamiFontFillChar( 1, BG_COLOR_BROWN, BG_COLOR_BROWN );
	kamiFontFillChar( 2, BG_COLOR_BROWN, BG_COLOR_TRANS );

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( WriteDataProcess1 );
}

/*---------------------------------------------------------------------------*
  Name:         WriteData �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess1(void)
{
	// �J�[�\���z�u
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y + sMenuSelectNo * DOT_OF_MENU_SPACE));

	// ����
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		FADE_OUT_RETURN( WriteDataProcess2 );
	}

	return WriteDataProcess1;
}

/*---------------------------------------------------------------------------*
  Name:         WriteData �v���Z�X�Q

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess2(void)
{
    FSFile    dir;

    // SD�J�[�h�̃��[�g�f�B���N�g��������
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R | FS_FILEMODE_W) )
    {
        kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory(sdmc:/)\n");
    }
    else
    {
        FSDirectoryEntryInfo   info[1];
        OS_Printf("[%s]:\n", "sdmc:/");

		kamiFontPrintfConsole(CONSOLE_ORANGE, "------ font file List -----\n");

		// .dat ��T���ăt�@�C������ۑ����Ă���
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
					if (!STD_CompareString( pExtension, ".dat") || !STD_CompareString( pExtension, ".DAT")  )
					{
						char full_path[FS_ENTRY_LONGNAME_MAX+6];

						// �t���p�X���쐬
						MakeFullPathForSD(info->longname, full_path);

						STD_CopyString( sFilePath[sFileNum], info->longname );
						kamiFontPrintfConsole(CONSOLE_ORANGE, "%d:%s\n", sFileNum, info->longname);

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

//		DumpTadInfo();
    }
	ShowDatList();

	// �J�[�\������
	SetCursorPos((u16)200, (u16)200);

	FADE_IN_RETURN( WriteDataProcess3 );
}

/*---------------------------------------------------------------------------*
  Name:         WriteData �v���Z�X�R

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess3(void)
{

	// �I�����j���[�̕ύX
    if ( kamiPadIsRepeatTrigger(PAD_KEY_UP) )
	{
		if (--sMenuSelectNoIndividually < 0) 
		{
			sMenuSelectNoIndividually = sFileNum - 1;
			if (sFileNum > VIEW_LINES_MAX)
			{
				sDatListViewOffset = sFileNum - VIEW_LINES_MAX;
			}
			else
			{
				sDatListViewOffset = 0;
			}
		}
		if (sMenuSelectNoIndividually < sDatListViewOffset)
		{
			sDatListViewOffset--;
		}
	}
	else if ( kamiPadIsRepeatTrigger(PAD_KEY_DOWN) )
	{
		if (++sMenuSelectNoIndividually > sFileNum - 1) 
		{
			sMenuSelectNoIndividually = 0;
			sDatListViewOffset = 0;
		}
		if ((sMenuSelectNoIndividually - sDatListViewOffset) > VIEW_LINES_MAX - 1)
		{
			sDatListViewOffset++;
		}

	}

	// �J�[�\���z�u
	SetCursorPos((u16)CURSOR_ORIGIN_X, (u16)(CURSOR_ORIGIN_Y_INDIVIDUALLY + (sMenuSelectNoIndividually - sDatListViewOffset) * DOT_OF_MENU_SPACE_INDIVIDUALLY));

	// ����
    if (kamiPadIsTrigger(PAD_BUTTON_A))
	{
		return WriteDataProcess4;
	}
	// �ЂƂO�̃��j���[�֖߂�
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( WriteDataProcess0 );
	}

	return WriteDataProcess3;
}

/*---------------------------------------------------------------------------*
  Name:         WriteData �v���Z�X4

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess4(void)
{
	BOOL ret;

	if (STD_GetStringLength(sFilePath[sMenuSelectNoIndividually]))
	{
		// �ʃC���|�[�g
		ret = WriteFontData();
	}
	else
	{
		// ���^�[��
		return WriteDataProcess0;
	}

	// ����̌��ʂ�\��
	if ( ret == TRUE )
	{
		kamiFontPrintf(26, (s16)(5+sMenuSelectNoIndividually*CHAR_OF_MENU_SPACE_INDIVIDUALLY), FONT_COLOR_GREEN, "OK");
    	kamiFontPrintfConsoleEx(CONSOLE_GREEN, "Write Font Success!\n");
	}
	else
	{
		kamiFontPrintf(26, (s16)(5+sMenuSelectNoIndividually*CHAR_OF_MENU_SPACE_INDIVIDUALLY), FONT_COLOR_RED, "NG");
    	kamiFontPrintfConsoleEx(CONSOLE_RED, "Write Font Error!\n");
	}

	// �t�H���g�X�N���[���f�[�^���[�h
	kamiFontLoadScreenData();

	return WriteDataProcess3;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

static BOOL WriteFontData(void)
{
    FSFile  file;	
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	void* pTempBuf;
	u32 file_size;
	u32 alloc_size;
	BOOL result = TRUE;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];

	// �t���p�X���쐬
	MakeFullPathForSD(sFilePath[sMenuSelectNoIndividually], full_path);

	// ROM�t�@�C���I�[�v��
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, full_path);
	if (!open_is_ok)
	{
    	kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_OpenFile(\"%s\") ... ERROR!\n", full_path);
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

	// ��U�t�H���g�f�[�^���폜����
	(void)FS_DeleteFile(FONT_DATA_FILE_PATH_IN_NAND);

	// nand:sys/TWLFontTable.dat�쐬
    if (!FS_CreateFile(FONT_DATA_FILE_PATH_IN_NAND, FS_PERMIT_R | FS_PERMIT_W))
    {
        kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_CreateFile(%s) failed.\n", FONT_DATA_FILE_PATH_IN_NAND);
		result = FALSE;
    }
    else
    {
		// nand:sys/TWLFontTable.dat�I�[�v��
		FS_InitFile(&file);
        open_is_ok = FS_OpenFileEx(&file, FONT_DATA_FILE_PATH_IN_NAND, FS_FILEMODE_W);
        if (!open_is_ok)
        {
            kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_OpenFile(%s) failed.\n", FONT_DATA_FILE_PATH_IN_NAND);
			result = FALSE;
        }
		// nand:sys/TWLFontTable.dat��������
        else if (FS_WriteFile(&file, pTempBuf, (s32)file_size) == -1)
        {
            kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_WritFile() failed.\n");
			result = FALSE;
        }
        (void)FS_CloseFile(&file);
    }

	OS_Free(pTempBuf);

	return result;
}

/*---------------------------------------------------------------------------*
  Name:         ShowDatList

  Description:  .dad �̃��X�g��\������

  Arguments:    arg -   �g�p���Ȃ��B

  Returns:      None.
 *---------------------------------------------------------------------------*/
static void ShowDatList(void)
{
	int i;

	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "FONT WRITER");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// ���j���[�ꗗ
	kamiFontPrintf(3,  4, FONT_COLOR_BLACK, "+--------------------+----+");
	if (sFileNum > 15)  { sLines = VIEW_LINES_MAX; }
	else                { sLines = sFileNum; }
	for (i=0;i<sLines;i++)
	{
		kamiFontPrintf(3, (s16)(5+i), FONT_COLOR_BLACK, "l                    l    l");
	}
	kamiFontPrintf(3, (s16)(5+sLines), FONT_COLOR_BLACK, "+--------------------+----+");

	// dat �t�@�C�����X�g��\��
	for (i=0;i<sLines; i++)
	{
		// �t�@�C�����ǉ�
		kamiFontPrintf(3,  (s16)(5+CHAR_OF_MENU_SPACE_INDIVIDUALLY*i), FONT_COLOR_BLACK, "l   %-16.16s l    l", sFilePath[sDatListViewOffset+i]);
	}
}
