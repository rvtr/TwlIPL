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
#include <sysmenu/settings/common/TWLHWInfo.h>

// define data----------------------------------------------------------
//#define USE_SHA1_SIGNATURE				// �������̃n�b�V����SHA1���g�p�i����`�Ȃ�HMAC-SHA1���g�p�j

// function's prototype-------------------------------------------------
static BOOL THWIi_CheckDigest( void *pTgt, u32 length, u8 *pDigest );
static BOOL THWIi_CheckNormalValue( const TWLHWNormalInfo *pSrcInfo );
static BOOL THWIi_CheckSignature( void *pTgt, u32 length, u8 *pSignature );
static BOOL THWIi_CheckSecureValue( const TWLHWSecureInfo *pSecure );
static void DEBUG_PrintDigest( u8 *pDigest );
static void DEBUG_Dump( u8 *pSrc, u32 len );
static void HMACSHA1( u8 *pDigest, u8 *pSrc, u32 len, u8 *pKey, u32 keyLen );

// static variables-----------------------------------------------------
TWLHWNormalInfo s_hwInfoN ATTRIBUTE_ALIGN(32);
TWLHWSecureInfo s_hwInfoS ATTRIBUTE_ALIGN(32);
static BOOL s_isReadNormal;
static BOOL s_isReadSecure;

// global variables-----------------------------------------------------

// const data-----------------------------------------------------------

// �m�[�}�����@�f�t�H���g�l
static TWLHWNormalInfo s_normalDefault = {
	0x5a,
	{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f },
};


// �m�[�}�����@�o�[�W�����݊����X�g
static const u8 s_normalVersionList[] = { 1, TSF_VERSION_TERMINATOR };


// �m�[�}�����@TSF���[�h�p�����[�^
static const TSFParam s_normalParam = {
	sizeof(TWLHWNormalInfo),
	TWL_HWINFO_FILE_LENGTH,
	s_normalVersionList,
	&s_normalDefault,
	THWIi_CheckDigest,
	(int (*)(void *))THWIi_CheckNormalValue,
};


// �Z�L���A���@�f�t�H���g�l
static TWLHWSecureInfo s_secureDefault = {
	TWL_REGION_AMERICA,
	{ TWL_HWINFO_SERIALNO_LEN_AMERICA, "0123456789A" },
};


// �Z�L���A���@�o�[�W�����݊����X�g
static const u8 s_secureVersionList[] = { 1, TSF_VERSION_TERMINATOR };


// �Z�L���A���@TSF���[�h�p�����[�^
static const TSFParam s_secureParam = {
	sizeof(TWLHWSecureInfo),
	TWL_HWINFO_FILE_LENGTH,
	s_secureVersionList,
	&s_secureDefault,
	THWIi_CheckSignature,
	(int (*)(void *))THWIi_CheckSecureValue,
};


// �Z�L���A���@���J��
static const u8 s_publicKey[ RSA_KEY_LENGTH ] = {
	0xcf, 0x1a, 0xe0, 0xf7, 0xd1, 0x27, 0x59, 0xfb, 0x3a, 0x8c, 0xf5, 0x58, 0xc7, 0x17,	0xc1, 0xf2,
	0xa9, 0x77, 0xbd, 0x53, 0x59, 0xe4, 0xd3, 0x01, 0x27, 0xca, 0xb7, 0xc9, 0x92, 0x32, 0x2e, 0xb5,
	0xeb, 0x13, 0xbf, 0xca, 0xe2, 0x1b, 0xe7, 0x5c, 0xb0, 0x68, 0x5c, 0x58, 0x87, 0x84, 0x8c, 0x69,
	0xb2, 0x59, 0x6b, 0x89, 0xd7, 0xa1, 0x89, 0x4c, 0x46, 0x36, 0xd8, 0xbb, 0xbd, 0xc8, 0x9d, 0xf5,
	0xd2, 0x64, 0xb6, 0xeb, 0x71, 0x56, 0x2a, 0x69, 0xbd, 0x1d, 0xa3, 0xf6, 0xa1, 0x64, 0xe4, 0x8b,
	0xda, 0x9b, 0xd2, 0x67, 0x19, 0xb5, 0xf0, 0xe0, 0xbb, 0xe3, 0x2c, 0xa9, 0xb4, 0x5c, 0xfb, 0x2c,
	0x66, 0x61, 0xee, 0x18, 0x87, 0x23, 0x86, 0x34, 0xe7, 0xed, 0x1a, 0x75, 0x6e, 0x58, 0xb8, 0x81,
	0x61, 0x4e, 0x9d, 0x4f, 0x12, 0x44, 0x27, 0xf4, 0x16, 0x65, 0xa8, 0x9e, 0x0c, 0x18, 0x34, 0x43,
};


