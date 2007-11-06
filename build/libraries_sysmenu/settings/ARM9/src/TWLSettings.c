/*---------------------------------------------------------------------------*
  Project:  TwlIPL
  File:     TWLSettings.c

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
#include <sysmenu/settings/common/NTRSettings.h>
#include <sysmenu/settings/common/TWLSettings.h>

// define data----------------------------------------------------------
#define DEFAULT_TSD_FILE_LENGTH			( 16 * 1024 )
#define TSD_FILE_MIRROR_NUM				2

#define SAVE_COUNT_MAX					0x0080		// TWLSettingsData.saveCount�̍ő�l
#define SAVE_COUNT_MASK					0x007f		// TWLSettingsData.saveCount�̒l�͈̔͂��}�X�N����B(0x00-0x7f�j
#define TSD_NOT_CORRECT					0x00ff		// TSD�ݒ�f�[�^���ǂݏo����Ă��Ȃ� or �L���Ȃ��̂��Ȃ����Ƃ������B

// function's prototype-------------------------------------------------
static BOOL TSDi_WriteSettingsDirect( TSDStore *pTSDStore );
static int  TSDi_RecoveryTSDFile( TSDStore *pTSDStoreOrg, u8 existErrFlag, u8 lengthErrFlag, u8 dataErrFlag );
static BOOL TSDi_CheckSettingsValue( TWLSettingsData *pTSD );
static void TSDi_ClearSettings( TWLSettingsData *pTSD );

// static variables-----------------------------------------------------
static TSDStore	s_TSDStore ATTRIBUTE_ALIGN(32);
static int s_indexTSD = TSD_NOT_CORRECT;

#ifndef SDK_FINALROM
static TSDStore (*s_pTSDStoreArray)[2];
#endif
// global variables-----------------------------------------------------
TWLSettingsData *g_pTSD = &s_TSDStore.tsd;

// const data-----------------------------------------------------------
static const char *s_TSDPath[ TSD_FILE_MIRROR_NUM ] = {
	(const char *)"nand:/shared1/TWLCFG0.dat",
	(const char *)"nand:/shared1/TWLCFG1.dat",
};

static const u16 s_validLangBitmapList[] = {
	TWL_LANG_BITMAP_WW,
	TWL_LANG_BITMAP_CHINA,
	TWL_LANG_BITMAP_KOREA,
};

// function's description-----------------------------------------------

// TWL�ݒ�f�[�^�̃��C�g
BOOL TSD_WriteSettings( void )
{
	return TSDi_WriteSettingsDirect( &s_TSDStore );
}


static BOOL TSDi_WriteSettingsDirect( TSDStore *pTSDStore )
{
	FSFile file;
	
	if( !TSD_IsReadSettings() ) {
		OS_TPrintf( "ERROR: Need call TSD_ReadSetting.\n" );
		return FALSE;
	}
	s_indexTSD ^= 0x01;
	pTSDStore->tsd.saveCount = (u8)( ( pTSDStore->tsd.saveCount + 1 ) & SAVE_COUNT_MASK );
	
	// �Ή�����r�b�g�}�b�v�̐ݒ�
	pTSDStore->tsd.valid_language_bitmap = s_validLangBitmapList[ pTSDStore->tsd.region ];
	
	// �_�C�W�F�X�g�Z�o
	SVC_CalcSHA1( pTSDStore->digest, &pTSDStore->tsd, sizeof(TWLSettingsData) );
	
	FS_InitFile( &file );
	
	OS_TPrintf( "Write TSD > %s : 0x%02x\n", s_TSDPath[ s_indexTSD ], pTSDStore->tsd.saveCount );
	// �t�@�C���I�[�v��
	if( !FS_OpenFileEx( &file, s_TSDPath[ s_indexTSD ], FS_FILEMODE_R | FS_FILEMODE_W ) ) {		// R|W���[�h�ŊJ���ƁA�����t�@�C�����c�����܂܍X�V�B
		OS_TPrintf( " TSD[%d] : file open error.\n" );
		return FALSE;
	}
	
	// TSDStore�̃��C�g
	if( FS_WriteFile( &file, pTSDStore, sizeof(TSDStore) ) < sizeof(TSDStore) ) {
		OS_TPrintf( " TSD[%d] : file read error.\n" );
		return FALSE;
	}
	
	FS_CloseFile( &file );
	
	return TRUE;
}


// TWL�ݒ�f�[�^�ǂݏo���ς݁H
BOOL TSD_IsReadSettings( void )
{
	return ( s_indexTSD != TSD_NOT_CORRECT );
}


// TWL�ݒ�f�[�^�̓ǂݏo��
BOOL TSD_ReadSettings( TSDStore (*pTempBuffer)[2] )
{
	int i;
	FSFile file;
	u8 digest[ SVC_SHA1_DIGEST_SIZE ];
	TSDStore *pTSDStore = (TSDStore *)pTempBuffer;
	u8  existErrFlag  = 0;
	u8  lengthErrFlag = 0;
	u8  dataErrFlag   = 0;
	u8  enableTSDFlag = 0;
	BOOL retval = FALSE;
	
#ifndef SDK_FINALROM
	s_pTSDStoreArray = pTempBuffer;
	OS_TPrintf( "TSDStoreBuff : %08x %08x\n", &(*s_pTSDStoreArray)[ 0 ], &(*s_pTSDStoreArray)[ 1 ] );
#endif
	
	FS_InitFile( &file );
	
	s_indexTSD = 0;
	enableTSDFlag = 0;
	
	// TSD�t�@�C���`�F�b�N
	for( i = 0; i < TSD_FILE_MIRROR_NUM; i++ ) {
		// �t�@�C���I�[�v��
		if( !FS_OpenFileEx( &file, s_TSDPath[ i ], FS_FILEMODE_R ) ) {
			OS_TPrintf( "TSD[%d] : file open error.\n", i );
			existErrFlag |= 0x01 << i;
			continue;
		}
		
		// �t�@�C�����`�F�b�N
		if( FS_GetFileLength( &file ) != DEFAULT_TSD_FILE_LENGTH ) {
			OS_TPrintf( "TSD[%d] : file length error. : length = %d\n", i, FS_GetFileLength( &file ) );
			lengthErrFlag |= 0x01 << i;
			goto NEXT;
		}
		
		// TSDStore�̃��[�h
		if( FS_ReadFile( &file, &pTSDStore[ i ], sizeof(TSDStore) ) < sizeof(TSDStore) ) {
			OS_TPrintf( "TSD[%d] : file read error.\n", i );
			dataErrFlag |= 0x01 << i;
			goto NEXT;
		}
		
		// �f�[�^�̃_�C�W�F�X�g�`�F�b�N
		SVC_CalcSHA1( digest, &pTSDStore[ i ].tsd, sizeof(TWLSettingsData) );
		if( !SVC_CompareSHA1( digest, pTSDStore[ i ].digest ) ) {
			OS_TPrintf( "TSD[%d] : file digest error.\n", i );
			dataErrFlag |= 0x01 << i;
			goto NEXT;
		}
		
		// �f�[�^�̒l�`�F�b�N
		if( !TSDi_CheckSettingsValue( &pTSDStore[ i ].tsd ) ) {
			OS_TPrintf( "TSD[%d] : file format error.\n", i );
			dataErrFlag |= 0x01 << i;
			goto NEXT;
		}
		enableTSDFlag |= 0x01 << i;
		s_indexTSD = i;
	NEXT:
		// �t�@�C���N���[�Y
		FS_CloseFile( &file );
		if( enableTSDFlag & ( 0x01 << i ) ) {
			OS_TPrintf("TSD[%d] valid : saveCount = %d\n", i, pTSDStore[ i ].tsd.saveCount );
		}else {
			OS_TPrintf("TSD[%d] invalid\n", i );
		}
	}
	
	// �ÓI�o�b�t�@�ɗL����TSD���R�s�[
	if( enableTSDFlag ) {
		// �ǂ����TSD���g�p���邩����
		if( enableTSDFlag == 0x03 ) {
			s_indexTSD = ( ( ( pTSDStore[ 0 ].tsd.saveCount + 1 ) & SAVE_COUNT_MASK ) ==
							pTSDStore[ 1 ].tsd.saveCount ) ? 1 : 0;
		}
		MI_CpuCopyFast( &pTSDStore[ s_indexTSD ], &s_TSDStore, sizeof(TSDStore) );
		retval = TRUE;
	}else {
		// TSD���N���A
		OS_TPrintf( "TSD clear.\n" );
		TSDi_ClearSettings( &s_TSDStore.tsd );
		retval = FALSE;
	}
	
	// ����ɓǂݍ��߂Ȃ������t�@�C��������Ȃ�A���J�o��
	if( enableTSDFlag != 0x03 ) {
		TSDStore *pOrg = ( enableTSDFlag ) ? &pTSDStore[ s_indexTSD ] : NULL;
		enableTSDFlag |= TSDi_RecoveryTSDFile( pOrg, existErrFlag, lengthErrFlag, dataErrFlag );
	}
	
	OS_TPrintf( "Use TSD[%d]   : saveCount = %d\n",
				s_indexTSD, pTSDStore[ s_indexTSD ].tsd.saveCount );
	
	return retval;
}


// TWL�ݒ�f�[�^�t�@�C���̃��J�o��
static int TSDi_RecoveryTSDFile( TSDStore *pTSDStoreOrg, u8 existErrFlag, u8 lengthErrFlag, u8 dataErrFlag )
{
	int i;
	FSFile file;
	FS_InitFile( &file );
	
	OS_TPrintf( "existErr = %02x lengthErr = %02x dataErr = %02x\n", existErrFlag, lengthErrFlag, dataErrFlag );
	
	// ��{���G���[�̃t�@�C���͌�i�K�̕������G���[�Ƃ���
	lengthErrFlag |= existErrFlag;
	dataErrFlag   |= lengthErrFlag;
	
	// �t�@�C�����J�o��
	for( i = 0; i < TSD_FILE_MIRROR_NUM; i++ ) {
		// �t�@�C������
		if( existErrFlag & ( 0x01 << i ) ) {
			if( !FS_CreateFile( s_TSDPath[ i ], FS_PERMIT_R | FS_PERMIT_W ) ) {
				OS_TPrintf( " TSD[%d] : create file error.\n" );
				continue;
			}
			existErrFlag ^= 0x01 << i;
		}
		
		// �t�@�C���I�[�v��
		if( !FS_OpenFileEx( &file, s_TSDPath[ i ], FS_FILEMODE_R | FS_FILEMODE_W ) ) {
			OS_TPrintf( " TSD[%d] : file open error.\n" );
			continue;
		}
		
		// �t�@�C�����ύX
		if( lengthErrFlag & ( 0x01 << i ) ) {
			if( FS_SetFileLength( &file, DEFAULT_TSD_FILE_LENGTH ) != FS_RESULT_SUCCESS ) {
				OS_TPrintf( " TSD[%d] : set file length error.\n" );
				goto NEXT;
			}
			lengthErrFlag ^= 0x01 << i;
		}
		
		// �f�[�^����
		if( dataErrFlag & ( 0x01 << i ) ) {
			if( pTSDStoreOrg ) {
				if( FS_WriteFile( &file, pTSDStoreOrg, sizeof(TSDStore) ) != sizeof(TSDStore) ) {
					OS_TPrintf( " TSD[%d] : write file length error.\n" );
					goto NEXT;
				}
			}else {
				// �f�t�H���g�l���������݁B
				TSDStore tempTSDS;
				TSDi_ClearSettings( &tempTSDS.tsd );
				s_indexTSD = i ^ 0x01;
				TSDi_WriteSettingsDirect( &tempTSDS );
			}
			dataErrFlag ^= 0x01 << i;
		}
		
	NEXT:
		// �t�@�C���N���[�Y
		FS_CloseFile( &file );
	}
	
	return ( existErrFlag | lengthErrFlag | dataErrFlag ) ^ 0x03;
}


// TWL�ݒ�f�[�^�̃`�F�b�N
static BOOL TSDi_CheckSettingsValue( TWLSettingsData *pTSD )
{
#pragma unused( pTSD )
	
	return TRUE;
}


// TWL�ݒ�f�[�^�̃N���A
static void TSDi_ClearSettings( TWLSettingsData *pTSD )
{
	MI_CpuClearFast( pTSD, sizeof(TWLSettingsData) );
	// �����l���O�ȊO�̂���
	pTSD->version = TWL_SETTINGS_DATA_VERSION;
	pTSD->region  = TWL_DEFAULT_REGION;				// ���[�W�����͖{�̐ݒ肩��Ȃ��Ȃ�\��
	pTSD->owner.birthday.month = 1;
	pTSD->owner.birthday.day   = 1;
}

