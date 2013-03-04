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
 
 // ファイルリカバリを行うためのライブラリ
 // 受け取ったパスのファイルを確認、存在しなかった場合は作成してサイズを設定する。

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

  Description:  この関数は受け取ったパスにあるファイルをチェックします。
				対象のファイルが存在してサイズが等しければ何もしません。
				サイズが異なった場合は指定サイズに変更を行い、
				ファイルが存在しなければ作成したうえで、サイズ設定を行います。

  Arguments:    path:		チェックを行うファイルのパス
  				filesize:	対象ファイルのサイズ

  Returns:      ファイルが存在し、サイズも適正な場合は場合はUTL_RCV_OKを、
  				サイズが異なり、変更した場合はUTL_RCV_SIZE_CHANGEDを、
  				ファイルが存在せず、作成した場合はUTL_RCV_FILE_CREATED、
  				ファイル作成に失敗した場合はUTL_RCV_FAILEDを返します。
 *---------------------------------------------------------------------------*/

UTL_RecoveryStatus UTL_RecoveryFile( const char* path, const u32 filesize )
{
	FSFile file;
	BOOL openRes;

	if( !FS_IsAvailable() )
	{
		// FSがInitされてなかったらInitする
		FS_Init( FS_DMA_NOT_USE );
	}

	FS_InitFile( &file );
	
	if( UTL_CheckAndCreateDirectoryRec( path ) == CHECK_FAILED )
	{
		// ファイルに至るまでのディレクトリを作成できなかった。
		return UTL_RCV_FAILED;
	}
	
	if( (openRes = FS_OpenFileEx( &file, path, FS_FILEMODE_RWL )) == TRUE )
	{
		if (FS_GetFileLength( &file ) == filesize )
		{
			// ファイルが存在してサイズが合ってれば何も言わない
			return UTL_RCV_OK;
		}
	}

	// ファイルが存在しないので作成する	
	if( (!openRes) && !FS_CreateFile( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("UTL RecoveryFile Error: FS_CreateFile() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		// ファイル作成に失敗
		return UTL_RCV_FAILED;
	}

	// ファイル作成に成功
	if( !FS_OpenFileEx( &file, path, FS_FILEMODE_RW ) )
	{
		// 作成したファイルをopenできなかった場合
		OS_TPrintf("UTL RecoveryFile Error: FS_OpenFileEx() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		return UTL_RCV_OK;
	}
	
		
	if( FS_SetFileLength( &file, filesize ) != FS_RESULT_SUCCESS )
	{
		// 作成したファイルのサイズを設定できなかった
		OS_TPrintf(" UTL RecoveryFile Error: FS_SetFileLength() failed. FSResult: %d\n", FS_GetArchiveResultCode(path) );
		FS_CloseFile( &file );
		return UTL_RCV_FAILED; 
	}

	
	return openRes ? UTL_RCV_SIZE_CHANGED : UTL_RCV_FILE_CREATED;
}

/*---------------------------------------------------------------------------*
  Name:         UTL_CheckAndCreateDirectoryRec

  Description:  該当パスに到達するまでのディレクトリを再起的にチェックし、
				存在しないディレクトリの場合は全て作成します。
				パスの最後が'/'なら最後までディレクトリして扱い、
				そうでない場合は最後をファイル名とし、'/'の手前に記述された
				ディレクトリまでをチェックします。
				
  Arguments:    path:		チェックを行うパス

  Returns:      ディレクトリが最後まで存在した場合はCHECK_EXISTを、
  				存在しておらず作成し、最後まで成功した場合はCHECK_CREATEを、
  				途中でディレクトリ作成に失敗した場合はCHECK_FAILEDを返します。
 *---------------------------------------------------------------------------*/


CheckStatus UTL_CheckAndCreateDirectoryRec( const char *path )
{
	BOOL createFlag = FALSE;
	char subPath[FS_FILE_NAME_MAX];
	char *p;
	int pathLength = STD_StrLen( path );
	
	p = STD_StrChr( path, '/');
	
	while( p != NULL )
	// スラッシュが出てこなくなるまで続ける
	{
		CheckStatus result;
		
		// 次のスラッシュまでの部分文字列を作る
		int slashPos = pathLength - STD_StrLen( p );
		STD_StrLCpy( subPath, path, slashPos+1 );
		subPath[slashPos+1] = '\0';
		
		result = UTL_CheckAndCreateDirectory( subPath );

		switch( result )
		{
			case CHECK_FAILED:
				// 失敗したらそこで終わり
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

  Description:  この関数は該当ディレクトリが存在していれば何もしません。
				該当ディレクトリが存在していなかった場合は
				ディレクトリを作成します。

  Arguments:    path:		チェックを行うディレクトリのパス

  Returns:      ディレクトリが存在した場合はCHECK_EXISTを、
  				存在しておらず作成した場合はCHECK_CREATEを、
  				ディレクトリ作成に失敗した場合はCHECK_FAILEDを返します。
 *---------------------------------------------------------------------------*/

CheckStatus UTL_CheckAndCreateDirectory( const char *path )
{
	FSFile dir;

	FS_InitFile( &dir );
	
	if( FS_OpenDirectory( &dir, path, FS_FILEMODE_RW ) )
	{
		// ディレクトリが存在していたらそのままCloseして戻る
		FS_CloseDirectory( &dir );
		return CHECK_EXIST;
	}

	// ディレクトリが存在しないのでディレクトリを作成
	if( ! FS_CreateDirectory( path, FS_PERMIT_R | FS_PERMIT_W ) )
	{
		OS_TPrintf("EL Error: FS_CreateDirectory() failed. path: %s FSResult: %d\n", path, FS_GetArchiveResultCode(path) );
		// ディレクトリ作成に失敗
		return CHECK_FAILED;
	}

	// ディレクトリ作成に成功
	OS_TPrintf("EL createDirectory succeeded. : %s\n", path );
	return CHECK_CREATE;
}