// ---------------------------------------------------------------------
// HW�m�[�}�����
// ---------------------------------------------------------------------

// ���[�h
TSFReadResult THW_ReadNormalInfo( void )
{
	s_isReadNormal = TRUE;
	return TSF_ReadFile( (char *)TWL_HWINFO_NORMAL_PATH, &s_hwInfoN, &s_normalParam, NULL );
}


// ���C�g
BOOL THW_WriteNormalInfo( void )
{
	if( !s_isReadNormal ) {
		return FALSE;
	}
	return THW_WriteNormalInfoDirect( &s_hwInfoN );
}


// ���ڃ��C�g
BOOL THW_WriteNormalInfoDirect( const TWLHWNormalInfo *pSrcInfo )
{
	// �w�b�_�̍쐬
	TSFHeader header;
	MI_CpuClear8( &header, sizeof(TSFHeader) );
	header.version = TWL_HWINFO_NORMAL_VERSION;
	header.bodyLength  = sizeof( TWLHWNormalInfo );
	SVC_CalcSHA1( header.digest.sha1, pSrcInfo, sizeof(TWLHWNormalInfo) );
	// ���C�g
	if( !TSF_WriteFile( (char *)TWL_HWINFO_NORMAL_PATH,
						&header,
						(const void *)pSrcInfo,
						DISABLE_SAVE_COUNT ) ) {
		return FALSE;
	}
	// �����[�h���Astatic�o�b�t�@�ւ̃R�s�[���s��
	if( !s_isReadNormal ) {
		s_isReadNormal = TRUE;
		MI_CpuCopy8( pSrcInfo, &s_hwInfoN, sizeof(TWLHWNormalInfo) );
	}
	return TRUE;
}


// �t�@�C���̃��J�o��
BOOL THW_RecoveryNormalInfo( TSFReadResult err )
{
	return TSF_RecoveryFile( err, (char *)TWL_HWINFO_NORMAL_PATH, TWL_HWINFO_FILE_LENGTH );

}


// �_�C�W�F�X�g�`�F�b�N
static BOOL THWIi_CheckDigest( void *pTgt, u32 length, u8 *pDigest )
{
	u8 digest[ SVC_SHA1_DIGEST_SIZE ];
	
	SVC_CalcSHA1( digest, pTgt, length );
	return SVC_CompareSHA1( digest, pDigest );
}


// �l�`�F�b�N
static BOOL THWIi_CheckNormalValue( const TWLHWNormalInfo *pSrcInfo )
{
#pragma unused(pSrcInfo)
	return TRUE;
}


// �V�����f�t�H���g�l�̃Z�b�g
void TWH_SetNormalDefaultValue( const TWLHWNormalInfo *pSrcInfo )
{
	MI_CpuCopy8( pSrcInfo, &s_normalDefault, sizeof(TWLHWNormalInfo) );
}


// ---------------------------------------------------------------------
// HW�Z�L���A���
// ---------------------------------------------------------------------

// ���[�h
TSFReadResult THW_ReadSecureInfo( void )
{
	s_isReadSecure = TRUE;
	return TSF_ReadFile( (char *)TWL_HWINFO_SECURE_PATH, &s_hwInfoS, &s_secureParam, NULL );
}

#ifdef HW_SECURE_INFO_WRITE_ENABLE_

// ���C�g
BOOL THW_WriteSecureInfo( const u8 *pPrivKeyDER )
{
	if( !s_isReadSecure ) {
		return FALSE;
	}
	return THW_WriteSecureInfoDirect( &s_hwInfoS, pPrivKeyDER );
}


// ���ڃ��C�g
BOOL THW_WriteSecureInfoDirect( const TWLHWSecureInfo *pSrcInfo, const u8 *pPrivKeyDER )
{
	// �w�b�_�̍쐬
	TSFHeader header;
	u8	digest[ SVC_SHA1_DIGEST_SIZE ];
	u64	id = SCFG_ReadFuseData();
	MI_CpuClear8( &header, sizeof(TSFHeader) );
	header.version = TWL_HWINFO_SECURE_VERSION;
	header.bodyLength  = sizeof( TWLHWSecureInfo );
	
#ifdef USE_SHA1_SIGNATURE
	SVC_CalcSHA1( digest, pSrcInfo, sizeof(TWLHWSecureInfo) );
#else
//	SVC_CalcHMACSHA1( digest, pSrcInfo, sizeof(TWLHWSecureInfo), &id, sizeof(u64) );
	HMACSHA1( digest, (unsigned char *)pSrcInfo, sizeof(TWLHWSecureInfo), (u8 *)&id, sizeof(u64) );
#endif
	if( !ACSign_Encrypto( header.digest.rsa,
						  pPrivKeyDER,
						  digest,
						  SVC_SHA1_DIGEST_SIZE ) ) {
		return FALSE;
	}
	// ���C�g
	if( !TSF_WriteFile( (char *)TWL_HWINFO_SECURE_PATH,
						&header,
						(const void *)pSrcInfo,
						DISABLE_SAVE_COUNT ) ) {
		return FALSE;
	}
	// �����[�h���Astatic�o�b�t�@�ւ̃R�s�[���s��
	if( !s_isReadSecure ) {
		s_isReadSecure = TRUE;
		MI_CpuCopy8( pSrcInfo, &s_hwInfoS, sizeof(TWLHWSecureInfo) );
	}
	return TRUE;
}


