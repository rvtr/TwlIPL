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

#define SAVE_COUNT_MAX					0x0080		// TWLSettingsData.saveCountの最大値
#define SAVE_COUNT_MASK					0x007f		// TWLSettingsData.saveCountの値の範囲をマスクする。(0x00-0x7f）
#define TSD_NOT_CORRECT					0x00ff		// TSD設定データが読み出されていない or 有効なものがないことを示す。

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

// TWL設定データのライト
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
	
	// 対応言語ビットマップの設定
	pTSDStore->tsd.valid_language_bitmap = s_validLangBitmapList[ pTSDStore->tsd.region ];
	
	// ダイジェスト算出
	SVC_CalcSHA1( pTSDStore->digest, &pTSDStore->tsd, sizeof(TWLSettingsData) );
	
	FS_InitFile( &file );
	
	OS_TPrintf( "Write TSD > %s : 0x%02x\n", s_TSDPath[ s_indexTSD ], pTSDStore->tsd.saveCount );
	// ファイルオープン
	if( !FS_OpenFileEx( &file, s_TSDPath[ s_indexTSD ], FS_FILEMODE_R | FS_FILEMODE_W ) ) {		// R|Wモードで開くと、既存ファイルを残したまま更新。
		OS_TPrintf( " TSD[%d] : file open error.\n" );
		return FALSE;
	}
	
	// TSDStoreのライト
	if( FS_WriteFile( &file, pTSDStore, sizeof(TSDStore) ) < sizeof(TSDStore) ) {
		OS_TPrintf( " TSD[%d] : file read error.\n" );
		return FALSE;
	}
	
	FS_CloseFile( &file );
	
	return TRUE;
}


// TWL設定データ読み出し済み？
BOOL TSD_IsReadSettings( void )
{
	return ( s_indexTSD != TSD_NOT_CORRECT );
}


// TWL設定データの読み出し
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
	
	// TSDファイルチェック
	for( i = 0; i < TSD_FILE_MIRROR_NUM; i++ ) {
		// ファイルオープン
		if( !FS_OpenFileEx( &file, s_TSDPath[ i ], FS_FILEMODE_R ) ) {
			OS_TPrintf( "TSD[%d] : file open error.\n", i );
			existErrFlag |= 0x01 << i;
			continue;
		}
		
		// ファイル長チェック
		if( FS_GetFileLength( &file ) != DEFAULT_TSD_FILE_LENGTH ) {
			OS_TPrintf( "TSD[%d] : file length error. : length = %d\n", i, FS_GetFileLength( &file ) );
			lengthErrFlag |= 0x01 << i;
			goto NEXT;
		}
		
		// TSDStoreのリード
		if( FS_ReadFile( &file, &pTSDStore[ i ], sizeof(TSDStore) ) < sizeof(TSDStore) ) {
			OS_TPrintf( "TSD[%d] : file read error.\n", i );
			dataErrFlag |= 0x01 << i;
			goto NEXT;
		}
		
		// データのダイジェストチェック
		SVC_CalcSHA1( digest, &pTSDStore[ i ].tsd, sizeof(TWLSettingsData) );
		if( !SVC_CompareSHA1( digest, pTSDStore[ i ].digest ) ) {
			OS_TPrintf( "TSD[%d] : file digest error.\n", i );
			dataErrFlag |= 0x01 << i;
			goto NEXT;
		}
		
		// データの値チェック
		if( !TSDi_CheckSettingsValue( &pTSDStore[ i ].tsd ) ) {
			OS_TPrintf( "TSD[%d] : file format error.\n", i );
			dataErrFlag |= 0x01 << i;
			goto NEXT;
		}
		enableTSDFlag |= 0x01 << i;
		s_indexTSD = i;
	NEXT:
		// ファイルクローズ
		FS_CloseFile( &file );
		if( enableTSDFlag & ( 0x01 << i ) ) {
			OS_TPrintf("TSD[%d] valid : saveCount = %d\n", i, pTSDStore[ i ].tsd.saveCount );
		}else {
			OS_TPrintf("TSD[%d] invalid\n", i );
		}
	}
	
	// 静的バッファに有効なTSDをコピー
	if( enableTSDFlag ) {
		// どちらのTSDを使用するか判定
		if( enableTSDFlag == 0x03 ) {
			s_indexTSD = ( ( ( pTSDStore[ 0 ].tsd.saveCount + 1 ) & SAVE_COUNT_MASK ) ==
							pTSDStore[ 1 ].tsd.saveCount ) ? 1 : 0;
		}
		MI_CpuCopyFast( &pTSDStore[ s_indexTSD ], &s_TSDStore, sizeof(TSDStore) );
		retval = TRUE;
	}else {
		// TSDをクリア
		OS_TPrintf( "TSD clear.\n" );
		TSDi_ClearSettings( &s_TSDStore.tsd );
		retval = FALSE;
	}
	
	// 正常に読み込めなかったファイルがあるなら、リカバリ
	if( enableTSDFlag != 0x03 ) {
		TSDStore *pOrg = ( enableTSDFlag ) ? &pTSDStore[ s_indexTSD ] : NULL;
		enableTSDFlag |= TSDi_RecoveryTSDFile( pOrg, existErrFlag, lengthErrFlag, dataErrFlag );
	}
	
	OS_TPrintf( "Use TSD[%d]   : saveCount = %d\n",
				s_indexTSD, pTSDStore[ s_indexTSD ].tsd.saveCount );
	
	return retval;
}


