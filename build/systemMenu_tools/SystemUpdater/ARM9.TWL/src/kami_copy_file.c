/*---------------------------------------------------------------------------*
  Project:  TwlSDK - NandInitializer
  File:     kami_copy_file.c

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
#include "kami_copy_file.h"

/*---------------------------------------------------------------------------*
    �}�N��
 *---------------------------------------------------------------------------*/

#define ROUND_UP(value, alignment) \
    (((u32)(value) + (alignment-1)) & ~(alignment-1))

/*---------------------------------------------------------------------------*
    �����֐���`
 *---------------------------------------------------------------------------*/

BOOL kamiCopyFile(char* srcPath, char* dstPath)
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
    open_is_ok = FS_OpenFile(&file, srcPath);
	if (!open_is_ok)
	{
    	OS_Printf("FS_OpenFile(\"%s\") ... ERROR!\n", srcPath);
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
	    kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_ReadFile(\"%s\") ... ERROR!\n", srcPath);
		FS_CloseFile(&file);
		OS_Free(pTempBuf);
		return FALSE;
	}

	// ROM�t�@�C���N���[�Y
	FS_CloseFile(&file);

	// ��U�Ώۃf�[�^���폜����
//	(void)FS_DeleteFile(dstPath);

	// �^�[�Q�b�g�t�@�C���쐬
    if (!FS_CreateFileAuto(dstPath, FS_PERMIT_R | FS_PERMIT_W))
    {
        kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_CreateFile(%s) failed.\n", dstPath);
		result = FALSE;
    }
    else
    {
		// �^�[�Q�b�g�t�@�C���I�[�v��
		FS_InitFile(&file);
        open_is_ok = FS_OpenFileEx(&file, dstPath, FS_FILEMODE_W);
        if (!open_is_ok)
        {
            kamiFontPrintfConsoleEx(CONSOLE_RED, "FS_OpenFile(%s) failed.\n", dstPath);
			result = FALSE;
        }
		// �^�[�Q�b�g�t�@�C���֏�������
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

// �_�~�[��DS���j���[���b�s���O�p�t�@�C���쐬�iUIG�����`���[������Ă�����́j
BOOL kamiWriteWrapData(void)
{
    FSFile  file;	
    BOOL    open_is_ok;
	const int  FATFS_CLUSTER_SIZE = 16 * 1024;

	// ���ɑ��݂���Ȃ牽�����Ȃ�
    FS_InitFile(&file);
    open_is_ok = FS_OpenFile(&file, WRAP_DATA_FILE_PATH_IN_NAND);
	if (open_is_ok)
	{
		FS_CloseFile(&file);
    	OS_Printf("%s is already exist.\n", WRAP_DATA_FILE_PATH_IN_NAND);
		return TRUE;
	}

	if( FS_CreateFileAuto( WRAP_DATA_FILE_PATH_IN_NAND, FS_PERMIT_R | FS_PERMIT_W ) ) 
	{
		FSFile file;
		if( FS_OpenFileEx( &file, WRAP_DATA_FILE_PATH_IN_NAND, FS_FILEMODE_RW ) ) 
		{
			(void)FS_SetFileLength( &file, FATFS_CLUSTER_SIZE );
			FS_CloseFile( &file );
			return TRUE;
		}
	}

	return FALSE;
}