// �t�@�C���̃��J�o��
BOOL THW_RecoverySecureInfo( TSFReadResult err )
{
	return TSF_RecoveryFile( err, (char *)TWL_HWINFO_SECURE_PATH, TWL_HWINFO_FILE_LENGTH );
}

#endif // HW_SECURE_INFO_WRITE_ENABLE_


// �����`�F�b�N
static BOOL THWIi_CheckSignature( void *pTgt, u32 length, u8 *pSignature )
{
	static u32 heap[ 4096 / sizeof(u32) ];
	SVCSignHeapContext acmemoryPool;
	u8 digest_sign[ SVC_SHA1_DIGEST_SIZE ];
	u8 digest_calc[ SVC_SHA1_DIGEST_SIZE ];
	u64	id = SCFG_ReadFuseData();

#ifdef USE_SHA1_SIGNATURE
	SVC_CalcSHA1( digest_calc, pTgt, length );
#else
//	SVC_CalcHMACSHA1( digest_calc, pTgt, length, (u8 *)&id, sizeof(u64) );
	HMACSHA1( digest_calc, pTgt, length, (u8 *)&id, sizeof(u64) );
#endif
	SVC_InitSignHeap( &acmemoryPool, heap, 4096 );
	SVC_DecryptSign( &acmemoryPool, digest_sign, pSignature, s_publicKey );
	return SVC_CompareSHA1( digest_sign, digest_calc );
}


// HW Secure��� �l�`�F�b�N
static BOOL THWIi_CheckSecureValue( const TWLHWSecureInfo *pSrcInfo )
{
	if(
		( pSrcInfo->region >= TWL_REGION_MAX ) ||
		( ( pSrcInfo->serialNo.length != TWL_HWINFO_SERIALNO_LEN_AMERICA ) &&
		  ( pSrcInfo->serialNo.length != TWL_HWINFO_SERIALNO_LEN_OTHERS ) )
		) {
		return FALSE;
	}
	
	return TRUE;
}


// �V�����f�t�H���g�l�̃Z�b�g
void TWH_SetSecureDefaultValue( const TWLHWSecureInfo *pSrcInfo )
{
	MI_CpuCopy8( pSrcInfo, &s_secureDefault, sizeof(TWLHWSecureInfo) );
}

const TWLHWNormalInfo *THW_GetDefaultNormalInfo( void );
const TWLHWSecureInfo *THW_GetDefaultSecureInfo( void );
const TWLHWNormalInfo *THW_GetNormalInfo( void );
const TWLHWSecureInfo *THW_GetSecureInfo( void );

const TWLHWNormalInfo *THW_GetDefaultNormalInfo( void )
{
	return &s_normalDefault;
}

const TWLHWSecureInfo *THW_GetDefaultSecureInfo( void )
{
	return &s_secureDefault;
}

const TWLHWNormalInfo *THW_GetNormalInfo( void )
{
	return &s_hwInfoN;
}

const TWLHWSecureInfo *THW_GetSecureInfo( void )
{
	return &s_hwInfoS;
}

static void DEBUG_PrintDigest( u8 *pDigest )
{
	int i;
	for( i = 0; i < SVC_SHA1_DIGEST_SIZE; i++ ) {
		OS_TPrintf( "%02x", *pDigest++ );
	}
	OS_TPrintf( "\n" );
}

static void DEBUG_Dump( u8 *pSrc, u32 len )
{
	int i;
	for( i = 0; i < len; i++ ) {
		if( ( i & 0x0f ) == 0 ) {
			OS_TPrintf( "\n" );
		}
		OS_TPrintf( "%02x ", *pSrc++ );
	}
	OS_TPrintf( "\n" );
}


// HMAC SHA-1�Z�o�i��SDK�ł�SVC_CalcHMACSHA1�Ƀo�O������̂ŁA������g�p�j
static void HMACSHA1( u8 *pDigest, u8 *pSrc, u32 len, u8 *pKey, u32 keyLen )
{
	static SVCHMACSHA1Context s_hmac;
	SVC_HMACSHA1Init( &s_hmac, pKey, keyLen );
	SVC_HMACSHA1Update( &s_hmac, pSrc, len );
	SVC_HMACSHA1GetHash( &s_hmac, pDigest );
}

