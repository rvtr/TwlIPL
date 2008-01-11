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
#include <sysmenu/settings/common/TWLHWInfo.h>

// define data----------------------------------------------------------
#define TSD_FILE_MIRROR_NUM				2
#define TSD_NOT_CORRECT					0x00ff		// TSD設定データが読み出されていない or 有効なものがないことを示す。

// function's prototype-------------------------------------------------
static BOOL TSDi_RecoveryFile( TSFReadResult err, char *pFilePath );
static BOOL TSDi_CheckDigest( void *pTgt, u32 length, u8 *pDigest );
static BOOL TSDi_CheckValue( const TWLSettingsData *pSecure );
static void TSDi_ClearSettingsDirect( TWLSettingsData *pTSD );
static BOOL TSDi_CheckDate( u8 month, u8 day );
static BOOL TSDi_CheckTime( u8 hour, u8 minute );
static BOOL TSDi_CheckString( const u16 *pStr, u8 maxLength );

// static variables-----------------------------------------------------
TWLSettingsData s_settings ATTRIBUTE_ALIGN(32);
static int s_indexTSD = TSD_NOT_CORRECT;
static u8 s_saveCount;

// global variables-----------------------------------------------------

// const data-----------------------------------------------------------
static const char *s_TSDPath[ TSD_FILE_MIRROR_NUM ] = {
	(const char *)"nand:/shared1/TWLCFG0.dat",
	(const char *)"nand:/shared1/TWLCFG1.dat",
};


// バージョン互換リスト
static const u8 s_settingsVersionList[] = { 1, TSF_VERSION_TERMINATOR };


// TSFリードパラメータ
static const TSFParam s_settingsParam = {
	sizeof(TWLSettingsData),
	TWL_SETTINGS_FILE_LENGTH,
	s_settingsVersionList,
	(void (*)(void *))TSDi_ClearSettingsDirect,
	TSDi_CheckDigest,
	(int (*)(void *))TSDi_CheckValue,
};

// 各リージョンでの国コード範囲リスト
static const u32 s_regionCountryList[ TWL_REGION_MAX ] = {
	TWL_COUNTRY_MAPPING_JAPAN,
	TWL_COUNTRY_MAPPING_AMERICA,
	TWL_COUNTRY_MAPPING_EUROPE,
	TWL_COUNTRY_MAPPING_AUSTRALIA,
	TWL_COUNTRY_MAPPING_CHINA,
	TWL_COUNTRY_MAPPING_KOREA,
};


// ---------------------------------------------------------------------
// TWL本体設定データ
// ---------------------------------------------------------------------

// ファイルから内部変数にリード
BOOL TSD_ReadSettings( void )
{
	int i;
	u8 saveCount[ TSD_FILE_MIRROR_NUM ];
	TWLSettingsData settings[ TSD_FILE_MIRROR_NUM ];
	BOOL retval = FALSE;
	
	// リード
	s_saveCount = 0;
	s_indexTSD = TSD_NOT_CORRECT;
	for( i = 0; i < TSD_FILE_MIRROR_NUM; i++ ) {
		TSFReadResult rdResult = TSF_ReadFile( (char *)s_TSDPath[ i ],
						  &settings[ i ],
						  &s_settingsParam,
						  &saveCount[ i ] );
		OS_TPrintf( "TSD[%d] saveCount = %d : ", i, saveCount[ i ] );
		if( rdResult == TSF_READ_RESULT_SUCCEEDED ) {
			OS_TPrintf( "enable.\n" );
			// どちらのTSDを使用するか判定
			if( s_indexTSD == TSD_NOT_CORRECT ) {		// 最初に有効なTSDがあったら、まずはそれを使用することに。
				s_indexTSD = i;
				s_saveCount = saveCount[ i ];
			}else {										// もう１つも有効なら、saveCount値を比較して選択。
				if( ( ( saveCount[ 0 ] + 1 ) & SAVE_COUNT_MASK ) == saveCount[ 1 ] ) {
					s_indexTSD = 1;
					s_saveCount = saveCount[ 1 ];
				}
			}
		}else {
			OS_TPrintf( "disable.\n" );
			// リードに失敗した場合はファイルリカバリ
			(void)TSDi_RecoveryFile( rdResult, (char *)s_TSDPath[ i ] );
			(void)TSD_WriteSettingsDirect( &settings[ i ] );
		}
	}
	
	// 有効なTSDを静的バッファにコピー
	if( s_indexTSD != TSD_NOT_CORRECT ) {
		MI_CpuCopyFast( &settings[ s_indexTSD ], GetTSD(), sizeof(TWLSettingsData) );
		retval = TRUE;
	}else {
		// 有効なTSDがないなら静的バッファを初期化
		TSDi_ClearSettingsDirect( GetTSD() );
		s_indexTSD = 1;
	}
	
	OS_TPrintf( "TSD[%d] saveCount = %d : Use.\n", s_indexTSD, s_saveCount );
	
	return retval;

}


