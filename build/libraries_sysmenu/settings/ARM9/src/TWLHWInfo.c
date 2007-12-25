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
#include <sysmenu/settings/common/TWLSettings.h>
#include <sysmenu/settings/common/TWLHWInfo.h>

// define data----------------------------------------------------------
//#define USE_SHA1_SIGNATURE				// �������̃n�b�V����SHA1���g�p�i����`�Ȃ�HMAC-SHA1���g�p�j

// function's prototype-------------------------------------------------
static BOOL THWi_CalcSignature( void *pDstSign, const void *pSrc, u32 len, const u8 *pPrivKeyDER );
static BOOL THWi_CheckDigest( void *pTgt, u32 length, u8 *pDigest );
static BOOL THWi_CheckNormalInfoValue( const TWLHWNormalInfo *pSrcInfo );
static BOOL THWi_CheckSignature( void *pTgt, u32 length, u8 *pSignature );
static BOOL THWi_CheckSecureInfoValue( const TWLHWSecureInfo *pSecure );
static void DEBUG_PrintDigest( u8 *pDigest );
static void DEBUG_Dump( u8 *pSrc, u32 len );

static inline u16 SCFG_GetBondingOption(void)
{
	return (u16)(*(u8*)(HW_SYS_CONF_BUF+HWi_WSYS08_OFFSET) & HWi_WSYS08_OP_OPT_MASK);
}

// static variables-----------------------------------------------------
TWLHWNormalInfo s_hwInfoN ATTRIBUTE_ALIGN(32);
TWLHWSecureInfo s_hwInfoS ATTRIBUTE_ALIGN(32);
static BOOL s_isReadNormal;
static BOOL s_isReadSecure;
static BOOL s_isSignCheck;

// global variables-----------------------------------------------------

// const data-----------------------------------------------------------

// �m�[�}�����@�f�t�H���g�l
static TWLHWNormalInfo s_normalDefault = {
	0x5a,
	{ 0x00 },
};


// �m�[�}�����@�o�[�W�����݊����X�g
static const u8 s_normalVersionList[] = { 1, TSF_VERSION_TERMINATOR };


// �m�[�}�����@TSF���[�h�p�����[�^
static const TSFParam s_normalParam = {
	sizeof(TWLHWNormalInfo),
	TWL_HWINFO_FILE_LENGTH,
	s_normalVersionList,
	(void (*)(void *))THW_ClearNormalInfoDirect,
	THWi_CheckDigest,
	(int (*)(void *))THWi_CheckNormalInfoValue,
};


// �Z�L���A���@�f�t�H���g�l
static TWLHWSecureInfo s_secureDefault = {
	TWL_LANG_BITMAP_AMERICA,
	TWL_REGION_AMERICA,
	{ "0123456789A\0\0\0\0" },
};


// �Z�L���A���@�o�[�W�����݊����X�g
static const u8 s_secureVersionList[] = { 1, TSF_VERSION_TERMINATOR };


// �Z�L���A���@TSF���[�h�p�����[�^
static const TSFParam s_secureParam = {
	sizeof(TWLHWSecureInfo),
	TWL_HWINFO_FILE_LENGTH,
	s_secureVersionList,
	(void (*)(void *))THW_ClearSecureInfoDirect,
	THWi_CheckSignature,
	(int (*)(void *))THWi_CheckSecureInfoValue,
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
	return TSF_ReadFile( (char *)TWL_HWINFO_NORMAL_PATH, GetHWN(), &s_normalParam, NULL );
}


// ���C�g
BOOL THW_WriteNormalInfo( void )
{
	if( !s_isReadNormal ) {
		return FALSE;
	}
	return THW_WriteNormalInfoDirect( GetHWN() );
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
						NULL ) ) {
		return FALSE;
	}
	// �����[�h���Astatic�o�b�t�@�ւ̃R�s�[���s��
	if( !s_isReadNormal ) {
		s_isReadNormal = TRUE;
		MI_CpuCopy8( pSrcInfo, GetHWN(), sizeof(TWLHWNormalInfo) );
	}
	return TRUE;
}


// �t�@�C���̃��J�o��
BOOL THW_RecoveryNormalInfo( TSFReadResult err )
{
	return TSF_RecoveryFile( err, (char *)TWL_HWINFO_NORMAL_PATH, TWL_HWINFO_FILE_LENGTH );

}


