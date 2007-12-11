/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     TWLFileStore.h

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


#ifndef	TWL_STORE_FILE_H_
#define	TWL_STORE_FILE_H_
#if		defined(SDK_CW)


#include <twl.h>
#include <twl/os/common/format_rom.h>

#ifdef __cplusplus
extern "C" {
#endif


// define data ------------------------------------
#define TSF_VERSION_TERMINATOR			0xff		// version�I�[
#define SAVE_COUNT_MAX					0x80		// saveCount�̍ő�l
#define SAVE_COUNT_MASK					0x7f		// saveCount�̒l�͈̔͂��}�X�N����B(0x00-0x7f�j

// TSF���[�h����
typedef enum TSFReadResult {
	TSF_READ_RESULT_SUCCEEDED = 0,
	TSF_READ_RESULT_ERROR_FILE_EXIST = 1,
	TSF_READ_RESULT_ERROR_FILE_LENGTH = 2,
	TSF_READ_RESULT_ERROR_DATA = 3
}TSFReadResult;


// TSF�A�N�Z�X�p�p�����[�^
typedef struct TSFParam {	// TSF ( TWL Store File )
	u32			dataLength;								// �ۑ�����f�[�^��
	u32			fileLength;								// �ۑ�����t�@�C����
	const u8	*conpatibleVerList;						// �ߋ�ver.�̌݊�ver.���X�g�BTSF_VERSION_TERMINATOR�ŏI�[�B
	void		(*pClearFunc)( void *pDst );			// �ۑ�����f�[�^�̒l�N���A�֐�
	BOOL		(*pCheckDigestFunc)( void *pTgt, u32 len, u8 *pDigest );	// �_�C�W�F�X�g�`�F�b�N�֐��ւ̃|�C���^
	BOOL		(*pCheckValueFunc)( void *pTgt );		// �l�`�F�b�N�֐��ւ̃|�C���^
}TSFParam;


// TSF�w�b�_
typedef struct TSFHeader{
	union digest {
		u8				sha1[ SVC_SHA1_DIGEST_SIZE ];	// SHA-1�_�C�W�F�X�g
		u8				rsa[ RSA_KEY_LENGTH ];			// RSA����
		u8				dst[ RSA_KEY_LENGTH ];			// �]���p�̍ő�T�C�Y�v�f
	}digest;
	u8					version;						// �f�[�^ver.
	u8					saveCount;						// �Z�[�u�J�E���g�i�~���[�����O���Ȃ��t�@�C���͎g�p���Ȃ��j
	u8					rsv[2];							// �\��
	u32					bodyLength;						// �f�[�^��
}TSFHeader;	// 134bytes

// ���uTSFHeader+�f�[�^�{�́v�̌`�Ńt�@�C���ɕۑ�����܂��B

#ifdef SDK_ARM9

//=========================================================
// NAND�t�@�C���ւ̃��[�h���C�g�֐�
//=========================================================

// TSF�t�H�[�}�b�g�̃t�@�C���̃��[�h
extern TSFReadResult TSF_ReadFile ( char *pPath, void *pDstBody, const TSFParam *pParam, u8 *pSaveCount );
// TSF�t�H�[�}�b�g�̃t�@�C���̃��C�g�ipSaveCount��NULL��^�����ꍇ�́AsaveCount��"0"�Œ�ɂ��܂��B�j
extern BOOL          TSF_WriteFile( char *pPath, TSFHeader *pHeader, const void *pSrcBody, u8 *pSaveCount );
// TSF�t�H�[�}�b�g�̃t�@�C���̃��J�o��
extern BOOL          TSF_RecoveryFile( TSFReadResult err, char *pPath, u32 fileLength );

#endif // SDK_ARM9


#ifdef __cplusplus
}
#endif

#endif		// SDK_CW
#endif		// TWL_STORE_FILE_H_
