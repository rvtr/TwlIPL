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
#include <sysmenu/errorLog.h>

/*---------------------------------------------------------------------------*
    �^��`
 *---------------------------------------------------------------------------*/

enum {
	MENU_CERT=0,
	MENU_WRAP,
	MENU_FONT,
	

#ifdef WRITE_DEVKP_ENABLE
	MENU_DEVKP,
#endif
	MENU_RETURN,
	NUM_OF_MENU_SELECT
};

/*---------------------------------------------------------------------------*
    �萔��`
 *---------------------------------------------------------------------------*/

#define DOT_OF_MENU_SPACE    16
#define CURSOR_ORIGIN_X      32
#define CURSOR_ORIGIN_Y      56
#define CHAR_OF_MENU_SPACE    2

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    �����ϐ���`
 *---------------------------------------------------------------------------*/

static s8 sMenuSelectNo;

/*---------------------------------------------------------------------------*
    �����֐��錾
 *---------------------------------------------------------------------------*/

void WriteDataProcessDrawMenu(void);
static BOOL WriteFontData(void);
static BOOL WriteDummyData(const char* nandpath);
static BOOL WriteCertData(void);

/*---------------------------------------------------------------------------*
    �v���Z�X�֐���`
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*
  Name:         WriteData �v���Z�X�O

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/


void* WriteDataProcessPre0(void)
{
	WriteDataProcessDrawMenu();

#ifndef NAND_INITIALIZER_LIMITED_MODE
	// �I�[�g���s�p
	if (gAutoFlag)
	{
		sMenuSelectNo = MENU_CERT;
		FADE_IN_RETURN( WriteDataProcess2 );
	}
#endif


	FADE_IN_RETURN( WriteDataProcess1 );
}

void* WriteDataProcessAfter0(void)
{
	WriteDataProcessDrawMenu();

#ifndef NAND_INITIALIZER_LIMITED_MODE
	// �I�[�g���s�p
	if (gAutoFlag)
	{
#ifdef MARIOCLUB_VERSION
		sMenuSelectNo = MENU_WRAP;
#else  // MARIOCLUB_VERSION
		sMenuSelectNo = MENU_FONT;
#endif // MARIOCLUB_VERSION
		FADE_IN_RETURN( WriteDataProcess2 );
	}
#endif

// NEVER REACHED
	FADE_IN_RETURN( WriteDataProcess1 );
}


void WriteDataProcessDrawMenu(void)
{
	int i;
	
	// ������S�N���A
	kamiFontClear();

	// �o�[�W�����\��
	kamiFontPrintf(2, 1, FONT_COLOR_BLACK, "Write Various Data");
	kamiFontPrintf(0, 2, FONT_COLOR_BLACK, "--------------------------------");

	// ���j���[�ꗗ
	kamiFontPrintf(3,  6, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  7, FONT_COLOR_BLACK, "l   WRITE CERT.SYS  l     l");
	kamiFontPrintf(3,  8, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3,  9, FONT_COLOR_BLACK, "l   WRITE WRAP DATA l     l");
	kamiFontPrintf(3, 10, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3, 11, FONT_COLOR_BLACK, "l   WRITE FONT DATA l     l");
	kamiFontPrintf(3, 12, FONT_COLOR_BLACK, "+-------------------+-----+");
#ifdef WRITE_DEVKP_ENABLE
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "l   WRITE DEV.KP    l     l");
	kamiFontPrintf(3, 14, FONT_COLOR_BLACK, "+-------------------+-----+");
	kamiFontPrintf(3, 15, FONT_COLOR_BLACK, "l   RETURN          l     l");
	kamiFontPrintf(3, 16, FONT_COLOR_BLACK, "+-------------------+-----+");
#else
	kamiFontPrintf(3, 13, FONT_COLOR_BLACK, "l   RETURN          l     l");
	kamiFontPrintf(3, 14, FONT_COLOR_BLACK, "+-------------------+-----+");
#endif

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

}

/*---------------------------------------------------------------------------*
  Name:         WriteData �v���Z�X�P

  Description:  

  Arguments:    None.

  Returns:      next sequence
 *---------------------------------------------------------------------------*/