// TWL設定データファイルのリカバリ
static int TSDi_RecoveryTSDFile( TSDStore *pTSDStoreOrg, u8 existErrFlag, u8 lengthErrFlag, u8 dataErrFlag )
{
	int i;
	FSFile file;
	FS_InitFile( &file );
	
	OS_TPrintf( "existErr = %02x lengthErr = %02x dataErr = %02x\n", existErrFlag, lengthErrFlag, dataErrFlag );
	
	// 大本がエラーのファイルは後段階の部分もエラーとする
	lengthErrFlag |= existErrFlag;
	dataErrFlag   |= lengthErrFlag;
	
	// ファイルリカバリ
	for( i = 0; i < TSD_FILE_MIRROR_NUM; i++ ) {
		// ファイル生成
		if( existErrFlag & ( 0x01 << i ) ) {
			if( !FS_CreateFile( s_TSDPath[ i ], FS_PERMIT_R | FS_PERMIT_W ) ) {
				OS_TPrintf( " TSD[%d] : create file error.\n" );
				continue;
			}
			existErrFlag ^= 0x01 << i;
		}
		
		// ファイルオープン
		if( !FS_OpenFileEx( &file, s_TSDPath[ i ], FS_FILEMODE_R | FS_FILEMODE_W ) ) {
			OS_TPrintf( " TSD[%d] : file open error.\n" );
			continue;
		}
		
		// ファイル長変更
		if( lengthErrFlag & ( 0x01 << i ) ) {
			if( FS_SetFileLength( &file, DEFAULT_TSD_FILE_LENGTH ) != FS_RESULT_SUCCESS ) {
				OS_TPrintf( " TSD[%d] : set file length error.\n" );
				goto NEXT;
			}
			lengthErrFlag ^= 0x01 << i;
		}
		
		// データ復旧
		if( dataErrFlag & ( 0x01 << i ) ) {
			if( pTSDStoreOrg ) {
				if( FS_WriteFile( &file, pTSDStoreOrg, sizeof(TSDStore) ) != sizeof(TSDStore) ) {
					OS_TPrintf( " TSD[%d] : write file length error.\n" );
					goto NEXT;
				}
			}else {
				// デフォルト値を書き込み。
				TSDStore tempTSDS;
				TSDi_ClearSettings( &tempTSDS.tsd );
				s_indexTSD = i ^ 0x01;
				TSDi_WriteSettingsDirect( &tempTSDS );
			}
			dataErrFlag ^= 0x01 << i;
		}
		
	NEXT:
		// ファイルクローズ
		FS_CloseFile( &file );
	}
	
	return ( existErrFlag | lengthErrFlag | dataErrFlag ) ^ 0x03;
}


// TWL設定データのチェック
static BOOL TSDi_CheckSettingsValue( TWLSettingsData *pTSD )
{
#pragma unused( pTSD )
	
	return TRUE;
}


// TWL設定データのクリア
static void TSDi_ClearSettings( TWLSettingsData *pTSD )
{
	MI_CpuClearFast( pTSD, sizeof(TWLSettingsData) );
	// 初期値が０以外のもの
	pTSD->version = TWL_SETTINGS_DATA_VERSION;
	pTSD->region  = TWL_DEFAULT_REGION;				// リージョンは本体設定からなくなる予定
	pTSD->owner.birthday.month = 1;
	pTSD->owner.birthday.day   = 1;
}

