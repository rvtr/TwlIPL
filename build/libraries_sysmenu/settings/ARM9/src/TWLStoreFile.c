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

// TSF�t�@�C���̓ǂݏo��
TSFReadResult TSF_ReadFile( char *pPath, void *pDstBody, const TSFParam *pParam, u8 *pSaveCount )
{
	TSFHeader header;
	TSFReadResult retval;
	FSFile file;
	FS_InitFile( &file );
	
	// �w�b�_�p�o�b�t�@�̃N���A
	MI_CpuClear8( &header, sizeof(TSFHeader) );
	
	// �{�f�B�p�o�b�t�@�̃N���A
	if( pParam->pClearFunc ) {
		// �N���A�p�֐�����̎��́A�o�[�W�������ʌ݊��̏ꍇ���l�����āA�f�t�H���g�l���Z�b�g���Ă���
		pParam->pClearFunc( pDstBody );
	}else {
		// �����łȂ��ꍇ�́A���[�h�o�b�t�@���N���A
		MI_CpuClear8( pDstBody, pParam->dataLength );
	}
	
	// �t�@�C���I�[�v��
	if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R ) ) {
		OS_TPrintf( "Read : file open error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_FILE_EXIST;
		goto END2;
	}
	
	// �t�@�C�����`�F�b�N
	if( FS_GetFileLength( &file ) != pParam->fileLength ) {
		OS_TPrintf( "Read : file length error. : %s length = %d\n", pPath, FS_GetFileLength( &file ) );
		retval = TSF_READ_RESULT_ERROR_FILE_LENGTH;
		goto END;
	}
	
	// �w�b�_���[�h
	if( FS_ReadFile( &file, &header, sizeof(TSFHeader) ) < sizeof(TSFHeader) ) {
		OS_TPrintf( "Read : file read error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// �w�b�_�`�F�b�N
	if( header.bodyLength > pParam->dataLength ) {
		OS_TPrintf( "Read : file header error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// �{�f�B���[�h
	if( FS_ReadFile( &file, pDstBody, (long)header.bodyLength ) < pParam->dataLength ) {
		OS_TPrintf( "Read : file read error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// �f�[�^�̃_�C�W�F�X�g�`�F�b�N
	if(  pParam->pCheckDigestFunc &&
		!pParam->pCheckDigestFunc( pDstBody, header.bodyLength, header.digest.dst ) ) {
		OS_TPrintf( "Read : file digest error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// �f�[�^�̒l�`�F�b�N
	if(  pParam->pCheckValueFunc &&
		!pParam->pCheckValueFunc( pDstBody ) ) {
		OS_TPrintf( "Read : file data value error. %s\n", pPath );
		retval = TSF_READ_RESULT_ERROR_DATA;
		goto END;
	}
	
	// �o�[�W�����݊��`�F�b�N
	if( pParam->conpatibleVerList ) {
		const u8 *pVersion = pParam->conpatibleVerList;
		while( *pVersion++ != TSF_VERSION_TERMINATOR ) {
			if( *pVersion == header.version ) {
				break;	//�@�݊�����
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
	// �t�@�C���N���[�Y
	FS_CloseFile( &file );
END2:
	
	if( retval != TSF_READ_RESULT_SUCCEEDED ) {
		// �{�f�B�p�o�b�t�@�̃N���A
		if( pParam->pClearFunc ) {
			// �N���A�p�֐�����̎��́A�o�[�W�������ʌ݊��̏ꍇ���l�����āA�f�t�H���g�l���Z�b�g���Ă���
			pParam->pClearFunc( pDstBody );
		}else {
			// �����łȂ��ꍇ�́A���[�h�o�b�t�@���N���A
			MI_CpuClear8( pDstBody, pParam->dataLength );
		}
	}
	
	return retval;
}


// TWL�t�@�C���̃��C�g
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
	
	// �t�@�C���I�[�v��
	if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R | FS_FILEMODE_W ) ) {		// R|W���[�h�ŊJ���ƁA�����t�@�C�����c�����܂܍X�V�B
		OS_TPrintf( "Write : file open error. %s\n", pPath );
		return FALSE;
	}
	
	// ���C�g
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
	// �t�@�C���N���[�Y
	FS_CloseFile( &file );
	
	return retval;
}


// TWL�t�@�C���̃��J�o��
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
	
	// �t�@�C������
	if( err == TSF_READ_RESULT_ERROR_FILE_EXIST ) {
		if( !FS_CreateFile( pPath, FS_PERMIT_R | FS_PERMIT_W ) ) {
			OS_TPrintf( "Recovery: create file error. %s\n", pPath );
			goto END2;
		}
	}
	
	// �t�@�C���I�[�v��
	if( !FS_OpenFileEx( &file, pPath, FS_FILEMODE_R | FS_FILEMODE_W ) ) {
		OS_TPrintf( "Recovery : file open error. %s\n", pPath );
		goto END2;
	}
	
	// �t�@�C�����ύX
	if( ( err == TSF_READ_RESULT_ERROR_FILE_EXIST ) ||
		( err == TSF_READ_RESULT_ERROR_FILE_LENGTH ) ) {
		if( FS_SetFileLength( &file, fileLength ) != FS_RESULT_SUCCESS ) {
			OS_TPrintf( "Recovery : set file length error. %s\n", pPath );
			goto END;
		}
	}
	
	// �f�[�^�x���t�@�C
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
	// �x���t�@�C�G���[�̏ꍇ�́A�t�@�C�����f�[�^�������i0xff���߁j
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
	// �t�@�C���N���[�Y
	FS_CloseFile( &file );
END2:
	
	return retval;
}