void* WriteDataProcess1(void)
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
		return WriteDataProcess2;
	}
	// �g�b�v���j���[�֖߂�
    else if (kamiPadIsTrigger(PAD_BUTTON_B))
	{
		FADE_OUT_RETURN( TopmenuProcess0 );
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
	BOOL result = TRUE;
	s16 y_pos = (s16)(7 + sMenuSelectNo * CHAR_OF_MENU_SPACE);

	switch( sMenuSelectNo )
	{
	case MENU_CERT:
#ifdef   MARIOCLUB_VERSION
		result = WriteCertData();
		
		// sysmenu.log�̐��������̃^�C�~���O�ōs���Ă���
		ERRORLOG_Init(OS_AllocFromMain, OS_FreeToMain);
#endif //MARIOCLUB_VERSION
		break;
	case MENU_WRAP:
		// �_�~�[��DS���j���[���b�s���O�p�t�@�C���쐬�iUIG�����`���[������Ă�����́j
		result = WriteDummyData(WRAP_DATA_FILE_PATH_IN_NAND);
		break;
	case MENU_FONT:
		result = WriteFontData();
		break;
#ifdef WRITE_DEVKP_ENABLE
	case MENU_DEVKP:
		result = WriteDummyData(DEVKP_DATA_FILE_PATH_IN_NAND);
		break;
#endif
	case MENU_RETURN:
		FADE_OUT_RETURN( TopmenuProcess0 );
	}

	if (result)
	{
		kamiFontPrintf(25, y_pos, FONT_COLOR_GREEN, "OK");
	}
	else
	{
		kamiFontPrintf(25, y_pos, FONT_COLOR_RED, "NG");
	}

#ifndef NAND_INITIALIZER_LIMITED_MODE
	// Auto�p
	if (gAutoFlag)
	{
		static BOOL total_result = TRUE;
		total_result &= result;

		switch(sMenuSelectNo)
		{
		case MENU_CERT:
#ifdef   MARIOCLUB_VERSION
			if (total_result) 
			{
				gAutoProcessResult[AUTO_PROCESS_MENU_VARIOUS_DATA_1] = AUTO_PROCESS_RESULT_SUCCESS; 
				FADE_OUT_RETURN( AutoProcess1 ); 
			}
			else 
			{ 
				gAutoProcessResult[AUTO_PROCESS_MENU_VARIOUS_DATA_1] = AUTO_PROCESS_RESULT_FAILURE; 
				FADE_OUT_RETURN( AutoProcess2); 
			}
#else  //MARIOCLUB_VERSION
				FADE_OUT_RETURN( AutoProcess1 ); 			
#endif //MARIOCLUB_VERSION
			
			/* NOTREACHED */
		case MENU_WRAP:
			sMenuSelectNo++;
			return WriteDataProcess2;
		case MENU_FONT:
#ifdef   WRITE_DEVKP_ENABLE
			sMenuSelectNo = MENU_DEVKP;
			return WriteDataProcess2;
		case MENU_DEVKP:
#endif //WRITE_DEVKP_ENABLE
			if (total_result) 
			{
				gAutoProcessResult[AUTO_PROCESS_MENU_VARIOUS_DATA_2] = AUTO_PROCESS_RESULT_SUCCESS; 
				FADE_OUT_RETURN( AutoProcess1 ); 
			}
			else 
			{ 
				gAutoProcessResult[AUTO_PROCESS_MENU_VARIOUS_DATA_2] = AUTO_PROCESS_RESULT_FAILURE; 
				FADE_OUT_RETURN( AutoProcess2); 
			}
		}
	}
#endif

	return WriteDataProcess1;
}

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