// 内部変数の値をファイルにライト
BOOL TSD_WriteSettings( void )
{
	if( s_indexTSD == TSD_NOT_CORRECT ) {
		return FALSE;
	}
	return TSD_WriteSettingsDirect( GetTSD() );
}


// 指定データの値をファイルに直接ライト
BOOL TSD_WriteSettingsDirect( const TWLSettingsData *pSrcInfo )
{
	// ヘッダの作成
	TSFHeader header;
	MI_CpuClear8( &header, sizeof(TSFHeader) );
	header.version = TWL_SETTINGS_DATA_VERSION;
	header.bodyLength = sizeof(TWLSettingsData);
	SVC_CalcSHA1( header.digest.sha1, pSrcInfo, sizeof(TWLSettingsData) );
	// まだ一度もリードされていないなら、staticバッファへのコピーを行う
	if( s_indexTSD == TSD_NOT_CORRECT ) {
		s_saveCount = 0;
		s_indexTSD  = 1;
		MI_CpuCopy8( pSrcInfo, GetTSD(), sizeof(TWLSettingsData) );
	}
	// ファイルにライト
	s_indexTSD ^= 0x01;
	if( !TSF_WriteFile( (char *)s_TSDPath[ s_indexTSD ],
						&header,
						(const void *)pSrcInfo,
						&s_saveCount ) ) {
		return FALSE;
	}
	return TRUE;
}


// ファイルのリカバリ
static BOOL TSDi_RecoveryFile( TSFReadResult err, char *pFilePath )
{
	return TSF_RecoveryFile( err, pFilePath, TWL_HWINFO_FILE_LENGTH );

}


// ダイジェストチェック
static BOOL TSDi_CheckDigest( void *pTgt, u32 length, u8 *pDigest )
{
	u8 digest[ SVC_SHA1_DIGEST_SIZE ];
	
	SVC_CalcSHA1( digest, pTgt, length );
	return SVC_CompareSHA1( digest, pDigest );
}


// TWL設定データのクリア
void TSD_ClearSettings( void )
{
	TSDi_ClearSettingsDirect( GetTSD() );
}


// TWL設定データの直接クリア
static void TSDi_ClearSettingsDirect( TWLSettingsData *pTSD )
{
	int i;
	MI_CpuClearFast( pTSD, sizeof(TWLSettingsData) );
	// 初期値が"0"以外のもの
	pTSD->backLightBrightness   = TWL_BACKLIGHT_LEVEL_MAX;
	pTSD->owner.birthday.month  = 1;
	pTSD->owner.birthday.day    = 1;
	// 言語コードはHW情報の言語ビットマップから算出
	for( i = 0; i < TWL_LANG_CODE_MAX; i++ ) {
		if( THW_GetValidLanguageBitmap() & ( 0x0001 << i ) ) {
			pTSD->language = (TWLLangCode)i;
			break;
		}
	}
}


