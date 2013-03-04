/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     util_recoveryFile.h

  Copyright **** Nintendo.  All rights reserved.

  These coded instructions, statements, and computer programs contain
  proprietary information of Nintendo of America Inc. and/or Nintendo
  Company Ltd., and are protected by Federal copyright law.  They may
  not be disclosed to third parties or copied or duplicated in any form,
  in whole or in part, without the prior written consent of Nintendo.

  $Date::            $
  $Rev$
  $Author$
 *---------------------------------------------------------------------------*/
 
 // �t�@�C�����J�o�����s�����߂̃��C�u����
 // �󂯎�����p�X�̃t�@�C�����m�F�A���݂��Ȃ������ꍇ�͍쐬���ăT�C�Y��ݒ肷��B

#include <twl.h>
#include <sysmenu/util_recoveryFile.h>

typedef enum CheckStatus {
	CHECK_EXIST = 0,
	CHECK_CREATE = 1,
	CHECK_FAILED = 2
} CheckStatus;

CheckStatus UTL_CheckAndCreateDirectory( const char *path );
CheckStatus UTL_CheckAndCreateDirectoryRec( const char *path );
 
/*---------------------------------------------------------------------------*
  Name:         UTL_RecoveryFile

  Description:  ���̊֐��͎󂯎�����p�X�ɂ���t�@�C�����`�F�b�N���܂��B
				�Ώۂ̃t�@�C�������݂��ăT�C�Y����������Ή������܂���B
				�T�C�Y���قȂ����ꍇ�͎w��T�C�Y�ɕύX���s���A
				�t�@�C�������݂��Ȃ���΍쐬���������ŁA�T�C�Y�ݒ���s���܂��B

  Arguments:    path:		�`�F�b�N���s���t�@�C���̃p�X
  				filesize:	�Ώۃt�@�C���̃T�C�Y

  Returns:      �t�@�C�������݂��A�T�C�Y���K���ȏꍇ�͏ꍇ��UTL_RCV_OK���A
  				�T�C�Y���قȂ�A�ύX�����ꍇ��UTL_RCV_SIZE_CHANGED���A
  				�t�@�C�������݂����A�쐬�����ꍇ��UTL_RCV_FILE_CREATED�A
  				�t�@�C���쐬�Ɏ��s�����ꍇ��UTL_RCV_FAILED��Ԃ��܂��B
 *---------------------------------------------------------------------------*/

