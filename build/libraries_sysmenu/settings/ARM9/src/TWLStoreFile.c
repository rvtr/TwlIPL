/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     TWLHWInfo.c

  Copyright 2007 Nintendo.  All rights reserved.

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
#include <sysmenu/settings/common/TWLStoreFile.h>

// define data----------------------------------------------------------
// function's prototype-------------------------------------------------
// static variables-----------------------------------------------------
// global variables-----------------------------------------------------
// const data-----------------------------------------------------------

// function's description-----------------------------------------------

// TSFファイルの読み出し
TSFReadResult TSF_ReadFile( char *pPath, void *pDstBody, const TSFParam *pParam, u8 *pSaveCount )
{
	TSFHeader header;
	TSFReadResult retval;
	FSFile file;
	FS_InitFile( &file );
	
	// ヘッダ用バッファのクリア
	MI_CpuClear8( &header, sizeof(TSFHeader) );
	
	// ボディ用バッファのクリア
	if( pParam->pClearFunc ) {
		// クリア用関数ありの時は、バージョン下位互換の場合を考慮して、デフォルト値をセットしておく
		pParam->pClearFunc( pDstBody );
	}else {
		// そうでない場合は、リードバッファをクリア
		MI_CpuClear8( pDstBody, pParam->dataLength );
	}
	
	// ファイルオープン
	if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R ) ) {
		OS_TPrintf( "Read : file open error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_FILE_EXIST;
		goto END2;
	}
	
	// ファイル長チェック
	if( FS_GetFileLength( &file ) != pParam->fileLength ) {
		OS_TPrintf( "Read : file length error. : %s length = %d\n", pPath, FS_GetFileLength( &file ) );
		retval = TSF_READ_RESULT_ERROR_FILE_LENGTH;
		goto END;
	}
	
	// ヘッダリード
	if( FS_ReadFile( &file, &header, sizeof(TSFHeader) ) < sizeof(TSFHeader) ) {
		OS_TPrintf( "Read : file read error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// ヘッダチェック
	if( header.bodyLength > pParam->dataLength ) {
		OS_TPrintf( "Read : file header error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// ボディリード
	if( FS_ReadFile( &file, pDstBody, (long)header.bodyLength ) < pParam->dataLength ) {
		OS_TPrintf( "Read : file read error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// データのダイジェストチェック
	if(  pParam->pCheckDigestFunc &&
		!pParam->pCheckDigestFunc( pDstBody, header.bodyLength, header.digest.dst ) ) {
		OS_TPrintf( "Read : file digest error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// データの値チェック
	if(  pParam->pCheckValueFunc &&
		!pParam->pCheckValueFunc( pDstBody ) ) {
		OS_TPrintf( "Read : file data value error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// バージョン互換チェック
	if( pParam->conpatibleVerList ) {
		const u8 *pVersion = pParam->conpatibleVerList;
		while( *pVersion++ != TSF_VERSION_TERMINATOR ) {
			if( *pVersion == header.version ) {
				break;	//　互換あり
			}
		}
		if( *pVersion == TSF_VERSION_TERMINATOR ) {
			retval = TSF_READ_RESULT_ERROR_DATA;
			goto END;
		}
	}
	
	if( pSaveCount ) {
		*pSaveCount = header.saveCount;
	}
	retval = TSF_READ_RESULT_SUCCEEDED;
END:
	// ファイルクローズ
	FS_CloseFile( &file );
END2:
	
	if( retval != TSF_READ_RESULT_SUCCEEDED ) {
		// ボディ用バッファのクリア
		if( pParam->pClearFunc ) {
			// クリア用関数ありの時は、バージョン下位互換の場合を考慮して、デフォルト値をセットしておく
			pParam->pClearFunc( pDstBody );
		}else {
			// そうでない場合は、リードバッファをクリア
			MI_CpuClear8( pDstBody, pParam->dataLength );
		}
	}
	
	return retval;
}


// TWLファイルのライト
BOOL TSF_WriteFile( char *pPath, TSFHeader *pHeader, const void *pSrcBody, u8 *pSaveCount )
{
	BOOL retval = FALSE;
	FSFile file;
	FS_InitFile( &file );
	
	if( pSaveCount ) {
		*pSaveCount = (u8)( ( *pSaveCount + 1 ) & SAVE_COUNT_MASK );
		pHeader->saveCount = *pSaveCount;
	}else {
		pHeader->saveCount = 0;
	}
	
	OS_TPrintf( "Write > %s : %d\n", pPath, pHeader->saveCount );
	
	// ファイルオープン
	if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R | FS_FILEMODE_W ) ) {		// R|Wモードで開くと、既存ファイルを残したまま更新。
		OS_TPrintf( "Write : file open error. %s\n", pPath );
		return FALSE;
	}
	
	// ライト
	if( FS_WriteFile( &file, pHeader, sizeof(TSFHeader) ) < sizeof(TSFHeader) ) {
		OS_TPrintf( "Write : file header write error. %s\n", pPath );
		goto END;
	}
	if( FS_WriteFile( &file, pSrcBody, (long)pHeader->bodyLength ) < pHeader->bodyLength ) {
		OS_TPrintf( "Write : file body write error. %s\n", pPath );
		goto END;
	}
	
	retval = TRUE;
END:
	// ファイルクローズ
	FS_CloseFile( &file );
	
	return retval;
}


// TWLファイルのリカバリ
BOOL TSF_RecoveryFile( TSFReadResult err, char *pPath, u32 fileLength )
{
#define INITIAL_DATA_PATTERN	0xffffffff
#define READ_SIZE				1024
	u32	buffer[ READ_SIZE / sizeof(u32) ];
	u32 length;
	BOOL retval = FALSE;
	BOOL isWrite = FALSE;
	FSFile file;
	FS_InitFile( &file );
	
	if( err == TSF_READ_RESULT_SUCCEEDED ) {
		return TRUE;
	}
	
	// ファイル生成
	if( err == TSF_READ_RESULT_ERROR_FILE_EXIST ) {
		if( !FS_CreateFile( pPath, FS_PERMIT_R | FS_PERMIT_W ) ) {
			OS_TPrintf( "Recovery: create file error. %s\n", pPath );
			goto END2;
		}
	}
	
	// ファイルオープン
	if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R | FS_FILEMODE_W ) ) {
		OS_TPrintf( "Recovery : file open error. %s\n", pPath );
		goto END2;
	}
	
	// ファイル長変更
	if( ( err == TSF_READ_RESULT_ERROR_FILE_EXIST ) ||
		( err == TSF_READ_RESULT_ERROR_FILE_LENGTH ) ) {
		if( FS_SetFileLength( &file, fileLength ) != FS_RESULT_SUCCESS ) {
			OS_TPrintf( "Recovery : set file length error. %s\n", pPath );
			goto END;
		}
	}
	
	// データベリファイ
	MI_CpuFillFast( buffer, INITIAL_DATA_PATTERN, sizeof(buffer) );
	length = fileLength;
	while( length ) {
		int i;
		u32 rdSize = ( length > READ_SIZE ) ? READ_SIZE : length;
		FS_ReadFile( &file, buffer, (s32)rdSize );
		for( i = 0; i < rdSize / sizeof(u32); i++  ) {
			if( buffer[ i ] != INITIAL_DATA_PATTERN ) {
				isWrite = TRUE;
				goto NEXT;
			}
		}
		length -= rdSize;
	}

NEXT:
	// ベリファイエラーの場合は、ファイル内データ初期化（0xff埋め）
	if( isWrite ) {
		FS_SeekFile( &file, 0, FS_SEEK_SET );
		length = fileLength;
		while( length ) {
			u32 wrSize = ( length > READ_SIZE ) ? READ_SIZE : length;
			if( FS_WriteFile( &file, buffer, (s32)wrSize ) < wrSize ) {
				OS_TPrintf( "Recovery : write error. %s\n", pPath );
				goto END;
			}
			length -= wrSize;
		}
	}
	
	retval = TRUE;
END:
	// ファイルクローズ
	FS_CloseFile( &file );
END2:
	
	return retval;
}