// ---------------------------------------------------------------------
// 値チェック
// ---------------------------------------------------------------------

// 値チェック
static BOOL TSDi_CheckValue( const TWLSettingsData *pSrc )
{
	// 国コード
	if( pSrc->flags.isSetCountry ) {
		u32 countryStart = (u32)( s_regionCountryList[ THW_GetRegion() ] >> 16 );
		u32 countryEnd   = (u32)( s_regionCountryList[ THW_GetRegion() ] & 0x0000ffff );
		if( ( pSrc->country < countryStart ) ||
			( pSrc->country > countryEnd ) ) {
			return FALSE;
		}
	}else if( pSrc->country != TWL_COUNTRY_UNDEFINED ) {
		return FALSE;
	}
	
	// 言語コード
	if(	pSrc->flags.isSetLanguage &&
		!( THW_GetValidLanguageBitmap() & ( 0x0001 << pSrc->language ) )
		) {
		return FALSE;
	}
	
	// バックライト輝度
	if( pSrc->backLightBrightness > TWL_BACKLIGHT_LEVEL_MAX ) {
		return FALSE;
	}
	
	// u8	rtcLastSetYear;				// RTCの前回設定年（チェックの必要なし）
	// s64	rtcOffset;					// RTC設定時のオフセット値（チェックの必要なし）
	
	// オーナー情報
	if( !TSDi_CheckDate( pSrc->owner.birthday.month, pSrc->owner.birthday.day ) ||
		!TSDi_CheckString( pSrc->owner.nickname, TWL_NICKNAME_LENGTH ) ||
		!TSDi_CheckString( pSrc->owner.comment, TWL_COMMENT_LENGTH ) ) {
		return FALSE;
	}
	
	// アラーム
	if( !TSDi_CheckTime( pSrc->alarm.hour, pSrc->alarm.minute ) ) {
		return FALSE;
	}
	
	// TWLTPCalibData		tp;			// TP補正データ（チェックの必要なし。確認済み。）
	
	// パレンタルコントロール
	if( ( pSrc->parental.ogn >= TWL_RATING_OGN_MAX ) ||
		( pSrc->parental.ratingAge > TWL_PARENTAL_CONTROL_RATING_AGE_MAX ) ||
//		( pSrc->parental.secretQuestion > TWL_PARENTAL_CONTROL_SECRET_QUESTION_MAX ) ||
//		( pSrc->parental.secretAnswerLength > TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX ) ||
		( STD_StrLen( pSrc->parental.password ) > TWL_PARENTAL_CONTROL_PASSWORD_LENGTH ) ||
		!TSDi_CheckString( pSrc->parental.secretAnswer, TWL_PARENTAL_CONTROL_SECRET_ANSWER_LENGTH_MAX ) ) {
		return FALSE;
	}
	// インストール可能なNANDアプリ個数
	if( pSrc->freeSoftBoxCount > TWL_FREE_SOFT_BOX_COUNT_MAX ) {
		return FALSE;
	}
	
	return TRUE;
}


// 日付が正しいかチェック
static BOOL TSDi_CheckDate( u8 month, u8 day )
{
	static const u8 dayNumList[ 12 ] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	
	if( ( month <  1 ) ||
		( month > 12 ) ||
	  	( day   <  1 ) ||
		( day   > dayNumList[ month - 1 ] ) ) {
		return FALSE;
	}
	return TRUE;
}


// 時刻が正しいかチェック
static BOOL TSDi_CheckTime( u8 hour, u8 minute )
{
	if( ( hour > 23 ) ||
		( minute > 59 ) ) {
		return FALSE;
	}
	return TRUE;
}


// 文字列長が正しいかチェック
static BOOL TSDi_CheckString( const u16 *pStr, u8 maxLength )
{
	while( maxLength-- ) {
		if( *pStr++ == 0 ) {
			return TRUE;
		}
	}
	return FALSE;
}