// �_�C�W�F�X�g�`�F�b�N
static BOOL THWi_CheckDigest( void *pTgt, u32 length, u8 *pDigest )
{
	u8 digest[ SVC_SHA1_DIGEST_SIZE ];
	
	SVC_CalcSHA1( digest, pTgt, length );
	return SVC_CompareSHA1( digest, pDigest );
}


// �l�`�F�b�N
static BOOL THWi_CheckNormalInfoValue( const TWLHWNormalInfo *pSrcInfo )
{
#pragma unused(pSrcInfo)
	return TRUE;
}


// �V�����f�t�H���g�l�̃Z�b�g
void THW_SetDefaultNormalInfo( const TWLHWNormalInfo *pSrcInfo )
{
	MI_CpuCopy8( pSrcInfo, &s_normalDefault, sizeof(TWLHWNormalInfo) );
}


// �l�̃N���A
void THW_ClearNormalInfoDirect( TWLHWNormalInfo *pDstInfo )
{
	MI_CpuCopy8( &s_normalDefault, pDstInfo, sizeof(TWLHWNormalInfo) );
}


// �f�t�H���g�l�̎擾
const TWLHWNormalInfo *THW_GetDefaultNormalInfo( void )
{
	return &s_normalDefault;
}


// ���ݒl�̎擾
const TWLHWNormalInfo *THW_GetNormalInfo( void )
{
	return GetHWN();
}


// ---------------------------------------------------------------------
// HW�Z�L���A���
// ---------------------------------------------------------------------

// ���[�h
TSFReadResult THW_ReadSecureInfo( void )
{
	s_isReadSecure = TRUE;
	s_isSignCheck = TRUE;
	return TSF_ReadFile( (char *)TWL_HWINFO_SECURE_PATH, GetHWS(), &s_secureParam, NULL );
}


// �����m�[�`�F�b�N���[�h
TSFReadResult THW_ReadSecureInfo_NoCheck( void )
{
	s_isReadSecure = TRUE;
	s_isSignCheck = FALSE;
	return TSF_ReadFile( (char *)TWL_HWINFO_SECURE_PATH, GetHWS(), &s_secureParam, NULL );
}


// ���C�g
BOOL THW_WriteSecureInfo( const u8 *pPrivKeyDER )
{
	if( !s_isReadSecure ) {
		return FALSE;
	}
	return THW_WriteSecureInfoDirect( GetHWS(), pPrivKeyDER );
}