static BOOL WriteFontData(void)
{
    FSFile  dir, file;	
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	void* pTempBuf;
	u32 file_size;
	u32 alloc_size;
	BOOL result = TRUE;
	char full_path[FS_ENTRY_LONGNAME_MAX+6];

	FS_InitFile(&dir);

    // SD�J�[�h�̃��[�g�f�B���N�g��������
    if ( !FS_OpenDirectory(&dir, "sdmc:/", FS_FILEMODE_R) )
    {
		kamiFontPrintfConsole(CONSOLE_RED, "Error FS_OpenDirectory(sdmc:/)");
    }
    else
    {
        FSDirectoryEntryInfo   info[1];

		// .dat ��T��
        while (FS_ReadDirectory(&dir, info))
        {
            if ((info->attributes & (FS_ATTRIBUTE_DOS_DIRECTORY | FS_ATTRIBUTE_IS_DIRECTORY)) == 0)
            {
				char* pExtension;

				// �g���q�̃`�F�b�N
				pExtension = STD_SearchCharReverse( info->longname, '.');
				if (pExtension)
				{
					if (!STD_CompareString( pExtension, ".dat") || !STD_CompareString( pExtension, ".DAT"))
					{
						if (!STD_CompareNString(info->longname, "TWLFontTable", STD_GetStringLength("TWLFontTable")))
						{
							MakeFullPathForSD(info->longname, full_path);
							break;
						}
					}
				}
            }
        }
        (void)FS_CloseDirectory(&dir);
    }

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

// �_�~�[�t�@�C���쐬
static BOOL WriteDummyData(const char* nandpath)
{
    FSFile  file;	
    BOOL    open_is_ok;
	const int  FATFS_CLUSTER_SIZE = 16 * 1024;

	// ���ɑ��݂���Ȃ牽�����Ȃ�
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, nandpath);
	if (open_is_ok)
	{
		FS_CloseFile(&file);
    	OS_Printf("%s is already exist.\n", nandpath);
		return TRUE;
	}

	if( FS_CreateFileAuto( nandpath, FS_PERMIT_R | FS_PERMIT_W ) ) 
	{
		FSFile file;
		if( FS_OpenFileEx( &file, nandpath, FS_FILEMODE_RW ) ) 
		{
			(void)FS_SetFileLength( &file, FATFS_CLUSTER_SIZE );
			FS_CloseFile( &file );
			return TRUE;
		}
	}

	return FALSE;
}

static BOOL WriteCertData(void)
{
    FSFile  file;	
    BOOL    open_is_ok;
	BOOL    read_is_ok;
	void* pTempBuf;
	u32 file_size;
	u32 alloc_size;
	BOOL result = TRUE;

	// nand:/sys/cert.sys�����ɑ��݂���Ȃ牽�����Ȃ�
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, CERT_DATA_FILE_PATH_IN_NAND);
	if (open_is_ok)
	{
		FS_CloseFile(&file);
    	OS_Printf("%s is already exist.\n", CERT_DATA_FILE_PATH_IN_NAND);
		return TRUE;
	}

	// ROM�t�@�C���I�[�v��
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, CERT_DATA_FILE_PATH_IN_ROM);
	if (!open_is_ok)
	{
    	OS_Printf("FS_OpenFile(\"%s\") ... ERROR!\n", CERT_DATA_FILE_PATH_IN_ROM);
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
	    kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_ReadFile(\"%s\") ... ERROR!\n", CERT_DATA_FILE_PATH_IN_ROM);
		FS_CloseFile(&file);
		OS_Free(pTempBuf);
		return FALSE;
	}

	// ROM�t�@�C���N���[�Y
	FS_CloseFile(&file);

	// nand:sys/cert.sys�쐬
    if (!FS_CreateFile(CERT_DATA_FILE_PATH_IN_NAND, FS_PERMIT_R | FS_PERMIT_W))
    {
        kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_CreateFile(%s) failed.\n", CERT_DATA_FILE_PATH_IN_NAND);
		result = FALSE;
    }
    else
    {
		// nand:sys/cert.sys�I�[�v��
		FS_InitFile(&file);
        open_is_ok = FS_OpenFileEx(&file, CERT_DATA_FILE_PATH_IN_NAND, FS_FILEMODE_W);
        if (!open_is_ok)
        {
            kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_OpenFile(%s) failed.\n", CERT_DATA_FILE_PATH_IN_NAND);
			result = FALSE;
        }
		// nand:sys/cert.sys��������
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