UTL_RecoveryStatus UTL_RecoveryFile( const char* path, const u32 filesize )
{
	FSFile file;
	BOOL openRes;

	if( !FS_IsAvailable() )
	{
		// FS��Init����ĂȂ�������Init����
		FS_Init( FS_DMA_NOT_USE );
	}

	FS_InitFile( &file );
	
	if( UTL_CheckAndCreateDirectoryRec( path ) == CHECK_FAILED )
	{
		// �t�@�C���Ɏ���܂ł̃f�B���N�g�����쐬�ł��Ȃ������B
		return UTL_RCV_FAILED;
	}
	
	if( (openRes = FS_OpenFileEx( &file, path, FS_FILEMODE_RWL )) == TRUE )
	{
		if (FS_GetFileLength( &file ) == filesize )
		{
			// �t�@�C�������݂��ăT�C�Y�������Ă�Ή�������Ȃ�
			return UTL_RCV_OK;
		}
	}

	// �t�@�C�������݂��Ȃ��̂ō쐬����	
	if( (!openRes) && !FS_CreateFile( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("UTL RecoveryFile Error: FS_CreateFile() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		// �t�@�C���쐬�Ɏ��s
		return UTL_RCV_FAILED;
	}

	// �t�@�C���쐬�ɐ���
	if( !FS_OpenFileEx( &file, path, FS_FILEMODE_RW ) )
	{
		// �쐬�����t�@�C����open�ł��Ȃ������ꍇ
		OS_TPrintf("UTL RecoveryFile Error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		return UTL_RCV_OK;
	}
	
		
	if( FS_SetFileLength( &file, filesize ) != FS_RESULT_SUCCESS )
	{
		// �쐬�����t�@�C���̃T�C�Y��ݒ�ł��Ȃ�����
		OS_TPrintf(" UTL RecoveryFile Error: FS_SetFileLength() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		FS_CloseFile( &file );
		return UTL_RCV_FAILED; 
	}

	
	return openRes ? UTL_RCV_SIZE_CHANGED : UTL_RCV_FILE_CREATED;
}

/*---------------------------------------------------------------------------*
  Name:         UTL_CheckAndCreateDirectoryRec

  Description:  �Y���p�X�ɓ��B����܂ł̃f�B���N�g�����ċN�I�Ƀ`�F�b�N���A
				���݂��Ȃ��f�B���N�g���̏ꍇ�͑S�č쐬���܂��B
				�p�X�̍Ōオ'/'�Ȃ�Ō�܂Ńf�B���N�g�����Ĉ����A
				�����łȂ��ꍇ�͍Ō���t�@�C�����Ƃ��A'/'�̎�O�ɋL�q���ꂽ
				�f�B���N�g���܂ł��`�F�b�N���܂��B
				
  Arguments:    path:		�`�F�b�N���s���p�X

  Returns:      �f�B���N�g�����Ō�܂ő��݂����ꍇ��CHECK_EXIST���A
  				���݂��Ă��炸�쐬���A�Ō�܂Ő��������ꍇ��CHECK_CREATE���A
  				�r���Ńf�B���N�g���쐬�Ɏ��s�����ꍇ��CHECK_FAILED��Ԃ��܂��B
 *---------------------------------------------------------------------------*/


CheckStatus UTL_CheckAndCreateDirectoryRec( const char *path )
{
	BOOL createFlag = FALSE;
	char subPath[FS_FILE_NAME_MAX];
	char *p;
	int pathLength = STD_StrLen( path );
	
	p = STD_StrChr( path, '/');
	
	while( p != NULL )
	// �X���b�V�����o�Ă��Ȃ��Ȃ�܂ő�����
	{
		CheckStatus result;
		
		// ���̃X���b�V���܂ł̕�������������
		int slashPos = pathLength - STD_StrLen( p );
		STD_StrLCpy( subPath, path, slashPos+1 );
		subPath[slashPos+1] = '\0';
		
		result = UTL_CheckAndCreateDirectory( subPath );

		switch( result )
		{
			case CHECK_FAILED:
				// ���s�����炻���ŏI���
				return result;
			case CHECK_CREATE:
				createFlag = TRUE;
				break;
		}
	
		p = STD_StrChr( ++p, '/' );
	}
	
	return createFlag ? CHECK_CREATE : CHECK_EXIST;
	
}

/*---------------------------------------------------------------------------*
  Name:         UTL_CheckAndCreateDirectory

  Description:  ���̊֐��͊Y���f�B���N�g�������݂��Ă���Ή������܂���B
				�Y���f�B���N�g�������݂��Ă��Ȃ������ꍇ��
				�f�B���N�g�����쐬���܂��B

  Arguments:    path:		�`�F�b�N���s���f�B���N�g���̃p�X

  Returns:      �f�B���N�g�������݂����ꍇ��CHECK_EXIST���A
  				���݂��Ă��炸�쐬�����ꍇ��CHECK_CREATE���A
  				�f�B���N�g���쐬�Ɏ��s�����ꍇ��CHECK_FAILED��Ԃ��܂��B
 *---------------------------------------------------------------------------*/

CheckStatus UTL_CheckAndCreateDirectory( const char *path )
{
	FSFile dir;

	FS_InitFile( &dir );
	
	if( FS_OpenDirectory( &dir, path, FS_FILEMODE_RW ) )
	{
		// �f�B���N�g�������݂��Ă����炻�̂܂�Close���Ė߂�
		FS_CloseDirectory( &dir );
		return CHECK_EXIST;
	}

	// �f�B���N�g�������݂��Ȃ��̂Ńf�B���N�g�����쐬
	if( ! FS_CreateDirectory( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("EL Error: FS_CreateDirectory() failed. path: %s FSResult: %d\n", path, FS_GetArchiveResultCode(path) );
		// �f�B���N�g���쐬�Ɏ��s
		return CHECK_FAILED;
	}

	// �f�B���N�g���쐬�ɐ���
	OS_TPrintf("EL createDirectory succeeded. : %s\n", path );
	return CHECK_CREATE;
}