// ���ڃ��C�g
BOOL THW_WriteSecureInfoDirect( const TWLHWSecureInfo *pSrcInfo, const u8 *pPrivKeyDER )
{
	// �w�b�_�̍쐬
	TSFHeader header;
	OSTick start = OS_GetTick();
	MI_CpuClear8( &header, sizeof(TSFHeader) );
	header.version = TWL_HWINFO_SECURE_VERSION;
	header.bodyLength  = sizeof( TWLHWSecureInfo );
	
	if( !THWi_CalcSignature( (void *)header.digest.rsa,
							 (const void *)pSrcInfo,
							 sizeof(TWLHWSecureInfo),
							 pPrivKeyDER ) ) {
		return FALSE;
	}
	
	// ���C�g
	if( !TSF_WriteFile( (char *)TWL_HWINFO_SECURE_PATH,
						&header,
						(const void *)pSrcInfo,
						NULL ) ) {
		return FALSE;
	}
	
	OS_TPrintf( "RSA sign encrypt time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
	
	// �����[�h���Astatic�o�b�t�@�ւ̃R�s�[���s��
	if( !s_isReadSecure ) {
		s_isReadSecure = TRUE;
		MI_CpuCopy8( pSrcInfo, GetHWS(), sizeof(TWLHWSecureInfo) );
	}
	return TRUE;
}


// �����̎Z�o
static BOOL THWi_CalcSignature( void *pDstSign, const void *pSrc, u32 len, const u8 *pPrivKeyDER )
{
#ifdef HW_SIGNATURE_ENABLE_
	u8	digest[ SVC_SHA1_DIGEST_SIZE ];
	u8  key[ SVC_SHA1_DIGEST_SIZE ];
	u64	id = SCFG_ReadFuseData();
	
	// �閧�����w�肳��Ă��Ȃ��ꍇ�͏����Ȃ��B
	if( !pPrivKeyDER ) {
		return TRUE;
	}
	
	// �閧�����w�肳�ꂽ�ꍇ�́A�{���f�B���O�I�v�V�����Ɋ֌W�Ȃ������t�����s���B
#ifdef USE_SHA1_SIGNATURE
	SVC_CalcSHA1( digest, pSrc, len );
#else
	SVC_CalcSHA1( key, &id, sizeof(u64) );		// id��SHA1�n�b�V���l���L�[�Ƃ��Ďg�p
	SVC_CalcHMACSHA1( digest, pSrc, len, key, SVC_SHA1_DIGEST_SIZE );
#endif
	return ACSign_Encrypto( pDstSign,
						 	pPrivKeyDER,
							digest,
							SVC_SHA1_DIGEST_SIZE );
#else  // HW_SIGNATURE_ENABLE_
	return TRUE;
#endif // HW_SIGNATURE_ENABLE_
}


// �t�@�C���̃��J�o��
BOOL THW_RecoverySecureInfo( TSFReadResult err )
{
	return TSF_RecoveryFile( err, (char *)TWL_HWINFO_SECURE_PATH, TWL_HWINFO_FILE_LENGTH );
}


// �����`�F�b�N
static BOOL THWi_CheckSignature( void *pTgt, u32 length, u8 *pSignature )
{
	static u32 heap[ 4096 / sizeof(u32) ];
	SVCSignHeapContext acmemoryPool;
	u8 digest_sign[ SVC_SHA1_DIGEST_SIZE ];
	u8 digest_calc[ SVC_SHA1_DIGEST_SIZE ];
	u8  key[ SVC_SHA1_DIGEST_SIZE ];
	u64	id = SCFG_ReadFuseData();
	OSTick start = OS_GetTick();
	
#ifdef USE_SHA1_SIGNATURE
	SVC_CalcSHA1( digest_calc, pTgt, length );
#else
	SVC_CalcSHA1( key, &id, sizeof(u64) );		// id��SHA1�n�b�V���l���L�[�Ƃ��Ďg�p
	SVC_CalcHMACSHA1( digest_calc, pTgt, length, key, SVC_SHA1_DIGEST_SIZE );
#endif
	SVC_InitSignHeap( &acmemoryPool, heap, 4096 );
	SVC_DecryptSign( &acmemoryPool, digest_sign, pSignature, s_publicKey );
	
	OS_TPrintf( "RSA sign decrypt time = %dms\n", OS_TicksToMilliSeconds( OS_GetTick() - start ) );
	
	// �����`�F�b�N
	{
		BOOL retval = SVC_CompareSHA1( digest_sign, digest_calc );
		
		// �{���f�B���O�I�v�V�������u���i�Łv�̎��̂ݏ����`�F�b�N���ʂ�Ԃ��B
		if( SCFG_GetBondingOption() == 0 ) {
			return ( s_isSignCheck ) ? retval : TRUE;
		}else {
			OS_TPrintf( "Development Machine : signature check trough.\n" );
			return TRUE;
		}
	}
}


// HW Secure��� �l�`�F�b�N
static BOOL THWi_CheckSecureInfoValue( const TWLHWSecureInfo *pSrcInfo )
{
	int serialNoLen = STD_StrLen( (const char *)pSrcInfo->serialNo );
	if(
		( pSrcInfo->region >= TWL_REGION_MAX ) ||
		( serialNoLen < TWL_HWINFO_SERIALNO_LEN_AMERICA ) ||
		( serialNoLen > TWL_HWINFO_SERIALNO_LEN_OTHERS )
		) {
		return FALSE;
	}
	
	return TRUE;
}


// �V�����f�t�H���g�l�̃Z�b�g
void THW_SetDefaultSecureInfo( const TWLHWSecureInfo *pSrcInfo )
{
	MI_CpuCopy8( pSrcInfo, &s_secureDefault, sizeof(TWLHWSecureInfo) );
}


// �l�̃N���A
void THW_ClearSecureInfoDirect( TWLHWSecureInfo *pDstInfo )
{
	MI_CpuCopy8( &s_secureDefault, pDstInfo, sizeof(TWLHWSecureInfo) );
}


// �f�t�H���g�l�̎擾
const TWLHWSecureInfo *THW_GetDefaultSecureInfo( void )
{
	return &s_secureDefault;
}

// ���ݒl�̎擾
const TWLHWSecureInfo *THW_GetSecureInfo( void )
{
	return GetHWS();
}


// ---------------------------------------------------------------------
// �f�o�b�O
// ---------------------------------------------------------------------

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

